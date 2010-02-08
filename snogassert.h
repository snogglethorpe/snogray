// snogassert.h -- Simple sanity checking
//
//  Copyright (C) 2009, 2010  Miles Bader <miles@gnu.org>
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
#include "excepts.h"


//
// Note that we avoid using the name "assert", as it's too easy to conflict
// with the common C macro definition in <assert.h>.  Although we don't use
// <assert.h>, it can inadvertently be dragged in by system or 3rd-party
// headers (boost in particular uses it way too freely).
//


namespace snogray {


struct assertion_failure : public std::runtime_error
{
  assertion_failure (const std::string &msg) : std::runtime_error (msg) { }
  assertion_failure () : std::runtime_error ("assertion failure") { }
};


extern void assert_fail (const char *failure_msg,
			 const char *src_file = 0, unsigned src_line = 0);

// Assert that COND should be true.  If COND is false, throw an
// exception with FAILURE_MSG as a message.
//
inline void assert_with_msg (bool cond, const char *failure_msg,
			     const char *src_file = 0, unsigned src_line = 0)
{
  if (unlikely (! cond))
    assert_fail (failure_msg, src_file, src_line);
}


// Our version of the common assert macro.
//
#define ASSERT(cond) \
  assert_with_msg (cond, #cond, __FILE__, __LINE__)

// An assert macro that allows a custom message.
//
#define ASSERT_WITH_MSG(cond, msg) \
  assert_with_msg (cond, msg, __FILE__, __LINE__)


}


#endif /* __SNOGASSERT_H__ */
