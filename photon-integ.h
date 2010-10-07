// photon-integ.h -- Photon-mapping surface integrator
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

#ifndef __PHOTON_INTEG_H__
#define __PHOTON_INTEG_H__

#include "bsdf.h"
#include "photon-map.h"
#include "photon-eval.h"
#include "direct-illum.h"

#include "recursive-integ.h"


namespace snogray {


class PhotonInteg : public RecursiveInteg
{
public:

  // Global state for PhotonInteg, for rendering an entire scene.
  //
  class GlobalState;

protected:

  // This method is called by RecursiveInteg to return any radiance
  // not due to specular reflection/transmission or direct emission.
  //
  virtual Color Lo (const Intersect &isec, const Media &media,
		    const SampleSet::Sample &sample);

private:

  class Shooter;

  // Integrator state for rendering a group of related samples.
  //
  PhotonInteg (RenderContext &context, GlobalState &global_state);

  // Return the light emitted from ISEC by photons found nearby in
  // PHOTON_MAP.  NUM_PATHS is the number of paths used when generating
  // PHOTON_MAP, and is used to scale the result.  FLAGS gives the types
  // of BSDF interaction to consider (by default, all).
  //
  Color Lo_photon (const Intersect &isec, const PhotonMap &photon_map,
		   float scale, unsigned flags = Bsdf::ALL)
  {
    return photon_eval.Lo (isec, photon_map, scale, flags);
  }

  // "Final gathering": Do a quick calculation of indirection
  // illumination by sampling the BRDF, shooting another level of
  // rays, and using only photon maps to calculate outgoing
  // illumination from the resulting intersections.
  //
  // For samples that strike perfectly specular materials, recursive
  // sampling is used used until a non-specular surface is hit, and
  // then the photon-map is evaluated at that point; this handles
  // indirect illumination due to caustics, etc.
  //
  // If AVOID_CAUSTICS_ON_DIFFUSE is true, then any contribution of
  // caustics on diffuse surfaces is intentionally ignored (this is
  // useful because such effects are usually handled via a separate
  // caustics photon-pap).
  //
  Color Lo_fgather (const Intersect &isec, const Media &media,
		    const SampleSet::Sample &sample,
		    bool avoid_caustics_on_diffuse);

  // Return a quick estimate of the outgoing radiance from ISEC which
  // is due to BSDF_SAMP.  INDIR_EMISSION_SCALE is used to scale
  // direct surface (or background) emission for recursive calls
  // (i.e., when DEPTH > 0; direct emission is always omitted when
  // DEPTH == 0).  DEPTH is the recursion depth; it is zero for all
  // external callers, and incremented during recursive calls.
  //
  Color Lo_fgather_samp (const Intersect &isec, const Media &media,
			 const Bsdf::Sample &bsdf_samp,
			 const Color &indir_emission_scale, unsigned depth = 0);

  // Pointer to our global state info.
  //
  const GlobalState &global;

  // The photon-map evaluator.
  //
  PhotonEval photon_eval;

  // State used by the direct-lighting calculator.
  //
  DirectIllum direct_illum;

  // Sample channels for sampling during final-gathering.
  //
  SampleSet::Channel<UV> fgather_bsdf_chan;
  SampleSet::Channel<float> fgather_bsdf_layer_chan;
  SampleSet::Channel<UV> fgather_photon_chan;
};



// PhotonInteg::GlobalState

// Global state for PhotonInteg, for rendering an entire scene.
//
class PhotonInteg::GlobalState : public SurfaceInteg::GlobalState
{
public:

  GlobalState (const GlobalRenderState &rstate, const ValTable &params);

  // Return a new integrator, allocated in context.
  //
  virtual SurfaceInteg *make_integrator (RenderContext &context);

private:

  friend class PhotonInteg;

  // Generate the specified number of photons and add them to our photon-maps.
  //
  void generate_photons (unsigned num_caustic, unsigned num_direct, unsigned num_indirect);

  // Photon-maps for various types of photons.
  //
  PhotonMap direct_photon_map;
  PhotonMap caustic_photon_map;
  PhotonMap indirect_photon_map;

  // Amount by which we scale each photon during rendering.
  //
  float caustic_scale, direct_scale, indirect_scale;

  PhotonEval::GlobalState photon_eval;

  DirectIllum::GlobalState direct_illum;

  // True if we should use the DIRECT_ILLUM for direction illumination;
  // otherwise, we use DIRECT_PHOTON_MAP instead (which is less
  // accurate, but probably cheaper).
  //
  bool use_direct_illum;

  // Number of samples to use in "final gathering" for indirect
  // illumination.  If zero, final gathering is not done.
  //
  unsigned num_fgather_samples;
  unsigned num_fgather_photon_samples;
  unsigned num_fgather_bsdf_samples;
};


}

#endif // __PHOTON_INTEG_H__
