// mesh.cc -- Mesh surface			-*- coding: utf-8 -*-
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <fstream>

#include "globals.h"
#include "excepts.h"
#include "string-funs.h"

#include "tripar-isec.h"
#include "tessel.h"

#include "mesh.h"

using namespace Snogray;
using namespace std;


// Add a vertex to the mesh

// This simple version always adds a new vertex
//
Mesh::vert_index_t
Mesh::add_vertex (const Pos &pos)
{
  vert_index_t vert_index = vertices.size ();
  vertices.push_back (MPos (pos));
  return vert_index;
}

// This version uses VGROUP to keep track of vertex positions, and only
// adds new vertices.
//
Mesh::vert_index_t
Mesh::add_vertex (const Pos &pos, VertexGroup &vgroup)
{
  VertexGroup::iterator prev_entry = vgroup.find (pos);

  if (prev_entry != vgroup.end ())
    return prev_entry->second;
  else
    {
      vert_index_t vert_index = add_vertex (pos);
      vgroup.insert (prev_entry, VertexGroup::value_type (pos, vert_index));
      return vert_index;
    }
}


// Add a vertex with normal to the mesh

// This simple version always adds a new vertex+normal
//
Mesh::vert_index_t
Mesh::add_vertex (const Pos &pos, const Vec &normal)
{
  vert_index_t vert_index = vertices.size ();

  // Make sure the vertex_normals vector contains entries for all previous
  // vertices (the effect of this is that if a mesh contains vertices with
  // explicit normals, all triangles will have interpolated normals, even
  // those using vertices with implicit normals).
  //
  if (vertex_normals.size() < vert_index)
    {
      compute_vertex_normals ();
      vert_index = vertices.size (); // compute_vertex_normals can add vertices
    }

  vertices.push_back (MPos (pos));
  vertex_normals.push_back (MVec (normal));

  return vert_index;
}

// This version uses VGROUP to keep track of vertex positions and normal
// values, and only adds new vertices (but a vertex with a different
// normal is considered "new").
//
Mesh::vert_index_t
Mesh::add_vertex (const Pos &pos, const Vec &normal, VertexNormalGroup &vgroup)
{
  VertexNormalGroup::key_type key (pos, normal);
  VertexNormalGroup::iterator prev_entry = vgroup.find (key);

  if (prev_entry != vgroup.end ())
    return prev_entry->second;
  else
    {
      vert_index_t vert_index = add_vertex (pos, normal);
      vgroup.insert (prev_entry,
		     VertexNormalGroup::value_type (key, vert_index));
      return vert_index;
    }
}


// Add just a normal

// Add NORMAL as the vertex normal for the previously-added vertex at
// VERT_INDEX.  In certain cases where not all vertices have explicit
// normals, new vertices can be added by implicit mesh smoothing; the
// actual index where NORMAL was added is returned.
//
Mesh::vert_index_t
Mesh::add_normal (vert_index_t vert_index, const Vec &normal)
{
  // Make sure the vertex_normals vector contains entries for all previous
  // vertices (the effect of this is that if a mesh contains vertices with
  // explicit normals, all triangles will have interpolated normals, even
  // those using vertices with implicit normals).
  //
  if (vertex_normals.size() < vert_index)
    {
      compute_vertex_normals ();
      vert_index = vertices.size (); // compute_vertex_normals can add vertices
    }

  vertex_normals.push_back (MVec (normal));

  return vert_index;
}


// Add a triangle to the mesh

void
Mesh::add_triangle (vert_index_t v0i, vert_index_t v1i, vert_index_t v2i,
		    const Material *mat)
{
  if (!mat && !material)
    throw std::runtime_error ("Mesh triangle with no material");

  Triangle triang (*this, v0i, v1i, v2i, mat);

  triangles.push_back (triang);
}

void
Mesh::add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		    VertexGroup &vgroup, const Material *mat)
{
  vert_index_t v0i = add_vertex (v0, vgroup);
  vert_index_t v1i = add_vertex (v1, vgroup);
  vert_index_t v2i = add_vertex (v2, vgroup);

  add_triangle (v0i, v1i, v2i, mat);
}

