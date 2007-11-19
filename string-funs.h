// string-funs.h -- Random string helper functions
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __STRING_FUNS__
#define __STRING_FUNS__

#include <string>

namespace snogray {

// Return a string version of NUM
//
extern std::string stringify (unsigned num);

// Return a string version of NUM, with commas added every 3rd place
//
extern std::string commify (unsigned long long num, unsigned sep_count = 1);

// Return a string version of NUM, with commas added every 3rd place, and
// either the phrase UNIT_NAME or UNITS_NAME appended, depending on whether
// NUM has the value 1 or not.
//
extern std::string commify_with_units (unsigned long long num,
				       const std::string &unit_name,
				       const std::string &units_name);

// Return a lower-case version of STR.
//
extern std::string downcase (const std::string &str);

// Return a copy of STR with any character in REM_CHARS removed.
//
extern std::string strip (const std::string &str, const std::string &chars);

// If STR contains any of the characters in SEP_CHARS, remove the prefix
// until the first such character, and that character, from STR.  Return
// removed prefix (without the separator character) is returned.  If no
// character in SEP_CHARS occurs in STR, "" is returned, and STR left
// unmodified
//
std::string strip_prefix (std::string &str, const std::string &sep_chars);

// Return FILENAME's extension, converted to lower-case.
//
extern std::string filename_ext (const std::string &filename);

static inline bool
ends_in (const std::string &str, const std::string &sfx)
{
  size_t str_len = str.length ();
  size_t sfx_len = sfx.length ();
  return str_len >= sfx_len && str.substr (str_len - sfx_len) == sfx;
}

static inline bool
begins_with (const std::string &str, const std::string &pfx)
{
  size_t str_len = str.length ();
  size_t pfx_len = pfx.length ();
  return str_len >= pfx_len && str.substr (0, pfx_len) == pfx;
}

}

#endif /* __STRING_FUNS__ */

// arch-tag: 9fcb681e-6711-4d6c-bc4a-293f5cbfabe3
