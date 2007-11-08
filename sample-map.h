// sample-map.h -- Visual representation of sample distribution
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
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

namespace snogray {

class Scene;
class TraceParams;

// An image that holds a visual representation, in the form of a
// longitude-latitude map, of a light sample distribution.
//
class SampleMap
{
public:

  SampleMap () : num_samples (0) { }

  // Add NUM samples from the first intersection reached by tracing EYE_RAY
  // into SCENE.
  //
  unsigned sample (const Ray &eye_ray, Scene &scene,
		   const TraceParams &trace_params);

  // Normalize samples (so that the maximum sample has value 1)
  //
  void normalize ();

  // Draw a picture of the samples to MAP.  RADIUS is how wide a circle to
  // use for drawing each sample; if RADIUS is zero then each sample is drawn
  // with a single pixel in MAP.  COLOR is a color in which to draw the
  // samples; if omitted (or negative), the actual color of the sample will
  // be used.
  //
  void draw (Image &map, unsigned radius = 2, Color color = -1);

  // Various statistics
  //
  Color min, max, sum;
  unsigned num_samples;

  // Samples we've collected
  //
  IllumSampleVec samples;

private:

  void process_samples (const IllumSampleVec &samples);
};

}

#endif /* __SAMPLE_MAP_H__ */

// arch-tag: eba7cd69-4c62-45e2-88e0-400cc3b22158
