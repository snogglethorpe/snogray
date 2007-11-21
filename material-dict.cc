// material-dict.cc -- Named set of materials
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <utility>		// for std::pair

#include "material-dict.h"

using namespace snogray;


// Return the material called NAME, or DEF_MAT if there is none.
//
const Material *
MaterialDict::get (const std::string &name, const Material *def_mat) const
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
MaterialDict::add (const std::string &name, const Material *mat)
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
