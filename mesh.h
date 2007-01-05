// mesh.h -- Mesh surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MESH_H__
#define __MESH_H__

#include <stdexcept>
#include <string>
#include <istream>
#include <vector>
#include <map>
#include <utility>		// for std::pair

#include "surface.h"
#include "tessel.h"
#include "pos.h"
#include "xform.h"

namespace snogray {

class Tessel;

class Mesh : public Surface
{
public:

  typedef SPos MPos;		// position type used in mesh
  typedef SVec MVec;		// vector type used in mesh

  // Index of a vertex in the mesh.
  //
  typedef unsigned vert_index_t;

  // A vertex group can be used to group vertices together.
  //
  typedef std::map<Pos, vert_index_t> VertexGroup;
  typedef std::map<std::pair<Pos, Vec>, vert_index_t> VertexNormalGroup;

  // Basic constructor.  Actual contents must be defined later.  If no
  // material is defined, all triangles added must have an explicit material.
  //
  Mesh (const Material *mat = 0) : Surface (mat), left_handed (true) { }

  // All-in-one constructor for loading a mesh from FILE_NAME.
  //
  Mesh (const Material *mat, const std::string &file_name,
	const Xform &xform = Xform::identity, bool smooth = true)
    : Surface (mat), left_handed (true)
  {
    load (file_name, xform);
    if (smooth)
      compute_vertex_normals ();
  }
  Mesh (const Material *mat, const std::string &file_name,
	const Xform &xform, const std::string &mat_name)
    : Surface (mat), left_handed (true)
  {
    load (file_name, xform, 0, mat_name);
  }
  Mesh (const Material *mat, const std::string &file_name,
	const Xform &xform, const char *mat_name)
    : Surface (mat), left_handed (true)
  {
    load (file_name, xform, 0, mat_name);
  }

  // All-in-one constructor for a tessellated mesh.
  //
  Mesh (const Material *mat, const Tessel::Function &tessel_fun,
	const Tessel::MaxErrCalc &max_err, bool smooth = true)
    : Surface (mat), left_handed (true)
  {
    add (tessel_fun, max_err, smooth);
  }

  // Add a triangle to the mesh
  //
  void add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i,
		     const Material *mat = 0);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		     const Material *mat = 0);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		     VertexGroup &vgroup, const Material *mat = 0);

  // Add a vertex to the mesh
  //
  vert_index_t add_vertex (const Pos &pos);
  vert_index_t add_vertex (const Pos &pos, VertexGroup &vgroup);

  // Add a vertex with normal to the mesh
  //
  vert_index_t add_vertex (const Pos &pos, const Vec &normal);
  vert_index_t add_vertex (const Pos &pos, const Vec &normal,
			   VertexNormalGroup &vgroup);

  // Add NORMAL as the vertex normal for the previously-added vertex at
  // VERT_INDEX.  In certain cases where not all vertices have explicit
  // normals, new vertices can be added by implicit mesh smoothing; the
  // actual index where NORMAL was added is returned.
  //
  vert_index_t add_normal (vert_index_t vert_index, const Vec &normal);

  // Add the results of tessellating TESSEL_FUN with MAX_ERR.
  //
  void add (const Tessel::Function &tessel_fun,
	    const Tessel::MaxErrCalc &max_err,
	    bool smooth = true);

  // For loading mesh from any file-type (automatically determined)
  //
  void load (const std::string &file_name,
	     const Xform &xform = Xform::identity,
	     const Material *mat = 0,
	     const std::string &mat_name = "");

  // For loading mesh from .msh file
  //
  void load_msh_file (std::istream &stream,
		      const Xform &xform = Xform::identity,
		      const Material *mat = 0,
		      const std::string &mat_name = "");

  // Add this (or some other ...) surfaces to SPACE
  //
  virtual void add_to_space (Space &space);

  // Compute a normal vector for each vertex that doesn't already have one,
  // by averaging the normals of the triangles that use the vertex.
  // MAX_ANGLE is the maximum angle allowed between two triangles that share
  // a vertex (and thus a vertex normal); in order to maintain this
  // constraint, compute_vertex_normals may split vertices, so the number of
  // vertices may increase (to prevent this, specify a sufficiently large
  // MAX_ANGLE, e.g. 2 * PI).
  //
  void compute_vertex_normals (float max_angle = 45 * M_PIf / 180);

  Pos vertex (vert_index_t index) const { return Pos (vertices[index]); }
  Vec vertex_normal (vert_index_t index) const
  { return Vec (vertex_normals[index]); }

  unsigned num_vertices () const { return vertices.size (); }
  unsigned num_triangles () const { return triangles.size (); }

  // Resize the internal data structures in advance for NUM_VERTS more
  // vertices and normals (if WITH_NORMALS is true), and NUM_TRIS more
  // triangles.
  //
  void reserve (unsigned num_verts, unsigned num_tris,
		bool with_normals = false)
  {
    vertices.reserve (num_vertices() + num_verts);
    triangles.reserve (num_triangles() + num_tris);
    if (with_normals)
      vertex_normals.reserve (num_vertices() + num_verts);
  }

  // Return a bounding box for the entire mesh
  //
  virtual BBox bbox () const;

  void transform (Xform &xform);

