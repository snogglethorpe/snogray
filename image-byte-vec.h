// image-byte-vec.h -- Common code for image formats based on vectors of bytes
//
//  Copyright (C) 2005-2008, 2010-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_INT_VEC_H
#define SNOGRAY_IMAGE_INT_VEC_H

#include <vector>

#include "snogmath.h"
#include "image-io.h"


namespace snogray {


// Common data and types for byte-vec I/O.
//
class ByteVecIo
{
public:

  // Supported pixel formats.
  //
  enum PixelFormat
  {
    PIXEL_FORMAT_GREY		= 1, // one grey component
    PIXEL_FORMAT_RGB		= 2, // red, green, blue components

    PIXEL_FORMAT_BASE_MASK	= 3,

    PIXEL_FORMAT_ALPHA_FLAG	= 4, // additional alpha (opacity) channel

    PIXEL_FORMAT_GREY_ALPHA	= PIXEL_FORMAT_GREY | PIXEL_FORMAT_ALPHA_FLAG,
    PIXEL_FORMAT_RGBA		= PIXEL_FORMAT_RGB | PIXEL_FORMAT_ALPHA_FLAG
  };

  typedef unsigned char byte;
  typedef std::vector<byte> ByteVec;

  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  static float default_target_gamma () { return 2.2; }

  ByteVecIo (PixelFormat pxfmt, unsigned _bytes_per_component,
	     float _target_gamma);
  ByteVecIo (const ValTable &params);

  // Set the pixel format.
  //
  void set_pixel_format (PixelFormat pxfmt, unsigned _bytes_per_component);

  // Set the target gamma correction factor that should be used when
  // converting image bytes into internal linear values.
  //
  void set_target_gamma (float _target_gamma)
  {
    target_gamma = _target_gamma;
  }

  // Methods for operating on a PixelFormat.
  //
  bool pixel_format_has_alpha_channel (PixelFormat pixel_format) const
  {
    return pixel_format & PIXEL_FORMAT_ALPHA_FLAG;
  }
  PixelFormat pixel_format_base (PixelFormat pixel_format) const
  {
    return PixelFormat (pixel_format & ~PIXEL_FORMAT_ALPHA_FLAG);
  }
  PixelFormat pixel_format_add_alpha_channel (PixelFormat pixel_format) const
  {
    return PixelFormat (pixel_format | PIXEL_FORMAT_ALPHA_FLAG);
  }

  // What pixel format is being used.
  //
  PixelFormat pixel_format;

  // Name of that format (used for error messages).
  //
  std::string pixel_format_name;

  // Number of components in each pixel.
  //
  unsigned num_channels;

  // Bytes per pixel-component (1 or 2)
  //
  unsigned bytes_per_component;

  // TARGET_GAMMA is the gamma factor the final _target_ (e.g. a
  // display) would file would use when displaying an image being read:
  //
  //    DISPLAY_VAL = FILE_VAL ^ TARGET_GAMMA.
  //
  // The actual transformation we apply depends on the I/O direction;
  // for reading, it's the same as TARGET_GAMMA (as we're "the target"),
  // and for writing it's the inverse (as we want to correct for the
  // eventual transformation the real target will apply, thus the term
  // "gamma correction").
  //
  // This value should be set by subclass when appropriate (in many
  // cases it's read from the file header, so can't actually be passed
  // during superclass construction time, but should be set as soon as
  // it is known).
  //
  // Note that in some cases, the value stored in a file may be it may
  // be the inverse of this.  For instance in the case of the PNG
  // format, the value in a "gAMA chunk" is the correction value
  // _applied at image encoding time_; we invert that upon reading to
  // get TARGET_GAMMA.
  //
  float target_gamma;
};


// Output

class ByteVecImageSink : public ImageSink, public ByteVecIo
{
public:

  ByteVecImageSink (const std::string &filename,
		    unsigned width, unsigned height,
		    const ValTable &params = ValTable::NONE);

  // Return true if output has an alpha (opacity) channel.
  //
  virtual bool has_alpha_channel () const
  {
    return pixel_format_has_alpha_channel (pixel_format);
  }

  // Return the maximum sample value.  A value of zero means that
  // there's no real maximum.
  //
  virtual float max_intens () const { return 1; }

  // We define these, and our superclass calls them.
  //
  virtual void write_row (const ImageRow &row);

  // Subclasses should define this (instead of the generic write_row),
  // and we will call it.
  //
  virtual void write_row (const ByteVec &byte_vec) = 0;

protected:

  // Floating-point to integer and range conversion for color
  // components.
  //
  unsigned color_component_to_int (Color::component_t com) const
  {
    com = max (com, 0.f);

    if (gamma_correction != 0)
      com = pow (com, gamma_correction);

    return unsigned (min (com * component_scale, max_component));
  }

