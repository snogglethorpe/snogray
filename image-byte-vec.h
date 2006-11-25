// image-byte-vec.h -- Common code for image formats based on vectors of bytes
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_INT_VEC_H__
#define __IMAGE_INT_VEC_H__

#include <vector>

#include "snogmath.h"
#include "image-io.h"

namespace Snogray {

// Output

class ByteVecImageSink : public ImageSink
{
public:

  typedef unsigned char byte;
  typedef std::vector<byte> ByteVec;

  ByteVecImageSink (const std::string &filename,
		    unsigned width, unsigned height,
		    const ValTable &params = ValTable::NONE);

  // We define these
  //
  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  // Subclasses should define this instead of the generic write_row
  //
  virtual void write_row (const ByteVec &byte_vec) = 0;

  // Floating-point to byte conversion
  //
  byte color_component_to_byte (Color::component_t com)
  {
    if (com < 0)
      return 0;

    if (gamma_correction != 0)
      com = pow (com, gamma_correction);

    if (com >= 0.9999)
      return 255;
    else
      return byte (256.0 * com);
  }

  // TARGET_GAMMA is the gamma correction factor the _target_ (e.g. a display)
  // uses when it eventually reads the image we're writing:
  //
  //   DISPLAY_VAL = IMAGE_VAL ^ TARGET_GAMMA.
  //
  // GAMMA_CORRECTION is the gamma-correction factor _we_ use to
  // "correct" for the target's calculation:
  //
  //   IMAGE_VAL = SOURCE_VAL ^ GAMMA_CORRECTION.
  //
  // Since GAMMA_CORRECTION == 1 / TARGET_GAMMA, the overall result is
  // that DISPLAY_VAL == SOURCE_VAL, which is our goal.
  //
  float target_gamma, gamma_correction;

private:

  // A single row of bytes we use as temporary storage during output
  //
  ByteVec output_row;
};


// Input

class ByteVecImageSource : public ImageSource
{
public:

  typedef unsigned char byte;
  typedef std::vector<byte> ByteVec;

  static const float DEFAULT_SOURCE_GAMMA = 2.2;
 
  ByteVecImageSource (const std::string &filename, const ValTable &params);

  // We define this
  //
  virtual void read_row (ImageRow &row);

  // Subclasses should define this instead of the generic read_row.
  //
  virtual void read_row (std::vector<byte> &byte_vec) = 0;

  // Explicit gamma-correction factor:
  //
  //    FINAL_VAL = IMAGE_VAL ^ GAMMA_CORRECTION
  //
  // Should be set by subclass when appropriate (in many cases it's read
  // from the file header, so can't actually be passed during superclass
  // construction time, but should be set as soon as it is known).
  //
  // Note that when the gamma correction is stored in the file itself,
  // it may be the inverse of this -- for instance in the case of the
  // PNG format, the value in a "gAMA chunk" is the correction value
  // _applied at image encoding time_; our value of GAMMA_CORRECTION
  // needs to be the inverse of that, to invert the encoding
  // transformation.
  //
  float gamma_correction;

protected:

  // Called by subclass (usually after reading image header) to finish
  // setting up stuff.
  //
  void set_specs (unsigned _width, unsigned _height,
		  unsigned _num_channels = 3, unsigned bit_depth = 8);

  // Set the gamma correction factor that should be used when converting
  // image bytes into internal linear values.
  //
  void set_gamma_correction (float _gamma_correction)
  {
    gamma_correction = _gamma_correction;
  }

  Color::component_t int_to_color_component (unsigned int_cc)
  {
    Color::component_t com = int_cc * component_scale;
    com = pow (com, gamma_correction); // undo gamma correction
    return com;
  }
  Color::component_t int_to_alpha_component (unsigned int_alpha)
  {
    return int_alpha * component_scale;
  }

  unsigned next_int_component (ByteVec::const_iterator &byte_ptr)
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

  Color::component_t
  next_color_component (ByteVec::const_iterator &byte_ptr)
  {
    return int_to_color_component (next_int_component (byte_ptr));
  }
  Color::component_t
  next_alpha_component (ByteVec::const_iterator &byte_ptr)
  {
    return int_to_alpha_component (next_int_component (byte_ptr));
  }

private:

  // A single row of bytes we use as temporary storage during input
  //
  ByteVec input_row;

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
