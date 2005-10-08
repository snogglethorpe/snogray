// mirror.h -- Mirror lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIRROR_H__
#define __MIRROR_H__

#include "material.h"

namespace Snogray {

class Mirror : public Material
{
public:
  Mirror (float _reflectance, Material *_material)
    : reflectance (_reflectance), material (_material)
  { }

  virtual const Color render (const Intersect &isec,
			      const Vec &light_dir, const Color &light_color,
			      Scene &scene, unsigned depth)
    const;

  float reflectance;
  Material *material;
};

}

#endif /* __MIRROR_H__ */

// arch-tag: 59035a54-d50f-4c36-9308-9caa8dd13854
