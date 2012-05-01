// string-funs.cc -- Random string helper functions
//
//  Copyright (C) 2005, 2007, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstring>
#include <cctype>
#include <algorithm>

#include "string-funs.h"


using namespace snogray;


// Return a string version of NUM
//
std::string 
snogray::stringify (unsigned num)
{
  std::string str = (num > 9) ? stringify (num / 10) : "";
  char ch = (num % 10) + '0';
  str += ch;
  return str;
}

// Return a string version of NUM, with commas added every 3rd place
//
std::string 
snogray::commify (unsigned long long num, unsigned sep_count)
{
  std::string str = (num > 9) ? commify (num / 10, sep_count % 3 + 1) : "";
  char ch = (num % 10) + '0';
  if (sep_count == 3 && num > 9)
    str += ',';
  str += ch;
  return str;
}

// Return a string version of NUM, with commas added every 3rd place, and
// either the phrase UNIT_NAME or UNITS_NAME appended, depending on whether
// NUM has the value 1 or not.
//
std::string
snogray::commify_with_units (unsigned long long num,
			     const std::string &unit_name,
			     const std::string &units_name)
{
  std::string out = commify (num);
  out += ' ';
  if (num == 1)
    out += unit_name;
  else
    out += units_name;
  return out;
}

// Return a lower-case version of STR.
//
std::string
snogray::downcase (const std::string &str)
{
  std::string rval = str;
  transform (rval.begin(), rval.end(), rval.begin(), ::tolower);
  return rval;
}

// Return a copy of STR with any character in REM_CHARS removed.
//
std::string
snogray::strip (const std::string &str, const std::string &rem_chars)
{
  std::string rval = str;  

  std::string::iterator i = rval.begin ();
  while (i != rval.end ())
    if (rem_chars.find (*i) == std::string::npos)
      ++i;
    else
      i = rval.erase (i);

  return rval;
}

// If STR contains any of the characters in SEP_CHARS, remove the prefix
// until the first such character, and that character, from STR.  Return
// removed prefix (without the separator character) is returned.  If no
// character in SEP_CHARS occurs in STR, "" is returned, and STR left
// unmodified
//
std::string
snogray::strip_prefix (std::string &str, const std::string &sep_chars)
{
  std::string::size_type pfx_end = str.find_first_of (sep_chars);

  if (pfx_end == std::string::npos)
    return "";
  else
    {
      std::string pfx = str.substr (0, pfx_end);
      str.erase (0, pfx_end + 1);
      return pfx;
    }
}


// Return FILENAME's extension, converted to lower-case.
//
std::string
snogray::filename_ext (const std::string &filename)
{
  unsigned dot = filename.find_last_of (".");

  if (dot == filename.length ())
    return "";

  std::string ext = filename.substr (dot + 1);

  return downcase (ext);
}

// arch-tag: c8a36b93-7176-431a-b46b-6cf51c7eff55
