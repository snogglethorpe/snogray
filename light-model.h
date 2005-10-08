// light-model.h -- Lighting-model abstraction
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_MODEL_H__
#define __LIGHT_MODEL_H__

#include "pos.h"
#include "vec.h"
#include "color.h"

namespace Snogray {

class Intersect;

class LightModel
{
public:
  virtual ~LightModel (); // stop gcc bitching

  virtual Color illum (const Intersect &isec, const Color &color,
		       const Vec &light_dir, const Color &light_color)
    const = 0;
};

}

#endif /* __LIGHT_MODEL_H__ */

// arch-tag: 8360ddd7-dc17-40b8-8319-8f6d61fe62bf
