// image.h -- Image datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "tuple-matrix.h"


namespace Snogray {

typedef TupleMatrix<Color> Image;

}

#endif /* __IMAGE_H__ */

// arch-tag: 2dfba5e6-470e-4dfa-8bbb-4fbd41fbf303
