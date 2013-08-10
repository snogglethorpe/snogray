// photon-eval.h -- Photon-map evaluation (lighting, etc)
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PHOTON_EVAL_H
#define SNOGRAY_PHOTON_EVAL_H

#include "material/bsdf.h"
#include "dir-hist.h"
#include "dir-hist-dist.h"
#include "photon-map.h"


namespace snogray {

class RenderContext;


class PhotonEval
{
public:

  // Global state for PhotonEval, for rendering an entire scene.
  //
  class GlobalState;

  PhotonEval (RenderContext &context, const GlobalState &global_state);

  // Return the light emitted from ISEC by photons found nearby in
  // PHOTON_MAP.  NUM_PATHS is the number of paths used when generating
  // PHOTON_MAP, and is used to scale the result.  FLAGS gives the types
  // of BSDF interaction to consider (by default, all).
  //
  Color Lo (const Intersect &isec, const PhotonMap &photon_map,
	    float scale, unsigned flags = Bsdf::ALL);

  // Return a reference to a DirHistDist object containing the
  // distribution of photons nearby ISEC in PHOTON_MAP.
  //
  // Note that return value is just a reference to a field in the
  // PhotonEval object, so if this method is called again, previous
  // results are invalidated.
  //
  DirHistDist &photon_dist (const Intersect &isec, const PhotonMap &photon_map);

  // Pointer to our global state info.
  //
  const GlobalState &global;

  // This is a temporary vector used by PhotonEval::Lo.  We keep it as
  // a field here to avoid memory-allocation churn.
  //
  std::vector<const Photon *> found_photons;

  // Temporary objects used by PhotonEval::photon_dist to avoid memory
  // allocation overhead.
  //
  DirHist photon_dir_hist;
  DirHistDist photon_dir_dist;
};


// Global state for PhotonEval, for rendering an entire scene.
//
class PhotonEval::GlobalState
{
public:

  GlobalState (unsigned num_search_photons,
	       dist_t photon_search_radius,
	       dist_t marker_radius = 0);

  // Number of photons (within the search radius) to use.
  //
  unsigned num_photons;

  // Radius-squared of the area around a point, within which to search
  // for photons.
  //
  dist_t search_radius_sq;

  // Radius-squared of photon position markers (for debugging).
  //
  dist_t marker_radius_sq;
};


}

#endif // SNOGRAY_PHOTON_EVAL_H
