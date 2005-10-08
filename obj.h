// obj.h -- Root of object class hierarchy
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __OBJ_H__
#define __OBJ_H__

#include "vec.h"
#include "color.h"
#include "ray.h"
#include "material.h"

namespace Snogray {

class Obj 
{
public:
  Obj (const Material *mat) : material (mat), no_shadow (false) { }

  virtual bool intersects (const Ray &ray) const;

  virtual void closest_intersect (class Intersect &isec) const;
  virtual void finish_intersect (class Intersect &isec) const;

  const Material *material;
  bool no_shadow : 1;
};

}

#endif /* __OBJ_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
