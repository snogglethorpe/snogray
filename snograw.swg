# snograw.swg -- Swig interface specification for snogray
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

//
// Swig's understanding of C++ is imperfect, so in various cases we
// lie to it, giving different source code for the swig input than is
// actually compiled elsewhere:
//
//   (1) For function arguments with a type of "const std::string &",
// 	 we use "const char *" in the declarations below (and rely on
// 	 the automatic C++ conversion to translate), because swig is
// 	 more functional with char pointers (e.g. it automatically
// 	 converts to/from the client language's string type).
//
//   (2) We generally omit "const" from types used in Ref's, where
//       const is used in the real code; e.g., if the real code says
//       "Ref<const T>", we tell swig it's "Ref<T>" instead.  The
//       reason is that whereas the compiler understands that a value
//       with type "Ref<T>" can be assigned to or used to initialize a
//       variable of type "Ref<const T>" (there are appropriate
//       overloads in the Ref definition to allow this), swig doesn't
//       understand this, and generally mucks up type-checking as a
//       result.  To avoid these problems we basically just always use
//       Ref<T> and let the compiler deal with making everything right
//       (which it does).
//

# This is the "raw" interface to snogray
#
%module snograw

// Tell swig to only make consturctor when we specifically ask for them.
//
%nodefaultctor;

%include "config.h"

%{
#include <sstream>
#include <cstring>

#include "coords.h"
#include "material.h"
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
#include "image.h"
#include "envmap.h"
#include "matrix-tex.h"
#include "arith-tex.h"
#include "grey-tex.h"
#include "intens-tex.h"
#include "xform-tex.h"
#include "check-tex.h"
#include "misc-map-tex.h"
#include "perlin-tex.h"
#include "coord-tex.h"
#include "interp-tex.h"
#include "cmp-tex.h"
#include "perturb-tex.h"
#include "rescale-tex.h"

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

    static Mesh::VertexGroup *
    mesh_vertex_group ()
    {
      return new Mesh::VertexGroup;
    }
    static Mesh::VertexNormalGroup *
    mesh_vertex_normal_group ()
    {
      return new Mesh::VertexNormalGroup;
    }

  }

