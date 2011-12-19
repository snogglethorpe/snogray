# scene.swg -- SWIG interfaces for snogray scenes
#
#  Copyright (C) 2011  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "coords.h"
#include "scene.h"
#include "bbox-io.h"
#include "color-io.h"
#include "pos-io.h"
#include "vec-io.h"
#include "xform-io.h"
#include "xform-tex.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  // fwd decls
  class Color;
  class Surface;
  class Light;
  class SpaceBuilderFactory;


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

  class BBox
  {
  public:

    BBox ();
    BBox (const Pos &_min, const Pos &_max);
    BBox (const Pos &_pos);
    BBox (const Pos &_min, dist_t size);

    %rename(add) operator+= (const Pos &pos);
    BBox &operator+= (const Pos &pos);
    %rename(add) operator+= (const BBox &bbox);
    BBox &operator+= (const BBox &bbox);

    Vec extent () const;

    dist_t max_size () const;
    dist_t min_size () const;
    dist_t avg_size () const;
    dist_t median_size () const;    

    Pos min, max;
  };
  %extend BBox
  {
    BBox operator+ (const BBox &bbox) { return *$self + bbox; }
    BBox operator+ (const Pos &pos) { return *$self + pos; }

    const char* __str__() { return static_rep (*$self); }
  }

  class Frame
  {
  public:

    Frame (const Pos &org, const Vec &_x, const Vec &_y, const Vec &_z);
    Frame ();
    Frame (const Pos &org);
    Frame (const Vec &_x, const Vec &_y, const Vec &_z);
    Frame (const Vec &_z);
    Frame (const Pos &org, const Vec &_z);
    Frame (const Matrix4<dist_t> &M);

    Vec to (const Vec &vec) const;
    Vec to (const Pos &pos) const;
    Vec from (const Vec &vec) const;
    Pos from (const Pos &pos) const;

    Frame transformed (const Matrix4<dist_t> &xform) const;
    void transform (const Matrix4<dist_t> &xform);

    Pos origin;
    Vec x, y, z;
  };

  template<typename T>
  class TUV
  {
  public:
    TUV (T _u, T _v);
    T u, v;
  };
  %template(UV) TUV<float>;
  typedef TUV<float> UV;

  // Fwd declare texture templates for SWIG
  template<typename T>
  class Tex;
  template<typename T>
  class TexVal;

  // Fwd declare TRay template for swig
  template<typename T>
  class TRay { TRay (); }; // make it clear there's no default ctor
  %template(Ray) TRay<dist_t>;
  typedef TRay<dist_t> Ray;

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

    // Try to make transforms appliable to textures too.. ugh... soooo
    // many SWIG bugs....
    //
    snogray::Ref<snogray::Tex<float> > operator () (const snogray::Ref<Tex<float> > &obj) const
    {
      return new snogray::XformTex<float> (*$self, obj);
    }
    snogray::Ref<snogray::Tex<Color> > operator () (const snogray::Ref<Tex<snogray::Color> > &obj) const
    {
      return new snogray::XformTex<snogray::Color> (*$self, obj);
    }
  }
  %template(Xform) TXform<dist_t>;
  typedef TXform<dist_t> Xform;


  class Scene
  {
  public:

    void add (Surface *surface);
    void add (Light *light);

    void setup (const SpaceBuilderFactory &space_builder_factory);
  };
  %extend Scene
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"scene<nobjs=%d, nlights=%d>",
		$self->num_surfaces(), $self->num_lights());
      return static_rep_buf;
    }
  };


} // namespace snogray


// The following are object-creation wrappers, which we use in place of
// swig-generated constructors for any classes that we want to manage via
// reference-counting.  This is because swig will do its own memory
// management for object which it constructs, and we want to avoid that.
//
// These wrapper functions simply create the object using new and return a
// reference to it; swig happily uses the reference instead, with almost
// exactly the same functionality as the underlying object pointer.
//
%inline %{
  namespace snogray {


    // This is basically an Xform constructor that takes an array
    // argument.  Using a separate function shouldn't actually be
    // necessary -- we should be able to just use overload the TXform
    // constructor -- but SWIG has bugs in handling overloads in
    // conjunction with table-to-array conversion (it correctly
    // generates code to do the conversion, but the type-checking code
    // for distinguishing the various overload cases checks the wrong
    // type).  Using a separate named function for this case works
    // around the bug.
    //
    static Xform raw_xform (const float els[16])
    {
      return Xform (els);
    }


  }
%}
