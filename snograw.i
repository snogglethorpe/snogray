# snograw.i -- Swig interface specification for snogray
#
#  Copyright (C) 2007  Miles Bader <miles@gnu.org>
#
# This file is subject to the terms and conditions of the GNU General
# Public License.  See the file COPYING in the main directory of this
# archive for more details.
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
#include "sphere.h"
#include "sphere2.h"
#include "cylinder.h"
#include "instance.h"
#include "subspace.h"
#include "glow.h"
#include "glass.h"
#include "mirror.h"
#include "rect-light.h"
#include "sphere-light.h"
#include "cook-torrance.h"

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

    TVec (T _x, T _y, T _z);
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

  template<typename T>
  class TPos : public Tuple3<T>
  {
  public:

    TPos (T _x, T _y, T _z);
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

    TXform &translate (dist_t x, dist_t y, dist_t z);
    TXform &translate (const TVec<T> &offs);

    TXform &scale (T scale);
    TXform &scale (T s_x, T s_y, T s_z);

    TXform &rotate_x (T angle);
    TXform &rotate_y (T angle);
    TXform &rotate_z (T angle);
    TXform &rotate (TVec<T> axis, T angle);

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

    void include (const Pos &pos);

    Vec extent () const;

    dist_t max_size () const;
    dist_t min_size () const;
    dist_t avg_size () const;
    dist_t median_size () const;    

    Pos min, max;
  };
  %extend BBox
  {
    const char* __str__() { return static_rep (*$self); }
  }

  %ignore Brdf;
  class Brdf
  {
  };

  class Material
  {
  public:
    enum ShadowType { SHADOW_OPAQUE, SHADOW_NONE, SHADOW_MEDIUM };

    Material (const Color &col, const Brdf *brdf = lambert,
	      ShadowType _shadow_type = SHADOW_OPAQUE);
    Material (const Color &col, ShadowType _shadow_type);
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

  class CookTorrance : public Brdf
  {
  public:

    CookTorrance (const Color &_spec_col, float _m, const Ior &_ior);
  };

  const snogray::CookTorrance *cook_torrance (const snogray::Color &spec_col, float m, const snogray::Ior &ior);
  const snogray::CookTorrance *cook_torrance (const snogray::Color &spec_col, float m, const float ior = 1.5);

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
	    const Color &col, const Brdf *underlying_brdf);
    Mirror (const Ior &_ior, const Color &_reflectance, const Color &col = 0);
    Mirror (float _ior, const Color &_reflectance,
	    const Color &col, const Brdf *underlying_brdf);
    Mirror (float _ior, const Color &_reflectance, const Color &col = 0);
  };

  class Glow : public Material
  {
  public:

    Glow (const Color &_color);
  };

  %ignore Surface;
  class Surface
  {
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

  class MaterialMap
  {
  public:

    MaterialMap (const Material *default_material = 0);

    const Material *get (const char *name) const;
    void add (const char *name, const Material *mat);
    bool contains (const char *name) const;

    const Material *get_default () const;
    void set_default (const Material *mat);
  };
  %extend MaterialMap
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
    // mapping to a zero pointer in the native MaterialMap type).
    //
    void __setitem__ (const char *name, bool flag)
    {
      $self->add (name, 0);
    }

    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"material<nentries=%d%s>",
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
	  const snogray::Xform &xform = Xform::identity,
	  bool smooth = true);
    Mesh (const char *file_name, const MaterialMap &mat_map,
	  const Xform &xform = Xform::identity);

    typedef unsigned vert_index_t;
    class VertexGroup;
    class VertexNormalGroup;

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

    void load (const char *file_name,
	       const MaterialMap &mat_map = MaterialMap (),
	       const Xform &xform = Xform::identity);
    void load (const char *file_name, const Xform &xform);

    void compute_vertex_normals (float max_angle = 45 * M_PIf / 180);

    virtual BBox bbox () const;

    void transform (Xform &xform);
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
  }

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
