#include "material.h"
using namespace Snogray;
Material::~Material () { } // stop gcc bitching

Color
Material::light (const Intersect &isec, const Color &color,
		 Scene &scene, unsigned depth)
{
  // Iterate over every light, calculating its contribution to our
  // color.

  Color total_color;	// Accumulated colors from all light sources

  for (list<Light *>::const_iterator li = scene.lights.begin();
       li != scene.lights.end(); li++)
    {
      Light *light = *li;
      Ray light_ray = Ray (isec.point, light->pos);

      // If the dot-product of the light-ray with the surface normal
      // is negative, that means the light is behind the surface, so
      // cannot light it ("self-shadowing"); otherwise, see if some
      // other object casts a shadow.

      if (isec.normal.dot (light_ray.dir) >= 0
	  && !shadowed (*light, light_ray, isec.obj))
	{
	  // This point is not shadowed, either by ourselves or by any
	  // other object, so calculate the lighting contribution from
	  // LIGHT.

	  Color light_color = light->color / (light_ray.len * light_ray.len);

	  total_color += light_model->render (isec, color,
					      light_ray.dir, light_color);
	}
    }

  return total_color;
}

Color
Material::render (const Intersect &isec, Scene &scene, unsigned depth)
{
  // If the dot product of the surface normal with the eye ray is
  // negative, we're looking at the back of the surface; we render
  // this as a striking color to make it easier to detect mistakes.
  // However to accomodate small cumulative errors, we allow very
  // small negative dot-products as if they were zero.

  if (isec.normal.dot (isec.eye_dir) >= -0.0001)
    return light (isec, color, scene, depth);
  else
    // We're looking at the back of the surface, render as wacky color.
    //
    return Color::funny;
}

// arch-tag: 3d971faa-322c-4479-acf0-effb05aca10a
