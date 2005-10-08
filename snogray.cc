#include <iostream>

#include "snogray.h"

const Color
SnogRay::render (float u, float v)
{
  Ray camera_ray = camera.get_ray (u, v);

//   cout << "camera_ray (" << u << ", " << v << ") = " << camera_ray << endl;

  Intersect isec = scene.closest_intersect (camera_ray);
  if (isec.obj)
    return scene.render (isec);
  else
    return Color::black;
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
