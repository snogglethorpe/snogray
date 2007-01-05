// val_table.h -- Tables of named values
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
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

  Val *get (const std::string &name);
  const Val *get (const std::string &name) const;
  void set (const std::string &name, const Val &val);

  bool contains (const std::string &name) const
  {
    return !! get (name);
  }
  
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
  uint get_uint (const std::string &name, unsigned default_val = 0) const
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

  void parse (const std::string &input);
  void parse (const std::string &input, const std::string &multiple_seps);
};


}


#endif /* __VAL_TABLE_H__ */


// arch-tag: 6e4c7d8e-7c7d-4552-9c88-c610896d12b6
