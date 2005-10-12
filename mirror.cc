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
Mirror::reflection (const Intersect &isec, TraceState &tstate) const
{
  // Render reflection

  Vec mirror_dir = isec.ray.dir.reflection (isec.normal);
  Ray mirror_ray (isec.point, mirror_dir);

  TraceState &sub_tstate = tstate.subtrace_state (TraceState::TRACE_REFLECTION);

  return sub_tstate.render (mirror_ray, isec.obj);
}

Color
Mirror::render (const Intersect &isec, TraceState &tstate) const
{
  Color total_color = reflectance * reflection (isec, tstate);

  // Render contribution from surface.
  //
  total_color += Material::render (isec, tstate);

  return total_color;
}

// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
