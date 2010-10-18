# snograw.swg -- Swig interface specification for snogray
#
#  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
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
#include "primitive.h"
#include "material.h"
#include "material-dict.h"
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
#include "thin-glass.h"
#include "mirror.h"
#include "stencil.h"
#include "point-light.h"
#include "sphere-light.h"
#include "far-light.h"
#include "envmap-light.h"
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
#include "worley-tex.h"
#include "coord-tex.h"
#include "interp-tex.h"
#include "cmp-tex.h"
#include "perturb-tex.h"
#include "rescale-tex.h"
#include "tessel-sphere.h"
#include "tessel-sinc.h"
#include "tessel-torus.h"
#include "bbox-io.h"
#include "color-io.h"
#include "pos-io.h"
#include "vec-io.h"
#include "xform-io.h"
#include "ray-io.h"

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

// For handling fixed-length array arguments
//
%include <typemaps.i>
%apply (float INPUT[ANY]) {(const float els[16])};
%apply (float INPUT[ANY]) {(const dist_t els[16])};

// For handling variable-length array arguments, passed as std::vectors.
//
%{
#include <vector>

// Reads the contents of the table at INDEX on the Lua stack into a
// vector of numbers, and returns a heap-allocated vector of type
// std::vector<T>*.  Lua numbers will be cast into the type T.  If the
// object at INDEX is not a table, or the table contains non-numbers,
// zero is returned instead.
//
template<class T>
std::vector<T> *
SWIG_read_numeric_vector (lua_State *L, int index)
{
  if (! lua_istable (L, index))
    return 0;			// error

  std::vector<T> *vec = new std::vector<T> ();

  size_t len = lua_objlen (L, index);
  vec->reserve (len);

  for (size_t i = 0; i < len; i++)
    {
      lua_rawgeti (L, index, i + 1);
      if (! lua_isnumber (L, -1))
	{
	  lua_pop (L, 1);
	  delete vec;
	  return 0;		// error
	}
      vec->push_back (T (lua_tonumber (L, -1)));
      lua_pop (L, 1);
    }

  return vec;
}
%}

%define SWIG_TYPEMAP_NUM_VECTOR(T)
%typemap(in) const std::vector<T> &INPUT
%{	$1 = SWIG_read_numeric_vector<T> (L,$input);
	if (!$1) SWIG_fail; %}
%typemap(freearg) const std::vector<T> &INPUT
%{	delete $1; %}
%enddef

SWIG_TYPEMAP_NUM_VECTOR(float)
SWIG_TYPEMAP_NUM_VECTOR(unsigned)


