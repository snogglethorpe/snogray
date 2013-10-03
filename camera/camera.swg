# camera.swg -- SWIG interfaces for snogray cameras
#
#  Copyright (C) 2011, 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "camera/camera.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class Camera
  {
  public:

    class Format;

    static float default_scene_unit ();

    enum orient_t { ORIENT_VERT, ORIENT_HORIZ };

    static const Format FMT_35mm, FMT_6x6, FMT_6x7;
    static const Format FMT_APS_C, FMT_APS_H, FMT_APS_P;
    static const Format FMT_4x3, FMT_5x4, FMT_16x9;

    Camera (const Format &fmt = FMT_35mm,
	    float _scene_unit = default_scene_unit(),
	    float focal_length = 0 /* 0==auto */);

    void move (const Pos &_pos);
    void move (const Vec &offs);

    void point (const Pos &targ, const Vec &_user_up);
    void point (const Pos &targ);
    void point (const Vec &vec, const Vec &_user_up);
    void point (const Vec &vec);

    void rotate (const Xform &rot_xform);
    void orbit (const Xform &xform);
    void transform (const Xform &xform);

    void set_scene_unit (float camera_units);

    void set_focus (float distance);
    void set_focus (const Pos &point_on_focus_plane);
    void set_focus (const Vec &vec);

    float focus_distance () const;

    float focal_length () const;
    void set_focal_length (float focal_len);

    float focal_length (const Format &foc_len_fmt) const;
    void set_focal_length (float focal_len, const Format &foc_len_fmt);

    void zoom (float magnification);

    void set_horiz_fov (float fov);
    void set_vert_fov (float fov);
    void set_diag_fov (float fov);

    float aspect_ratio () const;
    void set_aspect_ratio (float aspect_ratio);

    void set_format (const Format &fmt);
    void set_orientation (orient_t orient);

    float f_stop () const;
    void set_f_stop (float f_stop);

    Ray eye_ray (const UV &film_loc, dist_t len) const;
    Ray eye_ray (const UV &film_loc, const UV &focus_param, dist_t len) const;

    // Fields
    //
    Pos pos;
    Vec forward, up, right;
    bool handedness_reversed;
    dist_t target_dist;
    float aperture;
    float focus;
    float scene_unit;
  };


} // namespace snogray
