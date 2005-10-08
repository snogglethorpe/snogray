// phog.cc -- Phong lighting model
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

#include "phong.h"

#include "intersect.h"

using namespace Snogray;

Color
Phong::illum (const Intersect &isec, const Color &color,
	      const Vec &light_dir, const Color &light_color)
  const
{
  float diffuse_component = isec.normal.dot (light_dir);

  if (diffuse_component >= 0)
    {
      float specular_component
	= powf (isec.normal.dot ((isec.eye_dir + light_dir).unit()), exponent);

      Color total_color = color * diffuse_component;

      total_color += specular_color * specular_component;

      return total_color.lit_by (light_color);
    }
  else
    return Color::black;
}

// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
