// intersect.h -- Datatype for recording surface-ray intersection results
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface.h"
#include "material.h"
#include "brdf.h"
#include "lsamples.h"
#include "trace.h"
#include "scene.h"
#include "global-tstate.h"

#include "intersect.h"

using namespace Snogray;



Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Pos &_point, const Vec &_normal, bool _back,
		      Trace &_trace, const void *_smoothing_group)
  : ray (_ray), surface (_surface),
    point (_point), normal (_back ? -_normal : _normal), back (_back),
    viewer (-_ray.dir), nv (dot (viewer, normal)),
    material (*_surface->material ()),
    brdf (material.brdf), color (material.color),
    smoothing_group (_smoothing_group), trace (_trace)
{ }

// For surfaces with non-interpolated normals, we can calculate
// whether it's a backface or not using the normal; they typically
// also have a zero smoothing group, so we omit that parameter.
//
Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Pos &_point, const Vec &_normal, Trace &_trace)
  : ray (_ray), surface (_surface),
    point (_point), normal (_normal), back (dot (normal, _ray.dir) > 0),
    viewer (-_ray.dir), nv (dot (viewer, normal)),
    material (*_surface->material ()),
    brdf (material.brdf), color (material.color),
    smoothing_group (0), trace (_trace)
{
  // We want to flip the sign on `normal' if `back' is true, but we've
  // declared `normal' const to avoid anybody mucking with it...
  //
  if (back)
    const_cast<Vec&> (normal) = -normal;
}




// Iterate over every light, calculating its contribution the color of ISEC.
// BRDF is used to calculate the actual effect; COLOR is the "base color"
//
Color
Intersect::illum () const
{
  LightSamples &lsamples = trace.global.light_samples;

  lsamples.generate (*this, trace.scene.lights);

  trace.global.stats.illum_calls++;
  trace.global.stats.illum_samples += lsamples.size ();

  // The maximum _possible_ output radiance (from lights), if all remaining
  // samples are not shadowed.
  //
  Color poss_radiance;
  for (LightSamples::iterator s = lsamples.begin (); s != lsamples.end (); s++)
    poss_radiance += s->val;

  // Accumulated output radiance
  //
  Color radiance;

  // Now shoot shadow rays to check the visibility of the chosen set of
  // light samples, accumulating the resulting radiance in RADIANCE.

  Trace &sub_trace = subtrace (Trace::SHADOW);

  for (LightSamples::iterator s = lsamples.begin (); s != lsamples.end (); s++)
    if (s->val > 0)
      {
	// If RADIANCE is beyond some threshold, give up even though we
	// haven't finished all samples yet.  This means that in cases
	// where the output radiance is dominated by very bright lights,
	// we won't waste time calculating the remaining dim ones.
	//
	if (radiance > poss_radiance * 0.95 /* XXX make a variable */)
	  {
	    // XXX somehow use the unused samples, except without sending out
	    // shadow rays [perhaps (1) keep track of last-known visibility
	    // per-light, and (2) update the visibility of some random subset
	    // of the remaining unused lights]
	    //
	    break;
	  }

	const Ray shadow_ray (point, s->dir, s->dist);

	// Find any surface that's shadowing LIGHT_RAY.
	//
	const Surface *shadower
	  = sub_trace.shadow_caster (shadow_ray, *s->light, *this);

	if (! shadower)
	  //
	  // The surface is not shadowed at all, just add the light.
	  //
	  radiance += s->val;

	else if (shadower->shadow_type != Material::SHADOW_OPAQUE)
	  //
	  // There's a shadower, but it's not opaque, so give it (and
	  // any further surfaces) a chance to attentuate the color.
	  {
	    trace.global.stats.scene_slow_shadow_traces++;

	    // The reflected radiance from this sample, after being adjusted
	    // for the filtering of the source irradiance through SHADOWER.
	    //
	    Color filtered = sub_trace.shadow (shadow_ray, s->val, *s->light);

	    // Add the final filtered radiance to RADIANCE, and also
	    // adjust our estimation of the total possible radiance to
	    // account for the filtering.
	    //
	    radiance += filtered;
	    poss_radiance -= s->val - filtered;
	  }

	else
	  // The surface is shadowed; subtract this light from the
	  // possible radiance.
	  //
	  poss_radiance -= s->val;
      }

  return radiance;
}


// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
