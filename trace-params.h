// trace-params.h -- Parameters for tracing
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRACE_PARAMS_H__
#define __TRACE_PARAMS_H__

#include "coords.h"
#include "val-table.h"

namespace snogray {

class TraceParams
{
public:

  // Number of BRDF samples to take, per intersection.
  //
  static const unsigned DEFAULT_BRDF_SAMPLES = 16;

  // Number of light samples.
  //
  static const unsigned DEFAULT_LIGHT_SAMPLES = 16;

  // The minimum ray-length that will be accepted when tracing a ray; any
  // intersection closer than this to the ray origin will be ignored.
  //
  // If we're using single-precision coordinates, we have to allow a lot of
  // margin for imprecision to avoid self-intersection problems.  We do
  // this by bumping up the minimum trace distance to something fairly large.
  //
#if USE_FLOAT_COORDS
  static const dist_t DEFAULT_MIN_TRACE = 1e-3;
#else
  static const dist_t DEFAULT_MIN_TRACE = 1e-10;
#endif

  // Recursive tracing depth at which we begin using russian-roulette to
  // prune recursion due to specular reflection/refraction.
  //
  static const unsigned DEFAULT_SPEC_RR_DEPTH = 5;

  // Deepest level of recursive tracing allowed.  Non-opaque shadow rays
  // use twice this depth.  Because we use russian-roulette to avoid
  // excessive recursion, this limit should never be hit in practice.
  //
  static const unsigned DEFAULT_MAX_DEPTH = 100; // shouldn't ever be hit

  static const float DEFAULT_SPECULAR_THRESHOLD = 50;
  static const float DEFAULT_ENVLIGHT_INTENS_FRAC = 0.5;

  TraceParams ()
    : num_brdf_samples (DEFAULT_BRDF_SAMPLES),
      num_light_samples (DEFAULT_LIGHT_SAMPLES),
      min_trace (DEFAULT_MIN_TRACE),
      spec_rr_depth (DEFAULT_SPEC_RR_DEPTH),
      max_depth (DEFAULT_MAX_DEPTH),
      specular_threshold (DEFAULT_SPECULAR_THRESHOLD),
      envlight_intens_frac (DEFAULT_ENVLIGHT_INTENS_FRAC)
  { }
  TraceParams (const ValTable &params)
    : num_brdf_samples (
	params.get_uint ("brdf-samples", DEFAULT_BRDF_SAMPLES)),
      num_light_samples (
	params.get_uint ("light-samples", DEFAULT_LIGHT_SAMPLES)),
      min_trace (
	params.get_float ("min-trace", DEFAULT_MIN_TRACE)),
      spec_rr_depth (
	params.get_uint ("spec-rr-depth", DEFAULT_SPEC_RR_DEPTH)),
      max_depth (
	params.get_uint ("max-depth", DEFAULT_MAX_DEPTH)),
      specular_threshold (
	params.get_float ("specular-threshold", DEFAULT_SPECULAR_THRESHOLD)),
      envlight_intens_frac (
	params.get_float ("envlight-intens-frac", DEFAULT_ENVLIGHT_INTENS_FRAC))
  { }

  // Number of BRDF samples to take, per intersection.
  //
  unsigned num_brdf_samples;

  // Number of light samples.
  //
  unsigned num_light_samples;

  // Minimum length of a traced ray; any objects closer than this to the
  // ray origin are ignored.  This doesn't apply to ordinary (opaque)
  // shadow rays, just recursive traces such as used by reflection or
  // refraction, and non-opaque shadow rays.  As other mechanisms avoid
  // hitting the surface of origin when tracing such rays, min_trace
  // really only helps if the model has multiple surfaces precisely
  // located on top of each other (which does does happen in practice,
  // though usually in weird cases like models with multiple "optional"
  // parts which are all enabled by default).
  //
  dist_t min_trace;

  // Recursive tracing depth at which we begin using russian-roulette to
  // prune recursion due to specular reflection/refraction.
  //
  unsigned spec_rr_depth;

  // Deepest level of recursive tracing allowed.  Non-opaque shadow rays
  // use twice this depth.  Because we use russian-roulette to avoid
  // excessive recursion, this limit should never be hit in practice.
  //
  unsigned max_depth;

  // Threshold of BRDF reflectivity above which the BRDF is treated as
  // "specular" even if it's not truly specular.
  //
  float specular_threshold;

  float envlight_intens_frac;
};

}

#endif /* __TRACE_PARAMS_H__ */

// arch-tag: eeaa2e45-8ad3-4174-bac9-6814468841ee
