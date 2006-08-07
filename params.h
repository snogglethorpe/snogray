// params.h -- General parameter lists
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <list>
#include <string>


namespace Snogray {

class Param
{
public:

  enum Type { STRING, INT, UINT, FLOAT };
  
  Param (const std::string &_name, std::string val)
    : name (_name), type (STRING), _string_val (val)
  { }
  Param (const std::string &_name, int val)
    : name (_name), type (INT), _int_val (val)
  { }
  Param (const std::string &_name, unsigned val)
    : name (_name), type (UINT), _uint_val (val)
  { }
  Param (const std::string &_name, float val)
    : name (_name), type (FLOAT), _float_val (val)
  { }

  std::string string_val () const;
  int int_val () const;
  unsigned uint_val () const;
  float float_val () const;

  void set_val (std::string val) { type = STRING; _string_val = val; }
  void set_val (int val) { type = INT; _int_val = val; }
  void set_val (unsigned val) { type = UINT; _uint_val = val; }
  void set_val (float val) { type = FLOAT; _float_val = val; }

  std::string name;
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

class Params : public std::list<Param>
{
public:

  static Params NONE;

  Params () { }
  Params (const std::string &init) { parse (init); }
  Params (const char *init) { parse (init); }

  Param *get (const std::string &name);
  const Param *get (const std::string &name) const;

  std::string get_string (const std::string &name, std::string default_val = "")
    const
  {
    const Param *p = get (name);
    return p ? p->string_val() : default_val;
  }
  int get_int (const std::string &name, int default_val = 0) const
  {
    const Param *p = get (name);
    return p ? p->int_val() : default_val;
  }
  uint get_uint (const std::string &name, unsigned default_val = 0) const
  {
    const Param *p = get (name);
    return p ? p->uint_val() : default_val;
  }
  float get_float (const std::string &name, float default_val = 0) const
  {
    const Param *p = get (name);
    return p ? p->float_val() : default_val;
  }

  template<typename T>
  void set (const std::string &name, T val)
  {
    Param *p = get (name);
    if (p)
      p->set_val (val);
    else
      push_back (Param (name, val));
  }

  void parse (const std::string &input);
};

}

#endif /* __PARAMS_H__ */

// arch-tag: 6e4c7d8e-7c7d-4552-9c88-c610896d12b6
