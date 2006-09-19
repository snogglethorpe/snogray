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


const Camera::Format Camera::FMT_35mm (36, 24); // mm
const Camera::Format Camera::FMT_6x6 (56, 56); // mm
const Camera::Format Camera::FMT_6x7 (70, 56); // mm
const Camera::Format Camera::FMT_APS_C (25.1, 16.7); // mm
const Camera::Format Camera::FMT_APS_H (30.2, 16.7); // mm
const Camera::Format Camera::FMT_APS_P (30.2, 9.5); // mm


Camera::Camera (const Format &fmt, float focal_len)
  : format (fmt),
    user_up (Vec (0, 1, 0)),
    forward (Vec (0, 0, 1)), up (Vec (0, 1, 0)), right (Vec (1, 0, 0)),
    target_dist (1), z_mode (Z_INCREASES_FORWARD)
{
  // By default, set the focal length proportional to a 50mm lens for 35mm film
  //
  if (focal_len == 0)
    {
      float aspect_ratio = fmt.film_width / fmt.film_height;
      float aspect_ratio_35mm = FMT_35mm.film_width / FMT_35mm.film_height;

      if (aspect_ratio > aspect_ratio_35mm)
	focal_len = 50 * (fmt.film_width / FMT_35mm.film_width);
      else
	focal_len = 50 * (fmt.film_height / FMT_35mm.film_height);
    }

  set_focal_length (focal_len);
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
