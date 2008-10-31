// val-table.cc -- General value lists
//
//  Copyright (C) 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <sstream>
#include <cstdlib>
#include <climits>

#include "excepts.h"
#include "string-funs.h"

#include "val-table.h"

using namespace snogray;


ValTable ValTable::NONE;



std::string
Val::as_string () const
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
	case BOOL: s << _bool_val; break;
	case STRING:; // make gcc happy
	}

      return s.str ();
    }
}



void
Val::type_err (const char *msg) const
{
  std::string text = as_string ();
  text += msg;
  throw bad_format (text);
}

void
Val::invalid (const char *type_name) const
{
  std::string msg = "invalid ";
  msg += type_name;
  type_err (msg.c_str ());
}



int
Val::as_int () const
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
    case BOOL:
      return _bool_val;
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

unsigned
Val::as_uint () const
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
    case BOOL:
      return _bool_val;
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

float
Val::as_float () const
{
  switch (type)
    {
    case STRING:
      {
	char *end;
	float val = strtof (_string_val.c_str(), &end);
	if (end && *end)
	  invalid ("float");
	return val;
      }
    case INT:
      return float (_int_val);
    case UINT:
      return float (_uint_val);
    case FLOAT:
      return _float_val;
    case BOOL:
      invalid ("float");
    }
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}

bool
Val::as_bool () const
{
  switch (type)
    {
    case STRING:
      if (_string_val.length() == 1)
	{
	  char ch = _string_val[0];
	  if (ch == '0' || ch == 'n' || ch == 'N' || ch == 'f' || ch == 'F')
	    return false;
	  else if(ch == '1' || ch == 'y' || ch == 'Y' || ch == 't' || ch == 'T')
	    return true;
	}
      else if (_string_val == "no" || _string_val == "NO"
	       || _string_val == "false" || _string_val == "FALSE"
	       || _string_val == "off" || _string_val == "OFF")
	return false;
      else if (_string_val == "yes" || _string_val == "YES"
	       || _string_val == "true" || _string_val == "TRUE"
	       || _string_val == "on" || _string_val == "on")
	return true;
      break;
    case INT:
    case UINT:
      if (! (_uint_val & ~1))
	return _uint_val;
      break;
    case FLOAT:
      break;
    case BOOL:
      return _bool_val;
    }
  invalid ("bool");
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}



// Return the value called NAME, or zero if there is none.  NAME may also
// be a comma-separated list of names, in which case the value of the first
// name which has one is returned (zero is returned if none does).
//
Val *
ValTable::get (const std::string &name)
{
  unsigned name_start = 0;

  std::string::size_type sep;
  while ((sep = name.find_first_of (",", name_start)) != std::string::npos)
    {
      iterator i = find (name.substr (name_start, sep - name_start));

      if (i != end())
	return &i->second;

      name_start = sep + 1;
    }
    
  iterator i = name_start == 0 ? find (name) : find (name.substr (name_start));

  return i == end () ? 0 : &i->second;
}

// Set the entry called NAME to VAL (overwriting any old value).
//
void
ValTable::set (const std::string &name, const Val &val)
{
  iterator i = find (name);
  if (i == end ())
    insert (value_type (name, val));
  else
    i->second = val;
}



void
ValTable::parse (const std::string &input)
{
  std::string::size_type inp_len = input.length ();
  std::string::size_type p_assn = input.find_first_of ("=:");

  if (p_assn < inp_len)
    set (input.substr (0, p_assn), input.substr (p_assn + 1));
  else if (input[0] == '!')
    set (input.substr (1), false);
  else if (begins_with (input, "no-"))
    set (input.substr (3), false);
  else
    set (input, true);
}

void
ValTable::parse (const std::string &input, const std::string &multiple_seps)
{
  std::string::size_type p_end = input.find_first_of (multiple_seps);

  if (p_end == std::string::npos)
    {
      parse (input);
    }
  else
    {
      std::string::size_type p_start = 0;
      do
	{
	  parse (input.substr (p_start, p_end - p_start));

	  p_start = input.find_first_not_of (multiple_seps, p_end);
	  p_end = input.find_first_of (multiple_seps, p_start);
	}
      while (p_end != std::string::npos);

      if (p_start != std::string::npos)
	parse (input.substr (p_start));
    }
}


// arch-tag: e5ca6fdf-8e80-4541-b5fd-1bdea3214ef3
