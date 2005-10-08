#include "lambert.h"

Color Lambert::render (const class Intersect &isec,
		       const Color &light_color, const Vec &light_dir)
  const
{
  if (light_dir.dot (isec.normal) >= 0)
    return color.lit_by (light_color) * max (0, light_dir.dot (isec.normal));
  else
    return Color::black;
}

// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
