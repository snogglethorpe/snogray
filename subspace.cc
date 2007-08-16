// subspace.cc -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "octree.h"
#include "shadow-ray.h"

#include "subspace.h"


using namespace snogray;


// Make sure our acceleration structure is set up.
//
inline void
Subspace::ensure_space () const
{
  if (! space)
    {
      space = new Octree;
      surface->add_to_space (space);
    }
}


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Subspace::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  ensure_space ();
  return space->intersect (ray, isec_ctx);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Subspace::shadow (const ShadowRay &sray) const
{
  ensure_space ();
  return space->shadow (sray, sray.isec.trace);
}

// Return a bounding box for this surface.
//
BBox
Subspace::bbox () const
{
  return surface->bbox ();
}
