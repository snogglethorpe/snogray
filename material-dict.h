// material-dict.h -- Named set of materials
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_DICT_H__
#define __MATERIAL_DICT_H__

#include <string>
#include <map>

#include "material.h"


namespace snogray {


// A named set of material pointers, for loading meshes with multiple
// materials, and also a default material.
//
// When loading from a file, the priority in which materials are used is:
//
//   1) Named material from material-dict (overrides everything)
//   2) Named material from the file being loaded
//   3) Default material from material-dict
//   4) "Global" default (e.g. from a mesh being loaded into)
//   5) Error
//
class MaterialDict : std::map<std::string, Ref<const Material> >
{
private:

  typedef std::map<std::string, Ref<const Material> > super;

public:

  MaterialDict (const Ref<const Material> &default_material = 0)
    : _default_material (default_material)
  { }

  using super::iterator;
  using super::const_iterator;
  using super::begin;
  using super::end;

  // Return the material called NAME, or DEF_MAT if there is none.
  //
  Ref<const Material> get (const std::string &name,
			   const Ref<const Material> &def_mat)
    const;

  // Return the material called NAME, or the default material if there is none.
  //
  Ref<const Material> get (const std::string &name) const
  {
    return get (name, _default_material);
  }

  // Return the default material.
  //
  Ref<const Material> get_default () const { return _default_material; }

  // Add a name->material dictping.
  //
  void add (const std::string &name, const Ref<const Material> &mat);

  // Return true if this dict contains a dictping for NAME.
  //
  bool contains (const std::string &name) const;

  void set_default (const Ref<const Material> &mat) { _default_material = mat; }

  unsigned num_entries () const { return size (); }

private:

  Ref<const Material> _default_material;
};


}


#endif // __MATERIAL_DICT_H__

// arch-tag: 9022ef65-f4b5-4064-885f-75edfb5b99c3
