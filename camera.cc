// camera.h -- Camera datatype
//
//  Copyright (C) 2005, 2006, 2007, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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


// Camera::eye_ray

// Return an eye-ray from this camera for position (U, V) on the film
// plane, with the random perturbation (FOCUS_U, FOCUS_V) for
// depth-of-field simulation no depth-of-field.  All paramters have a
// range of 0-1.
//
Ray
Camera::eye_ray (float u, float v, float focus_u, float focus_v) const
{
  // The source of the camera ray, which is the camera position
  // (actually the optical center of the lens), possibly perturbed for
  // depth-of-field simulation
  //
  Pos src = pos;

  // A vector from SRC to the point on the virtual film plane (one
  // unit in front of the camera position, projected from the actual
  // film plane which lies behind the camera position) which is the
  // end of the camera ray.
  //
  Vec targ = eye_vec (u, v);

  if (aperture != 0)
    {
      // The radius of the camera aperture in scene units.
      //
      float aperture_radius = aperture / 2 / scene_unit;

      // The camera aperture is circular, so convert the independent
      // random variables FOCUS_U and FOCUS_V into a sample uniformly
      // distributed on a disk.
      //
      float coc_x, coc_y;	// "Circle of Confusion"
      sample_disk (focus_u, focus_v, coc_x, coc_y);

      // How much we will randomly perturb the camera position to simulate
      // depth-of-field.
      //
      float src_perturb_x = aperture_radius * coc_x;
      float src_perturb_y = aperture_radius * coc_y;

      // The end of the camera-ray pointed to by TARG should be perturbed
      // slightly less than SRC, by a factor of 1 / FOCUS_DISTANCE.
      // [Note that if FOCUS_DISTANCE is exactly 1, the end of the camera
      // ray won't be perturbed at all, meaning that everything at a
      // distance of 1 will be in focus, as expected.]
      //
      float targ_perturb_adj = -1 / focus_distance ();
      float targ_perturb_x = src_perturb_x * targ_perturb_adj;
      float targ_perturb_y = src_perturb_y * targ_perturb_adj;

      // Perturb the camera position.
      //
      src += right * src_perturb_x + up * src_perturb_y;

      // Add the compensation factor to TARG so that the end of
      // the camera-ray is perturbed slightly less than SRC.
      //
      targ += right * targ_perturb_x + up * targ_perturb_y;
    }

  return Ray (src, targ);
}


// arch-tag: f04add77-70cc-40db-b9c4-fb17ad1f66c9
