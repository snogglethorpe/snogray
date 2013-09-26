// camera.h -- Camera datatype
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_CAMERA_H
#define SNOGRAY_CAMERA_H

#include "util/snogmath.h"
#include "geometry/coords.h"
#include "intersect/ray.h"
#include "geometry/xform.h"
#include "geometry/disk-sample.h"


namespace snogray {


class Camera {
public:

  class Format {
  public:

    enum Axis { HORIZ, VERT, DIAG };

    Format (float _width, float _height) : width (_width), height (_height) { }

    dist_t dimension (Axis axis) const
    {
      switch (axis)
	{
	case HORIZ: return width;
	case VERT:  return height;
	default:    return sqrt (height * height + width * width);
	}
    }

    // Return the field-of-view on the given axis, of a lens with the given
    // focal length.
    //
    float fov (Axis axis, float focal_length) const
    {
      return atan2 (dimension (axis) / 2, focal_length) * 2;
    }

    // Size of film
    //
    dist_t width, height;
  };

  enum orient_t { ORIENT_VERT, ORIENT_HORIZ };

  // Various pre-defined camera formats
  //
  static const Format FMT_35mm, FMT_6x6, FMT_6x7;
  static const Format FMT_APS_C, FMT_APS_H, FMT_APS_P; // who cares, but ...
  static const Format FMT_4x3, FMT_5x4, FMT_16x9; // ersatz formats for video

  // One scene unit in camera units.
  //
  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  //
  static float default_scene_unit () { return 25.4; }

  Camera (const Format &fmt = FMT_35mm,
	  float _scene_unit = default_scene_unit(),
	  float focal_length = 0 /* 0==auto */);


  // Move the camera to absolution position POS
  //
  void move (const Pos &_pos) { pos = _pos; }
  void move (const Vec &offs) { pos += offs; }

  // Point at TARG, using the old "up" direction if not specified.
  //
  void point (const Pos &targ, const Vec &_user_up)
  {
    point (targ - pos, _user_up);
  }
  void point (const Pos &targ)
  {
    point (targ, user_up);
  }

  // Point at the target with offset VEC from the camera location, using
  // the old "up" direction if not specified.
  //
  void point (const Vec &vec, const Vec &_user_up)
  {
    target_dist = vec.length ();
    _point (vec.unit(), _user_up);
  }
  void point (const Vec &vec)
  {
    point (vec, user_up);
  }

  // Point the camera in direction DIR.  DIR is assumed to be a unit
  // vector; the target distance is left unchanged.
  //
  void _point (const Vec &dir, const Vec &_user_up)
  {
    if (cross (_user_up, dir).length_squared() < Eps)
      user_up = dir.perpendicular (); // avoid degenerate case
    else
      user_up = _user_up;

    forward = dir.unit ();

    right = cross (_user_up, forward).unit ();
    up = cross (forward, right).unit ();

    if (handedness_reversed)
      right = -right;		// right is really left
  }

  // Change the current camera direction according to the rotational
  // transform ROT_XFORM (ROT_XFORM is assume to be a pure rotational
  // transform -- no scaling, no translation).
  //
  void rotate (const Xform &rot_xform);

  // Apply XFORM with the target at the origin, then move target back to
  // original location.
  //
  void orbit (const Xform &xform);

  // This moves the camera such that if the rest of the scene is
  // transformed with the same matrix XFORM, the apparent view will not
  // change.
  //
  void transform (const Xform &xform);


  // Set the mapping of "scene units" to camera units (nominally mm).
  // This is only used for depth-of-field simulation.  The default value is
  // 25.4mm, or 1 inch
  //
  void set_scene_unit (float camera_units)
  {
    scene_unit = camera_units;
  }

  // Set the distance to the focus plane, in scene units.  This defaults to
  // the "target distance" (the distance between the camera position and
  // the position passed to the `point' method).
  //
  void set_focus (float distance)
  {
    focus = distance;
  }

  // Similarly, but to a point in the scene, or along a scene vector.
  //
  void set_focus (const Pos &point_on_focus_plane)
  {
    set_focus (point_on_focus_plane - pos);
  }
  void set_focus (const Vec &vec)
  {
    // The distance to the focus plane is parallel to the camera forward
    // vector.
    //
    Vec components = vec.to_basis (right, up, forward);
    set_focus (components.z);
  }

  // Return the distance to the focus plane, in scene units.
  //
  dist_t focus_distance () const
  {
    return (focus == 0) ? target_dist : focus;
  }


  // Return / set the focal length in camera units (nominally mm).
  //
  dist_t focal_length () const
  {
    return format.width / 2 / tan_half_fov_x;
  }
  void set_focal_length (dist_t focal_len)
  {
    tan_half_fov_x = format.width / 2 / focal_len;
    tan_half_fov_y = format.height / 2 / focal_len;
  }