// Lua table -> ValTable conversion
//
%{ #include "lua-funs.h" %}
%typemap(in, checkfn="lua_istable") const snogray::ValTable& (snogray::ValTable valtab)
%{	lua_pushvalue (L, $input);
	snogray::lua_store_into_val_table (L, valtab);
	$1 = &valtab; %}
%typecheck(SWIG_TYPECHECK_POINTER) const snogray::ValTable&
%{	$1 = lua_istable (L, $input); %}


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

    bool operator== (const Color &col) { return *$self == col; }
    bool operator< (const Color &col) { return *$self < col; }
    bool operator<= (const Color &col) { return *$self <= col; }
    
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

  class Primitive : public Surface
  {
  public:

    Primitive (const Ref<const Material> &mat);
  };

#if HAVE_SWIG_DISOWN
  // The only time we pass Surface pointers is when handing over control
  // of Surface objects to C++, in which case we want to avoid any GC by
  // Lua.
  //
  %apply SWIGTYPE* DISOWN {Surface *}
#endif

  %ignore LocalSurface;
  class LocalSurface : public Surface
  {
  };

  %ignore LocalPrimitive;
  class LocalPrimitive : public LocalSurface
  {
  };

  class Sphere : public Primitive
  {
  public:

    Sphere (const Ref<Material> &mat, dist_t radius, const Frame &_frame);
    Sphere (const Ref<Material> &mat, const Pos &_center, dist_t _radius);
    Sphere (const Ref<Material> &mat, const Pos &_center, const Vec &_radius);
  };

  class Cylinder : public LocalPrimitive
  {
  public:

    Cylinder (const Ref<Material> &mat, const Xform &obj_to_world_xform);
    Cylinder (const Ref<Material> &mat, const Pos &base, const Vec &axis,
	      float radius);
  };

  class Sphere2 : public LocalPrimitive
  {
  public:

    Sphere2 (const Ref<Material> &mat, const Xform &_xform);
  };

  class Tripar : public Primitive
  {
  public:

    Tripar (const Ref<Material> &mat, const Pos &_v0,
	    const Vec &_e1, const Vec &_e2, bool _parallelogram = false);
  };

  class Ellipse : public Primitive
  {
  public:

    Ellipse (const Ref<Material> &mat,
	     const Pos &center, const Vec &radius1, const Vec &radius2);
  };

  %ignore Subspace;
  class Subspace
  {
  public:

    virtual BBox bbox () const;
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
    void __setitem__ (const char *name, bool)
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

  class Mesh : public Primitive
  {
  public:

    Mesh (const Ref<Material> &mat);
    Mesh (const Ref<Material> &mat, const char *file_name,
	  bool smooth = true);

    typedef unsigned vert_index_t;

    typedef std::map<Pos, vert_index_t> VertexGroup;
    typedef std::map<std::pair<Pos, Vec>, vert_index_t> VertexNormalGroup;

    void add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i);
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2);
    void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		       VertexGroup &vgroup);

    vert_index_t add_vertex (const Pos &pos);
    vert_index_t add_vertex (const Pos &pos, VertexGroup &vgroup);

    vert_index_t add_vertex (const Pos &pos, const Vec &normal);
    vert_index_t add_vertex (const Pos &pos, const Vec &normal,
			     VertexNormalGroup &vgroup);

    vert_index_t add_normal (vert_index_t vert_index, const Vec &normal);

    vert_index_t add_vertices (const std::vector<float> &INPUT);
    void add_normals (const std::vector<float> &INPUT,
		      vert_index_t base_vert);
    void add_uvs (const std::vector<float> &INPUT, vert_index_t base_vert);
    void add_triangles (const std::vector<unsigned> &INPUT,
			vert_index_t base_vert);

    void reserve (unsigned num_verts, unsigned num_tris,
		  bool with_normals = false);
    void reserve_normals ();

    void load (const char *file_name);

    void compute_vertex_normals (float max_angle = 45 * PIf / 180);

    Pos vertex (vert_index_t index) const;
    Vec vertex_normal (vert_index_t index) const;

    unsigned num_vertices () const;
    unsigned num_triangles () const;

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

    Ray eye_ray (const UV &film_loc) const;
    Ray eye_ray (const UV &film_loc, const UV &focus_param) const;

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

#if HAVE_SWIG_DISOWN
  // The only time we pass Light pointers is when handing over control
  // of Light objects to C++, in which case we want to avoid any GC by
  // Lua.
  //
  %apply SWIGTYPE* DISOWN {Light *}
