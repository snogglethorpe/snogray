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

#define E 0.0001

Color
Mirror::render (const Intersect &isec, Scene &scene, unsigned depth) const
{
  Vec mirror_dir = isec.eye_dir.reflection (isec.normal);
  Ray mirror_ray (isec.point, mirror_dir);

  Color total_color (reflectance * scene.render (mirror_ray, depth, isec.obj));

  // Render contribution from surface
  if (reflectance < 1 - E)
    total_color += (1 - reflectance) * Material::render (isec, scene, depth);

  return total_color;
}

// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
