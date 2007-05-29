// envmap-light.cc -- Lighting from an environment map
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "scene.h"
#include "globals.h"
#include "spheremap.h"
#include "light-map.h"
#include "lmap-analyzer.h"
#include "grid-iter.h"
#include "cos-dist.h"

#include "envmap-light.h"


using namespace snogray;


// How many leaf regions (covering the entire sphere) we'll try to produce
// for a completely featureless input image.  Real scenes varied light
// intensities will roughly double this number.
//
#define NOMINAL_NUM_REGIONS 	1000


// minimum angle for a envap light region, measured in terms of a
// fraction of 360 degrees.
//
#define MIN_ANGLE (2.f / 360.f)


class LatLongLmap : public LightMap
{
public:

  LatLongLmap (Ref<Image> _map) : LightMap (_map) { }

  virtual bool too_small (float, float, float w, float h) const
  {
    return
      w <= 2 || (w / width) < MIN_ANGLE
      || h <= 2 || (h / height) < MIN_ANGLE;
  }

  virtual float aspect_ratio (float, float y, float w, float h) const
  {
    w *= cos ((y + h  / 2) / height * M_PIf - M_PI_2f);
    return w / h;
  }

  virtual float area (float, float y, float w, float h) const
  {
    return w * h * cos ((y + h  / 2) / height * M_PIf - M_PI_2f);
  }
};


EnvmapLight::EnvmapLight (const Envmap &envmap)
{
  if (!quiet)
    {
      std::cout << "* envmap-light: generating light map...";
      std::cout.flush ();
    }

  // An image holding light from envmap.
  //
  LatLongLmap lmap (envmap.light_map ());

  if (!quiet)
    {
      std::cout << lmap.map->width << " x " << lmap.map->height
		<< "; analyzing...";
      std::cout.flush ();
    }

  // Analyze the resulting light map image.
  //
  LmapAnalyzer lmap_analyzer (lmap, NOMINAL_NUM_REGIONS);

  analyze (lmap_analyzer);

  if (! quiet)
    {
      unsigned num_sample_regions, num_leaf_regions;
      Color mean_intensity;
      get_stats (num_sample_regions, num_leaf_regions, mean_intensity);
      std::cout << "done" << std::endl
		<< "* envmap-light: " << num_leaf_regions
		<< "+" << (num_sample_regions - num_leaf_regions) << " regions"
		<< ", mean intensity = " << mean_intensity
		<< std::endl;
      std::cout.flush ();
    }
}



// Generate some samples of this light and add them to SAMPLES.
//
unsigned
EnvmapLight::gen_samples (const Intersect &isec, unsigned num,
			  IllumSampleVec &samples)
  const
{
  float intens_frac = isec.trace.global.params.envlight_intens_frac;
  float hemi_frac = 1 - intens_frac;

  float inv_hemi_frac = (hemi_frac == 0) ? 0 : 1 / hemi_frac;
  float inv_intens_frac = (hemi_frac == 1) ? 0 : 1 / intens_frac;

  CosDist hemi_dist;

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      // The intensity of this sample.
      //
      Color intens;

      // The pdf in the light's intensity distribution for this sample.
      //
      float intens_pdf;

      // The direction of this sample.
      //
      Vec dir;

      // Choose hemi or intensity sampling based on the V parameter.
      //
      if (v < hemi_frac)
	{
	  // Map U,V to the hemisphere around ISEC's normal, with
	  // distribution HEMI_DIST.
	  // 
	  float rescaled_v = v * inv_hemi_frac;
	  dir = isec.z_normal_to_world (hemi_dist.sample (u, rescaled_v));
	  UV map_pos = LatLongMapping::map (dir);
	  intens = intensity (map_pos.u, map_pos.v, intens_pdf);
	}
      else
	{
	  // Map U,V to a direction (which may be anywhere in the
	  // sphere) based on the light's intensity distribution.
	  //
	  float rescaled_v = (v - hemi_frac) * inv_intens_frac;
	  UV map_pos = intensity_sample (u, rescaled_v, intens, intens_pdf);
	  dir = LatLongMapping::map (map_pos);

	  // If this sample is in the wrong hemisphere, throw it away.
	  //
	  if (isec.cos_n (dir) < 0)
	    continue;
	}

      // The intensity distribution covers the entire sphere, so adjust
      // the pdf to reflect that.
      //
      intens_pdf *= 0.25f * M_1_PI;

      float hemi_pdf = hemi_dist.pdf (isec.cos_n (dir));
      float pdf = hemi_frac * hemi_pdf + intens_frac * intens_pdf;

      samples.push_back (IllumSample (dir, intens, pdf, 0, this));
    }

  return grid_iter.num_samples ();
}



// For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
// light, and where light is closer than the sample's previously recorded
// light distance (or the previous distance is zero), overwrite the
// sample's light-related fields with information from this light.
//
void
EnvmapLight::filter_samples (const Intersect &isec,
			     const IllumSampleVec::iterator &beg_sample,
			     const IllumSampleVec::iterator &end_sample)
  const
{
  float intens_frac = isec.trace.global.params.envlight_intens_frac;
  float hemi_frac = 1 - intens_frac;

  CosDist hemi_dist;

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
    if (s->dist == 0)
      {
	// Find S's direction in our light map.
	//
	UV map_pos = LatLongMapping::map (s->dir);

	// Look up the intensity at that point.
	//
	float intens_pdf;
	s->val = intensity (map_pos.u, map_pos.v, intens_pdf);

	// The intensity distribution covers the entire sphere, so adjust
	// the pdf to reflect that.
	//
	intens_pdf *= 0.25f * M_1_PI;

	float hemi_pdf = hemi_dist.pdf (isec.cos_n (s->dir));
	s->light_pdf = hemi_frac * hemi_pdf + intens_frac * intens_pdf;
	s->light = this;
      }
}


// arch-tag: 3fc8e1d6-6760-4983-84cc-b2c2e6674644
