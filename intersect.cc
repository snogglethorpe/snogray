#include "intersect.h"
#include "obj.h"

Color Intersect::render (const Vec &eye_dir, const Vec &light_dir,
			 const Color &light_color)
  const
{
  if (obj)
    return obj->material->render (*this, eye_dir, light_dir, light_color);
  else
    return Color::black;
}

// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
