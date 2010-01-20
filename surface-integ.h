// surface-integ.h -- Light integrator interface for surfaces
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SURFACE_INTEG_H__
#define __SURFACE_INTEG_H__

#include "intersect.h"

#include "integ.h"


namespace snogray {


class SurfaceInteg : public Integ
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public Integ::GlobalState
  {
  public:

    GlobalState (const Scene &_scene) : Integ::GlobalState (_scene) { }

    // Return a new surface integrator, allocated in context.
    //
    virtual SurfaceInteg *make_integrator (SampleSet &sample,
					   RenderContext &context)
      = 0;
  };

  // Return light from the scene arriving from the direction of RAY at its
  // origin.  SAMPLE_NUM is the sample to use.
  // "li" means "light incoming" (to ray).
  //
  // The SurfaceInteg definition of this implements li in terms of lo.
  //
  virtual Tint li (const Ray &ray, unsigned sample_num);

  // Return the color emitted from the ray-surface intersection ISEC.
  // "lo" means "light outgoing".
  //
  virtual Color lo (const Intersect &isec, unsigned sample_num) const = 0;

protected:

  SurfaceInteg (SampleSet &samples, RenderContext &_context)
    : Integ (samples, _context)
  { }
};


}

#endif // __SURFACE_INTEG_H__