void
Mesh::add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		    const Material *mat)
{
  add_triangle (add_vertex (v0), add_vertex (v1), add_vertex (v2), mat);
}


// Tessellation support

// Add the results of tessellating TESSEL_FUN with MAX_ERR.
//
void
Mesh::add (const Tessel::Function &tessel_fun,
	   const Tessel::MaxErrCalc &max_err,
	   bool smooth)
{
  // Do the tessellation
  //
  Tessel tessel (tessel_fun, max_err);

  vert_index_t base_vert = vertices.size ();

  tessel.get_vertices (vertices);

  std::vector<vert_index_t> tri_vert_indices;
  tessel.get_triangle_vertex_indices (tri_vert_indices);

  unsigned num_tris = tri_vert_indices.size () / 3;

  triangles.reserve (triangles.size() + num_tris);

  unsigned tvi_num = 0;
  for (unsigned t = 0; t < num_tris; t++)
    {
      add_triangle (base_vert + tri_vert_indices[tvi_num + 0],
		    base_vert + tri_vert_indices[tvi_num + 1],
		    base_vert + tri_vert_indices[tvi_num + 2]);
      tvi_num += 3;
    }

  if (smooth)
    {
      tessel.get_vertex_normals (vertex_normals);
      if (vertex_normals.size() <= base_vert)
	compute_vertex_normals ();
    }
}


// Mesh triangles

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
dist_t
Mesh::Triangle::intersection_distance (const Ray &ray, IsecParams &isec_params,
				       unsigned num)
  const
{
  if (num != 0)
    return 0;

  // We have to convert the types to match that of RAY first.
  //
  Pos corner = v(0);
  Vec edge1 = v(1) - corner, edge2 = v(2) - corner;

  return triangle_intersect (corner, edge1, edge2, ray.origin, ray.dir,
			     isec_params.u, isec_params.v);
}

Intersect
Mesh::Triangle::intersect_info (const Ray &ray, const IsecParams &isec_params,
				Trace &trace)
  const
{
  // We first use the real "raw" normal to determine if this is a back
  // face or not (for back-face determination the normal doesn't need
  // to be a unit vector, so it is only made a unit vector later,
  // perhaps after replacing it w ith the interpolated normal).
  //
  Vec norm = raw_normal_unscaled ();
  bool back = dot (norm, ray.dir) > 0;

  // Point of intersection.
  //
  Pos point = ray.end ();

  // Now if we're using normal interpolation, calculate the
  // interpolated normal.
  //
  if (! mesh.vertex_normals.empty ())
    {
      norm =  vnorm(0) * (1 - isec_params.u - isec_params.v);
      norm += vnorm(1) * isec_params.u;
      norm += vnorm(2) * isec_params.v;

      // If the interpolated normal is pointing in (roughly) the same
      // direction as RAY, it means normal interpolation has
      // interpolated past a virtual tangent point on the surface.
      //
      if (back != dot (norm, ray.dir) > 0)
	{
	  // In this case there's not much we can do -- it will look
	  // ugly no matter what -- but try to keep things as sane as
	  // possible by clamping the normal at the point where it's
	  // perpendicular to RAY.
	  //
	  Vec cx = cross (norm, ray.dir);
	  norm = cross (ray.dir, cx);

	  // Now very slightly nudge the resulting "perfectly perpendicular"
	  // normal back towards the viewer, which will avoid problems
	  // caused by precision errors pushing it in the other direction.
	  //
	  norm -= ray.dir * Eps;
	}
    }

  return Intersect (ray, this, point, norm, back, trace,
		    static_cast<const void *>(&mesh));
}

// Return a bounding box for this surface.
BBox
Mesh::Triangle::bbox () const
{
  BBox bbox (v (0));
  bbox.include (v (1));
  bbox.include (v (2));
  return bbox;
}

// The "smoothing group" this surface belongs to, or zero if it belongs
// to none.  The smoothing group affects shadow-casting: if two objects
// are in the same smoothing group, they will not be shadowed by
// back-surface shadows from each other; typically all triangles in a
// mesh are in the same smoothing group.
//
const void *
Mesh::Triangle::smoothing_group () const
{
  // We only need a smoothing group if the mesh uses interpolated normals.

  if (! mesh.vertex_normals.empty ())
    return static_cast<const void *>(&mesh);
  else
    return 0;
}

