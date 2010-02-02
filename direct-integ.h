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

#include "direct-illum.h"

#include "surface-integ.h"


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
		   const SampleSet::Sample &sample);

private:

  // Integrator state for rendering a group of related samples.
  //
  DirectInteg (RenderContext &context, GlobalState &global_state);

  // Return the light arriving at RAY's origin from the direction it
  // points in (the length of RAY is ignored).  MEDIA is the media
  // environment through which the ray travels.
  //
  // This method also calls the volume-integrator's Li method, and
  // includes any light it returns for RAY as well.
  //
  // "Li" means "Light incoming".
  //
  // This an internal variant of Integ::lo which has an additional DEPTH
  // argument.  If DEPTH is greater than some limit, recursion will
  // stop.  It also returns a Color instead of a Tint, as alpha values
  // are only meaningful at the the top-level.
  //
  Color Li (const Ray &ray, const Media &media,
	   const SampleSet::Sample &sample,
	   unsigned depth)
    const;

  // Return the light emitted from ISEC.
  //
  Color Lo (const Intersect &isec, const Media &media,
	    const SampleSet::Sample &sample, unsigned depth)
    const;

  // State used by the direct-lighting calculator.
  //
  DirectIllum direct_illum;
};


}

#endif // __DIRECT_INTEG_H__