private:

  class Triangle : public Surface
  {
  public:

    Triangle (const Mesh &_mesh)
      : Surface (_mesh.material), mesh (_mesh)
    { }
    Triangle (const Mesh &_mesh,
	      vert_index_t v0i, vert_index_t v1i, vert_index_t v2i,
	      const Material *mat = 0)
      : Surface (mat ? mat : _mesh.material), mesh (_mesh)
    {
      vi[0] = v0i;
      vi[1] = v1i;
      vi[2] = v2i;
    }

    void operator= (const Triangle &triang)
    {
      vi[0] = triang.vi[0];
      vi[1] = triang.vi[1];
      vi[2] = triang.vi[2];
      material = triang.material;
    }

    // Confirm that this surfaces blocks RAY, which emanates from the
    // intersection ISEC.  DIST is the distance between ISEC and the
    // position where RAY intersects this surface.
    //
    virtual bool confirm_shadow (const Ray &ray, dist_t dist,
				 const Intersect &isec)
      const;

    // Return the distance from RAY's origin to the closest intersection
    // of this surface with RAY, or 0 if there is none.  RAY is considered
    // to be unbounded.
    //
    // If intersection succeeds, then ISEC_PARAMS is updated with other
    // (surface-specific) intersection parameters calculated.
    //
    // NUM is which intersection to return, for non-flat surfaces that may
    // have multiple intersections -- 0 for the first, 1 for the 2nd, etc
    // (flat surfaces will return failure for anything except 0).
    //
    virtual dist_t intersection_distance (const Ray &ray,
					  IsecParams &isec_params,
					  unsigned num)
      const;

    // Return more information about the intersection of RAY with this
    // surface; it is assumed that RAY does actually hit the surface, and
    // RAY's length gives the exact point of intersection (the `intersect'
    // method modifies RAY so that this is true).
    //
    virtual Intersect intersect_info (const Ray &ray,
				      const IsecParams &isec_params,
				      Trace &trace)
      const;

    // Return a bounding box for this surface.
    //
    virtual BBox bbox () const;

    // The "smoothing group" this surface belongs to, or zero if it belongs
    // to none.  The smoothing group affects shadow-casting: if two objects
    // are in the same smoothing group, they will not be shadowed by
    // back-surface shadows from each other; typically all triangles in a
    // mesh are in the same smoothing group.
    //
    virtual const void *smoothing_group () const;

    // Vertex NUM of this triangle
    //
    Pos v (unsigned num) const { return Pos (mesh.vertices[vi[num]]); }

    // Normal of vertex NUM (assuming this mesh contains vertex normals!)
    //
    Vec vnorm (unsigned num) const
    {
      return Vec (mesh.vertex_normals[vi[num]]);
    }

    // These both return the "raw" normal of this triangle, not doing
    // any normal interpolation.  Note that they return ordinary
    // double-precision vectors, not the single-precision vectors used
    // in the mesh (because most uses want the former).
    //
    const Vec raw_normal_unscaled () const
    {
      Vec e1 = v(1) - v(0), e2 = v(2) - v(0);
      return mesh.left_handed ? cross (e2, e1) : cross (e1, e2);
    }
    const Vec raw_normal () const
    {
      return raw_normal_unscaled().unit ();
    }

    const Mesh &mesh;

    // Indices into mesh vertices array
    //
    vert_index_t vi[3];
  };

  // A list of vertices used in this part.
  //
  std::vector<MPos> vertices;
  std::vector<MVec> vertex_normals;

  // A vector of Mesh::Triangle surfaces that use this part.
  //
  std::vector<Triangle> triangles;

public:

  // Whether this mesh uses left-handed or right-handed conventions by
  // default -- basically whether the triangle vertices are in a
  // counter-clockwise or clockwise order, when the normal is coming
  // towards the viewer.  This affects normal calculations.
  //
  // Snogray uses a left-handed coordinate system, but meshes read from
  // an external file may be from a system with different conventions,
  // and need to have their normals reversed.
  //
  bool left_handed;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
