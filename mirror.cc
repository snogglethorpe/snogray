// mirror.cc -- Mirror (reflective) material
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "mirror.h"

#include "intersect.h"
#include "scene.h"

using namespace Snogray;

Color
Mirror::reflection (const Intersect &isec) const
{
  // Render reflection

  Vec mirror_dir = isec.ray.dir.reflection (isec.normal);
  Ray mirror_ray (isec.point, mirror_dir);

  Trace &sub_trace = isec.trace.subtrace (Trace::REFLECTION, isec.surface);

  return sub_trace.render (mirror_ray);
}

Color
Mirror::render (const Intersect &isec) const
{
  Color total_color = reflectance * reflection (isec);

  // Render contribution from surface.
  //
  total_color += (1 - reflectance) * Material::render (isec);

  return total_color;
}

// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
