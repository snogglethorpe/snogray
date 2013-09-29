// mesh.h -- Mesh surface
//
//  Copyright (C) 2005-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MESH_H
#define SNOGRAY_MESH_H

#include <string>
#include <vector>
#include <map>

#include "geometry/pos.h"
#include "geometry/xform.h"

#include "primitive.h"


namespace snogray {


class Mesh : public Primitive
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
  Mesh (const Ref<const Material> &mat)
    : Primitive (mat), axis (Vec (0, 0, 1)), left_handed (true)
  { }


  // Add a triangle to the mesh.
  //
  void add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		     VertexGroup &vgroup);

  // Add a vertex to the mesh (with no normal).
  //
  vert_index_t add_vertex (const Pos &pos);
  vert_index_t add_vertex (const Pos &pos, VertexGroup &vgroup);

  // Add a vertex with normal to the mesh.
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

  // Add all the positions in NEW_VERTS as vertices in this mesh.
  // The index in the mesh of the first of the new vertices is returned;
  // it should be passed to any subsequent calls to Mesh::add_triangles
  // or Mesh::add_normals.
  //
  vert_index_t add_vertices (const std::vector<MPos> &new_verts);

  // Add all the positions described by NEW_VERTS as vertices in this
  // mesh.  NEW_VERTS should contain three elements for each vertex,
  // to be used as the x, y, and z coordinates of the vertex.
  //
  // The index in the mesh of the first of the new vertices is
  // returned; it should be passed to any subsequent calls to
  // Mesh::add_triangles or Mesh::add_normals.
  //
  vert_index_t add_vertices (const std::vector<scoord_t> &new_verts);

  // Add all the normal vectors in NEW_NORMALS as vertex normals in this
  // mesh, corresponding to all the vertices starting from BASE_VERT
  // (which should be a value returned from an earlier call to
  // Mesh::add_vertices).
  //
  void add_normals (const std::vector<MVec> &new_normals,
		    vert_index_t base_vert);

  // Add all the normal vectors described by NEW_NORMALS as vertex
  // normals in this mesh, corresponding to all the vertices starting
  // from BASE_VERT (which should be a value returned from an earlier
  // call to Mesh::add_vertices).
  //
  // NEW_NORMALS should contain three elements for each normal, to be
  // used as the x, y, and z components of the normal.
  //
  void add_normals (const std::vector<sdist_t> &new_normals,
		    vert_index_t base_vert);

  // Add all the UV values in NEW_UVS as vertex UV values in this mesh,
  // corresponding to all the vertices starting from BASE_VERT (which
  // should be a value returned from an earlier call to
  // Mesh::add_vertices).
  //
  void add_uvs (const std::vector<UV> &new_uvs, vert_index_t base_vert);

  // Add all the UV values described by NEW_UVS as vertex UV values in
  // this mesh, corresponding to all the vertices starting from
  // BASE_VERT (which should be a value returned from an earlier call
  // to Mesh::add_vertices).
  //
  // NEW_UVS should contain two elements for each UV.
  //
  void add_uvs (const std::vector<float> &new_uvs, vert_index_t base_vert);

  // Add new triangles to the mesh using vertices from TRI_VERT_INDICES.
  // TRI_VERT_INDICES should contain three entries for each new triangle;
  // the indices in TRI_VERT_INDICES are relative to BASE_VERT (which
  // should be a value returned from an earlier call to
  // Mesh::add_vertices).
  //
  void add_triangles (const std::vector<vert_index_t> &tri_vert_indices,
		      vert_index_t base_vert);

  // Add this (or some other) surfaces to the space being built by
  // SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;

  // Compute a normal vector for each vertex that doesn't already have one,
  // by averaging the normals of the triangles that use the vertex.
  // MAX_ANGLE is the maximum angle allowed between two triangles that share
  // a vertex (and thus a vertex normal); in order to maintain this
  // constraint, compute_vertex_normals may split vertices, so the number of
  // vertices may increase (to prevent this, specify a sufficiently large
  // MAX_ANGLE, e.g. 2 * PI).
  //
  void compute_vertex_normals (float max_angle = 45 * PIf / 180);

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

  // Resize the internal data structures so that room is reserved for as
  // as many vertex normals as there are currently vertices.
  //
  void reserve_normals ()
  {
    vertex_normals.reserve (num_vertices ());
  }

  // Resize the internal data structures so that room is reserved for as as
  // many vertex UV values as there are currently vertices.
  //
  void reserve_uvs ()
  {
    vertex_uvs.reserve (num_vertices ());
  }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const;

  // Return true if this surface completely occludes RAY.  If it does
  // not completely occlude RAY, then return false, and multiply
  // TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
  // MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it).
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const;

  // Return a bounding box for the entire mesh
  //
  virtual BBox bbox () const { return _bbox; }

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is internal to the Surface class hierachy, but cannot
  // be protected: due to pecularities in the way that is defined in
  // C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const;

  // Transform all vertices in the mesh by XFORM.
  //
  void transform (const Xform &xform);


