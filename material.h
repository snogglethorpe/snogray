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
#include "light-model.h"

namespace Snogray {

class Intersect;

class Material
{
public:
  Material (const Color &col) : color (col) { }
  virtual ~Material ();

  virtual Color render (const Intersect &isec, Scene &scene, unsigned depth);

  Color light (const Intersect &isec, const Color &color,
	       Scene &scene, unsigned depth);

  LightModel *light_model;
  Color color;
};

}

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
