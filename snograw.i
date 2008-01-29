# snograw.i -- Swig interface specification for snogray
#
#  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

# This is the "raw" interface to snogray
#
%module snograw

%include "config.h"

%{
#include <sstream>
#include <cstring>

#include "coords.h"
#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "tripar.h"
#include "ellipse.h"
#include "sphere.h"
#include "sphere2.h"
#include "cylinder.h"
#include "instance.h"
#include "subspace.h"
#include "surface-group.h"
#include "glow.h"
#include "norm-glow.h"
#include "glass.h"
#include "mirror.h"
#include "point-light.h"
#include "rect-light.h"
#include "sphere-light.h"
#include "cook-torrance.h"
#include "lambert.h"

  static char static_rep_buf[255];

  template<typename T>
  const char *
  static_rep(const T &v)
  {
    std::ostringstream s;
    s << v;
    strcpy (static_rep_buf, s.str().c_str());
    return static_rep_buf;
  }

  namespace snogray
  {

    Mesh::VertexGroup *mesh_vertex_group ()
    {
      return new Mesh::VertexGroup;
    }
    Mesh::VertexNormalGroup *mesh_vertex_normal_group ()
    {
      return new Mesh::VertexNormalGroup;
    }

  }

%}

//
// Note: for function arguments with a type of "const std::string &", we
// use "const char *" in the declarations below (and rely on the
// automatic C++ conversion to translate), because swig is more
// functional with char pointers (e.g. it automatically converts to/from
// the client language's string type).
//

%include "coords.h"

namespace snogray {

  template<typename T>
  class Matrix4
  {
  public:

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

    TVec operator* (const Matrix4<T> &xform) const;

    T length_squared () const;
    T length () const;

    TVec unit () const;

    TVec perpendicular () const;

    T latitude () const;
    T colatitude () const;
    T longitude () const;
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

    TPos operator* (const Matrix4<T> &xform) const;

