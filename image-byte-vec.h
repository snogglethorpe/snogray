// image-byte-vec.h -- Common code for image formats based on vectors of bytes
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_INT_VEC_H__
#define __IMAGE_INT_VEC_H__

#include "image-io.h"

namespace Snogray {

// Output

struct ByteVecImageSinkParams : public ImageFmtSinkParams
{
  ByteVecImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (! target_gamma)
      target_gamma = ImageSinkParams::DEFAULT_TARGET_GAMMA;
  }
};

class ByteVecImageSink : public ImageSink
{
public:

  typedef unsigned char byte;

  ByteVecImageSink (const ByteVecImageSinkParams &params)
    : ImageSink (params),
      gamma_correction (
	params.target_gamma == 0 ? 0 : 1 / params.target_gamma),
      output_row (new byte[params.width * 3])
  { }
  ~ByteVecImageSink ();

  // We define these
  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  // Subclasses can define this instead of the generic write_row
  virtual void write_row (const byte *byte_vec) = 0;

  // Floating-point to byte conversion
  byte color_component_to_byte (Color::component_t com)
  {
    if (com < 0)
      return 0;

    if (gamma_correction != 0)
      com = powf (com, gamma_correction);

    if (com >= 0.9999)
      return 255;
    else
      return (byte)(256.0 * com);
  }

  // Explicit gamma-correction factor
  float gamma_correction;

private:

  // A single row of bytes we use as temporary storage during output
  byte *output_row;
};


// Input

struct ByteVecImageSourceParams : public ImageFmtSourceParams
{
  ByteVecImageSourceParams (const ImageSourceParams &params)
    : ImageFmtSourceParams (params)
  { }
};

class ByteVecImageSource : public ImageSource
{
public:

  typedef unsigned char byte;

  static const float DEFAULT_SOURCE_GAMMA = 2.2;
 
  ByteVecImageSource (const ByteVecImageSourceParams &params,
		      float _gamma_correction = DEFAULT_SOURCE_GAMMA)
    : gamma_correction (_gamma_correction),
      //
      // The following must be set by subclass using `set_size' after
      // reading image header
      //
      width (0), height (0), input_row (0),
      bytes_per_component (1), component_scale (1 / 255.0),
      num_channels (3)
  { }
  ~ByteVecImageSource ();

  // We define these
  virtual void read_size (unsigned &width, unsigned &height);
  virtual void read_row (ImageRow &row);

  // Subclasses can define this instead of the generic read_row
  virtual void read_row (byte *byte_vec) = 0;

  // Explicit gamma-correction factor; should be set by subclass as
  // appropriate (in many cases it's read from the file header, so can't
  // actually be passed during superclass [us] construction time, but
  // should be set soon there after as soon as know).
  //
  float gamma_correction;

protected:

  // Called by subclass (usually after reading image header) to finish
  // setting up stuff.
  //
  void set_specs (unsigned _width, unsigned _height, unsigned _num_channels = 3,
		  unsigned bit_depth = 8);

  Color::component_t int_to_color_component (unsigned int_cc)
  {
    Color::component_t com = int_cc * component_scale;
    com = powf (com, gamma_correction); // undo gamma correction
    return com;
  }
  Color::component_t int_to_alpha_component (unsigned int_alpha)
  {
    return int_alpha * component_scale;
  }

  unsigned next_int_component (byte* &byte_ptr)
  {
    if (bytes_per_component == 2)
      {
	byte b0 = *byte_ptr++;
	byte b1 = *byte_ptr++;
	return b1 + (b0 << 8);
      }
    else
      return *byte_ptr++;
  }

  Color::component_t next_color_component (byte* &byte_ptr)
  {
    return int_to_color_component (next_int_component (byte_ptr));
  }
  Color::component_t next_alpha_component (byte* &byte_ptr)
  {
    return int_to_alpha_component (next_int_component (byte_ptr));
  }

private:

  unsigned width, height;

  // A single row of bytes we use as temporary storage during input
  //
  byte *input_row;

  // Bytes per pixel-component (1 or 2)
  //
  unsigned bytes_per_component;

  // Scale we use for each pixel component, converting from the integer
  // range [0, 2^bit_depth) to [0, 1]
  //
  Color::component_t component_scale;

  // Number of components in each pixel:  1 is grey-scale, 2 is
  // grey-scale-with-alpha, 3 is RGB, and 4 is RGBA.  We expect
  // components of each pixel to be packed into the rows in exactly
  // those orders (G, GA, RGB, RGBA), and 2-byte components to be in
  // big-endian order.  [These follow libpng's conventions, and are
  // sufficient to support libjpeg]
  //
  unsigned num_channels;
};

}

#endif /* __IMAGE_INT_VEC_H__ */

// arch-tag: e442d880-ba85-423e-8b1c-e3c4d9500528
