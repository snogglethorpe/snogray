// color-math.h -- Various standard math functions overloaded to work on colors
//
//  Copyright (C) 2008, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_COLOR_MATH_H
#define SNOGRAY_COLOR_MATH_H

#include "util/snogmath.h"
#include "color.h"


namespace snogray {


#define DEF_COLOR_MATH_FUN1(fun)				\
  inline Color fun (const Color &col)				\
  {								\
    Color rval;							\
    for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)	\
      rval[c] = fun (col[c]);					\
    return rval;						\
  }

#define DEF_COLOR_MATH_FUN2(fun)				\
  inline Color fun (const Color &col1, const Color &col2)	\
  {								\
    Color rval;							\
    for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)	\
      rval[c] = fun (col1[c], col2[c]);				\
    return rval;						\
  }


DEF_COLOR_MATH_FUN1 (floor)
DEF_COLOR_MATH_FUN1 (ceil)
DEF_COLOR_MATH_FUN1 (trunc)
DEF_COLOR_MATH_FUN1 (sin)
DEF_COLOR_MATH_FUN1 (cos)
DEF_COLOR_MATH_FUN1 (tan)

DEF_COLOR_MATH_FUN2 (atan2)


}

#endif /* SNOGRAY_COLOR_H */


// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
