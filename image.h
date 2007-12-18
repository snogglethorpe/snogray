// image.h -- Image datatype
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "tuple-matrix.h"


namespace snogray {

typedef TupleMatrix<Color> Image;

}

#endif /* __IMAGE_H__ */

// arch-tag: 2dfba5e6-470e-4dfa-8bbb-4fbd41fbf303
