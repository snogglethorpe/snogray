// zero-surface-integ.h -- Constant-zero SurfaceInteg
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

#ifndef __ZERO_SURFACE_INTEG_H__
#define __ZERO_SURFACE_INTEG_H__

#include "surface-integ.h"


namespace snogray {


// This is a SurfaceInteg subclass that always returns zero; it's useful
// for places where a SurfaceInteg is needed, but not really used.
//
class ZeroSurfaceInteg : public SurfaceInteg
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public SurfaceInteg::GlobalState
  {
  public:

    GlobalState (const Scene &_scene) : SurfaceInteg::GlobalState (_scene) { }

    // Return a new surface integrator, allocated in context.
    //
    virtual SurfaceInteg *make_integrator (RenderContext &context)
    {
      return new ZeroSurfaceInteg (context);
    }
  };

  // Return the color emitted from the ray-surface intersection ISEC.
  // "lo" means "light outgoing".
  //
  virtual Color lo (const Intersect &, const SampleSet::Sample &) const
  {
    return 0;
  }

protected:

  // Integrator state for rendering a group of related samples.
  //
  ZeroSurfaceInteg (RenderContext &context) : SurfaceInteg (context) { }
};


}

#endif // __ZERO_SURFACE_INTEG_H__
