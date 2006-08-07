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

// Return an appropriate subclass of Envmap, initialized from SPEC
// (usually a filename to load).  FMT is the type of environment-map.
//
// If FMT is "", any colon-separated prefix will be removed from SPEC,
// and used as the format name (and ther remainder of SPEC used as the
// actual filename); if FMT is "auto", SPEC will be left untouched, and
// an attempt will be made to guess the format based on the image size.
//
Envmap *load_envmap (const std::string &spec, const std::string &fmt = "");

// Return an appropriate subclass of Envmap, initialized from IMAGE.
// FMT is the type of environment-map (specifically, the type of mapping
// from direction to image coordinates).  If FMT is "" or "auto", an
// attempt will be made to guess the format based on the image size.
//
Envmap *make_envmap (const Ref<Image> &image, const std::string &fmt = "");

}

#endif /* __ENVMAP_H__ */

// arch-tag: 9695753e-771b-4555-83c4-593486374642
