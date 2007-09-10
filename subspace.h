// subspace.h -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SUBSPACE_H__
#define __SUBSPACE_H__


#include "surface.h"


namespace snogray {

class Space;


// This is a surface wrapper that puts wrapped surfaces into a separate
// subspace (acceleration structure).
//
class Subspace : public Surface
{
public:

  Subspace (Surface *surf) : Surface (0), surface (surf), space (0) { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with ISEC_CTX); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, const IsecCtx &isec_ctx) const;

  // Return the strongest type of shadowing effect this surface has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  virtual Material::ShadowType shadow (const ShadowRay &ray) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  // Make sure our acceleration structure is set up.
  //
  void ensure_space (GlobalTraceState &global) const;

  // Top-level surface in our subspace.
  //
  Surface *surface;

  // Space holding everything from SURFACE..
  //
  mutable const Space *space;
};


}


#endif /* __SUBSPACE_H__ */
