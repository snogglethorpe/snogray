// material-dict.cc -- Named set of materials
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

#include <utility>		// for std::pair

#include "material-dict.h"

using namespace snogray;


// Return the material called NAME, or DEF_MAT if there is none.
//
Ref<const Material>
MaterialDict::get (const std::string &name, const Ref<const Material> &def_mat)
  const
{
  const_iterator iter = find (name);
  if (iter != end())
    return iter->second;
  else
    return def_mat;
}

// Add a name->material dictping.
//
void
MaterialDict::add (const std::string &name, const Ref<const Material> &mat)
{
  std::pair<iterator, bool> ins_result = insert (std::make_pair (name, mat));

  if (! ins_result.second)
    ins_result.first->second = mat;
}

// Return true if this dict contains a dictping for NAME.
//
bool
MaterialDict::contains (const std::string &name) const
{
  const_iterator iter = find (name);
  return iter != end();
}

// arch-tag: 96d32d83-5f0d-4834-868a-f2f8702b6517
