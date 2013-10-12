// image-pfm.h -- PFM ("Portable Float Map") format image handling
//
//  Copyright (C) 2005-2007, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_PFM_H
#define SNOGRAY_IMAGE_PFM_H

#include <fstream>

#include "image-io.h"


namespace snogray {


// PFM image output.
//
class PfmImageSink : public ImageSink
{  
public:

  PfmImageSink (const std::string &filename, unsigned width, unsigned height,
		const ValTable &params = ValTable::NONE);
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


// PFM image input.
//
class PfmImageSource : public ImageSource
{  
public:

  PfmImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE);

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

#endif // SNOGRAY_IMAGE_PFM_H


// arch-tag: d5e5373d-68c5-4086-bc9b-add7f13e3e26
