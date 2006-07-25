// sample-map.h -- Visual representation of sample distribution
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE_MAP_H__
#define __SAMPLE_MAP_H__

#include "color.h"
#include "image.h"
#include "image-io.h"

namespace Snogray {

class Scene;
class Ray;
class TraceParams;

// An image that holds a visual representation, in the form of a
// longitude-latitude map, of a light sample distribution.
//
class SampleMap
{
public:

  // What sort of map to output: either raw brdf samples, raw light
  // samples, or the product of the two (the last is what is normally
  // used for rendering).
  //
  enum type { BRDF, LIGHTS, FILTERED };

  SampleMap (unsigned width, unsigned height, enum type _type)
    : map (width, height), map_type (_type), num_samples (0)
  { }

  void set_type (type _type) { map_type = _type; }

  // Add samples from the first intersection reached by tracing EYE_RAY
  // into SCENE.
  //
  void sample (const Ray &eye_ray, Scene &scene,
	       const TraceParams &trace_params);

  // Normalize samples (so that the maximum sample has value 1)
  //
  void normalize ();

  // Save this map to a file.
  //
  void save (const std::string &filename, const Params &params) const
  {
    map.save (filename, params);
  }

  // Return the map pixel in direction DIR.
  //
  Color get (const Vec &dir)
  {
    unsigned x = unsigned (map.width * (dir.longitude() + M_PI) / (M_PI * 2));
    unsigned y = unsigned (map.height * (-dir.latitude() + M_PI_2) / M_PI);
    return map (x, y);
  }

  // Set the map pixel in direction DIR to COL.
  //
  void put (const Vec &dir, const Color &col)
  {
    unsigned x = unsigned (map.width * (dir.longitude() + M_PI) / (M_PI * 2));
    unsigned y = unsigned (map.height * (-dir.latitude() + M_PI_2) / M_PI);
    map.put (x, y, col);
  }

  // The actual image map.
  //
  Image map;

  type map_type;

  // Various statistics
  //
  Color min, max, sum;
  unsigned num_samples;

  // Samples we've collected
  //
  SampleRayVec samples;

private:

  void process_samples (const SampleRayVec &samples);
};

}

#endif /* __SAMPLE_MAP_H__ */

// arch-tag: eba7cd69-4c62-45e2-88e0-400cc3b22158
