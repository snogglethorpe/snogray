// render-params.h -- Rendering parameters
//
//  Copyright (C) 2005-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RENDER_PARAMS_H
#define SNOGRAY_RENDER_PARAMS_H

#include "geometry/coords.h"
#include "val-table.h"

namespace snogray {

class RenderParams
{
public:

  // The minimum ray-length that will be accepted when tracing a ray; any
  // intersection closer than this to the ray origin will be ignored.
  //
  // If we're using single-precision coordinates, we have to allow a lot of
  // margin for imprecision to avoid self-intersection problems.  We do
  // this by bumping up the minimum trace distance to something fairly large.
  //
#if USE_DOUBLE_COORDS
  static dist_t default_min_trace () { return 1e-10; }
#else
  static dist_t default_min_trace () { return 1e-3; }
#endif

  RenderParams ()
    : min_trace (default_min_trace())
  { }
  RenderParams (const ValTable &params)
    : min_trace (params.get_float ("min_trace", default_min_trace()))
  { }

  // Minimum length of a traced ray; any objects closer than this to the
  // ray origin are ignored.  This doesn't apply to ordinary (opaque)
  // shadow rays, just recursive traces such as used by reflection or
  // refraction, and non-opaque shadow rays.  As other mechanisms avoid
  // hitting the surface of origin when tracing such rays, min_trace really
  // only helps if the model has multiple surfaces precisely located on top
  // of each other (which does does happen in practice, though usually in
  // weird cases like models with multiple "optional" parts which are all
  // enabled by default).
  //
  dist_t min_trace;
};

}

#endif /* SNOGRAY_RENDER_PARAMS_H */

// arch-tag: eeaa2e45-8ad3-4174-bac9-6814468841ee
