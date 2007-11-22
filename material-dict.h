// material-dict.h -- Named set of materials
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_DICT_H__
#define __MATERIAL_DICT_H__

#include <string>
#include <map>


namespace snogray {

class Material;


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
class MaterialDict : std::map<std::string, const Material *>
{
private:

  typedef std::map<std::string, const Material *> super;

public:

  MaterialDict (const Material *default_material = 0)
    : _default_material (default_material)
  { }

  using super::iterator;
  using super::const_iterator;
  using super::begin;
  using super::end;

  // Return the material called NAME, or DEF_MAT if there is none.
  //
  const Material *get (const std::string &name, const Material *def_mat) const;

  // Return the material called NAME, or the default material if there is none.
  //
  const Material *get (const std::string &name) const
  {
    return get (name, _default_material);
  }

  // Return the default material.
  //
  const Material *get_default () const { return _default_material; }

  // Add a name->material dictping.
  //
  void add (const std::string &name, const Material *mat);

  // Return true if this dict contains a dictping for NAME.
  //
  bool contains (const std::string &name) const;

  void set_default (const Material *mat) { _default_material = mat; }

  unsigned num_entries () const { return size (); }

private:

  const Material *_default_material;
};


}


#endif // __MATERIAL_DICT_H__

// arch-tag: 9022ef65-f4b5-4064-885f-75edfb5b99c3