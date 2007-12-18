// image-input.h -- High-level image input
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_INPUT_H__
#define __IMAGE_INPUT_H__

#include <memory>

#include "image-io.h"


namespace snogray {

class ImageInput
{
public:

  ImageInput (const std::string &filename,
	      const ValTable &params = ValTable::NONE)
    : source (ImageSource::open (filename, params)),
      width (source->width), height (source->height)
  { }

  // Reads a row of image data into ROW
  void read_row (ImageRow &row) { source->read_row (row); }

private:

  std::auto_ptr<ImageSource> source;

public:

  // Set from the image
  //
  unsigned width, height;
};

}

#endif /* __IMAGE_INPUT_H__ */

// arch-tag: 2c10d011-42a4-4ec4-b813-8e8f150c22ac
