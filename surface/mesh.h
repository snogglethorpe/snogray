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
#include "material/material.h"

#include "surface.h"


namespace snogray {


class Mesh : public Surface
{
public:

  class Part;			// A set of triangles + a material

  typedef SPos MPos;		// position type used in mesh
  typedef SVec MVec;		// vector type used in mesh

  // Index of a vertex in the mesh.
  //
  typedef unsigned vert_index_t;

  // Index of a mesh "part"; a mesh is divided into parts, each with
  // its own material and set of faces.
  //
  typedef unsigned part_index_t;

  // A vertex group can be used to group vertices together.
  //
  typedef std::map<Pos, vert_index_t> VertexGroup;
  typedef std::map<std::pair<Pos, Vec>, vert_index_t> VertexNormalGroup;

  // Basic constructor.  Actual contents must be defined later.
  //
  Mesh () : axis (Vec (0, 0, 1)), left_handed (true) { }

  ~Mesh ();


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

  // Add Surface::Renderable objects associated with this surface to
  // the space being built by SPACE_BUILDER.
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

  // Return the number of triangles in all mesh parts.
  //
  unsigned num_triangles () const;


  // Resize the internal data structures in advance for NUM_VERTS more
  // vertices.
  //
  void reserve_vertices (unsigned num_verts)
  {
    vertices.reserve (num_vertices() + num_verts);
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


  // Add a new part to the mesh.  Each part has its own material and
  // set of faces (all parts share the same vertices, normals, and
  // uvs).
  //
  part_index_t add_part (const Ref<const Material> &mat);

  // Add new triangles to mesh part PART, using vertices from
  // TRI_VERT_INDICES.  TRI_VERT_INDICES should contain three entries
  // for each new triangle; the indices in TRI_VERT_INDICES are
  // relative to BASE_VERT (which should be a value returned from an
  // earlier call to Mesh::add_vertices).  If there is no part PART,
  // an error is signaled.
  //
  void add_triangles (part_index_t part,
		      const std::vector<vert_index_t> &tri_vert_indices,
		      vert_index_t base_vert = 0)
    const;

  // Return the number of mesh parts.
  //
  unsigned num_parts () const { return parts.size (); }

  // Return a pointer to the material for mesh part PART.
  //
  // Note that this doesn't add a reference to the material, and is
  // only valid while the mesh still exists (as the mesh holds
  // references to all of its materials).
  //
  const Material *material (part_index_t part) const;


  // Return a bounding box for the entire mesh
  //
  virtual BBox bbox () const { return _bbox; }

  // Transform the geometry of this surface by XFORM.
  //
  virtual void transform (const Xform &xform);

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is intended for internal use in the Surface class
  // hierachy, but cannot be protected: due to pecularities in the way
  // that is defined in C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const;


private:

  // Recalculate this mesh's bounding box.
  //
  void recalc_bbox ();

  // A list of vertices used in this part.
  //
  std::vector<MPos> vertices;

  // Vectors of various per-vertext properties.  In general, these vectors
  // may be empty (meaning the given property is not known), otherwise they
  // are assumed to contain information for every vertex.
  //
  std::vector<MVec> vertex_normals;
  std::vector<UV> vertex_uvs;

  // Parts of this mesh, one per material.
  //
  std::vector<Part *> parts;

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


}

#endif /* SNOGRAY_MESH_H */


// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
