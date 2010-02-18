// light.h -- Light object
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "color.h"
#include "pos.h"
#include "vec.h"
#include "uv.h"


namespace snogray {

class Intersect;
class Scene;


class Light
{
public:

  // A sample of the light viewed from a particular point.
  //
  struct Sample
  {
    Sample (const Color &_val, float _pdf, const Vec &_dir, dist_t _dist)
      : val (_val), pdf (_pdf), dir (_dir), dist (_dist)
    { }
    Sample () : val (0), pdf (0), dist (0) { }

    // The value of the light for this sample.
    //
    Color val;

    // The value of the "probability density function" for this sample in the
    // light's sample distribution.
    //
    // However, if this is a specular (point) light, the value is not
    // defined (theoretically the value is infinity for specular samples).
    //
    float pdf;

    // The sample direction (the origin is implicit), in the
    // surface-normal coordinate system (where the surface normal is
    // (0,0,1)).
    //
    Vec dir;

    // How far the surface of the light is.
    //
    dist_t dist;
  };

  // A "free sample" of the light, from no particular point of view.
  //
  struct FreeSample
  {
    FreeSample (const Color &_val, float _pdf, const Pos &_pos, const Vec &_dir)
      : val (_val), pdf (_pdf), pos (_pos), dir (_dir)
    { }
    FreeSample () : val (0), pdf (0) { }

    // The value of the light for this sample.
    //
    Color val;

    // The value of the "probability density function" for this sample in the
    // light's sample distribution.
    //
    // However, if this is a specular (point) light, the value is not
    // defined (theoretically the value is infinity for specular samples).
    //
    float pdf;

    // The source position and direction of the sample, in world coordinates.
    //
    Pos pos;
    Vec dir;
  };

  struct Value
  {
    Value (const Color &_val, float _pdf, dist_t _dist)
      : val (_val), pdf (_pdf), dist (_dist)
    { }
    Value () : val (0), pdf (0), dist (0) { }

    // The value of the light for this value.
    //
    Color val;

    // The value of the "probability density function" for this value in the
    // light's value distribution.
    //
    // However, if this is a specular (point) light, the pdf is not defined
    // (theoretically the value is infinity for specular values).
    //
    float pdf;

    // How far the surface of the light is.
    //
    dist_t dist;
  };

  Light () : num (0) { }
  virtual ~Light () { }

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const = 0;

  // Return a "free sample" of this light.
  //
  virtual FreeSample sample (const UV &param, const UV &dir_param) const = 0;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const = 0;

  // Return true if this is a point light.
  //
  virtual bool is_point_light () const { return false; }

  // Return true if this is an "environmental" light, not associated
  // with any surface.
  //
  virtual bool is_environ_light () const { return false; }

  // Evaluate this environmental light in direction DIR (in world-coordinates).
  //
  virtual Color eval_environ (const Vec &/*dir*/) const { return 0; };

  // Do any scene-related setup for this light.  This is is called once
  // after the entire scene has been loaded.
  //
  virtual void scene_setup (const Scene &/*scene*/) { }

  // Each light has a number, which we use as a index to access various
  // data structures referring to lights.
  //
  unsigned num;
};


}

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
