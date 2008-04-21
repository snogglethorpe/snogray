// tessel-sphere.h -- Sphere tessellation
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_SPHERE_H__
#define __TESSEL_SPHERE_H__

#include "tessel-param.h"


namespace snogray {


class Mesh;


class SphereTesselFun : public ParamTesselFun
{
public:

  SphereTesselFun (const Xform &_xform) : ParamTesselFun (_xform) { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  virtual dist_t sample_resolution (Tessel::err_t max_err) const;

  // Add to TESSEL and return a new vertex which is on this function's
  // surface midway between VERT1 and VERT2 (for some definition of
  // "midway").  This is the basic operation used during tessellation.
  // VERT1 and VERT2 are guaranteed to have come from either the original
  // basis defined by `define_basis', or from a previous call to
  // `midpoint'; thus it is safe for subclasses to down-cast them to
  // whatever Vertex subclass they use.
  //
  virtual Tessel::Vertex *midpoint (Tessel &tessel,
				    const Tessel::Vertex *vert1,
				    const Tessel::Vertex *vert2)
    const;

  // Return the surface position corresponding to the parameters U, V.
  //
  virtual Pos surface_pos (param_t u, param_t v) const;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const;
};


// Simple interface to SphereTesselFun
//
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Xform &xform, dist_t max_err);
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Pos &origin, const Vec &axis, dist_t max_err);
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Pos &origin, const Vec &axis, const Vec &radius,
		     dist_t max_err);


}

#endif // __TESSEL_SPHERE_H__