// Confirm that this surfaces blocks RAY, which emanates from the
// intersection ISEC.  DIST is the distance between ISEC and the position
// where RAY intersects this surface.
//
bool
Mesh::Triangle::confirm_shadow (const Ray &ray, dist_t, const Intersect &isec)
  const
{
  if (isec.smoothing_group == static_cast<const void *>(&mesh))
    {    
      bool real_back = dot (raw_normal_unscaled(), ray.dir) > 0;

      // We only get suspicious about the validity of the shadow if RAY is
      // coming from a difference surface when we compare the virtual
      // smoothed normal with the real surface normal.
      //
      if (real_back != isec.back)
	{
	  const Triangle *other_tri
	    = static_cast<const Triangle *>(isec.surface);
	  bool other_back
	    = dot (other_tri->raw_normal_unscaled(), ray.dir) > 0;

	  return real_back == other_back;

#if 0
	  // Vertex indices of this and the other triangle
	  //
	  vert_index_t v0i = vi[0];
	  vert_index_t v1i = vi[1];
	  vert_index_t v2i = vi[2];
	  vert_index_t ov0i = other_tri->vi[0];
	  vert_index_t ov1i = other_tri->vi[1];
	  vert_index_t ov2i = other_tri->vi[2];

	  // We check to see if this triangle shares any vertices with the
	  // other triangle; if so, then if the source intersection was on the
	  // front/back of OTHER_TRI, we reject shadows if RAY hit the same
	  // surface (front/back) of this triangle.  Doing this eliminates the
	  // most common false shadowing cases in smooth meshes.

	  if (v0i == ov0i || v0i == ov1i || v0i == ov2i
	      || v1i == ov0i || v1i == ov1i || v1i == ov2i
	      || v2i == ov0i || v2i == ov1i || v2i == ov2i)
	    return isec.back == other_back;
#endif
	}
    }

  // By default, say the shadow's OK
  //
  return true;
}



// Object for calculating vertex normals.
//
class VertNormGroups
{
public:

  VertNormGroups (Mesh &_mesh, float max_angle,
		  Mesh::vert_index_t _base_vert = 0)
    : mesh (_mesh), min_cos (cos (max_angle)), base_vert (_base_vert)
  { }

  // A single vertex "group"
  struct Group
  {
    Group () : num_faces (0), next (0) { }

    // The number of faces in this group.
    //
    unsigned num_faces;

    // The sum of the normals of the faces in this group.
    //
    Mesh::MVec normal_sum;

    // The average of the normals of the faces in this group.
    //
    Mesh::MVec normal;

    // The vertex index of the next normal group.
    //
    Mesh::vert_index_t next;
  };

  Group &operator[] (Mesh::vert_index_t index)
  { return groups[index - base_vert]; }

  // Add a face with normal FACE_NORMAL to the normal group for VERTEX, or to
  // some other normal group derived from it, or to a new normal group, such
  // that the angle between FACE_NORMAL and the normal group's normal is not
  // greater than the maximum angle specified for this VertNormGroups object.
  // Returns the index of the vertex to which FACE_NORMAL was added.
  //
  Mesh::vert_index_t add_face (const Mesh::MVec &face_normal,
			       Mesh::vert_index_t vertex);

private:

  // Mesh we're calculating normals for.
  //
  Mesh &mesh;

  // The minimum cosine, and thus maximum angle, allowed between normals in
  // the same group.
  //
  float min_cos;

  // The first vertex we're calculating for.
  //
  Mesh::vert_index_t base_vert;

  // Normal groups allocated so far.  The first entry is for vertex
  // BASE_VERT, and all others follow in vertex order.
  //
  vector<Group> groups;
};

