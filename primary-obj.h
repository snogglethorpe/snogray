// primary-obj.h -- Standalone (non-embedded) objects
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PRIMARY_OBJ_H__
#define __PRIMARY_OBJ_H__

#include "obj.h"

namespace Snogray {

// "Primary" objects are those that are treated independently, as
// opposed to those which are embedded in other objects (and share
// state with them).
//
class PrimaryObj : public Obj
{
public:
  PrimaryObj (const Material *mat)
    : Obj (mat->shadow_type ()), _material (mat)
  { }

  // Returns the material this object is made from
  //
  virtual const Material *material () const;

  const Material *_material;
};

}

#endif /* __PRIMARY_OBJ_H__ */

// arch-tag: 58e34793-4724-4530-8f7a-0c6736037fd9
