// zero-surface-integ.h -- Constant-zero SurfaceInteg
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_ZERO_SURFACE_INTEG_H
#define SNOGRAY_ZERO_SURFACE_INTEG_H

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

    GlobalState (const GlobalRenderState &rstate)
      : SurfaceInteg::GlobalState (rstate)
    { }

    // Return a new surface integrator, allocated in context.
    //
    virtual SurfaceInteg *make_integrator (RenderContext &context)
    {
      return new ZeroSurfaceInteg (context);
    }
  };


  // Return the light arriving at RAY's origin from the direction it
  // points in (the length of RAY is ignored).  MEDIA is the media
  // environment through which the ray travels.
  //
  // This method also calls the volume-integrator's Li method, and
  // includes any light it returns for RAY as well.
  //
  // "Li" means "Light incoming".
  //
  virtual Tint Li (const Ray &, const Media &, const SampleSet::Sample &)
  {
    return 0;
  }

protected:

  // Integrator state for rendering a group of related samples.
  //
  ZeroSurfaceInteg (RenderContext &context) : SurfaceInteg (context) { }
};


}

#endif // SNOGRAY_ZERO_SURFACE_INTEG_H
