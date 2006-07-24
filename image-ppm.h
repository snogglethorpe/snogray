// image-ppm.h -- PPM format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PPM_H__
#define __IMAGE_PPM_H__

extern "C" {
#include <ppm.h>
}

#include "image-io.h"


// PPM has fixed gamma correction:  each sample is "...  proportional to
// the intensity of the CIE Rec. 709 red, green, and blue in the pixel,
// adjusted by the CIE Rec.  709 gamma transfer function.  (That transfer
// function specifies a gamma number of 2.2 and has a linear section for
// small intensities)."
//
#define IMAGE_PPM_GAMMA 2.2

// asdlkfjasf
#define _IMAGE_PPM_GAMMA_STRING "2.2"


namespace Snogray {


// Output

class PpmImageSink : public ImageSink
{
public:

  PpmImageSink (const std::string &filename, unsigned width, unsigned height,
		const Params &params = Params::NONE);
  ~PpmImageSink ();

  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  // Floating-point to pixval conversion
  pixval color_component_to_pixval (Color::component_t com)
  {
    if (com < 0)
      return 0;

    // Do gamma correction
    //
    com = pow (com, Color::component_t (1 / IMAGE_PPM_GAMMA));

    if (com >= 1)
      return max_pixval;
    else
      return pixval (max_pixval * com);
  }

private:

  FILE *stream;

  // A single row of bytes we use as temporary storage during output
  //
  pixel *output_row;

  // PPM params (currently these have fixed values)
  //
  pixval max_pixval;
  bool force_plain;
};


// Input

class PpmImageSource : public ImageSource
{  
public:

  PpmImageSource (const std::string &filename,
		  const Params &params = Params::NONE);
  ~PpmImageSource ();

  virtual void read_row (ImageRow &row);

  // Pixval to floating-point conversion
  Color::component_t pixval_to_color_component (pixval pv)
  {
    Color::component_t com = Color::component_t (pv) / max_pixval;

    // Undo gamma correction.
    //
    com = pow (com, Color::component_t (IMAGE_PPM_GAMMA));

    return com;
  }

private:

  FILE *stream;

  // A single row of bytes we use as temporary storage during input
  //
  pixel *input_row;

  // PPM params
  //
  pixval max_pixval;
  int format;
};

}

#endif /* __IMAGE_PPM_H__ */

// arch-tag: 0b6f409a-0140-4dd4-933a-2e8c16541952
