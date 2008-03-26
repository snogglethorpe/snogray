// image.h -- Image datatype
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "tuple-matrix.h"


namespace snogray {

// An image.
//
// This is optimized for size, and so while useful for typical images, may
// not have enough dynamic range for extreme uses (e.g., the result of some
// calculations).
//
typedef TupleMatrix<Color> Image;

// An image with at least enough dynamic range to store a single-precision
// float per pixel color component.
//
typedef TupleMatrix<Color, float> DeepImage;

}

#endif /* __IMAGE_H__ */

// arch-tag: 2dfba5e6-470e-4dfa-8bbb-4fbd41fbf303
