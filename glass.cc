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

Color
Glass::render (const Intersect &isec, Scene &scene, TraceState &tstate) const
{
  // Render transmission

  double new_ior, old_ior;
  TraceState::TraceType substate_type;

  if (isec.reverse)
    {
      // Exiting this object

      new_ior = tstate.enclosing_medium_ior ();
      old_ior = ior;
      substate_type = TraceState::TRACE_REFRACTION_OUT;
    }
  else
    {
      // Entering this object

      new_ior = ior;
      old_ior = tstate.ior;
      substate_type = TraceState::TRACE_REFRACTION_IN;
    }

  Vec xmit_dir = isec.ray.dir.refraction (isec.normal, old_ior, new_ior);
  Color total_color;

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      TraceState &sub_tstate = tstate.subtrace_state (substate_type, new_ior);

      total_color = transmittance * scene.render (xmit_ray, sub_tstate);
    }
  else if (old_ior > new_ior)
    total_color = Color (100, 0, 0);
  else
    total_color = Color::funny;
  // otherwise we need someway to signal our superclass that we need even
  // more reflection than normal???? XXX

  // Render contribution from reflections and surface
  //
  total_color += Mirror::render (isec, scene, tstate);

  return total_color;
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