%}

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
  class TXform : public Matrix4<T>
  {
  public:

    static const TXform identity;

    TXform (T scale = 1);
    TXform (const TXform &src);

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

  template<class T>
  class Ref
  {
  public:

    Ref ();
    Ref (T *_obj);
    Ref (const Ref &ref);

    ~Ref ();

    T &operator* ();
    T *operator-> ();

    bool null () const;

    %rename(set) operator= (T *new_obj);
    template<class T2> Ref &operator= (T2 *new_obj);

    %rename(set) operator= (const Ref &ref);
    Ref &operator= (const Ref &ref);
  };

  class TexCoords
  {
  public:

    TexCoords (const Pos &_pos, const UV &_uv);

    Pos pos;
    UV uv;
  };

  template<typename T>
  class Tex
  {
  };
  %template(ColorTex) Tex<Color>;
  %template(FloatTex) Tex<float>;
  %template(ColorTexRef) Ref<Tex<Color> >;
  %template(FloatTexRef) Ref<Tex<float> >;

  template<typename T>
  class TexVal
  {
  public:

    TexVal (const Ref<Tex<T> > &_tex);
    TexVal (const T &val);

    %rename(set) operator= (const Ref<Tex<T> > &_tex);
    TexVal &operator= (const Ref<Tex<T> > &_tex);
    %rename(set) operator= (const T &val);
    TexVal &operator= (const T &val);
  };
  %template(ColorTexVal) TexVal<Color>;
  %template(FloatTexVal) TexVal<float>;

  // Swig fails to define %renamed methods in a template'd class, so define
  // these manually.
  //
  %extend TexVal<float>
  {
    void set (const Ref<Tex<float> > &_tex) { *$self = _tex; }
    void set (float val) { *$self = val; }
  }
  %extend TexVal<Color>
  {
    void set (const Ref<Tex<Color> > &_tex) { *$self = _tex; }
    void set (Color val) { *$self = val; }
  }

  class Material
  {
  public:

    enum ShadowType { SHADOW_OPAQUE, SHADOW_NONE, SHADOW_MEDIUM };

    Ref<Tex<float> > bump_map;
  };
  %template(MatRef) Ref<Material>;

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

  class Frame
  {
  public:

    Frame (const Pos &org, const Vec &_x, const Vec &_y, const Vec &_z);
    Frame ();
    Frame (const Pos &org);
    Frame (const Vec &_x, const Vec &_y, const Vec &_z);
    Frame (const Vec &_z);
    Frame (const Pos &org, const Vec &_z);

    Vec to (const Vec &vec) const { return vec.to_basis (x, y, z); }
    Vec to (const Pos &pos) const { return to (pos - origin); }
    Vec from (const Vec &vec) const { return vec.from_basis (x, y, z); }

    Pos origin;
    Vec x, y, z;
  };

  class Medium
  {
  public:

    Medium (float _ior = 1, const Color &_absorb = 0);
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

    Sphere (const Ref<Material> &mat, const Pos &_center, dist_t _radius);
    Sphere (const Ref<Material> &mat, const Pos &_center, const Vec &_radius);
  };

  class Cylinder : public Surface
  {
  public:

    Cylinder (const Ref<Material> &mat, const Xform &obj_to_world_xform);
    Cylinder (const Ref<Material> &mat, const Pos &base, const Vec &axis,
	      float radius);
  };

  class Sphere2 : public Surface
  {
  public:

    Sphere2 (const Ref<Material> &mat, const Xform &_xform);
  };

  class Tripar : public Surface
  {
  public:

    Tripar (const Ref<Material> &mat, const Pos &_v0,
	    const Vec &_e1, const Vec &_e2, bool _parallelogram = false);
  };

  class Ellipse : public Surface
  {
  public:

    Ellipse (const Ref<Material> &mat,
	     const Pos &center, const Vec &radius1, const Vec &radius2);
  };

  %ignore LocalSurface;
  class LocalSurface : public Surface
  {
  };

  %ignore Subspace;
  class Subspace
  {
  };
  %template(SubspaceRef) Ref<Subspace>;

  class Instance : public LocalSurface
  {
  public:

    Instance (const Ref<Subspace> &subspace, const Xform &local_to_world_xform);
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

    MaterialDict (const Ref<Material> &default_material = 0);

    void add (const char *name, const Ref<Material> &mat);
    bool contains (const char *name) const;

    void set_default (const Ref<Material> &mat);

    // MaterialDict::get and MaterialDict::get_default are implemented
    // using %extend, so we can lie to swig about their return types.
  };
  %extend MaterialDict
  {
    Ref<Material> get (const char *name, const Ref<Material> &def_mat) const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get (name, def_mat);
      return const_cast<snogray::Material *> (&*cmat);
    }
    Ref<Material> get (const char *name) const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get (name);
      return const_cast<snogray::Material *> (&*cmat);
    }
    Ref<Material> get_default () const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get_default ();
      return const_cast<snogray::Material *> (&*cmat);
    }

    const Ref<const Material> __getitem__ (const char *name) const
    {
      return $self->get (name);
    }
    void __setitem__ (const char *name, const Ref<Material> &mat)
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

    Mesh (const Ref<Material> &mat = 0);
    Mesh (const Ref<Material> &mat, const char *file_name,
	  bool smooth = true);
    Mesh (const char *file_name, const MaterialDict &mat_dict);

    typedef unsigned vert_index_t;

    typedef std::map<Pos, vert_index_t> VertexGroup;
    typedef std::map<std::pair<Pos, Vec>, vert_index_t> VertexNormalGroup;

    void add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i,
		       const Ref<Material> &mat = Ref<Material> ());
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		       const Ref<Material> &mat = Ref<Material> ());
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		       VertexGroup &vgroup, const Ref<Material> &mat = Ref<Material> ());

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

  %ignore Envmap;
  class Envmap
  {
  };
  %template(EnvmapRef) Ref<Envmap>;

  class Scene
  {
  public:

    Surface *add (Surface *surface);
    Light *add (Light *light);

    void set_background (const Color &col);

    void load (const char *file_name, const char *fmt, Camera &camera);

    Ref<Envmap> env_map;
    Ref<Envmap> light_map;
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

  %ignore ValTable;
  class ValTable
  {
  public:
    static ValTable NONE;
  };

  template<class T>
  class TupleMatrix
  {
  public:

    void load (const char *filename, const ValTable &params,
	       unsigned border = 0);
    void save (const char *filename, const ValTable &params = ValTable::NONE)
      const;

    const unsigned tuple_len;
    const unsigned width, height;
  };
  %extend TupleMatrix<Color>
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"image<%d, %d>", $self->width, $self->height);
      return static_rep_buf;
    }
  }
  %template(Image) TupleMatrix<Color>;

}


