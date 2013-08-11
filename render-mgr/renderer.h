// renderer.h -- Low-level rendering driver
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RENDERER_H
#define SNOGRAY_RENDERER_H

#include "render/render-context.h"
#include "render/render-stats.h"
#include "render/sample-set.h"


namespace snogray {

class Camera;
class SampleGen;
class RenderPacket;


// Low-level rendering driver
//
class Renderer
{
public:

  Renderer (const GlobalRenderState &global_state,
	    const Camera &_camera,
	    unsigned _width, unsigned _height);

  
  // Render a single packet.
  //
  void render_packet (RenderPacket &packet);

  // Return rendering statistics for this renderer.
  //
  RenderStats stats () const { return context.stats; }

private:

  // The camera being used.
  //
  const Camera &camera;

  // Size of the virtual screen being rendered to.  These are floats
  // because they are always used as such.
  //
  float width, height;

  // Thread-local global R/W rendering state.
  //
  RenderContext context;

  // Sample channels for camera and camera-focus samples.
  //
  SampleSet::Channel<UV> camera_samples;
  SampleSet::Channel<UV> focus_samples;
};


}

#endif // SNOGRAY_RENDERER_H

// arch-tag: d7ddcb87-cf77-4eb0-a558-a73e4aa4a75f
