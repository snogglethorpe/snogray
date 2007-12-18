// val_table.h -- Tables of named values
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __VAL_TABLE_H__
#define __VAL_TABLE_H__

#include <map>
#include <string>


namespace snogray {


// An entry in a ValTable.
//
class Val
{
public:

  enum Type { STRING, INT, UINT, FLOAT };

  Val (std::string val) : type (STRING), _string_val (val) { }
  Val (int val) : type (INT), _int_val (val) { }
  Val (unsigned val) : type (UINT), _uint_val (val) { }
  Val (float val) : type (FLOAT), _float_val (val) { }

  std::string as_string () const;
  int as_int () const;
  unsigned as_uint () const;
  float as_float () const;

  void set (std::string val) { type = STRING; _string_val = val; }
  void set (int val) { type = INT; _int_val = val; }
  void set (unsigned val) { type = UINT; _uint_val = val; }
  void set (float val) { type = FLOAT; _float_val = val; }

  Type type;

private:

  void type_err (const char *msg) const;
  void invalid (const char *type_name) const;

  std::string _string_val;
  union {
    int _int_val;
    unsigned _uint_val;
    float _float_val;
  };
};



// A table of named values.
//
class ValTable : public std::map<const std::string, Val>
{
public:

  static ValTable NONE;

  ValTable () { }
  ValTable (const std::string &init) { parse (init); }
  ValTable (const char *init) { parse (init); }

  // Return the value called NAME, or zero if there is none.  NAME may also
  // be a comma-separated list of names, in which case the value of the first
  // name which has one is returned (zero is returned if none does).
  //
  Val *get (const std::string &name);
  const Val *get (const std::string &name) const
  {
    return const_cast<ValTable *> (this)->get (name);
  }

  // Set the entry called NAME to VAL (overwriting any old value).
  //
  void set (const std::string &name, const Val &val);

  // Return true if there's value called NAME.
  //
  bool contains (const std::string &name) const
  {
    return !! get (name);
  }
  
  // Return the value called NAME with the given type, or DEFAULT_VAL if
  // there's no value called NAME.  If the type of NAME's value is not
  // convertible to the given type, an error is signalled.
  //
  std::string get_string (const std::string &name, std::string default_val = "")
    const
  {
    const Val *v = get (name);
    return v ? v->as_string () : default_val;
  }
  int get_int (const std::string &name, int default_val = 0) const
  {
    const Val *v = get (name);
    return v ? v->as_int () : default_val;
  }
  unsigned get_uint (const std::string &name, unsigned default_val = 0) const
  {
    const Val *v = get (name);
    return v ? v->as_uint () : default_val;
  }
  float get_float (const std::string &name, float default_val = 0) const
  {
    const Val *v = get (name);
    return v ? v->as_float () : default_val;
  }

  template<typename T>
  void set (const std::string &name, T val)
  {
    Val *v = get (name);
    if (v)
      v->set (val);
    else
      set (name, Val (val));
  }

  void set (const std::string &name, const char *val)
  {
    set (name, std::string (val));
  }

  // Parse the named-value specification INPUT using "NAME=VALUE" syntax.
  // The syntax "NAME:VALUE" is also accepted.  The type of the new value
  // is always a string (which can be converted to another type when the
  // value is subsequently requested).
  //
  void parse (const std::string &input);

  // Parse the multiple-named-value specification INPUT.  Each value has
  // the syntax "NAME=VALUE" or "NAME:VALUE", and multiple values are
  // separated by any character in MULTIPLE_SEPS; any whitespace
  // surrounding a value separator is removed.
  //
  void parse (const std::string &input, const std::string &multiple_seps);
};


}


#endif /* __VAL_TABLE_H__ */


// arch-tag: 6e4c7d8e-7c7d-4552-9c88-c610896d12b6
