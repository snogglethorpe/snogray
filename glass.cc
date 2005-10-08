// glass.h -- Glass (transmissive, reflective) material
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

#include "glass.h"

#include "intersect.h"
#include "scene.h"

using namespace Snogray;

#define E 0.0001

Color
Glass::render (const Intersect &isec, Scene &scene, TraceState &tstate) const
{
  // Render transmission
  Vec xmit_dir = isec.ray.dir.refraction (isec.normal, ior);
  Ray xmit_ray (isec.point, xmit_dir);

  TraceState &sub_tstate
    = tstate.subtrace_state (TraceState::SUBTRACE_REFLECTION);

  Color total_color (transmittance
		     * scene.render (xmit_ray, sub_tstate, isec.obj));

  // Render contribution from reflections and surface
  total_color += Mirror::render (isec, scene, tstate);

  return total_color;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
