# surface.swg -- SWIG interfaces for snogray surfaces
#
#  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "mesh.h"
#include "tripar.h"
#include "ellipse.h"
#include "sphere.h"
#include "sphere2.h"
#include "cylinder.h"
#include "instance.h"
#include "model.h"
#include "surface-group.h"
#include "tessel-sphere.h"
#include "tessel-sinc.h"
#include "tessel-torus.h"

// Static helper functions
//
namespace snogray {
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


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  // fwd decls
  class Material;
  class BBox;
  class Frame;
  template<typename T> class TPos;
  typedef TPos<coord_t> Pos;
  template<typename T> class TVec;
  typedef TVec<coord_t> Vec;
  template<typename T> class TXform;
  typedef TXform<coord_t> Xform;
  class SpaceBuilderFactory;


  class Surface
  {
  private:

    Surface (); // inhibit swig constructor generation

  public:

    virtual BBox bbox () const;
  };

  class Primitive : public Surface
  {
  private:  // inhibit swig constructor generation

    Primitive (const Ref<const Material> &mat);
  };

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

  %ignore Model;
  class Model
  {
  public:

    virtual BBox bbox () const;
  };
  %template(ModelRef) Ref<Model>;

  class Instance : public LocalSurface
  {
  public:

    Instance (const Ref<Model> &model, const Xform &local_to_world_xform);
  };

  class SurfaceGroup : public Surface
  {
  public:

    SurfaceGroup ();

    void add (Surface *surface);
  };

  class Mesh : public Primitive
  {
  public:

    Mesh (const Ref<Material> &mat);

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


    // Model
    static Ref<Model>
    model (Surface *surf, const SpaceBuilderFactory &space_builder_factory)
    {
      return new Model (surf, space_builder_factory);
    }


  }
%}
