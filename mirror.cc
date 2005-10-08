// phong.cc -- Mirror lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "mirror.h"

#include "intersect.h"

using namespace Snogray;

const Color
Mirror::render (const Intersect &isec,
		const Vec &light_dir, const Color &light_color,
		Scene &scene, unsigned depth)
  const
{
  float diffuse_component = isec.normal.dot (light_dir);

  if (diffuse_component >= 0)
    {
      float specular_component
	= powf (isec.normal.dot ((isec.eye_dir + light_dir).unit()), exponent);

      Color color = diffuse_color * diffuse_component;

      color += specular_color * specular_component;

      return color.lit_by (light_color);
    }
  else
    return Color::black;
}

// arch-tag: 9f01db5e-c4b3-4d62-8fe3-e88243c025f9
