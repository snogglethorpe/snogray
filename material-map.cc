// material-map.cc -- Named set of materials
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

#include "material-map.h"

using namespace snogray;


// Get a name->material mapping.
//
const Material *
MaterialMap::get (const std::string &name) const
{
  const_iterator iter = find (name);
  if (iter != end())
    return iter->second;
  else
    return 0;
}

// Add a name->material mapping.
//
void
MaterialMap::add (const std::string &name, const Material *mat)
{
  std::pair<iterator, bool> ins_result = insert (std::make_pair (name, mat));

  if (! ins_result.second)
    ins_result.first->second = mat;
}

// Return true if this map contains a mapping for NAME.
//
bool
MaterialMap::contains (const std::string &name) const
{
  const_iterator iter = find (name);
  return iter != end();
}

// arch-tag: 96d32d83-5f0d-4834-868a-f2f8702b6517
