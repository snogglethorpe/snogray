// camera.h -- Camera datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "camera.h"

using namespace snogray;


// Real camera/film formats
//
const Camera::Format Camera::FMT_35mm (36, 24); // mm
const Camera::Format Camera::FMT_6x6 (56, 56); // mm
const Camera::Format Camera::FMT_6x7 (70, 56); // mm
const Camera::Format Camera::FMT_APS_C (25.1, 16.7); // mm
const Camera::Format Camera::FMT_APS_H (30.2, 16.7); // mm
const Camera::Format Camera::FMT_APS_P (30.2, 9.5); // mm

// Ersatz formats for other common aspect ratios; these are sized so that
// 35mm lens focal lengths more or less work with them (the diagonal size
// is the same as a 35mm frame).
//
const Camera::Format Camera::FMT_4x3 (34.613, 25.960); // mm
const Camera::Format Camera::FMT_5x4 (33.786, 27.028); // mm
const Camera::Format Camera::FMT_16x9 (37.710, 21.212); // mm


Camera::Camera (const Format &fmt, float _scene_unit, float focal_len)
  : format (fmt),
    user_up (Vec (0, 1, 0)),
    forward (Vec (0, 0, 1)), up (Vec (0, 1, 0)), right (Vec (1, 0, 0)),
    handedness_reversed (false),
    target_dist (1), aperture (0), focus (0), scene_unit (_scene_unit)
{
  // By default, set the focal length proportional to a 50mm lens for 35mm film
  //
  if (focal_len == 0)
    set_focal_length (50, FMT_35mm);
  else
    set_focal_length (focal_len);
}


// Change the current camera direction according to the rotational
// transform ROT_XFORM (ROT_XFORM is assume to be a pure rotational
// transform -- no scaling, no translation).
//
void
Camera::rotate (const Xform &rot_xform)
{
  if (rot_xform.reverses_handedness ())
    handedness_reversed = !handedness_reversed;

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
  pos *= xform;
  rotate (xform);
}


// arch-tag: f04add77-70cc-40db-b9c4-fb17ad1f66c9
