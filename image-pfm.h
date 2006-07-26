// image-pfm.h -- PFM ("Portable Float Map") format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PFM_H__
#define __IMAGE_PFM_H__

#include "image-io.h"


namespace Snogray {

class PfmImageSink : public ImageSink
{  
public:

  PfmImageSink (const std::string &filename, unsigned width, unsigned height,
		const Params &params = Params::NONE);
  ~PfmImageSink ();

  virtual void write_row (const ImageRow &row);

private:

  std::ofstream outf;

  // For whatever stupid reason, PFM files (unlike every other image
  // format) are store with the _last_ line first.  So for simplicity,
  // we just read the whole damn thing into memory.
  //
  std::vector<float> raster;

  unsigned next_y;
};

class PfmImageSource : public ImageSource
{  
public:

  PfmImageSource (const std::string &filename,
		  const Params &params = Params::NONE);

  virtual void read_row (ImageRow &row);

private:

  float maybe_byte_swap (const float &f)
  {
    if (byte_swap_floats)
      {
	union { float f; char c[4]; } u1, u2;
	u1.f = f;
	u2.c[0] = u1.c[3];
	u2.c[1] = u1.c[2];
	u2.c[2] = u1.c[1];
	u2.c[3] = u1.c[0];
	return u2.f;
      }
    else
      return f;
  }

  std::ifstream inf;

  std::vector<float> raster;

  bool byte_swap_floats;

  unsigned next_y;
};

}

#endif /* __IMAGE_PFM_H__ */


// arch-tag: d5e5373d-68c5-4086-bc9b-add7f13e3e26
