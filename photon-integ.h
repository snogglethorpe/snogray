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
#include "direct-illum.h"

#include "recursive-integ.h"


namespace snogray {


class PhotonInteg : public RecursiveInteg
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public SurfaceInteg::GlobalState
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

    // Parameters used when searching for photons during rendering.
    //
    unsigned num_search_photons;
    float photon_search_radius;

    // Amount by which we scale each photon during rendering.
    //
    float caustic_scale, direct_scale, indirect_scale;

    // Radius-squared of photon position markers (for debugging).
    //
    dist_t marker_radius_sq;

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
  };

protected:

  // This method is called by RecursiveInteg to return any radiance
  // not due to specular reflection/transmission or direct emission.
  //
  virtual Color Lo (const Intersect &isec, const Media &media,
		    const SampleSet::Sample &sample);

private:

  // Integrator state for rendering a group of related samples.
  //
  PhotonInteg (RenderContext &context, GlobalState &global_state);

  // Return the light emitted from ISEC by photons found nearby in PHOTON_MAP.
  // NUM_PATHS is the number of paths used when generating PHOTON_MAP, and
  // is used to scale the result.
  //
  Color Lo_photon (const Intersect &isec, const PhotonMap &photon_map,
		   float scale);

  // "Final gathering": Do a quick calculation of indirection
  // illumination by sampling the BRDF, shooting another level of rays,
  // and using only photon maps to calculate outgoing illumination from
  // the resulting intersections.
  //
  Color Lo_fgather (const Intersect &isec, const Media &media,
		    const SampleSet::Sample &sample, unsigned num_samples);

  // Return a quick estimate of the outgoing radiance from ISEC which is
  // due to BSDF_SAMP.
  //
  Color Lo_fgather_samp (const Intersect &isec, const Media &media,
			 const Bsdf::Sample &bsdf_samp, unsigned depth);

  // Pointer to our global state info.
  //
  const GlobalState &global;

  // This is a temporary vector used by PhotonInteg::Lo.  We keep it as
  // a field here to avoid memory-allocation churn.
  //
  std::vector<const Photon *> found_photons;

  // State used by the direct-lighting calculator.
  //
  DirectIllum direct_illum;

  // Sample channels for BSDF sampling during final-gathering.
  //
  SampleSet::Channel<UV> fgather_bsdf_chan;
  SampleSet::Channel<float> fgather_bsdf_layer_chan;
};


}

#endif // __PHOTON_INTEG_H__
