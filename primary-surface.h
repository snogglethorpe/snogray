// primary-surface.h -- Standalone (non-embedded) surfaces
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PRIMARY_SURFACE_H__
#define __PRIMARY_SURFACE_H__

#include "surface.h"

namespace Snogray {

// "Primary" surfaces are those that are treated independently, as
// opposed to those which are embedded in other surfaces (and share
// state with them).
//
class PrimarySurface : public Surface
{
public:
  PrimarySurface (const Material *mat)
    : Surface (mat->shadow_type ()), _material (mat)
  { }

  // Returns the material this surface is made from
  //
  virtual const Material *material () const;

  const Material *_material;
};

}

#endif /* __PRIMARY_SURFACE_H__ */

// arch-tag: 58e34793-4724-4530-8f7a-0c6736037fd9
