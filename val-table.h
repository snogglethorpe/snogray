// val_table.h -- Tables of named values
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

#ifndef SNOGRAY_VAL_TABLE_H
#define SNOGRAY_VAL_TABLE_H

#include <map>
#include <string>


namespace snogray {


class ValTable; // fwd decl


// An entry in a ValTable.
//
class Val
{
public:

  enum Type { STRING, INT, UINT, FLOAT, BOOL, TABLE };

  Val (std::string val) : type (STRING), _string_val (val) { }
  Val (int val) : type (INT), _int_val (val) { }
  Val (unsigned val) : type (UINT), _uint_val (val) { }
  Val (float val) : type (FLOAT), _float_val (val) { }
  Val (bool val) : type (BOOL), _bool_val (val) { }
  Val (const ValTable &val);

  // copy constructor and assignment
  Val (const Val &val) { init_from (val); }
  Val &operator= (const Val &val);

  ~Val () { delete_data (); }

  std::string as_string () const;
  int as_int () const;
  unsigned as_uint () const;
  float as_float () const;
  bool as_bool () const;

  const ValTable &as_table () const;
  ValTable &as_table ();

  void set (std::string val) { set_type (STRING); _string_val = val; }
  void set (int val) { set_type (INT); _int_val = val; }
  void set (unsigned val) { set_type (UINT); _uint_val = val; }
  void set (float val) { set_type (FLOAT); _float_val = val; }
  void set (bool val) { set_type (BOOL); _bool_val = val; }

  Type type;

private:

  void type_err (const char *msg) const;
  void invalid (const char *type_name) const;

  // Set the type to NEW_TYPE, and possibly free any external storage used
  // by the previous type.  The state of any data fields following this is
  // "undefined but safe to overwrite."
  //
  void set_type (Type new_type)
  {
    delete_data ();
    type = new_type;
  }

  // Free any external data storage used by the current type.  The state of
  // any data fields following this is "undefined but safe to overwrite."
  //
  void delete_data ();

  // Set value from VAL, overwriting current values as garbage.
  //
  void init_from (const Val &val);

  std::string _string_val;
  union {
    int _int_val;
    unsigned _uint_val;
    float _float_val;
    bool _bool_val;
    ValTable *_table_ptr;
  };
};



// A table of named values.
//
class ValTable : public std::map<const std::string, Val>
{
public:

  static const ValTable NONE;

  ValTable () { }

  // Return the value called NAME, or zero if there is none.  NAME may also
  // be a comma-separated list of names, in which case the value of the first
  // name which has one is returned (zero is returned if none does).
  //
  Val *get (const std::string &name);
  const Val *get (const std::string &name) const
  {
    return const_cast<ValTable *> (this)->get (name);
  }

  // Set the entry called NAME to VAL (overwriting any old value),
  // and return a reference to the "in table" copy of VAL (which, if
  // modified, will actually change the table entry).
  //
  Val &set (const std::string &name, const Val &val);

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
  bool get_bool (const std::string &name, bool default_val = false) const
  {
    const Val *v = get (name);
    return v ? v->as_bool () : default_val;
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

  // Returns a copy of this table containing only entries whose name begins
  // with PREFIX, with PREFIX removed from the entry names in the copy.
  //
  ValTable filter_by_prefix (const std::string &prefix) const;

  // Import all entries from TABLE into this table.  If PREFIX is given,
  // then it is prepended to each key.
  //
  void import (const ValTable &table, const std::string &prefix = "");

  // Set the entry called NAME to the value of FROM's entry of the
  // same name; if FROM doesn't contain any entry of that name,
  // nothing is done.  If NAME is a comma-separated list of names,
  // then it is used directly to lookup FROM's entry, but only the
  // first name is used to store the result into this table.
  //
  void set_from (const std::string &name, const ValTable &from);

  // If this table doesn't contains an entry called NAME, but FROM
  // does, then add one using the value of FROM's entry; if FROM
  // doesn't contain any entry of that name, nothing is done.  If NAME
  // is a comma-separated list of names, then it is used directly to
  // lookup FROM's entry, but only the first name is used to store the
  // result into this table.
  //
  void default_from (const std::string &name, const ValTable &from);
};


}


#endif /* SNOGRAY_VAL_TABLE_H */


// arch-tag: 6e4c7d8e-7c7d-4552-9c88-c610896d12b6
