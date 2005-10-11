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
Glass::render (const Intersect &isec, TraceState &tstate) const
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

  if (! xmit_dir.null ())
    {
      Ray xmit_ray (isec.point, xmit_dir);

      TraceState &sub_tstate = tstate.subtrace_state (substate_type, new_ior);

      // Start with refraction
      //
      Color refraction (transmittance * sub_tstate.render (xmit_ray, isec.obj));

      // ... and add contribution from reflections and surface
      //
      return refraction + Mirror::render (isec, tstate);
    }
  else
    // "Total internal reflection"
    //
    return reflection (isec, tstate);
}

// arch-tag: a8209bc5-a88c-4f6c-b598-ee89c9587a6f
