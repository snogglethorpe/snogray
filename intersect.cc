#include "intersect.h"
#include "obj.h"

Color Intersect::render (const Color &light_color, const Vec &light_dir) const
{
  if (obj)
    return obj->material->render (*this, light_color, light_dir);
  else
    return Color::black;
}

// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
