// photon-eval.cc -- Photon-map evaluation (lighting, etc)
//
//  Copyright (C) 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "intersect.h"
#include "bsdf.h"
#include "gaussian-filter.h"

#include "photon-eval.h"


using namespace snogray;



// Constructors etc

PhotonEval::GlobalState::GlobalState (unsigned num_search_photons,
				      dist_t photon_search_radius,
				      dist_t marker_radius)
 : num_photons (num_search_photons),
   search_radius_sq (photon_search_radius * photon_search_radius),
   marker_radius_sq (marker_radius * marker_radius)
{
}

PhotonEval::PhotonEval (RenderContext &, const GlobalState &global_state)
  : global (global_state),
    // photon_dir_hist (64, 64)
    photon_dir_hist (8, 8)
{
}


// PhotonEval::Lo

// Return the light emitted from ISEC by photons found nearby in
// PHOTON_MAP.  SCALE is the amount by which to scale each photon's
// radiance.  FLAGS gives the types of BSDF interaction to consider
// (by default, all).
//
Color
PhotonEval::Lo (const Intersect &isec, const PhotonMap &photon_map,
		float scale, unsigned flags)
{
  if (scale == 0)
    return 0;

  // Give up if this is a purely specular surface, or one that doesn't
  // support FLAGS.
  //
  if (! isec.bsdf->supports (flags & ~Bsdf::SPECULAR))
    return 0;

  const Pos &pos = isec.normal_frame.origin;
  unsigned num_photons = global.num_photons;
  dist_t max_dist_sq = global.search_radius_sq;

  found_photons.clear ();
  max_dist_sq = photon_map.find_photons (pos, num_photons, max_dist_sq,
					 found_photons);

  if (found_photons.size () == 0)
    return 0;

  // A gaussian filter, which emphasizes photons nearer to POS, and
  // de-emphasizes those farther away.
  //
  // GAUSS_ALPHA is a filter parameter which determines the shape of
  // the filter curve (larger values result in a sharper central
  // peak), and GAUSS_SCALE scales the result so that the resulting
  // filter will have an average value of 1 for a set of input points
  // uniformly distributed over a disk (as we're concerned with the
  // effect on surfaces).
  //
  dist_t gauss_alpha = 2;
  float gauss_scale = 2.90898;
  GaussianSqrtFilter<dist_t, float> gauss_filt (max_dist_sq, gauss_alpha);

  Color radiance = 0;

  for (std::vector<const Photon *>::iterator i = found_photons.begin();
       i != found_photons.end (); ++i)
    {
      const Photon &ph = **i;

      // Evaluate the BSDF in the photon's direction.
      //
      Vec dir = isec.normal_frame.to (ph.dir);
      Bsdf::Value bsdf_val = isec.bsdf->eval (dir, flags);

      if (bsdf_val.pdf != 0 && bsdf_val.val > 0)
	{
	  float filt = gauss_filt ((ph.pos - pos).length_squared ());
	  radiance += bsdf_val.val * ph.power * filt * gauss_scale;
	}

      // XXXX  PBRT avoids calling Bsdf::eval more than once for
      // diffuse surfaces (since they have a constant value)....
      // worthwhile?  probably not  XXXX
    }

  radiance *= scale;

  radiance /= float (max_dist_sq) * PIf;

  // Add photon position marker for debugging.
  //
  if (global.marker_radius_sq != 0)
    {
      for (std::vector<const Photon *>::iterator i = found_photons.begin();
	   i != found_photons.end (); ++i)
	{
	  const Photon &ph = **i;
	  dist_t dist_sq = (ph.pos - isec.normal_frame.origin).length_squared();
	  if (dist_sq < global.marker_radius_sq)
	    {
	      radiance = Color(0,1,0);
	      break;
	    }
	}
    }

  return radiance;
}


// PhotonEval::photon_dist

// Return a reference to a DirHistDist object containing the
// distribution of photons nearby ISEC in PHOTON_MAP.
//
// Note that return value is just a reference to a field in the
// PhotonEval object, so if this method is called again, previous
// results are invalidated.
//
DirHistDist &
PhotonEval::photon_dist (const Intersect &isec, const PhotonMap &photon_map)
{
  photon_dir_hist.clear ();

  // Find indirect photons near ISEC so we can sample based on their
  // distribution.
  //
  const Pos &pos = isec.normal_frame.origin;

  found_photons.clear ();
  photon_map.find_photons (pos, global.num_photons, global.search_radius_sq,
			   found_photons);

  // Generate a distribution from the photon directions we found.
  //
  for (std::vector<const Photon *>::iterator i = found_photons.begin();
       i != found_photons.end (); ++i)
    {
      const Vec &dir = (*i)->dir;

#if 0
      // Incorporate the BSDF response into the photon distribution
      //
      Vec bsdf_dir = isec.normal_frame.to (dir);
      Bsdf::Value bsdf_val
	= isec.bsdf->eval (bsdf_dir, Bsdf::ALL & ~Bsdf::SPECULAR);

      Color ph_pow = (*i)->power;
      Color filt_ph_pow = ph_pow * bsdf_val.val;
      intens_t filt_ph_intens = filt_ph_pow.intensity();
#else
      Color ph_pow = (*i)->power;
      intens_t filt_ph_intens = ph_pow.intensity();
#endif

      photon_dir_hist.add (dir, filt_ph_intens);
    }

  // Calculate a distribution from PHOTON_DIR_HIST.
  //
  photon_dir_dist.set_histogram (photon_dir_hist);

  return photon_dir_dist;
}
