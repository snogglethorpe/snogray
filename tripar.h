// tripar.h -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRIPAR_H__
#define __TRIPAR_H__

#include "surface.h"

namespace snogray {

class Tripar : public Surface
{
public:

  Tripar (const Material *mat, const Pos &_v0, const Vec &_e1, const Vec &_e2,
	  bool _parallelogram = false)
    : Surface (mat), v0 (_v0), e1 (_e1), e2 (_e2),
      parallelogram (_parallelogram)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return true; otherwise
  // return false.  ISEC_PARAMS maybe used to pass information to a later
  // call to Surface::intersect_info.
  //
  virtual bool intersect (Ray &ray, IsecParams &isec_params) const;

  // Return an Intersect object containing details of the intersection of
  // RAY with this surface; it is assumed that RAY does actually hit the
  // surface, and RAY's maximum bound (Ray::t1) gives the exact point of
  // intersection (the `intersect' method modifies RAY so that this is
  // true).  ISEC_PARAMS contains other surface-specific parameters
  // calculated by the previous call to Surface::intersects method.
  //
  virtual Intersect intersect_info (const Ray &ray,
				    const IsecParams &isec_params,
				    Trace &trace)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  Pos v0;
  Vec e1, e2;
  bool parallelogram;
};

}

#endif /* __TRIPAR_H__ */

// arch-tag: cf7f49db-937d-4328-b7c0-3959241a191e
