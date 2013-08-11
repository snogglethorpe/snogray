// surface-integ.h -- Light integrator interface for surfaces
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SURFACE_INTEG_H
#define SNOGRAY_SURFACE_INTEG_H

#include "ray.h"
#include "color/tint.h"
#include "sample-set.h"

#include "integ.h"


namespace snogray {

class Media;


class SurfaceInteg : public Integ
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public Integ::GlobalState
  {
  public:

    GlobalState (const GlobalRenderState &rstate)
      : Integ::GlobalState (rstate)
    { }

    // Return a new surface integrator, allocated in context.
    //
    virtual SurfaceInteg *make_integrator (RenderContext &context) = 0;
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
  virtual Tint Li (const Ray &ray, const Media &media,
		   const SampleSet::Sample &sample) = 0;

protected:

  SurfaceInteg (RenderContext &_context) : Integ (_context) { }
};


}

#endif // SNOGRAY_SURFACE_INTEG_H
