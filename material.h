// material.h -- Object material datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "pos.h"
#include "vec.h"
#include "color.h"

namespace Snogray {

class Material
{
public:
  virtual const Color render (const class Obj *obj, const Pos &point,
			      const Vec &normal, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const = 0;
};

}

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