  // Return the "equivalent" focal length in format FOC_LEN_FMT:  a
  // focal length that has the same field-of-view, on the axis
  // FOV_AXIS, in FOC_LEN_FMT as the camera's focal length does in its
  // current format.
  //
  dist_t focal_length (const Format &foc_len_fmt) const
  {
    float fov = format.fov (fov_axis, focal_length ());
    dist_t tan_half_fov = tan (fov / 2);
    return foc_len_fmt.dimension (fov_axis) / 2 / tan_half_fov;
  }
  // Set the actual focal length to something that has the same
  // field-of-view, on the axis FOV_AXIS, that FOCAL_LEN does in
  // FOC_LEN_FMT.
  //
  void set_focal_length (dist_t focal_len, const Format &foc_len_fmt)
  {
    set_fov (fov_axis, foc_len_fmt.fov (fov_axis, focal_len));
  }


  void zoom (float magnification)
  {
    tan_half_fov_x /= dist_t (magnification);
    tan_half_fov_y /= dist_t (magnification);
  }


  // Set the camera's field-of-view along the given axis.
  //
  void set_fov (Format::Axis axis, float fov)
  {
    float tan_half_fov = tan (fov / 2);
    float axis_dimen = format.dimension (axis);

    tan_half_fov_x = tan_half_fov * (format.width / axis_dimen);
    tan_half_fov_y = tan_half_fov * (format.height / axis_dimen);

    // Remember which axis was used, so set_focal_length can preserve it.
    //
    fov_axis = axis;
  }

  // Shortcuts for setting the field-of-view on specific axes.
  //
  void set_horiz_fov (float fov) { set_fov (Format::HORIZ, fov); }
  void set_vert_fov (float fov) { set_fov (Format::VERT, fov); }
  void set_diag_fov (float fov) { set_fov (Format::DIAG, fov); }

  float aspect_ratio () const
  {
    return float (format.width / format.height);
  }
  void set_aspect_ratio (float aspect_ratio)
  {
    dist_t old_focal_len = focal_length ();
    Format old_format = format;

    dist_t old_diagonal = format.dimension (Format::DIAG);
    dist_t new_diag_angle = atan (aspect_ratio);

    format.width = old_diagonal * sin (new_diag_angle);
    format.height = old_diagonal * cos (new_diag_angle);

    set_focal_length (old_focal_len, old_format);
  }


  void set_format (const Format &fmt)
  {
    dist_t old_focal_len = focal_length ();
    Format old_format = format;

    format = fmt;

    set_focal_length (old_focal_len, old_format);
  }


  void set_orientation (orient_t orient)
  {
    orient_t cur_orient = aspect_ratio () >= 1 ? ORIENT_HORIZ : ORIENT_VERT;

    if (orient != cur_orient)
      // Flip the current format
      //
      set_format (Format (format.height, format.width));
  }

  // Get/set the camera aperture for depth-of-field simulation, in f-stops.
  //
  // A value of zero is special-cased to mean an "infinite" f-stop (i.e.,
  // an aperture of zero).
  //
  float f_stop () const
  {
    return float (aperture ? focal_length () / aperture : 0);
  }
  void set_f_stop (float f_stop)
  {
    if (f_stop == 0)
      aperture = 0;
    else
      aperture = focal_length () / dist_t (f_stop);
  }


  // Return an eye-ray, of length LEN, from this camera for location
  // FILM_LOC on the film plane, with no depth-of-field.  FILM_LOC has
  // a range of 0-1.
  //
  Ray eye_ray (const UV &film_loc, dist_t len) const
  {
    return Ray (pos, eye_vec (film_loc), len);
  }

  // Return an eye-ray, of length LEN, from this camera for location
  // FILM_LOC on the film plane, with the random perturbation
  // FOCUS_PARAM for depth-of-field simulation.  All parameters have a
  // range of 0-1.
  //
  Ray eye_ray (const UV &film_loc, const UV &focus_param, dist_t len) const;

  Format format;

  Pos pos;

  // User-supplied verticle axis.
  //
  Vec user_up;

  // Camera basis vectors.  UP and RIGHT are derived (from FORWARD and
  // USER_UP).
  //
  Vec forward, up, right;

  // True if camera transforms have reversed the orientation of the camera
  // coordinate system.
  // 
  bool handedness_reversed;

  // How far it is to the "target".
  //
  dist_t target_dist;

  // Lens aperture.  This only affects depth-of-field, not exposure like in
  // a real camera.  Zero means perfect focus.
  //
  dist_t aperture;

  // The distance to the focus plane, from POS.  If zero, TARGET_DIST is used.
  //
  dist_t focus;

  // The length of one "scene unit", in "camera units" (the same units we
  // use for focal-length, aperture etc, nominally mm).
  //
  dist_t scene_unit;

  // Whichever axis was last used to set the field-of-view.  When the
  // camera aspect ratio is changed, this will be the axis where the
  // field-of-view is preserved.
  //
  Format::Axis fov_axis;

  dist_t tan_half_fov_x, tan_half_fov_y;

private:

  // Returns a vector which points from the camera position to location
  // FILM_LOC on the virtual film plane (one unit in front of the
  // camera).
  //
  Vec eye_vec (const UV &film_loc) const
  {
    dist_t x = 2 * dist_t (film_loc.u) - 1;
    dist_t y = 2 * dist_t (film_loc.v) - 1;
    return forward + right * x * tan_half_fov_x + up * y * tan_half_fov_y;
  }

};


}

#endif /* SNOGRAY_CAMERA_H */


// arch-tag: 45c04676-91b9-4fdb-ae65-2fd3ea0f0228
