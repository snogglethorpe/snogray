// camera.h -- Camera datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "camera.h"

using namespace Snogray;

const Pos Camera::DEFAULT_POS = Pos (0, 0, 0);
const float Camera::DEFAULT_ASPECT_RATIO;
const float Camera::DEFAULT_HORIZ_FOV;


Camera::Camera (const Pos &_pos, float aspect, float horiz_fov)
  : pos (_pos), user_up (Vec (0, 1, 0)),
    forward (Vec (0, 0, 1)), up (Vec (0, 1, 0)), right (Vec (1, 0, 0)),
    target_dist (1), z_mode (Z_INCREASES_FORWARD), fov_x (horiz_fov)
{
  set_aspect_ratio (aspect);
}

// Change the current camera direction according to the rotational
// transform ROT_XFORM (ROT_XFORM is assume to be a pure rotational
// transform -- no scaling, no translation).
//
void
Camera::rotate (const Xform &rot_xform)
{
  _point (forward * rot_xform, up * rot_xform);
}

// Apply XFORM with the target at the origin, then move target back to
// original location.
//
void
Camera::orbit (const Xform &xform)
{
  const Xform rot = xform.inverse ().transpose ();

  // Vector from origin to target
  //
  const Vec target (pos + forward * target_dist);

  Xform total (target);
  total *= xform;
  total.translate (-target);

  pos *= total;

  rotate (rot);
}

// This moves the camera such that if the rest of the scene is
// transformed with the same matrix XFORM, the apparent view will not
// change.
//
void
Camera::transform (const Xform &xform)
{
  const Xform rot_xform = xform.inverse().transpose();
  pos *= xform;
  rotate (rot_xform);
}


// arch-tag: f04add77-70cc-40db-b9c4-fb17ad1f66c9
