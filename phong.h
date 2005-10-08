// phog.h -- Phong lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHONG_H__
#define __PHONG_H__

#include "material.h"

namespace Snogray {

class Phong : public Material
{
public:
  Phong (float _exponent, const Color &_diffuse_color,
	 const Color &_specular_color = Color::white)
    : diffuse_color (_diffuse_color), specular_color (_specular_color),
      exponent (_exponent)
  { }

  virtual const Color render (const class Intersect &isec, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const;

  Color diffuse_color, specular_color;
  float exponent;
};

}

#endif /* __PHONG_H__ */

// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