// Add a face with normal FACE_NORMAL to the normal group for VERTEX, or to
// some other normal group derived from it, or to a new normal group, such
// that the angle between FACE_NORMAL and the normal group's normal is not
// greater than the maximum angle specified for this VertNormGroups object.
// Returns the index of the vertex to which FACE_NORMAL was added.
//
Mesh::vert_index_t
VertNormGroups::add_face (const Mesh::MVec &face_normal,
			  Mesh::vert_index_t vertex)
{
  if (vertex - base_vert >= groups.size ())
    groups.resize (vertex - base_vert + 1);

  // Vertex group for VERTEX.
  //
  Group &group = groups[vertex - base_vert];

  // First see if FACE_NORMAL is acceptable to merge with GROUP, either
  // because it's the first normal added to it, or because the angle
  // between FACE_NORMAL and the group's normal is sufficiently small.
  //
  if (group.num_faces == 0 || dot (face_normal, group.normal) >= min_cos)
    //
    // It fits, add FACE_NORMAL to GROUP, and return VERTEX.
    {
      group.num_faces++;
      group.normal_sum += face_normal;
      group.normal = group.normal_sum.unit ();
      return vertex;
    }
  else
    {
      // If no other vertices/groups have been split from this one yet,
      // make a new vertex by copying this one.
      //
      if (! group.next)
	group.next = mesh.add_vertex (mesh.vertex (vertex));

      // Continue our lookup with the next group.
      //
      return add_face (face_normal, group.next);
    }
}



// Compute a normal vector for each vertex that doesn't already have one,
// by averaging the normals of the triangles that use the vertex.
// MAX_ANGLE is the maximum angle allowed between two triangles that share
// a vertex (and thus a vertex normal); in order to maintain this
// constraint, compute_vertex_normals may split vertices, so the number of
// vertices may increase (to prevent this, specify a sufficiently large
// MAX_ANGLE, e.g. 2 * PI).
//
void
Mesh::compute_vertex_normals (float max_angle)
{
  unsigned num_verts = vertices.size ();
  unsigned num_triangs = triangles.size ();
  unsigned num_old_norms = vertex_normals.size();

  if (num_old_norms < num_verts)
    {
      VertNormGroups norm_groups (*this, max_angle, num_old_norms);

      for (unsigned t = 0; t < num_triangs; t++)
	{
	  Triangle &triang = triangles[t];
	  const MVec norm (triang.raw_normal ());

	  // Find a vertex normal group for this triangle's normal.
	  // We just update each vertex index to refer to the vertex to
	  // which the normal was added.
	  //
	  for (unsigned num = 0; num < 3; num++)
	    if (triang.vi[num] >= num_old_norms)
	      triang.vi[num] = norm_groups.add_face (norm, triang.vi[num]);
	}
	
      // The number of vertices may have increased due to vertex splitting.
      //
      num_verts = vertices.size ();

      vertex_normals.resize (num_verts);

      for (vert_index_t v = num_old_norms; v < num_verts; v++)
	vertex_normals[v] = norm_groups[v].normal;
    }
}



// Add this (or some other ...) surfaces to SPACE
//
void
Mesh::add_to_space (Space &space)
{
  // Shrink the allocated space in the various vectors to the amount
  // actually needed.
  //
  

  if (!quiet && triangles.size () > 50000)
    std::cout << "* adding large mesh: "
	      << commify (vertices.size ()) << " vertices"
	      << ", " << commify (triangles.size ()) << " triangles"
	      << std::endl;

  for (unsigned i = 0; i < triangles.size(); i++)
    triangles[i].add_to_space (space);
}

// Return a bounding box for this surface.
BBox
Mesh::bbox () const
{
  BBox bbox = vertex (0);

  for (vert_index_t v = 1; v < vertices.size (); v++)
    bbox.include (vertex (v));

  return bbox;
}

void
Mesh::transform (Xform &xform)
{
  SXform xf = SXform (xform);

  for (vert_index_t v = 0; v < vertices.size (); v++)
    vertices[v] *= xf;

  if (vertex_normals.size() > 0)
    {
      // Calculate a variant of XFORM suitable for transforming
      // normals.
      //
      SXform norm_xf = xf.inverse().transpose();

      for (vert_index_t v = 0; v < vertex_normals.size (); v++)
	vertex_normals[v] *= norm_xf;
    }
}


// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
