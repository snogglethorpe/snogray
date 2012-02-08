// val-table.cc -- General value lists
//
//  Copyright (C) 2006-2008, 2010-2012  Miles Bader <miles@gnu.org>
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


const ValTable ValTable::NONE;



Val::Val (const ValTable &val)
  : type (TABLE), _table_ptr (new ValTable (val))
{ }

// assignment operator
Val &
Val::operator= (const Val &val)
{
  if (&val != this)
    {
      delete_data ();
      init_from (val);
    }
  return *this;
}

// Free any external data storage used by the current type.  The state of
// any data fields following this is "undefined but safe to overwrite."
//
void
Val::delete_data ()
{
  if (type == TABLE)
    delete _table_ptr;
}

// Set value from VAL, overwriting current values as garbage.
//
void
Val::init_from (const Val &val)
{
  type = val.type;
  switch (type)
    {
    case STRING: _string_val = val._string_val; break;
    case INT:	 _int_val    = val._int_val;    break;
    case UINT:	 _uint_val   = val._uint_val;   break;
    case FLOAT:  _float_val  = val._float_val;  break;
    case BOOL:	 _bool_val   = val._bool_val;   break;
    case TABLE:  _table_ptr  = new ValTable (*val._table_ptr); break;
    }
}



void
Val::type_err (const char *msg) const
{
  throw bad_format (msg);
}

void
Val::invalid (const char *type_name) const
{
  std::string msg = "invalid ";
  msg += type_name;
  type_err (msg.c_str ());
}



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
	default:
	  invalid ("string");
	}

      return s.str ();
    }
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
	  break; // invalid
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
    default:
      break; // invalid
    }
  invalid ("integer");
  return 0; // gcc can't seem to detect that this is unreachable?!
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
	  break; // invalid
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
    default:
      break; // invalid
    }
  invalid ("integer");
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
	  break; // invalid
	return val;
      }
    case INT:
      return float (_int_val);
    case UINT:
      return float (_uint_val);
    case FLOAT:
      return _float_val;
    default:
      break; // invalid
    }
  invalid ("float");
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
      break; // invalid
    case INT:
    case UINT:
      if (! (_uint_val & ~1))
	return _uint_val;
      break;
    case BOOL:
      return _bool_val;
    default:
      break; // invalid
    }
  invalid ("bool");
  return 0;	    // gcc can't seem to detect that this is unreachable?!
}



const ValTable &
Val::as_table () const
{
  if (type != TABLE)
    invalid ("table");
  return *_table_ptr;
}

ValTable &
Val::as_table ()
{
  if (type != TABLE)
    invalid ("table");
  return *_table_ptr;
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

// Set the entry called NAME to VAL (overwriting any old value),
// and return a reference to the "in table" copy of VAL (which, if
// modified, will actually change the table entry).
//
Val &
ValTable::set (const std::string &name, const Val &val)
{
  iterator i = find (name);
  if (i == end ())
    i = insert (value_type (name, val)).first;
  else
    i->second = val;
  return i->second;
}


// Table subsetting with prefixed keys.

// Returns a copy of this table containing only entries whose name begins
// with PREFIX, with PREFIX removed from the entry names in the copy.
//
ValTable
ValTable::filter_by_prefix (const std::string &prefix) const
{
  ValTable copy;

  unsigned pfx_len = prefix.size ();

  for (const_iterator i = begin (); i != end (); ++i)
    if (begins_with (i->first, prefix))
      copy.set (i->first.substr (pfx_len), i->second);

  return copy;
}

// Import all entries from TABLE into this table.  If PREFIX is given,
// then it is prepended to each key.
//
void
ValTable::import (const ValTable &table, const std::string &prefix)
{
  for (const_iterator i = table.begin (); i != table.end (); ++i)
    if (prefix.empty ())
      set (i->first, i->second);
    else
      set (prefix + i->first, i->second);
}


// Table entry copying

// Set the entry called NAME to the value of FROM's entry of the same
// name; if FROM doesn't contain any entry of that name, nothing is
// done.  If NAME is a comma-separated list of names, then it is used
// directly to lookup FROM's entry, but only the first name is used to
// store the result into this table.
//
void
ValTable::set_from (const std::string &name, const ValTable &from)
{
  const Val *from_entry = from.get (name);

  if (from_entry)
    {
      std::string first_name = name;
      std::string::size_type name_sep_pos = name.find_first_of (",");

      // If FIRST_NAME contains multiple names, remove all but the
      // last.
      //
      if (name_sep_pos != std::string::npos)
	first_name.resize (name_sep_pos);

      set (first_name, *from_entry);
    }
}

// If this table doesn't contains an entry called NAME, but FROM does,
// then add one using the value of FROM's entry; if FROM doesn't
// contain any entry of that name, nothing is done.  If NAME is a
// comma-separated list of names, then it is used directly to lookup
// FROM's entry, but only the first name is used to store the result
// into this table.
//
void
ValTable::default_from (const std::string &name, const ValTable &from)
{
  if (! contains (name))
    set_from (name, from);
}


// arch-tag: e5ca6fdf-8e80-4541-b5fd-1bdea3214ef3