private:

  // A single triangle in the mesh.
  //
  class Triangle;

  // Recalculate this mesh's bounding box.
  //
  void recalc_bbox ();

  // A list of vertices used in this part.
  //
  std::vector<MPos> vertices;

  // A vector of Mesh::Triangle surfaces that use this part.
  //
  std::vector<Triangle> triangles;

  // Vectors of various per-vertext properties.  In general, these vectors
  // may be empty (meaning the given property is not known), otherwise they
  // are assumed to contain information for every vertex.
  //
  std::vector<MVec> vertex_normals;
  std::vector<UV> vertex_uvs;

  // Cached bounding box for the entire mesh.
  //
  BBox _bbox;

public:

  // A unit vector pointing along the "axis" of the mesh.  This is used
  // to compute consistent tangent vectors for intersections with the
  // mesh (which is good for anisotropic materials).
  //
  Vec axis;

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



// Mesh::Triangle

// A single triangle in a Mesh.
//
class Mesh::Triangle : public Surface
{
public:

  friend class Mesh;

  Triangle (const Mesh &_mesh) : mesh (_mesh) { }

  Triangle (const Mesh &_mesh,
	    vert_index_t v0i, vert_index_t v1i, vert_index_t v2i)
    : mesh (_mesh)
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
  }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context)
    const;

  // Return true if this surface completely occludes RAY.  If it does
  // not completely occlude RAY, then return false, and multiply
  // TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
  // MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it).
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is internal to the Surface class hierachy, but cannot
  // be protected: due to pecularities in the way that is defined in
  // C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const;

  // Vertex NUM of this triangle
  //
  Pos v (unsigned num) const { return Pos (mesh.vertices[vi[num]]); }

  // Normal of vertex NUM (assuming this mesh contains vertex normals!)
  //
  Vec vnorm (unsigned num) const
  {
    return Vec (mesh.vertex_normals[vi[num]]);
  }

  // UV value of vertex NUM (assuming this mesh contains vertex UV values!)
  //
  UV vuv (unsigned num) const
  {
    return mesh.vertex_uvs[vi[num]];
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

private:

  class IsecInfo;

  // Return 2D texture-coordinate information for this triangle.
  // The 2D texture-coordinate of vertex 0 (with barycentric
  // coordinate 0,0) is returned in T0.  The change in 2D
  // texture-coordinates between vertex 0 and vertex 1 (corresponding
  // to barycentric coordinate "u") is returned in DTDU, and the
  // change between vertex 0 and vertex 2 (corresponding to
  // barycentric coordinate "v") is returned in DTDV.
  //
  void get_texture_params (UV &T0, UV &dTdu, UV &dTdv) const
  {
    // Texture-coordinates for the three vertices of the triangle.
    //
    // If this mesh doesn't have per-vertex UV values, a per-triangle
    // mapping is used.
    //
    UV T1, T2;
    if (mesh.vertex_uvs.empty ())
      {
	// The assignment of UV values to triangle vertices in the
	// absence of UV-mapping information is fairly arbitrary.
	//
	// We just use a mapping compaible with PBRT, where the middle
	// vertex of a triangle has UV coordinates 0,0, and the first
	// and last vertices have coordinates 1,0 and 0,1
	// respectively.

	T0 = UV (1, 0);
	T1 = UV (0, 0);
	T2 = UV (0, 1);
      }
    else
      {
	T0 = vuv (0);
	T1 = vuv (1);
	T2 = vuv (2);
      }

    // Change in UV values for edge1 and edge2 of the triangle.
    //
    dTdu = T1 - T0;
    dTdv = T2 - T0;
  }

  const Mesh &mesh;

  // Indices into mesh vertices array
  //
  vert_index_t vi[3];
};


}

#endif /* SNOGRAY_MESH_H */


// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
