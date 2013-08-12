// sample-map.h -- Visual representation of sample distribution
//
//  Copyright (C) 2006, 2007, 2008, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SAMPLE_MAP_H
#define SNOGRAY_SAMPLE_MAP_H

#include "color.h"
#include "image.h"
#include "image-io.h"

namespace snogray {

class Scene;
class RenderParams;
class IllumMgr;


// An image that holds a visual representation, in the form of a
// longitude-latitude map, of a light sample distribution.
//
class SampleMap
{
public:

  SampleMap () : num_samples (0), samples (mempool) { }

  // Add NUM samples from the first intersection reached by tracing EYE_RAY
  // into SCENE.
  //
  unsigned sample (const Ray &eye_ray, Scene &scene,
		   const RenderParams &render_params,
		   const IllumMgr &illum_mgr);

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

  // Memory allocation pool used by SAMPLES.
  //
  Mempool mempool;

private:

  void process_samples (const IllumSampleVec &samples);
};

}

#endif /* SNOGRAY_SAMPLE_MAP_H */

// arch-tag: eba7cd69-4c62-45e2-88e0-400cc3b22158
