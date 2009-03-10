// snogassert.h -- Simple sanity checking
//
//  Copyright (C) 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SNOGASSERT_H__
#define __SNOGASSERT_H__

#include "compiler.h"


namespace snogray {


extern void assert_fail (const char *failure_msg);

inline void assert (bool cond, const char *failure_msg)
{
  if (unlikely (! cond))
    assert_fail (failure_msg);
}


}


#endif /* __SNOGASSERT_H__ */
