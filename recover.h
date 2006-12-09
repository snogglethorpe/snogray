// recover.h -- Recovery of partial output images
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RECOVER_H__
#define __RECOVER_H__


#include <string>

#include "image-input.h"
#include "image-output.h"


namespace Snogray {


// The number of rows at the end of a recovered image file that we throw
// away, to guard against garbage.
//
#define RECOVER_DISCARD_ROWS 4

// Initialize the output by reading as many image rows as possible from
// SRC and copying them to DST; if a read-error is encountered, a small
// number of the last rows read are discarded, to guard against garbaged
// final lines.  The number of rows recovered is returned.
//
// SRC is closed and deleted after recovering it.
//
extern unsigned recover_image (ImageInput *src, ImageOutput &dst);


}

#endif // __RECOVER_H__


// arch-tag: 84ae5ac2-df53-4f17-9a42-08d9d0fb51db
