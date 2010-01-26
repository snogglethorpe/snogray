// direct-integ.h -- Direct-lighting-only surface integrator
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

#ifndef __DIRECT_INTEG_H__
#define __DIRECT_INTEG_H__

#include "surface-integ.h"
#include "direct-illum.h"


namespace snogray {


class DirectInteg : public SurfaceInteg
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
    virtual SurfaceInteg *make_integrator (RenderContext &context);

  private:

    friend class DirectInteg;

    DirectIllum::GlobalState direct_illum;
  };

  // Return the color emitted from the ray-surface intersection ISEC.
  // "Lo" means "Light outgoing".
  //
  virtual Color Lo (const Intersect &isec, const SampleSet::Sample &sample)
    const
  {
    return Lo (isec, sample, 0);
  }

protected:

  // Integrator state for rendering a group of related samples.
  //
  DirectInteg (RenderContext &context, GlobalState &global_state);

private:

  // Return the color emitted from the ray-surface intersection ISEC.
  // "Lo" means "Light outgoing".
  //
  // This is an internal variant of Integ::lo which has an additional DEPTH
  // argument.  If DEPTH is greater than some limit, recursion will stop.
  //
  Color Lo (const Intersect &isec, const SampleSet::Sample &sample,
	    unsigned depth)
    const;

  // Return the light hitting TARGET_ISEC from direction DIR; DIR is in
  // TARGET_ISEC's surface-normal coordinate-system.  SUBTRACE_TYPE and
  // SUBTRACE_MEDIUM describe the type of transition represented by the new
  // ray, and the medium it has entered.  If SUBTRACE_MEDIUM is 0, then
  // TARGET_ISEC's medium is used instead.
  //
  Color Li (const Intersect &target_isec, const Vec &dir,
	    const SampleSet::Sample &sample,
	    Trace::Type subtrace_type,
	    const Medium *subtrace_medium,
	    unsigned depth)
    const;

  // State used by the direct-lighting calculator.
  //
  DirectIllum direct_illum;
};


}

#endif // __DIRECT_INTEG_H__
