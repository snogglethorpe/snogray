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


namespace snogray {

class GlobalRenderState;
class RenderContext;


class Integ
{
public:

  virtual ~Integ () { } 

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState
  {
  public:

    GlobalState (const GlobalRenderState &rstate)
      : global_render_state (rstate)
    { }
    virtual ~GlobalState () { } 

    const GlobalRenderState &global_render_state;
  };

protected:

  // Integrator state for rendering a group of related samples.
  //
  Integ (RenderContext &_context) : context (_context) { }

  RenderContext &context;
};


}

#endif // __INTEG_H__
