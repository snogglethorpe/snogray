#include "snogray.h"

const Color
SnogRay::render (float u, float v) const
{
  Intersect isec = scene.closest_intersect (camera.get_ray (u, v));
  if (isec.obj)
    return scene.render (isec);
  else
    return Color::black;
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
