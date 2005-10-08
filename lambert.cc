#include <iostream>

#include "lambert.h"

#include "intersect.h"

const Color Lambert::render (const class Intersect &isec, const Vec &eye_dir,
			     const Vec &light_dir, const Color &light_color)
  const
{
//   std::cout << "Lambert::render (" << light_color << ", " << light_dir << "):"
// 	    << std::endl;

  if (eye_dir.dot (isec.normal) >= 0)
    return color.lit_by (light_color) * (light_dir.dot (isec.normal) >? 0);
  else
    return Color::funny;
}

// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
