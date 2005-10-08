// material.cc -- Object material datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "material.h"

#include "scene.h"
#include "intersect.h"
#include "lambert.h"
#include "phong.h"

using namespace Snogray;

Material::~Material () { } // stop gcc bitching


// As a convenience, provide a global lookup service for common lighting models.

const Lambert *Material::lambert = new Lambert;

const Phong *
Material::phong (float exp, const Color &spec_col)
{
  static std::list<const Phong *> global_phongs;

  for (std::list<const Phong *>::const_iterator pi = global_phongs.begin ();
       pi != global_phongs.end (); pi++)
    {
      const Phong *phong = *pi;
      if (phong->exponent == exp && phong->specular_color == spec_col)
	return phong;
    }

  Phong *phong = new Phong (exp, spec_col);

  global_phongs.push_front (phong);

  return phong;
}



Color
Material::illum (const Intersect &isec, const Color &color,
		 Scene &scene, TraceState &tstate)
  const
{
  // Iterate over every light, calculating its contribution to our
  // color.

  Color total_color;	// Accumulated colors from all light sources

  for (Scene::light_iterator_t li = scene.lights.begin();
       li != scene.lights.end(); li++)
    {
      Light *light = *li;
      Ray light_ray (isec.point, light->pos);

      // If the dot-product of the light-ray with the surface normal
      // is negative, that means the light is behind the surface, so
      // cannot light it ("self-shadowing"); otherwise, see if some
      // other object casts a shadow.

      if (isec.normal.dot (light_ray.dir) >= 0
	  && !scene.shadowed (*light, light_ray, tstate, isec.obj))
	{
	  // This point is not shadowed, either by ourselves or by any
	  // other object, so calculate the lighting contribution from
	  // LIGHT.

	  Color light_color = light->color / (light_ray.len * light_ray.len);

	  total_color
	    += light_model->illum (isec, color, light_ray.dir, light_color);
	}
    }

  return total_color;
}

Color
Material::render (const Intersect &isec, Scene &scene, TraceState &tstate) const
{
  // If the dot product of the surface normal with the eye ray is
  // negative, we're looking at the back of the surface; we render
  // this as a striking color to make it easier to detect mistakes.
  // However to accomodate small cumulative errors, we allow very
  // small negative dot-products as if they were zero.

  if (isec.normal.dot (isec.eye_dir) >= -0.0001)
    return illum (isec, color, scene, tstate);
  else
    // We're looking at the back of the surface, render as wacky color.
    //
    return Color::funny;
}

// Return true if this material casts no shadow
//
bool
Material::no_shadow () const
{
  return false;
}

// arch-tag: 3d971faa-322c-4479-acf0-effb05aca10a
