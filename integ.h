// integ.h -- Light integrator interface
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

#ifndef __INTEG_H__
#define __INTEG_H__

#include "ray.h"
#include "tint.h"


namespace snogray {

class Scene;
class RenderContext;


class Integ
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState
  {
  public:

    GlobalState (const Scene &_scene) : scene (_scene) { }

    const Scene &scene;
  };

  // Return light from the scene arriving from the direction of RAY at its
  // origin.  SAMPLE_NUM is the sample to use.
  //
  virtual Tint li (const Ray &ray, unsigned sample_num) = 0;

protected:

  // Integrator state for rendering a group of related samples.
  //
  Integ (RenderContext &_context) : context (_context) { }

  RenderContext &context;
};


}

#endif // __INTEG_H__
