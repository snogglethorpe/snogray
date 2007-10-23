// norm-glow.h -- Material whose color indicates surface normal
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __NORM_GLOW_H__
#define __NORM_GLOW_H__

#include "material.h"

namespace snogray {

class NormGlow : public Material
{
public:

  NormGlow (Color::component_t _intens)
    : Material (_intens, Material::SHADOW_OPAQUE), intens (_intens)
  { }

  virtual Color render (const Intersect &isec) const;

private:

  Color::component_t intens;
};

}

#endif /* __NORM_GLOW_H__ */
