#include "intersect.h"
#include "obj.h"

Color Intersect::render () const
{
  if (obj)
    return obj->material->render (*this);
  else
    return Color::black;
}

// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
