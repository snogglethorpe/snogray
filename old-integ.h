// old-integ.h -- Surface integrator interface for old rendering system
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

#ifndef __OLD_INTEG_H__
#define __OLD_INTEG_H__

#include "trace-cache.h"
#include "illum-mgr.h"

#include "surface-integ.h"


namespace snogray {


class OldInteg : public SurfaceInteg
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public SurfaceInteg::GlobalState
  {
  public:

    GlobalState (const Scene &_scene, const ValTable &params);

    // Return a new integrator, allocated in context.
    //
    virtual Integ *make_integrator (SampleSet &samples, RenderContext &context);

  private:

    friend class OldInteg;

    TraceCache root_cache;

    IllumMgr illum_mgr;
  };

  // Return light from the scene arriving from the direction of RAY at its
  // origin.  SAMPLE_NUM is the sample to use.
  //
  virtual Tint li (const Ray &ray, unsigned sample_num);

protected:

  // Integrator state for rendering a group of related samples.
  //
  OldInteg (SampleSet &samples, RenderContext &context,
	    GlobalState &global_state)
    : SurfaceInteg (samples, context), global (global_state)
  { }

private:

  GlobalState &global;
};


}

#endif // __OLD_INTEG_H__
