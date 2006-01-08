// far-light.h -- Light at infinite distance
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FAR_LIGHT_H__
#define __FAR_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class FarLight : public Light
{
public:

  static const unsigned JITTER_STEPS = 5;

  FarLight (const Vec &_dir, dist_t _radius, const Color &col)
    : dir (_dir), radius (_radius), color (col),
      steps_radius (double (JITTER_STEPS) / 2)
  { init (); }

  // Return the color of the surface at ISEC, with nominal color
  // SURFACE_COLOR, as lit by this light with lighting-model
  // LIGHT_MODEL.
  //
  virtual Color illum (const Intersect &isec, const Color &surface_color,
		       const LightModel &light_model, TraceState &tstate)
    const;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale);

  Vec dir;
  dist_t radius;

  Color color;

private:

  void init ();
  
  Vec u, v;
  Vec u_inc, v_inc;

  double steps_radius;
};

}

#endif /* __FAR_LIGHT_H__ */

// arch-tag: 0691dd09-998d-4cdf-b5e9-da71aed2ec41
