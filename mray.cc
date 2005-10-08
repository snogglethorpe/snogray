#include "mray.h"

const Color
Mray::render (float u, float v) const
{
  return scene.closest_intersect (camera.get_ray (u, v)).render ();
}

// arch-tag: 9fd51a14-b97a-4976-918e-32b9dfa74cbd
