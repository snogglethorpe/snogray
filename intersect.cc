// intersect.h -- Datatype for recording object-ray intersection results
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "obj.h"

using namespace Snogray;

Color
Intersect::render (const Vec &light_dir, const Color &light_color)
  const
{
  if (obj)
    {
      Vec eye_dir = -ray.dir;
      Vec normal = obj->normal (point, eye_dir);

      if (normal.dot (eye_dir) >= 0)
	return obj->material->render (obj, point, normal,
				      eye_dir, light_dir, light_color);
      else
	return Color::funny;
    }
  else
    return Color::black;
}

// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
