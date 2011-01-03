// snogassert.cc -- Simple sanity checking
//
//  Copyright (C) 2009, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "string-funs.h"

#include "snogassert.h"


void
snogray::assert_fail (const char *failure_msg,
		      const char *src_file, unsigned src_line)
{
  std::string msg ("assertion failure: ");

  if (src_file)
    {
      msg += src_file;
      msg += ":";
      msg += stringify (src_line);
      msg += ": ";
    }
  
  msg += failure_msg;

  throw assertion_failure (msg);
}
