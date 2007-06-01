// material-map.h -- Named set of materials
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_MAP_H__
#define __MATERIAL_MAP_H__

#include <string>
#include <map>


namespace snogray {

class Material;


// A named set of material pointers, for loading meshes with multiple
// materials, and also a default material.
//
// When loading from a file, the priority in which materials are used is:
//
//   1) Named material from material-map (overrides everything)
//   2) Named material from the file being loaded
//   3) Default material from material-map
//   4) "Global" default (e.g. from a mesh being loaded into)
//   5) Error
//
class MaterialMap
{
public:

  MaterialMap (const Material *default_material = 0)
    : _default_material (default_material)
  { }

  // Map NAME to a material.  GLOBAL_DEFAULT corresponds
  // to steps (4) from the list above, and is used if there is
  // no mapping for NAME, and no default in this material-map.
  //
  const Material *map (const std::string &name, const Material *global_default)
    const
  {
    const Material *mat = get (name);
    if (! mat)
      {
	mat = _default_material;
	if (! mat)
	  mat = global_default;
      }
    return mat;
  }

  // Return either this material-maps default material, or GLOBAL_DEFAULT
  // if it has none.
  //
  const Material *map (const Material *global_default = 0) const
  {
    return _default_material ? _default_material : global_default;
  }

  // Get a name->material mapping.
  //
  const Material *get (const std::string &name) const;

  // Add a name->material mapping.
  //
  void add (const std::string &name, const Material *mat);

  // Return true if this map contains a mapping for NAME.
  //
  bool contains (const std::string &name) const;

  const Material *get_default () const { return _default_material; }
  void set_default (const Material *mat) { _default_material = mat; }

  unsigned num_entries () const { return _named_materials.size (); }

private:

  typedef std::map<std::string, const Material *> NamedMap;

  NamedMap _named_materials;

  const Material *_default_material;
};


}


#endif // __MATERIAL_MAP_H__

// arch-tag: 9022ef65-f4b5-4064-885f-75edfb5b99c3
