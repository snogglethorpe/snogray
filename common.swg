# common.swg -- Stuff that needs to be defined before most other SWIG interfaces
#
#  Copyright (C) 2012, 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "ray.h"
#include "ray-io.h"
#include "geometry/pos.h"
#include "geometry/pos-io.h"
#include "geometry/vec.h"
#include "geometry/vec-io.h"
#include "geometry/matrix4.h"
#include "geometry/xform.h"
#include "geometry/xform-io.h"
#include "texture/xform-tex.h" // ugh
%}


%exception {
  try {
    $action
  }
  catch (std::exception &e) {
    lua_pushstring (L, e.what ());
    lua_error (L);
  }
}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  template<typename T>
  class Matrix4
  {
  public:

    T det () const;
    bool is_identity () const;
  };
  %template(Matrix4d) Matrix4<dist_t>;


  template<typename T>
  class Tuple3
  {
  public:

    T x, y, z;
  };
  %template(Tuple3d) Tuple3<dist_t>;

  template<typename T>
  class TVec : public Tuple3<T>
  {
  public:

    TVec (T _x, T _y, T _z = 0);
    TVec () { }

    TVec (const Tuple3<T> &t);

    bool null () const;

    TVec operator+ (const TVec &v2) const;
    TVec operator- (const TVec &v2) const;
    TVec operator- () const;

    TVec operator* (T scale) const;
    TVec operator/ (T denom) const;

    TVec transformed (const Matrix4<T> &xform) const;
    void transform (const Matrix4<T> &xform);

    T length_squared () const;
    T length () const;

    TVec unit () const;

    TVec perpendicular () const;
  };
  %extend TVec<dist_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Vec) TVec<dist_t>;
  typedef TVec<dist_t> Vec;

  dist_t dot (const Vec &vec1, const Vec &vec2);
  Vec cross (const Vec &vec1, const Vec &vec2);

  template<typename T>
  class TPos : public Tuple3<T>
  {
  public:

    TPos (T _x, T _y, T _z = 0);
    TPos () { }

    TPos (const Tuple3<T> &t);

    TPos operator+ (const TVec<T> &v) const;
    TPos operator- (const TVec<T> &v) const;

    TVec<T> operator- (const TPos &p2) const;

    TPos operator* (T scale) const;
    TPos operator/ (T denom) const;

    TPos transformed (const Matrix4<T> &xform) const;
    void transform (const Matrix4<T> &xform);
  };
  %extend TPos<coord_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Pos) TPos<coord_t>;
  typedef TPos<coord_t> Pos;

  Pos midpoint (const Pos &p1, const Pos &p2);


  template<typename T>
  class TUV
  {
  public:
    TUV (T _u, T _v);
    T u, v;
  };
  %template(UV) TUV<float>;
  typedef TUV<float> UV;


  template<typename T>
  class TRay
  {
  public:

    TRay (TPos<T> _origin, TVec<T> _extent);
    TRay (TPos<T> _origin, TVec<T> _dir, T _t1);
    TRay (TPos<T> _origin, TVec<T> _dir, T _t0, T _t1);
    TRay (TPos<T> _origin, TPos<T> _targ);

    TPos<T> operator() (T t);
    TPos<T> begin () const;
    TPos<T> end () const;

    T length () const { return t1 - t0; }

    TPos<T> origin;
    TVec<T> dir;

    T t0, t1;
  };
  %extend TRay<dist_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Ray) TRay<dist_t>;
  typedef TRay<dist_t> Ray;


  // fwd decls of stuff TXform uses (but which are exported elsewhere)
  class BBox;
  class Frame;

  template<typename T>
  class TXform : public Matrix4<T>
  {
  public:

    static const TXform identity;

    TXform (T scale = 1);
    TXform (const TXform &src);

    // Doesn't actually work due to SWIG bugs; see the comment for the
    // "raw_xform" function later in this file. XXX
    //TXform (const T els[16]);

    static TXform translation (const TVec<T> &offs);
    static TXform translation (dist_t x, dist_t y, dist_t z = 0);
    static TXform scaling (T scale);
    static TXform scaling (T s_x, T s_y, T s_z = 1);

    static TXform x_rotation (T angle);
    static TXform y_rotation (T angle);
    static TXform z_rotation (T angle);
    static TXform rotation (TVec<T> axis, T angle);
    static TXform rotation (T angle);

    static TXform basis (const TVec<T> &x_axis, const TVec<T> &y_axis,
			 const TVec<T> &z_axis);

    TXform &translate (dist_t x, dist_t y, dist_t z = 0);
    TXform &translate (const TVec<T> &offs);

    TXform &scale (T scale);
    TXform &scale (T s_x, T s_y, T s_z = 1);

    TXform &rotate_x (T angle);
    TXform &rotate_y (T angle);
    TXform &rotate_z (T angle);
    TXform &rotate (TVec<T> axis, T angle);
    TXform &rotate (T angle);

    TXform &to_basis (const TVec<T> &x_axis, const TVec<T> &y_axis,
		      const TVec<T> &z_axis);

    bool reverses_handedness () const { return Matrix4<T>::det() < 0; }

    TXform &invert ();
    TXform inverse () const;
    TXform transpose () const;

    TXform transformed (const Matrix4<T> &xform) const;
    void transform (const Matrix4<T> &xform);

    TXform operator* (const TXform &xform) const;
    TXform operator* (T scale) const;

    TPos<T> operator() (const TPos<T> &obj) const;
    TVec<T> operator() (const TVec<T> &obj) const;
    TRay<T> operator() (const TRay<T> &obj) const;
    BBox operator() (const BBox &obj) const;
    Frame operator() (const Frame &obj) const;
    TXform operator () (const TXform &obj) const;

    %rename(mul_assn) operator*= (const TXform &mat);
    const TXform &operator*= (const TXform &mat);
  };
  %extend TXform<dist_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Xform) TXform<dist_t>;
  typedef TXform<dist_t> Xform;


  class Color;


} // namespace snogray
