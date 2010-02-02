// envmap-light.cc -- Lighting from an environment map
//
//  Copyright (C) 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "scene.h"
#include "globals.h"
#include "spheremap.h"
#include "light-map.h"
#include "lmap-analyzer.h"
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
    w *= cos ((y + h  / 2) / height * PIf - PIf/2);
    return w / h;
  }

  virtual float area (float, float y, float w, float h) const
  {
    return w * h * cos ((y + h  / 2) / height * PIf - PIf/2);
  }
};


EnvmapLight::EnvmapLight (const Ref<Envmap> &_envmap)
  : envmap (_envmap)
{
  if (!quiet)
    {
      std::cout << "* envmap-light: generating light map...";
      std::cout.flush ();
    }

  // An image holding light from envmap.
  //
  LatLongLmap lmap (envmap->light_map ());

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


// EnvmapLight::sample

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
EnvmapLight::sample (const Intersect &isec, const UV &param) const
{
  float intens_frac = isec.context.params.envlight_intens_frac;
  float hemi_frac = 1 - intens_frac;

  float inv_hemi_frac = (hemi_frac == 0) ? 0 : 1 / hemi_frac;
  float inv_intens_frac = (hemi_frac == 1) ? 0 : 1 / intens_frac;

  // True if we should use high-resolution intensity data.  Doing so
  // gives better results for specular reflections from very glossy
  // surfaces, but also results in more noise (variance) because the
  // intensity data doesn't precisely match the PDF (this is especially
  // noticeable for matte surfaces, where the increased accuracy doesn't
  // matter anyway).
  //
  // As a compromise, we currently use high-res intensity data for BRDF
  // samples (glossy surfaces will have tightly-grouped BRDF samples, so
  // inaccuracies in the intensity of BRDF samples will be more
  // obvious), but not for light samples (the main result of not using
  // hires data for light samples will be slightly inaccurate shadow
  // details, but this is usually much less obvious that inaccurate
  // glossy reflections).
  //
  // A further improvement would be to never use high-resolution data
  // for obviously non-glossy surfaces (lambertian, etc).
  //
  bool use_hires_intens = false;

  CosDist hemi_dist;

  // The intensity of this sample.
  //
  Color intens;

  // The pdf in the light's intensity distribution for this sample.
  //
  float intens_pdf;

  // The direction of this sample in the normal frame, and in the world
  // frame.
  //
  Vec dir, world_dir;

  float u = param.u, v = param.v;

  // Choose hemi or intensity sampling based on the V parameter.
  //
  if (v < hemi_frac)
    {
      // Map U,V to the hemisphere around ISEC's normal, with
      // distribution HEMI_DIST.

      float rescaled_v = v * inv_hemi_frac;

      dir = hemi_dist.sample (u, rescaled_v);
      world_dir = isec.normal_frame.from (dir);

      UV map_pos = LatLongMapping::map (world_dir);

      intens = intensity (map_pos.u, map_pos.v, intens_pdf);
    }
  else
    {
      // Map U,V to a direction (which may be anywhere in the
      // sphere) based on the light's intensity distribution.

      float rescaled_v = (v - hemi_frac) * inv_intens_frac;

      UV map_pos = intensity_sample (u, rescaled_v, intens, intens_pdf);

      world_dir = LatLongMapping::map (map_pos);
      dir = isec.normal_frame.to (world_dir);

      // If this sample is in the wrong hemisphere, throw it away.
      //
      if (isec.cos_n (dir) < 0 || isec.cos_geom_n (dir) < 0)
	return Sample ();
    }

  // If using "high-resolution intensity mode", get the actual intensity
  // from the environment map, which is more accurate.
  //
  if (use_hires_intens)
    intens = envmap->map (world_dir);

  // The intensity distribution covers the entire sphere, so adjust
  // the pdf to reflect that.
  //
  intens_pdf *= 0.25f * INV_PIf;

  float hemi_pdf = hemi_dist.pdf (isec.cos_n (dir));
  float pdf = hemi_frac * hemi_pdf + intens_frac * intens_pdf;

  return Sample (intens, pdf, dir, 0);
}


// EnvmapLight::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Value
EnvmapLight::eval (const Intersect &isec, const Vec &dir) const
{
  float intens_frac = isec.context.params.envlight_intens_frac;
  float hemi_frac = 1 - intens_frac;

  // True if we should use high-resolution intensity data.  Doing so
  // gives better results for specular reflections from very glossy
  // surfaces, but also results in more noise (variance) because the
  // intensity data doesn't precisely match the PDF (this is especially
  // noticeable for matte surfaces, where the increased accuracy doesn't
  // matter anyway).
  //
  // As a compromise, we currently use high-res intensity data for BRDF
  // samples (glossy surfaces will have tightly-grouped BRDF samples, so
  // inaccuracies in the intensity of BRDF samples will be more
  // obvious), but not for light samples (the main result of not using
  // hires data for light samples will be slightly inaccurate shadow
  // details, but this is usually much less obvious that inaccurate
  // glossy reflections).
  //
  // A further improvement would be to never use high-resolution data
  // for obviously non-glossy surfaces (lambertian, etc).
  //
  bool use_hires_intens = true;

  CosDist hemi_dist;

  // The sample direction in the world frame of reference.
  //
  Vec world_dir = isec.normal_frame.from (dir);

  // Find S's direction in our light map.
  //
  UV map_pos = LatLongMapping::map (world_dir);

  // Look up the intensity at that point.
  //
  float intens_pdf;
  Color intens = intensity (map_pos.u, map_pos.v, intens_pdf);

  // If using "high-resolution intensity mode", get the actual
  // intensity from the environment map, which is more accurate.
  //
  if (use_hires_intens)
    intens = envmap->map (world_dir);

  // The intensity distribution covers the entire sphere, so adjust
  // the pdf to reflect that.
  //
  intens_pdf *= 0.25f * INV_PIf;

  float hemi_pdf = hemi_dist.pdf (isec.cos_n (dir));

  return Value (intens, hemi_frac * hemi_pdf + intens_frac * intens_pdf, 0);
}


// arch-tag: 3fc8e1d6-6760-4983-84cc-b2c2e6674644
