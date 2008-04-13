// tessel-funs.h -- Simple interfaces to common tessellations
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_FUNS_H__
#define __TESSEL_FUNS_H__

#include "ref.h"
#include "material.h"
#include "xform.h"


namespace snogray {

class Mesh;


// Simple interface to SphereTesselFun
//
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Xform &xform, dist_t max_err);
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Pos &origin, const Vec &axis, dist_t max_err);
Mesh *tessel_sphere (const Ref<const Material> &mat,
		     const Pos &origin, const Vec &axis, const Vec &radius,
		     dist_t max_err);

// Simple interface to SincTesselFun
//
Mesh *tessel_sinc (const Ref<const Material> &mat,
		   const Xform &xform, dist_t max_err);
Mesh *tessel_sinc (const Ref<const Material> &mat,
		   const Pos &origin, const Vec &axis, const Vec &radius,
		   dist_t max_err);
Mesh *tessel_sinc (const Ref<const Material> &mat,
		   const Pos &origin, const Vec &axis, dist_t radius,
		   dist_t max_err);

// Simple interface to TorusTesselFun
//
Mesh *tessel_torus (const Ref<const Material> &mat,
		    const Xform &xform, dist_t hole_frac, dist_t max_err);
Mesh *tessel_torus (const Ref<const Material> &mat,
		    const Pos &origin, const Vec &axis, const Vec &radius,
		    dist_t max_err);
Mesh *tessel_torus (const Ref<const Material> &mat,
		    const Pos &origin, const Vec &axis, dist_t radius,
		    dist_t max_err);


}

#endif // __TESSEL_FUNS_H__
