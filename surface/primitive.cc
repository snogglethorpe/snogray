// surface.cc -- Primitive surface
//
//  Copyright (C) 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "light/surface-light-sampler.h"

#include "primitive.h"


using namespace snogray;


// Add light-samplers for this surface in SCENE to SAMPLERS.  Any
// samplers added become owned by the owner of SAMPLERS, and will be
// destroyed when it is.
//
void
Primitive::add_light_samplers (const Scene &,
			       std::vector<const Light::Sampler *> &samplers)
  const
{
  material->add_light_samplers (*this, samplers);
}

// Add light-samplers for this primitive's shape, with with intensity
// INTENSITY, to SAMPLERS.  An error will be signaled if this
// primitive does not support lighting.
//
void
Primitive::add_light_samplers (const TexVal<Color> &intens,
			       std::vector<const Light::Sampler *> &samplers)
  const
{
  samplers.push_back (new SurfaceLightSampler (*this, intens));
}


// Add statistics about this surface to STATS (see the definition of
// Surface::Stats below for details).  CACHE is used internally for
// coordination amongst nested surfaces.
//
// This method is intended for internal use in the Surface class
// hierachy, but cannot be protected: due to pecularities in the way
// that is defined in C++.
//
void
Primitive::accum_stats (Stats &stats, StatsCache &) const
{
  stats.num_render_surfaces++;
  stats.num_real_surfaces++;

  if (material->emits_light ())
    stats.num_lights++;
}
