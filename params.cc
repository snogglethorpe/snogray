// params.cc -- General parameter lists
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <sstream>
#include <cstdlib>
#include <climits>

#include "excepts.h"

#include "params.h"

using namespace Snogray;


Params Params::NONE;


std::string
Param::string_val () const
{
  if (type == STRING)
    return _string_val;
  else
    {
      std::ostringstream s;

      switch (type)
	{
	case INT: s << _int_val; break;
	case UINT: s << _uint_val; break;
	case FLOAT: s << _float_val; break;
	case STRING:; // make gcc happy
	}

      return s.str ();
    }
}

void
Param::type_err (const char *msg) const
{
  std::string text = name;
  text += ": ";
  text += string_val ();
  text += msg;
  throw bad_format (text);
}

void
Param::invalid (const char *type_name) const
{
  std::string msg = "invalid ";
  msg += type_name;
  type_err (msg.c_str ());
}

int
Param::int_val () const
{
  switch (type)
    {
    case STRING:
      {
	char *end;
	int val = strtol (_string_val.c_str(), &end, 0);
	if (end && *end)
	  invalid ("integer");
	return val;
      }
    case INT:
      return _int_val;
    case UINT:
      if (_uint_val > INT_MAX)
	type_err ("value too large");
      return int (_uint_val);
    case FLOAT:
      if (float (int (_float_val)) != _float_val)
	type_err ("not an integer");
      return int (_float_val);
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

unsigned
Param::uint_val () const
{
  switch (type)
    {
    case STRING:
      {
	char *end;
	unsigned val = strtoul (_string_val.c_str(), &end, 0);
	if (end && *end)
	  invalid ("unsigned integer");
	return val;
      }
    case INT:
      if (_int_val < 0)
	type_err ("value not positive");
      return int (_int_val);
    case UINT:
      return _uint_val;
    case FLOAT:
      if (float (unsigned (_float_val)) != _float_val)
	type_err ("not an integer");
      return unsigned (_float_val);
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

float
Param::float_val () const
{
  switch (type)
    {
    case STRING:
      {
	char *end;
	float val = strtof (_string_val.c_str(), &end);
	if (end && *end)
	  invalid ("integer");
	return val;
      }
    case INT:
      return float (_int_val);
    case UINT:
      return float (_uint_val);
    case FLOAT:
      return _float_val;
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

Param *
Params::get (const std::string &name)
{
  for (iterator p = begin(); p != end(); p++)
    if (p->name == name)
      return &(*p);
  return 0;
}

const Param *
Params::get (const std::string &name) const
{
  for (const_iterator p = begin(); p != end(); p++)
    if (p->name == name)
      return &(*p);
  return 0;
}

void
Params::parse (const std::string &input)
{
  unsigned inp_len = input.length ();
  unsigned p_assn = input.find_first_of ("=");

  if (p_assn < inp_len)
    set (input.substr (0, p_assn), input.substr (p_assn + 1));
}

void
Params::parse (const std::string &input, const std::string &multiple_seps)
{
  unsigned p_end = input.find_first_of (multiple_seps);

  if (p_end == std::string::npos)
    {
      parse (input);
    }
  else
    {
      unsigned p_start = 0;
      do
	{
	  parse (input.substr (p_start, p_end - p_start));

	  p_start = input.find_first_not_of (multiple_seps, p_end);
	  p_end = input.find_first_of (multiple_seps, p_start);
	}
      while (p_end != std::string::npos);
    }
}

// arch-tag: e5ca6fdf-8e80-4541-b5fd-1bdea3214ef3