    %rename(transform) operator*= (const Matrix4<T> &xform);
    const TPos &operator*= (const Matrix4<T> &xform);
  };
  %extend TPos<coord_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Pos) TPos<coord_t>;
  typedef TPos<coord_t> Pos;

  Pos midpoint (const Pos &p1, const Pos &p2);

  template<typename T>
  class TXform : public Matrix4<T>
  {
  public:

    static const TXform identity;

    TXform (T scale = 1);
    TXform (const TXform &src);

    static TXform translation (const TVec<T> &offs);
    static TXform translation (dist_t x, dist_t y, dist_t z);
    static TXform scaling (T scale);
    static TXform scaling (T s_x, T s_y, T s_z);

    static TXform x_rotation (T angle);
    static TXform y_rotation (T angle);
    static TXform z_rotation (T angle);
    static TXform rotation (TVec<T> axis, T angle);

    static TXform basis (const TVec<T> &x_axis, const TVec<T> &y_axis,
			 const TVec<T> &z_axis);

    TXform &translate (dist_t x, dist_t y, dist_t z);
    TXform &translate (const TVec<T> &offs);

    TXform &scale (T scale);
    TXform &scale (T s_x, T s_y, T s_z);

    TXform &rotate_x (T angle);
    TXform &rotate_y (T angle);
    TXform &rotate_z (T angle);
    TXform &rotate (TVec<T> axis, T angle);

    TXform &to_basis (const TVec<T> &x_axis, const TVec<T> &y_axis,
		      const TVec<T> &z_axis);

    bool reverses_handedness () const { return Matrix4<T>::det() < 0; }

    TXform &invert ();
    TXform inverse () const;
    TXform transpose () const;

    TXform operator* (const TXform &xform) const;
    TXform operator* (T scale) const;

    %rename(transform) operator*= (const TXform &mat);
    const TXform &operator*= (const TXform &mat);
  };
  %extend TXform<dist_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Xform) TXform<dist_t>;
  typedef TXform<dist_t> Xform;

  class Color
  {
  public:

    typedef float component_t;

    Color ();
    Color (component_t r, component_t g, component_t b);
    Color (double grey);

    Color operator- () const;
    Color operator/ (component_t denom) const;

    float intensity ();

    Color clamp (float max_intens) const;
    Color clamp (float min_intens, float max_intens) const;

    %rename(__pow) pow (float exp) const;
    Color pow (float exp) const;

    component_t min_component () const;
    component_t max_component () const;

    component_t r () const, g () const, b () const;

    void set_rgb (component_t r, component_t g, component_t b);
  };
  %extend Color
  {
    Color operator+ (const Color &col) { return *$self + col; }
    Color operator- (const Color &col) { return *$self - col; }
    Color operator* (const Color &col) { return *$self * col; }
    Color operator/ (const Color &col) { return *$self / col; }

    Color operator+ (component_t v) { return *$self + v; }
    Color operator- (component_t v) { return *$self - snogray::Color (v); }
    Color operator* (component_t v) { return *$self * v; }

    Color operator== (const Color &col) { return *$self == col; }
    Color operator< (const Color &col) { return *$self < col; }
    Color operator<= (const Color &col) { return *$self <= col; }
    
    const char* __str__() { return static_rep (*$self); }
  }

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

  class Material
  {
  public:
    enum ShadowType { SHADOW_OPAQUE, SHADOW_NONE, SHADOW_MEDIUM };

    Material (ShadowType _shadow_type = SHADOW_OPAQUE);
  };

  class Ior
  {
  public:

    Ior (float _n, float _k = 0);

    bool operator== (const Ior &ior2);

    float n, k;
  };
  %extend Ior
  {
    const char* __str__()
    {
      std::ostringstream s;
      s << "ior<" << $self->n << ", " << $self->k << ">";
      strcpy (static_rep_buf, s.str().c_str());
      return static_rep_buf;
    }
  }

  class CookTorrance : public Material
  {
  public:

    CookTorrance (const Color &col, const Color &spec_col,
		  float m, const Ior &ior);
    CookTorrance (const Color &col, const Color &spec_col,
		  float m, float ior);
  };

  class Lambert : public Material
  {
  public:

    Lambert (const Color &col) : color (col) { }
  };

  class Medium
  {
  public:

    Medium (float _ior = 1, const Color &_absorb = 0);
  };

  class Glass : public Material
  {
  public:

    Glass (Medium _medium);
  };

  class Mirror : public Material
  {
  public:

    Mirror (const Ior &_ior, const Color &_reflectance,
	    const Material *underlying_material);
    Mirror (const Ior &_ior, const Color &_reflectance, const Color &col = 0);
    Mirror (float _ior, const Color &_reflectance,
	    const Material *underlying_material);
    Mirror (float _ior, const Color &_reflectance, const Color &col = 0);
  };

  class Glow : public Material
  {
  public:

    Glow (const Color &_color);
  };

  class NormGlow : public Material
  {
  public:

    NormGlow (Color::component_t intens);
  };

  class Surface
  {
  private:

    Surface (); // inhibit swig constructor generation

  public:

    virtual BBox bbox () const;
  };

  class Sphere : public Surface
  {
  public:

    Sphere (const Material *mat, const Pos &_center, dist_t _radius);
  };

  class Cylinder : public Surface
  {
  public:

    Cylinder (const Material *mat, const Xform &obj_to_world_xform);
    Cylinder (const Material *mat, const Pos &base, const Vec &axis,
	      float radius);
  };

  class Sphere2 : public Surface
  {
  public:

    Sphere2 (const Material *mat, const Xform &_xform);
  };

  class Tripar : public Surface
  {
  public:

    Tripar (const Material *mat, const Pos &_v0, const Vec &_e1, const Vec &_e2,
	    bool _parallelogram = false);
  };

  class Ellipse : public Surface
  {
  public:

    Ellipse (const Material *mat,
	     const Pos &center, const Vec &radius1, const Vec &radius2);
  };

  %ignore LocalSurface;
  class LocalSurface : public Surface
  {
  };

  class Subspace : public Surface
  {
  public:

    Subspace (Surface *surf);
  };

  class Instance : public LocalSurface
  {
  public:

    Instance (Subspace *subspace, const Xform &local_to_world_xform);
  };

  class SurfaceGroup : public Surface
  {
  public:

    SurfaceGroup ();

    void add (Surface *surface);
  };

  class MaterialDict
  {
  public:

    MaterialDict (const Material *default_material = 0);

    const Material *get (const char *name, const Material *def_mat) const;
    const Material *get (const char *name) const;
    void add (const char *name, const Material *mat);
    bool contains (const char *name) const;

    const Material *get_default () const;
    void set_default (const Material *mat);
  };
  %extend MaterialDict
  {
    const Material *__getitem__ (const char *name) const
    {
      return $self->get (name);
    }
    void __setitem__ (const char *name, const Material *mat)
    {
      $self->add (name, mat);
    }

    // Since a "nil" table value in lua conventionally means "no entry",
    // support true/false for use with "negative" entries (an entry
    // mapping to a zero pointer in the native MaterialDict type).
    //
    void __setitem__ (const char *name, bool flag)
    {
      $self->add (name, 0);
    }

    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"material-dict<nentries=%d%s>",
		$self->num_entries(),
		$self->get_default() ? "+1" : "");
      return static_rep_buf;
    }
  }

  class Mesh : public Surface
  {
  public:

    Mesh (const snogray::Material *mat = 0);
    Mesh (const snogray::Material *mat, const char *file_name,
	  bool smooth = true);
    Mesh (const char *file_name, const MaterialDict &mat_dict);

    typedef unsigned vert_index_t;

    typedef std::map<Pos, vert_index_t> VertexGroup;
    typedef std::map<std::pair<Pos, Vec>, vert_index_t> VertexNormalGroup;

    void add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i,
		       const Material *mat = 0);
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		       const Material *mat = 0);
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		       VertexGroup &vgroup, const Material *mat = 0);

    vert_index_t add_vertex (const Pos &pos);
    vert_index_t add_vertex (const Pos &pos, VertexGroup &vgroup);

    vert_index_t add_vertex (const Pos &pos, const Vec &normal);
    vert_index_t add_vertex (const Pos &pos, const Vec &normal,
			     VertexNormalGroup &vgroup);

    vert_index_t add_normal (vert_index_t vert_index, const Vec &normal);

    void reserve (unsigned num_verts, unsigned num_tris,
		  bool with_normals = false);
    void reserve_normals ();

    void load (const char *file_name,
	       const MaterialDict &mat_dict = MaterialDict ());

    void compute_vertex_normals (float max_angle = 45 * PIf / 180);

    void transform (Xform &xform);

    bool left_handed;
  };
  %extend Mesh
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"mesh<nverts=%d, ntris=%d>",
		$self->num_vertices(), $self->num_triangles());
      return static_rep_buf;
    }

    // Variants of real methods that take the individual components of
    // position/vertex arguments (as creating a Pos/Vec object is much 
    // more expensive in scripting languages than in C++).
    //
    vert_index_t add_vertex (float x, float y, float z)
    {
      return $self->add_vertex (snogray::Pos (x, y, z));
    }
    vert_index_t add_vertex (float x, float y, float z,
			     VertexGroup &vg)
    {
      return $self->add_vertex (snogray::Pos (x, y, z), vg);
    }
    vert_index_t add_vertex (float x, float y, float z,
			     float nx, float ny, float nz)
    {
      return $self->add_vertex (snogray::Pos (x, y, z),
				snogray::Vec (nx, ny, nz));
    }
    vert_index_t add_vertex (float x, float y, float z,
			     float nx, float ny, float nz,
			     VertexNormalGroup &vng)
    {
      return $self->add_vertex (snogray::Pos (x, y, z),
				snogray::Vec (nx, ny, nz),
				vng);
    }

    vert_index_t add_normal (vert_index_t vert_index,
			     float nx, float ny, float nz)
    {
      return $self->add_normal (vert_index, snogray::Vec (nx, ny, nz));
    }
  }

  Mesh::VertexGroup *mesh_vertex_group ();
  Mesh::VertexNormalGroup *mesh_vertex_normal_group ();


  class Camera
  {
  public:

    class Format;

    enum orient_t { ORIENT_VERT, ORIENT_HORIZ };

    static const Format FMT_35mm, FMT_6x6, FMT_6x7;
    static const Format FMT_APS_C, FMT_APS_H, FMT_APS_P;
    static const Format FMT_4x3, FMT_5x4, FMT_16x9;

    static const float DEFAULT_SCENE_UNIT = 25.4;

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
    void set_diagonal_fov (float fov);

    float aspect_ratio () const;
    void set_aspect_ratio (float aspect_ratio);

    void set_format (const Format &fmt);
    void set_orientation (orient_t orient);

    float f_stop () const;
    void set_f_stop (float f_stop);

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

  %ignore Light;
  class Light
  {
  };

  class PointLight : public Light
  {
  public:

    PointLight (const Pos &_pos, const Color &col);
  };

  class RectLight : public Light
  {
  public:

    RectLight (const Pos &_pos, const Vec &_side1, const Vec &_side2,
	       const Color &_intensity);
  };

  class SphereLight : public Light
  {
  public:

    SphereLight (const Pos &_pos, float _radius, const Color &_intensity);
  };

  class Scene
  {
  public:

    Surface *add (Surface *surface);
    Light *add (Light *light);
    Material *add (Material *mat);

    void set_background (const Color &col);

    void load (const char *file_name, const char *fmt, Camera &camera);
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

}


# arch-tag: ced00636-fe46-45bd-b738-1949800e26f1
