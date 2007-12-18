// excepts.h -- Snogray exceptions
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __EXCEPTS_H__
#define __EXCEPTS_H__

#include <stdexcept>
#include <string>

namespace snogray {

struct bad_format : public std::runtime_error
{
  bad_format (const std::string &msg) : std::runtime_error (msg) { }
  bad_format () : std::runtime_error ("bad format") { }
};

struct file_error : public std::runtime_error
{
  file_error (const std::string &msg) : std::runtime_error (msg) { }
  file_error () : std::runtime_error ("") { }
};

}

#endif /* __EXCEPTS_H__ */

// arch-tag: 0a852d69-b721-4bc6-912c-5cb042fc6357
