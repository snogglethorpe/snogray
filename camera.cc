#include "camera.h"

Camera::camera ()
{
  aspect_ratio = 4.0 / 3.0;
  set_horiz_fov (45 / M_PI);
}

// arch-tag: f04add77-70cc-40db-b9c4-fb17ad1f66c9
