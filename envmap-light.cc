// envmap-light.cc -- Lighting from an environment map
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
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
#include "radiance-map.h"
#include "rmap-analyzer.h"

#include "envmap-light.h"


using namespace Snogray;


// How many leaf regions (covering the entire sphere) we'll try to produce
// for a completely featureless input image.  Real scenes varied light
// intensities will roughly double this number.
//
#define NOMINAL_NUM_REGIONS 	150


// minimum angle for a envap light region, measured in terms of a
// fraction of 360 degrees.
//
#define MIN_ANGLE (2.f / 360.f)


class LatLongRmap : public RadianceMap
{
public:

  LatLongRmap (Ref<Image> _map) : RadianceMap (_map) { }

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
};


EnvmapLight::EnvmapLight (const Envmap &envmap)
{
  if (!quiet)
    {
      std::cout << "* envmap-light: generating radiance map...";
      std::cout.flush ();
    }

  // An image holding radiance from envmap.
  //
  LatLongRmap rmap (envmap.radiance_map ());

  if (!quiet)
    {
      std::cout << "analyzing...";
      std::cout.flush ();
    }

  // Analyze the resulting radiance map image.
  //
  RmapAnalyzer rmap_analyzer (rmap, NOMINAL_NUM_REGIONS);

  analyze (rmap_analyzer);

  if (!quiet)
    {
      std::cout << "done" << std::endl
		<< "* envmap-light: " << num_leaf_regions << " regions"
		<< ", radiance = " << root_region->radiance
		<< std::endl;
      std::cout.flush ();
    }
}


// Generate some samples of this light and add them to SAMPLES.
//
void
EnvmapLight::gen_samples (const Intersect &isec, SampleRayVec &samples) const
{
  gen_region_samples (samples);

  // Fixup the samples generated
  //
  // StructLight::gen_region_samples doesn't know the mapping from its 2d
  // uv space to 3d directions, so it simply encodes the uv coordinates as
  // the x and y components of each sample's direction vector.  We complete
  // the process by doing the conversion to a real direction.
  //
  for (SampleRayVec::iterator i = samples.begin(); i != samples.end(); ++i)
    {
      UV ll_uv = UV (i->dir.x, i->dir.y);

      i->dir = LatLongMapping::map (ll_uv);
      i->dist = Scene::DEFAULT_HORIZON;

      if (dot (isec.n, i->dir) <= 0)
	i->invalidate ();
    }
}


// arch-tag: 3fc8e1d6-6760-4983-84cc-b2c2e6674644
