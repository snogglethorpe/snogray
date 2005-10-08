#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <cmath>

#include "space.h"
#include "ray.h"

class Camera {
public:
  static const Pos DEFAULT_POS;
  static const float DEFAULT_ASPECT_RATIO = 4.0 / 3.0;
  static const float DEFAULT_HORIZ_FOV = M_PI / 4;

  Camera (const Pos &_pos = DEFAULT_POS,
	  float aspect = DEFAULT_ASPECT_RATIO,
	  float horiz_fov = DEFAULT_HORIZ_FOV)
    : pos (_pos),
      forward (Vec (0, 0, 1)), right (Vec (1, 0, 0)), up (Vec (0, 1, 0)),
      fov_x (horiz_fov)
  {
    set_aspect_ratio (aspect);
  }

  void move (const Pos &_pos) { pos = _pos; }
  void move (const Vec &vec) { pos += vec; }

  void point (const Vec &vec, const Vec &user_up)
  {
    forward = vec.unit ();
    right = forward.cross (user_up.unit ()).unit ();
    up = right.cross (forward).unit ();
  }
  void point (const Pos &targ, const Vec &user_up)
  {
    point (targ - pos, user_up);
  }

  void set_aspect_ratio (float ratio)
  {
    aspect_ratio = ratio;
    fov_y = fov_x / ratio;
    tan_half_fov_x = tan (fov_x / 2);
    tan_half_fov_y = tan (fov_y / 2);
  }

  void set_horiz_fov (float fov)
  {
    fov_x = fov;
    set_aspect_ratio (aspect_ratio);
  }

  Ray get_ray (float u, float v) const
  {
    Pos targ = pos;

    targ += forward;
    targ += 2 * (u - 0.5) * right * tan_half_fov_x;
    targ += 2 * (v - 0.5) * up * tan_half_fov_y;

    return Ray (pos, targ);
  }

  Pos pos;

  Vec forward, up, right;

  float aspect_ratio; /* horiz / vert */

  float fov_x, fov_y;
  float tan_half_fov_x, tan_half_fov_y;
};

#endif /* __CAMERA_H__ */

// arch-tag: 45c04676-91b9-4fdb-ae65-2fd3ea0f0228
