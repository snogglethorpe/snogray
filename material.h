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
#include "phong.h"
#include "lambert.h"
#include "trace-state.h"

namespace Snogray {

class Intersect;
class Scene;

class Material
{
public:

  // As a convenience, provide a global lookup service for common lighting
  // models.
  static const Lambert *lambert;
  static const Phong *phong (float exp, const Color &spec_col = Color::white);
  static const Phong *phong (float exp, float intens)
  {
    return phong (exp, Color::white * intens);
  }

  Material (const Color &col, const LightModel *lmodel = lambert)
    : color (col), light_model (lmodel)
  { }
  virtual ~Material ();

  virtual Color render (const Intersect &isec, TraceState &tstate) const;

  Color illum (const Intersect &isec, const Color &color, TraceState &tstate)
    const;

  Color color;
  const LightModel *light_model;
};

}

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
