// envmap-light.cc -- Lighting from an environment map
//
//  Copyright (C) 2006-2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "render/scene.h"
#include "texture/spheremap.h"
#include "geometry/hist-2d.h"
#include "geometry/hist-2d-dist.h"
#include "geometry/sphere-sample.h"
#include "geometry/tangent-disk-sample.h"
#include "light-sampler.h"

#include "envmap-light.h"


using namespace snogray;



// EnvmapLight::Sampler

class EnvmapLight::Sampler : public Light::Sampler
{
public:

  Sampler (const EnvmapLight &_light, const BBox &scene_bbox)
    : light (_light),
      scene_center (scene_bbox.center ()),
      scene_radius (scene_bbox.radius ()),
      intensity_dist (envmap_histogram (light.envmap))
  {
  }

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const;

  // Return a "free sample" of this light.
  //
  virtual FreeSample sample (const UV &param, const UV &dir_param) const;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const;

  // Return true if this is an "environmental" light, not associated
  // with any surface.
  //
  virtual bool is_environ_light () const { return true; }

  // Evaluate this environmental light in direction DIR (in world-coordinates).
  //
  virtual Color eval_environ (const Vec &dir) const
  {
    return light.envmap->map (light.frame.to (dir));
  }

private:

  // Return a 2d histogram containing the intensity of ENVMAP, with the
  // intensity adusted to reflect the area distortion caused by mapping
  // it to a sphere.
  //
  Hist2d envmap_histogram (const Ref<Envmap> &envmap);

  // Light we're sampling.
  //
  const EnvmapLight &light;

  // Center and radius of a bounding sphere for the engire scene.
  //
  Pos scene_center;
  dist_t scene_radius;

  // Distribution for sampling the intensity of ENVMAP.
  //
  Hist2dDist intensity_dist;
};


// Add light-samplers for this light in SCENE to SAMPLERS.  Any
// samplers added become owned by the owner of SAMPLERS, and will be
// destroyed when it is.
//
void
EnvmapLight::add_light_samplers (const Scene &scene,
				 std::vector<const Light::Sampler *> &samplers)
  const
{
  samplers.push_back (new Sampler (*this, scene.bbox ()));
}



// EnvmapLight::Sampler::envmap_histogram

// Return a 2d histogram containing the intensity of ENVMAP, with the
// intensity adusted to reflect the area distortion caused by mapping
// it to a sphere.
//
Hist2d
EnvmapLight::Sampler::envmap_histogram (const Ref<Envmap> &envmap)
{
  Ref<Image> lmap = envmap->light_map ();
  
  unsigned w = lmap->width, h = lmap->height;

  Hist2d hist (w, h);

  double row_lat_inc = PI / h;
  double row_lat = -PI/2 + row_lat_inc/2;

  for (unsigned row = 0; row < hist.height; row++)
    {
      double row_scale = cos (row_lat);

      for (unsigned col = 0; col < hist.width; col++)
	{
	  Color color = (*lmap) (col, row);
	  double intens = double (color.intensity()) * row_scale;
	  hist.add (col, row, intens);
	}

      row_lat += row_lat_inc;
    }

  return hist;
}



// EnvmapLight::Sampler::sample (viewpoint)

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sampler::Sample
EnvmapLight::Sampler::sample (const Intersect &isec, const UV &param) const
{
  // The pdf in the light's intensity distribution for this sample.
  //
  float pdf;

  // Map U,V to a direction (which may be anywhere in the
  // sphere) based on the light's intensity distribution.
  //
  UV map_pos = intensity_dist.sample (param, pdf);

  // The direction of this sample in our frame.
  //
  Vec light_dir = LatLongMapping::map (map_pos);

  // Convert LIGHT_DIR to the world frame.
  //
  Vec world_dir = light.frame.from (light_dir);

  // ... and then to the normal frame.
  //
  Vec dir = isec.normal_frame.to (world_dir);

  // If this sample is in the wrong hemisphere, throw it away.
  //
  if (isec.cos_n (dir) < 0 || isec.cos_geom_n (dir) < 0)
    return Sample ();

  // The intensity distribution covers the entire sphere, so adjust
  // the pdf to reflect that.
  //
  pdf *= 0.25f * INV_PIf;

  return Sample (light.envmap->map (light_dir), pdf, dir, 0);
}



// EnvmapLight::Sampler::sample (free)

// Return a "free sample" of this light.
//
Light::Sampler::FreeSample
EnvmapLight::Sampler::sample (const UV &param, const UV &dir_param) const
{
  // The pdf in the light's intensity distribution for this sample.
  //
  float pdf;

  // Sample using intensity distribution
  //
  UV map_pos = intensity_dist.sample (dir_param, pdf);

  // Direction (in world coordinates) of the sample.
  //
  Vec light_dir = LatLongMapping::map (map_pos);

  // Convert to world coordinates.
  //
  Vec world_dir = light.frame.from (light_dir);

  // The intensity distribution covers the entire sphere, so adjust
  // the pdf to reflect that.
  //
  pdf = pdf * 0.25f * INV_PIf;

  // Choose a sample position "at infinity".
  //
  Pos pos = tangent_disk_sample (scene_center, scene_radius, world_dir, param);

  // The sample's PDF is the intensity PDF adjusted to reflect disk
  // sampling for the position.
  //
  pdf /= float (scene_radius * scene_radius) * PIf;

  // Return the sample; we invert the DIR we calculated above, as it
  // points _towards_ the sample point, and the return value should
  // have a direction _from_ the sample point.
  //
  return FreeSample (light.envmap->map (world_dir), pdf, pos, -world_dir);
}



// EnvmapLight::Sampler::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Sampler::Value
EnvmapLight::Sampler::eval (const Intersect &isec, const Vec &dir) const
{
  // The sample direction in the world frame of reference.
  //
  Vec world_dir = isec.normal_frame.from (dir);

  // ... and in the light's frame of reference.
  //
  Vec light_dir = light.frame.to (world_dir);

  // Find S's direction in our light map.
  //
  UV map_pos = LatLongMapping::map (light_dir);

  // Look up the intensity and PDF at that point.
  //
  Color intens = light.envmap->map (light_dir);
  float pdf = intensity_dist.pdf (map_pos);

  // The intensity distribution covers the entire sphere, so adjust
  // the pdf to reflect that.
  //
  pdf *= 0.25f * INV_PIf;

  return Value (intens, pdf, 0);
}



// EnvmapLight::transform


// Transform the geometry of this light by XFORM.
//
void
EnvmapLight::transform (const Xform &xform)
{
  frame.transform (xform);
}


// arch-tag: 3fc8e1d6-6760-4983-84cc-b2c2e6674644
