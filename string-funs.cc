// string-funs.cc -- Random string helper functions
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstring>
#include <cctype>
#include <algorithm>

#include "string-funs.h"


using namespace snogray;
using namespace std;


// Return a string version of NUM
//
string 
snogray::stringify (unsigned num)
{
  string str = (num > 9) ? stringify (num / 10) : "";
  char ch = (num % 10) + '0';
  str += ch;
  return str;
}

// Return a string version of NUM, with commas added every 3rd place
//
string 
snogray::commify (unsigned long long num, unsigned sep_count)
{
  string str = (num > 9) ? commify (num / 10, sep_count % 3 + 1) : "";
  char ch = (num % 10) + '0';
  if (sep_count == 3 && num > 9)
    str += ',';
  str += ch;
  return str;
}

// Return a lower-case version of STR.
//
string
snogray::downcase (const string &str)
{
  string rval = str;
  transform (rval.begin(), rval.end(), rval.begin(), ::tolower);
  return rval;
}

// Return a copy of STR with any character in REM_CHARS removed.
//
string
snogray::strip (const string &str, const string &rem_chars)
{
  string rval = str;  

  string::iterator i = rval.begin ();
  while (i != rval.end ())
    if (rem_chars.find (*i) == string::npos)
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
string
snogray::strip_prefix (string &str, const string &sep_chars)
{
  string::size_type pfx_end = str.find_first_of (sep_chars);

  if (pfx_end == string::npos)
    return "";
  else
    {
      string pfx = str.substr (0, pfx_end);
      str.erase (0, pfx_end + 1);
      return pfx;
    }
}


// arch-tag: c8a36b93-7176-431a-b46b-6cf51c7eff55
