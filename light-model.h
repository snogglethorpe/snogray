// light-model.h -- Object light-model datatype
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

  virtual const Color render (const Intersect &isec,
			      const Vec &light_dir, const Color &light_color);
};

}

#endif /* __LIGHT_MODEL_H__ */

// arch-tag: d86d77f0-5f92-4138-b48b-248ac8be90db
