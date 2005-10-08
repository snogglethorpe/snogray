// lambert.h -- Lambertian lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LAMBERT_H__
#define __LAMBERT_H__

#include "material.h"

namespace Snogray {

class Lambert : public Material
{
public:
  Lambert (const Color &_color) : color (_color) { }

  virtual const Color render (const Obj *obj, const Pos &point,
			      const Vec &normal, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const;

  Color color;
};

}

#endif /* __LAMBERT_H__ */

// arch-tag: ca8981f3-5471-4e8a-ac8b-2e3e54c83b64