//
// The following are object-creation wrappers, which we use in place of
// swig-generated constructors for any classes that we want to manage via
// reference-counting.  This is because swig will do its own memory
// management for object which it constructs, and we want to avoid that.
//
// These wrapper functions simply create the object using new and return a
// reference to it; swig happily uses the reference instead, with almost
// exactly the same functionality as the underlying object pointer.
//
// [This is at the end of the file because swig doesn't like %inline blocks
// inside namespaces.]
//
%inline %{
  namespace snogray {

    // Subspace
    static Ref<Subspace> subspace (Surface *surf)
    {
      return new Subspace (surf);
    }

    // Lambert
    static Ref<Material> lambert (const TexVal<Color> &col)
    {
      return new Lambert (col);
    }

    // CookTorrance
    static Ref<Material>
    cook_torrance (const TexVal<Color> &col, const TexVal<Color> &spec_col,
		   const TexVal<float> &m, const Ior &ior)
    {
      return new CookTorrance (col, spec_col, m, ior);
    }
    static Ref<Material>
    cook_torrance (const TexVal<Color> &col, const TexVal<Color> &spec_col,
		   const TexVal<float> &m, float ior)
    {
      return new CookTorrance (col, spec_col, m, ior);
    }
    
    // Mirror
    static Ref<Material>
    mirror (const Ior &_ior, const TexVal<Color> &_reflectance,
	    const Ref<Material> &underlying_material)
    {
      return new Mirror (_ior, _reflectance, underlying_material);
    }
    static Ref<Material>
    mirror (const Ior &_ior,
	    const TexVal<Color> &_reflectance,
	    const TexVal<Color> &col = Color(0))
    {
      return new Mirror (_ior, _reflectance, col);
    }
    static Ref<Material>
    mirror (float _ior, const TexVal<Color> &_reflectance,
	    const Ref<Material> &underlying_material)
    {
      return new Mirror (_ior, _reflectance, underlying_material);
    }
    static Ref<Material>
    mirror (float _ior,
	    const TexVal<Color> &_reflectance,
	    const TexVal<Color> &col = Color(0))
    {
      return new Mirror (_ior, _reflectance, col);
    }

    // Glass
    static Ref<Material> glass (const Medium &medium)
    {
      return new Glass (medium);
    }      

    // Glow, NormGlow
    static Ref<Material> glow (const Color &col)
    {
      return new Glow (col);
    }
    static Ref<Material> norm_glow (float intens)
    {
      return new NormGlow (intens);
    }

    // Image
    typedef TupleMatrix<Color> Image;
    static Ref<Image> image (unsigned _width, unsigned _height)
    {
      return new Image (_width, _height);
    }
    static Ref<Image> image (const char *filename, unsigned border = 0)
    {
      return new Image (filename, border);
    }
    // static Ref<Image> image (const std::string &filename, const ValTable &params,
    // 		   unsigned border = 0)
    // {
    // 	return new Image (filename, params, border);
    // }

    // ImageTex
    static Ref<Tex<Color> > image_tex (const char *filename)
    {
      return new MatrixTex<Color> (filename);
    }
    static Ref<Tex<Color> > image_tex (const Ref<Image> &contents)
    {
      return new MatrixTex<Color> (contents);
    }

    // "mono" ImageTex (float-valued)
    static Ref<Tex<float> > mono_image_tex (const char *filename)
    {
      return new MatrixTex<float> (filename);
    }
    static Ref<Tex<float> > mono_image_tex (const Ref<TupleMatrix<float> > &contents)
    {
      return new MatrixTex<float> (contents);
    }

    // ArithTex
    static Ref<Tex<Color> > arith_tex (unsigned op,
				  const TexVal<Color> &arg1,
				  const TexVal<Color> &arg2)
    {
      return new ArithTex<Color> (ArithTex<Color>::Op (op), arg1, arg2);
    }
    static Ref<Tex<float> > arith_tex (unsigned op,
				  const TexVal<float> &arg1,
				  const TexVal<float> &arg2)
    {
      return new ArithTex<float> (ArithTex<float>::Op (op), arg1, arg2);
    }

    // CmpTex
    static Ref<Tex<Color> > cmp_tex (unsigned op,
				     const TexVal<float> &cval1,
				     const TexVal<float> &cval2,
				     const TexVal<Color> &rval1,
				     const TexVal<Color> &rval2)
    {
      return new CmpTex<Color> (CmpTex<Color>::Op (op), cval1, cval2,
				rval1, rval2);
    }
    static Ref<Tex<float> > cmp_tex (unsigned op,
				     const TexVal<float> &cval1,
				     const TexVal<float> &cval2,
				     const TexVal<float> &rval1,
				     const TexVal<float> &rval2)
    {
      return new CmpTex<float> (CmpTex<float>::Op (op), cval1, cval2,
				rval1, rval2);
    }

    // GreyTex
    static Ref<Tex<Color> > grey_tex (const TexVal<float> &val)
    {
      return new GreyTex (val);
    }

    // IntensTex
    static Ref<Tex<float> > intens_tex (const TexVal<Color> &val)
    {
      return new IntensTex (val);
    }

    // XformTex
    static Ref<Tex<Color> > xform_tex (const Xform &xform,
				       const TexVal<Color> &tex)
    {
      return new XformTex<Color> (xform, tex);
    }
    static Ref<Tex<float> > xform_tex (const Xform &xform,
				       const TexVal<float> &tex)
    {
      return new XformTex<float> (xform, tex);
    }

    // CheckTex
    static Ref<Tex<Color> > check_tex (const TexVal<Color> &tex1,
				       const TexVal<Color> &tex2)
    {
      return new CheckTex<Color> (tex1, tex2);
    }
    static Ref<Tex<float> > check_tex (const TexVal<float> &tex1,
				       const TexVal<float> &tex2)
    {
      return new CheckTex<float> (tex1, tex2);
    }

    // CheckTex3d
    static Ref<Tex<Color> > check3d_tex (const TexVal<Color> &tex1,
					 const TexVal<Color> &tex2)
    {
      return new Check3dTex<Color> (tex1, tex2);
    }
    static Ref<Tex<float> > check3d_tex (const TexVal<float> &tex1,
					 const TexVal<float> &tex2)
    {
      return new Check3dTex<float> (tex1, tex2);
    }

    // PlaneMapTex
    static Ref<Tex<Color> > plane_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new PlaneMapTex<Color> (tex);
    }
    static Ref<Tex<float> > plane_map_tex (const Ref<Tex<float> > &tex)
    {
      return new PlaneMapTex<float> (tex);
    }

    // CylinderMapTex
    static Ref<Tex<Color> > cylinder_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new CylinderMapTex<Color> (tex);
    }
    static Ref<Tex<float> > cylinder_map_tex (const Ref<Tex<float> > &tex)
    {
      return new CylinderMapTex<float> (tex);
    }

    // LatLongMapTex
    static Ref<Tex<Color> > lat_long_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new LatLongMapTex<Color> (tex);
    }
    static Ref<Tex<float> > lat_long_map_tex (const Ref<Tex<float> > &tex)
    {
      return new LatLongMapTex<float> (tex);
    }

    // PerlinTex
    static Ref<Tex<float> > perlin_tex ()
    {
      return new PerlinTex ();
    }

    // CoordTex
    static Ref<Tex<float> > coord_tex (unsigned kind)
    {
      return new CoordTex (CoordTex::Kind (kind));
    }

    // LinterpTex
    static Ref<Tex<Color> > linterp_tex (const TexVal<float> &control,
					 const TexVal<Color> &val1,
					 const TexVal<Color> &val2)
    {
      return new LinterpTex<Color> (control, val1, val2);
    }
    static Ref<Tex<float> > linterp_tex (const TexVal<float> &control,
					 const TexVal<float> &val1,
					 const TexVal<float> &val2)
    {
      return new LinterpTex<float> (control, val1, val2);
    }

    // SinterpTex
    static Ref<Tex<Color> > sinterp_tex (const TexVal<float> &control,
					 const TexVal<Color> &val1,
					 const TexVal<Color> &val2)
    {
      return new SinterpTex<Color> (control, val1, val2);
    }
    static Ref<Tex<float> > sinterp_tex (const TexVal<float> &control,
					 const TexVal<float> &val1,
					 const TexVal<float> &val2)
    {
      return new SinterpTex<float> (control, val1, val2);
    }

    // RescaleTex
    static Ref<Tex<Color> > rescale_tex (const TexVal<Color> &val,
					 const Color &in_min,
					 const Color &in_max,
					 const Color &out_min,
					 const Color &out_max)
    {
      return new RescaleTex<Color> (val, in_min, in_max, out_min, out_max);
    }
    static Ref<Tex<float> > rescale_tex (const TexVal<float> &val,
					 float in_min, float in_max,
					 float out_min, float out_max)
    {
      return new RescaleTex<float> (val, in_min, in_max, out_min, out_max);
    }

    // PerturbPosTex
    static Ref<Tex<Color> > perturb_pos_tex (const TexVal<Color> &source,
					     const TexVal<float> &x,
					     const TexVal<float> &y,
					     const TexVal<float> &z)
    {
      return new PerturbPosTex<Color> (source, x, y, z);
    }
    static Ref<Tex<float> > perturb_pos_tex (const TexVal<float> &source,
					     const TexVal<float> &x,
					     const TexVal<float> &y,
					     const TexVal<float> &z)
    {
      return new PerturbPosTex<float> (source, x, y, z);
    }

    // PerturbUvTex
    static Ref<Tex<Color> > perturb_uv_tex (const TexVal<Color> &source,
					    const TexVal<float> &u,
					    const TexVal<float> &v)
    {
      return new PerturbUvTex<Color> (source, u, v);
    }
    static Ref<Tex<float> > perturb_uv_tex (const TexVal<float> &source,
					    const TexVal<float> &u,
					    const TexVal<float> &v)
    {
      return new PerturbUvTex<float> (source, u, v);
    }

  }
%}


# arch-tag: ced00636-fe46-45bd-b738-1949800e26f1
