// envmap.h -- Environment maps
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ENVMAP_H__
#define __ENVMAP_H__

#include <string>

#include "color.h"
#include "vec.h"
#include "image.h"

namespace Snogray {

class Envmap
{
public:

  virtual ~Envmap () { }

  virtual Color map (const Vec &dir) const = 0;
};

// Return an appropriate subclass of Envmap, based on details of the
// filename or image.
//
Envmap *load_envmap (const std::string &filename);
Envmap *make_envmap (const Image &image);

}

#endif /* __ENVMAP_H__ */

// arch-tag: 9695753e-771b-4555-83c4-593486374642