#endif

  class PointLight : public Light
  {
  public:

    PointLight (const Pos &pos, const Color &col);
    PointLight (const Pos &pos, const Color &col, float angle, const Vec &dir,
    	        float fringe_angle = 0);
  };

  class SphereLight : public Light
  {
  public:

    SphereLight (const Pos &_pos, float _radius, const Color &_intensity);
  };

  class FarLight : public Light
  {
  public:

    FarLight (const Vec &_dir, float _angle, const Color &_intensity);
  };

  %ignore Envmap;
  class Envmap
  {
  };
  %template(EnvmapRef) Ref<Envmap>;

  class EnvmapLight : public Light
  {
  public:

    EnvmapLight (const Ref<Envmap> &_envmap, const Frame &_frame = Frame ());
  };

  class Scene
  {
  public:

    void add (Surface *surface);
    void add (Light *light);
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

    void load (const char *filename, const ValTable &params = ValTable::NONE);
    void save (const char *filename, const ValTable &params = ValTable::NONE)
      const;

    T operator() (unsigned x, unsigned y);

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

    void set (unsigned x, unsigned y, const Color &val)
    {
      (*$self)(x,y) = val;
    }
  }
  %extend TupleMatrix<float>
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"mono_image<%d, %d>", $self->width, $self->height);
      return static_rep_buf;
    }

    void set (unsigned x, unsigned y, float val)
    {
      (*$self)(x,y) = val;
    }
  }
  %template(Image) TupleMatrix<Color>;
  %template(ImageRef) Ref<TupleMatrix<Color> >;


  // Simple interface to SphereTesselFun
  //
  %newobject tessel_sphere;
  Mesh *tessel_sphere (const Ref<Material> &mat,
		       const Xform &xform, dist_t max_err);
  Mesh *tessel_sphere (const Ref<Material> &mat,
		       const Pos &origin, const Vec &axis, dist_t max_err);
  Mesh *tessel_sphere (const Ref<Material> &mat,
		       const Pos &origin, const Vec &axis, const Vec &radius,
		       dist_t max_err);

  // Simple interface to SincTesselFun
  //
  %newobject tessel_sinc;
  Mesh *tessel_sinc (const Ref<Material> &mat,
		     const Xform &xform, dist_t max_err);
  Mesh *tessel_sinc (const Ref<Material> &mat,
		     const Pos &origin, const Vec &axis, const Vec &radius,
		     dist_t max_err);
  Mesh *tessel_sinc (const Ref<Material> &mat,
		     const Pos &origin, const Vec &axis, dist_t radius,
		     dist_t max_err);

  // Simple interface to TorusTesselFun
  //
  %newobject tessel_torus;
  Mesh *tessel_torus (const Ref<Material> &mat,
		      const Xform &xform, dist_t hole_frac, dist_t max_err);
  Mesh *tessel_torus (const Ref<Material> &mat,
		      const Pos &origin, const Vec &axis, const Vec &radius,
		      dist_t max_err);
  Mesh *tessel_torus (const Ref<Material> &mat,
		      const Pos &origin, const Vec &axis, dist_t radius,
		      dist_t max_err);

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

    // Stencil
    static Ref<Material>
    stencil (const TexVal<Color> &opacity,
	     const Ref<Material> &underlying_material)
    {
      return new Stencil (opacity, underlying_material);
    }

    // Glass
    static Ref<Material> glass (const Medium &medium)
    {
      return new Glass (medium);
    }      

    // ThinGlass
    static Ref<Material> thin_glass (const Color &col, float _ior = 1.5)
    {
      return new ThinGlass (col, _ior);
    }      
    static Ref<Material> thin_glass (float _ior = 1.5)
    {
      return new ThinGlass (_ior);
    }      

    // Glow, NormGlow
    static Ref<Material> glow (const TexVal<Color> &col)
    {
      return new Glow (col);
    }
    static Ref<Material> glow (const TexVal<Color> &col,
			       const Ref<Material> &underlying_material)
    {
      return new Glow (col, underlying_material);
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
    static Ref<Image> image (const char *filename,
			     const ValTable &params = ValTable::NONE)
    {
      return new Image (filename, params);
    }

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

    // WorleyTex
    static Ref<Tex<float> > worley_tex (float C0, float C1 = 0,
					float C2 = 0, float C3 = 0)
    {
      float coefs[WorleyTex::MAX_N] = { C0, C1, C2, C3 };
      return new WorleyTex (coefs);
    }
    static Ref<Tex<float> > worley_id_tex (int kind, float min)
    {
      return new WorleyIdTex (WorleyIdTex::Kind (kind), min);
    }
    static Ref<Tex<float> > worley_id_tex (int kind, float min, float max)
    {
      return new WorleyIdTex (WorleyIdTex::Kind (kind), min, max);
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

    static Ref<Envmap> envmap (const Ref<Image> &image, const char *fmt = "")
    {
      return make_envmap (image, fmt);
    }

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


# arch-tag: ced00636-fe46-45bd-b738-1949800e26f1
