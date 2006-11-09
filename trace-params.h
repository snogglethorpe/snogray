// trace-params.h -- Parameters for tracing
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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
#include "params.h"

namespace Snogray {

class TraceParams
{
public:

  static const unsigned DEFAULT_BRDF_SAMPLES = 16;
  static const unsigned DEFAULT_LIGHT_SAMPLES = 16;
  static const unsigned DEFAULT_MAX_DEPTH = 6;
  static const dist_t DEFAULT_MIN_TRACE = 1e-10;
  static const float DEFAULT_SPECULAR_THRESHOLD = 50;
  static const float DEFAULT_ENVLIGHT_INTENS_FRAC = 0.5;

  TraceParams ()
    : max_depth (DEFAULT_MAX_DEPTH), min_trace (DEFAULT_MIN_TRACE)
  { }
  TraceParams (const Params &params)
    : num_brdf_samples (
	params.get_uint ("brdf-samples", DEFAULT_BRDF_SAMPLES)),
      num_light_samples (
	params.get_uint ("light-samples", DEFAULT_LIGHT_SAMPLES)),
      max_depth (
	params.get_uint ("max-depth", DEFAULT_MAX_DEPTH)),
      min_trace (
	params.get_float ("min-trace", DEFAULT_MIN_TRACE)),
      specular_threshold (
	params.get_float ("specular-threshold", DEFAULT_SPECULAR_THRESHOLD)),
      envlight_intens_frac (
	params.get_float ("envlight-intens-frac", DEFAULT_ENVLIGHT_INTENS_FRAC))
  { }

  unsigned num_brdf_samples;

  unsigned num_light_samples;

  // Deepest level of recursive tracing allowed.  Non-opaque shadow rays
  // use twice this depth (they have purely linear complexity though,
  // unlike the 2^n complexity of many reflections/refractions).
  //
  unsigned max_depth;

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

  // Threshold of BRDF reflectivity above which the BRDF is treated as
  // "specular" even if it's not truly specular.
  //
  float specular_threshold;

  float envlight_intens_frac;
};

}

#endif /* __TRACE_PARAMS_H__ */

// arch-tag: eeaa2e45-8ad3-4174-bac9-6814468841ee