  // Floating-point to integer and range conversion for alpha
  // component (which isn't gamma corrected).
  //
  unsigned alpha_component_to_int (Tint::alpha_t alpha) const
  {
    return unsigned (clamp (alpha * component_scale, 0.f, max_component));
  }

  void put_int_component (ByteVec::iterator &byte_ptr, unsigned com) const
  {
    // Assumes big-endian

    if (bytes_per_component == 2)
      *byte_ptr++ = (com >> 8) & 0xFF;

    *byte_ptr++ = com & 0xFF;
  }

  void put_color_component (ByteVec::iterator &byte_ptr,
			    Color::component_t com)
    const
  {
    put_int_component (byte_ptr, color_component_to_int (com));
  }
  void put_alpha_component (ByteVec::iterator &byte_ptr,
			    Color::component_t com)
    const
  {
    put_int_component (byte_ptr, alpha_component_to_int (com));
  }

  // Scale factor to convert from our internal range of [0, 1] to the
  // external representation's integer range of [0, 2^bit_depth - 1].
  //
  // This is just 2^bit_depth (note that this isn't the reciprocal of
  // the scale factor used during input).
  //
  Color::component_t component_scale;

  // Maximum component value.
  //
  Color::component_t max_component;

  // GAMMA_CORRECTION is the gamma-correction factor _we_ use to
  // "correct" for the final target's gamma.  The final target
  // essentially applies the transformation:
  //
  //   DISPLAY_VAL = FILE_VAL ^ TARGET_GAMMA.
  //
  // so to compensate for this, when writing to the file, we do:
  //
  //   FILE_VAL = SOURCE_VAL ^ (1 / TARGET_GAMMA)
  //
  // Thus GAMMA_CORRECTION == 1 / TARGET_GAMMA.
  //
  float gamma_correction;

private:

  // A single row of bytes we use as temporary storage during output
  //
  ByteVec output_row;
};


// Input

class ByteVecImageSource : public ImageSource, public ByteVecIo
{
public:
 
  ByteVecImageSource (const std::string &filename, const ValTable &params);

  // Return true if input has an alpha (opacity) channel.
  //
  virtual bool has_alpha_channel () const
  {
    return pixel_format_has_alpha_channel (pixel_format);
  }

  // Return the maximum sample value.  A value of zero means that
  // there's no real maximum.
  //
  virtual float max_intens () const { return 1; }

  // We define this, and our superclass calls it.
  //
  virtual void read_row (ImageRow &row);

  // Subclasses should define this (instead of the generic read_row),
  // and we will call it.
  //
  virtual void read_row (std::vector<byte> &byte_vec) = 0;

protected:

  // Called by subclass (usually after reading image header) to finish
  // setting up stuff.  BYTES_PER_COMPONENT is optional and defaults
  // to 1; BITS_PER_COMPONENT should be less than or equal to
  // 8*BYTES_PER_COMPONENT, and defaults to 8*BYTES_PER_COMPONENT.
  //
  void set_specs (unsigned _width, unsigned _height,
		  PixelFormat pxfmt = PIXEL_FORMAT_RGB,
		  unsigned bytes_per_component = 1,
		  unsigned bits_per_component = 0 /* 0: use default */);

  Color::component_t int_to_color_component (unsigned int_cc) const
  {
    Color::component_t com = int_cc * component_scale;
    com = pow (com, target_gamma); // undo gamma correction
    return com;
  }
  Color::component_t int_to_alpha_component (unsigned int_alpha) const
  {
    return int_alpha * component_scale; // alpha doesn't use gamma correction
  }

  unsigned get_int_component (ByteVec::const_iterator &byte_ptr) const
  {
    if (bytes_per_component == 2)
      {
	// Assumes big-endian
	byte b0 = *byte_ptr++;
	byte b1 = *byte_ptr++;
	return b1 + (b0 << 8);
      }
    else
      return *byte_ptr++;
  }

  Color::component_t
  get_color_component (ByteVec::const_iterator &byte_ptr) const
  {
    return int_to_color_component (get_int_component (byte_ptr));
  }
  Color::component_t
  get_alpha_component (ByteVec::const_iterator &byte_ptr) const
  {
    return int_to_alpha_component (get_int_component (byte_ptr));
  }

private:

  // Scale factor to convert from the external representation's integer
  // range of [0, 2^bit_depth - 1] to our internal range of [0, 1].
  //
  // To ensure that the source input range covers the full destination
  // range, this is 1 / (2^bit_depth - 1).  Note that this isn't the
  // reciprocal of the scale factor used during output.
  //
  Color::component_t component_scale;

  // A single row of bytes we use as temporary storage during input
  //
  ByteVec input_row;
};


}

#endif // SNOGRAY_IMAGE_BYTE_VEC_H


// arch-tag: e442d880-ba85-423e-8b1c-e3c4d9500528
