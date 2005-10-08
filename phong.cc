#include <iostream>

#include "phong.h"

#include "intersect.h"

const Color Phong::render (const class Intersect &isec, const Vec &eye_dir,
			   const Vec &light_dir, const Color &light_color)
  const
{
//   std::cout << "Phong::render (" << light_color << ", " << light_dir << "):"
// 	    << std::endl;

  const Vec &norm = isec.normal;

  if (norm.dot (eye_dir) >= 0)
    {
      float light_dot = norm.dot (light_dir);
      if (light_dot >= 0)
	{
	  float specular_component
	    = powf ((eye_dir + light_dir).unit ().dot (norm), exponent);
	  Color color = diffuse_color * light_dot;
	  color += specular_color * specular_component;
	  return color.lit_by (light_color);
	}
      else
	return Color::black;
    }
  else
    return Color::funny;
}

// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
