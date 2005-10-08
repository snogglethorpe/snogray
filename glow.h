// glow.h -- Constant-color lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLOW_H__
#define __GLOW_H__

#include "material.h"

namespace Snogray {

class Glow : public Material
{
public:
  Glow (const Color &_color) : color (_color) { }

  virtual const Color render (const Intersect &isec,
			      const Vec &light_dir, const Color &light_color)
    const;

  Color color;
};

}

#endif /* __GLOW_H__ */

// arch-tag: d53c41c0-1970-4b3e-9047-2f67dd943922
