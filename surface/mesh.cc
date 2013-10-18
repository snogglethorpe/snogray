// mesh.cc -- Mesh surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "util/globals.h"
#include "util/excepts.h"
#include "util/string-funs.h"

#include "geometry/tripar-isec.h"
#include "space/space-builder.h"

#include "mesh.h"


using namespace snogray;



// Add a vertex to the mesh

// This simple version always adds a new vertex (with no normal).
//
Mesh::vert_index_t
Mesh::add_vertex (const Pos &pos)
{
  vert_index_t vert_index = vertices.size ();
  vertices.push_back (MPos (pos));
  _bbox += pos;		   // make sure POS is included in the bounding-box
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

  _bbox += pos;		   // make sure POS is included in the bounding-box

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



// Mesh::Part

struct Mesh::Part
{
  Part (const Mesh &_mesh, const Ref<const Material> &mat)
    : mesh (_mesh), material (mat)
  { }


  // Add new triangles to this mesh part using vertices from
  // TRI_VERT_INDICES.  TRI_VERT_INDICES should contain three entries
  // for each new triangle; the indices in TRI_VERT_INDICES are
  // relative to BASE_VERT (which should be a value returned from an
  // earlier call to Mesh::add_vertices).
  //
  void add_triangles (const std::vector<vert_index_t> &tri_vert_indices,
		      vert_index_t base_vert = 0);

  // Add Surface::Renderable objects associated with this mesh part to
  // the space being built by SPACE_BUILDER.
  //
  void add_to_space (SpaceBuilder &space_builder) const;


  // A single triangle in the mesh.
  //
  class Triangle;

  // The mesh this part belongs to.
  //
  const Mesh &mesh;

  // The material used by all triangle in this part.
  //
  Ref<const Material> material;

  // A vector of Mesh::Part::Triangle surfaces that use this part.
  //
  std::vector<Triangle> triangles;
};



// Mesh::Part::Triangle


// A single triangle in a Mesh.  These are the exported renderable
// object from meshes.
//
class Mesh::Part::Triangle : public Surface::Renderable
{
public:

  Triangle (const Mesh::Part &_part) : part (_part) { }

  Triangle (const Part &_part,
	    vert_index_t v0i, vert_index_t v1i, vert_index_t v2i)
    : part (_part)
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

  // If this surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::Renderable::IsecInfo object describing the intersection
  // (which should be allocated using placement-new with CONTEXT);
  // otherwise return zero.
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

  // Vertex NUM of this triangle
  //
  Pos v (unsigned num) const { return Pos (part.mesh.vertices[vi[num]]); }

  // Normal of vertex NUM (assuming this part contains vertex normals!)
  //
  Vec vnorm (unsigned num) const
  {
    return Vec (part.mesh.vertex_normals[vi[num]]);
  }

  // UV value of vertex NUM (assuming this part contains vertex UV values!)
  //
  UV vuv (unsigned num) const
  {
    return part.mesh.vertex_uvs[vi[num]];
  }

  // These both return the "raw" normal of this triangle, not doing
  // any normal interpolation.  Note that they return ordinary
  // double-precision vectors, not the single-precision vectors used
  // in the part (because most uses want the former).
  //
  const Vec raw_normal_unscaled () const
  {
    Vec e1 = v(1) - v(0), e2 = v(2) - v(0);
    return part.mesh.left_handed ? cross (e2, e1) : cross (e1, e2);
  }
  const Vec raw_normal () const
  {
    return raw_normal_unscaled().unit ();
  }


private:

  friend class Mesh;

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
    // If this part doesn't have per-vertex UV values, a per-triangle
    // mapping is used.
    //
    UV T1, T2;
    if (part.mesh.vertex_uvs.empty ())
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

  const Part &part;

  // Indices into mesh vertices array
  //
  vert_index_t vi[3];
};



// Bulk vertex addition


// Add all the positions in NEW_VERTS as vertices in this mesh.
// The index in the mesh of the first of the new vertices is returned;
// it should be passed to any subsequent calls to Mesh::add_triangles
// or Mesh::add_normals.
//
Mesh::vert_index_t
Mesh::add_vertices (const std::vector<MPos> &new_verts)
{
  vert_index_t base_vert = vertices.size ();
  vertices.insert (vertices.end(), new_verts.begin(), new_verts.end());
  return base_vert;
}

// Add all the positions described by NEW_VERTS as vertices in this
// mesh.  NEW_VERTS should contain three elements for each vertex, to
// be used as the x, y, and z coordinates of the vertex.
//
// The index in the mesh of the first of the new vertices is returned;
// it should be passed to any subsequent calls to Mesh::add_triangles
// or Mesh::add_normals.
//
Mesh::vert_index_t
Mesh::add_vertices (const std::vector<scoord_t> &new_verts)
{
  vert_index_t base_vert = vertices.size ();
  unsigned num_new_verts = new_verts.size () / 3;

  // Note that the Pos default constructor does not initialize the Pos
  // members, so the following will grow the Mesh::vertices vector,
  // but not give the new elements any value.
  //
  vertices.resize (base_vert + num_new_verts);

  unsigned nvi = 0;
  for (unsigned v = base_vert; v < base_vert + num_new_verts; v++)
    {
      vertices[v]
	= MPos (new_verts[nvi], new_verts[nvi + 1], new_verts[nvi + 2]);
      _bbox += vertices[v];
      nvi += 3;
    }

  return base_vert;
}

// Add all the normal vectors in NEW_NORMALS as vertex normals in this
// mesh, corresponding to all the vertices starting from BASE_VERT
// (which should be a value returned from an earlier call to
// Mesh::add_vertices).
//
void
Mesh::add_normals (const std::vector<MVec> &new_normals, vert_index_t base_vert)
{
  // Not sure what to do if normals after BASE_VERT already exist, of
  // if vertices before BASE_VERT don't have normals yet, so just barf
  // in those cases.
  //
  if (base_vert != vertex_normals.size ())
    throw std::runtime_error ("BASE_VERT incorrect in Mesh::add_normals");
  if (base_vert + new_normals.size() != vertices.size ())
    throw std::runtime_error (
		 "Size of NEW_NORMALS incorrect in Mesh::add_normals");

  vertex_normals.insert (vertex_normals.end(),
			 new_normals.begin(), new_normals.end());
}

// Add all the normal vectors described by NEW_NORMALS as vertex
// normals in this mesh, corresponding to all the vertices starting
// from BASE_VERT (which should be a value returned from an earlier
// call to Mesh::add_vertices).
//
// NEW_NORMALS should contain three elements for each normal, to be
// used as the x, y, and z components of the normal.
//
void
Mesh::add_normals (const std::vector<sdist_t> &new_normals,
		   vert_index_t base_vert)
{
  unsigned num_new_normals = new_normals.size () / 3;

  // Not sure what to do if normals after BASE_VERT already exist, of
  // if vertices before BASE_VERT don't have normals yet, so just barf
  // in those cases.
  //
  if (base_vert != vertex_normals.size ())
    throw std::runtime_error ("BASE_VERT incorrect in Mesh::add_normals");
  if (base_vert + num_new_normals != vertices.size ())
    throw std::runtime_error (
		 "Size of NEW_NORMALS incorrect in Mesh::add_normals");

  // Note that the Pos default constructor does not initialize the Pos
  // members, so the following will grow the Mesh::normals vector,
  // but not give the new elements any value.
  //
  vertex_normals.resize (base_vert + num_new_normals);

  unsigned nni = 0;
  for (unsigned ni = base_vert; ni < base_vert + num_new_normals; ni++)
    {
      vertex_normals[ni]
	= MVec (new_normals[nni], new_normals[nni + 1], new_normals[nni + 2]);
      nni += 3;
    }
}

// Add all the UV values in NEW_UVS as vertex UV values in this mesh,
// corresponding to all the vertices starting from BASE_VERT (which
// should be a value returned from an earlier call to
// Mesh::add_vertices).
//
void
Mesh::add_uvs (const std::vector<UV> &new_uvs, vert_index_t base_vert)
{
  // Not sure what to do if uvs after BASE_VERT already exist, if
  // if vertices before BASE_VERT don't have uvs yet, so just barf
  // in those cases.
  //
  if (base_vert != vertex_uvs.size ())
    throw std::runtime_error ("BASE_VERT incorrect in Mesh::add_uvs");
  if (base_vert + new_uvs.size() != vertices.size ())
    throw std::runtime_error ("Size of NEW_UVS incorrect in Mesh::add_uvs");

  vertex_uvs.insert (vertex_uvs.end(), new_uvs.begin(), new_uvs.end());
}

// Add all the UV values described by NEW_UVS as vertex UV values in
// this mesh, corresponding to all the vertices starting from
// BASE_VERT (which should be a value returned from an earlier call
// to Mesh::add_vertices).
//
// NEW_UVS should contain two elements for each UV.
//
void
Mesh::add_uvs (const std::vector<float> &new_uvs, vert_index_t base_vert)
{
  unsigned num_new_uvs = new_uvs.size () / 2;

  // Not sure what to do if uvs after BASE_VERT already exist, of
  // if vertices before BASE_VERT don't have uvs yet, so just barf
  // in those cases.
  //
  if (base_vert != vertex_uvs.size ())
    throw std::runtime_error ("BASE_VERT incorrect in Mesh::add_uvs");
  if (base_vert + num_new_uvs != vertices.size ())
    throw std::runtime_error ("Size of NEW_UVS incorrect in Mesh::add_uvs");

  // Note that the UV default constructor does not initialize the UV
  // members, so the following will grow the Mesh::uvs vector, but not
  // give the new elements any value.
  //
  vertex_uvs.resize (base_vert + num_new_uvs);

  unsigned nui = 0;
  for (unsigned ui = base_vert; ui < base_vert + num_new_uvs; ui++)
    {
      vertex_uvs[ui] = UV (new_uvs[nui], new_uvs[nui + 1]);
      nui += 2;
    }
}



// Mesh::Part::add_triangles


// Add new triangles to this mesh part using vertices from
// TRI_VERT_INDICES.  TRI_VERT_INDICES should contain three entries
// for each new triangle; the indices in TRI_VERT_INDICES are
// relative to BASE_VERT (which should be a value returned from an
// earlier call to Mesh::add_vertices).
//
void
Mesh::Part::add_triangles (const std::vector<vert_index_t> &tri_vert_indices,
			   vert_index_t base_vert)
{
  unsigned num_tris = tri_vert_indices.size () / 3;

  triangles.reserve (triangles.size() + num_tris);

  unsigned tvi_num = 0;
  for (unsigned t = 0; t < num_tris; t++)
    {
      triangles.push_back (
		  Triangle (*this,
			    base_vert + tri_vert_indices[tvi_num + 0],
			    base_vert + tri_vert_indices[tvi_num + 1],
			    base_vert + tri_vert_indices[tvi_num + 2]));
      tvi_num += 3;
    }
}



// Mesh::Part::Triangle::IsecInfo


class Mesh::Part::Triangle::IsecInfo : public Surface::Renderable::IsecInfo
{
public:

  IsecInfo (const Ray &ray, const Triangle &_triangle, dist_t _u, dist_t _v)
    : Surface::Renderable::IsecInfo (ray), triangle (_triangle), u (_u), v (_v)
  { }

  virtual Intersect make_intersect (const Media &media, RenderContext &context)
    const;

  virtual Vec normal () const;

private:

  // Return a normal frame FRAME at ORIGIN, with basis vectors
  // calculated from the normal NORM.
  //
  Frame make_frame (const Pos &orgin, const Vec &norm) const;

  const Triangle &triangle;
  dist_t u, v;
};

// Return a normal frame FRAME at ORIGIN, with basis vectors calculated
// from the normal NORM.
//
Frame
Mesh::Part::Triangle::IsecInfo::make_frame (const Pos &origin, const Vec &norm) const
{
  const Mesh &mesh = triangle.part.mesh;

  // The usual value is NORM x AXIS, where AXIS is an arbitrary axis
  // vector.  This yields a value for S that's pointing "around" AXIS,
  // but will fail if NORM is the same as AXIS (so for instance, if AXIS
  // is "up", then horizontal faces in the mesh will fail).
  //
  Vec s = cross (norm, mesh.axis);

  // Handle degenerate case where NORM == AXIS (making the cross-product zero).
  //
  if (s.length_squared() < Eps)
    {
      // CENT is a vector pointing towards the mesh bounding-box center.
      //
      Vec cent = midpoint (mesh._bbox.min, mesh._bbox.max) - origin;

      // Try to use the value (CENT x NORM) for S.  This helps keep the
      // direction of S consistent for the whole mesh.  However that
      // also will fail if NORM == CENT.
      //
      s = cross (norm, cent);

      // If that failed too, give up and use an arbitrary tangent
      // vector.
      //
      if (s.length_squared() < Eps)
	s = norm.perpendicular ();
    }

  s = s.unit ();		// normalize S

  // Calculate the second tangent vector.  This one is much easier... :-)
  //
  Vec t = cross (s, norm);

  return Frame (origin, s, t, norm);
}

// Create an Intersect object for this intersection.
//
Intersect
Mesh::Part::Triangle::IsecInfo::make_intersect (const Media &media, RenderContext &context)
  const
{
  // Point of intersection.
  //
  Pos point = ray.end ();

  // Our geometric frame uses the real surface geometry.
  //
  Frame geom_frame = make_frame (point, triangle.raw_normal ());

  // Calculate the normal frame; if the mesh contains vertex normal
  // information, calculate it by interpolating our vertex normals,,
  // otherwise just copy the geometric frame.
  //
  Frame normal_frame;
  if (! triangle.part.mesh.vertex_normals.empty ())
    {
      Vec norm = triangle.vnorm(0) * (1 - u - v);
      norm += triangle.vnorm(1) * u;
      norm += triangle.vnorm(2) * v;
      norm = norm.unit ();	// normalize

      // Make the normal consistent with the geometry.  Since the
      // geometric normal depends on the handedness of the mesh, which
      // is easy to get wrong, we assume the interpolated ("shading")
      // normal is probably the correct one, and flip the geometric
      // normal.
      //
      if (geom_frame.to (norm).z < 0)
	geom_frame.z = -geom_frame.z;

      normal_frame = make_frame (point, norm);
    }
  else
    normal_frame = geom_frame;

  // Calculate 2d texture-coordinates for POINT (as opposed to the
  // "raw" triangle UV value in the variables "u" and "v").
  //
  UV T0, dTdu, dTdv;
  triangle.get_texture_params (T0, dTdu, dTdv);
  UV T = T0 + dTdu * u + dTdv * v;

  //
  // We calculate the texture-coordinate partial derivatives as:
  //
  //    dT/ds = du/ds * dT/du + dv/ds * dT/dv
  //    dT/dt = du/dt * dT/du + dv/dt * dT/dv
  //
  // e1 and e2 are the coordinate deltas of triangle edges 1 (vertex 0 -
  // vertex 1) and 2 (vertex 0 - vertex 2), in normal space, so "x" is
  // really "s", "y", is "t", and "z" is normal to the surface:
  //   
  //                            e2.y + e2.z
  //    du/ds = -------------------------------------------
  //            e1.x * (e2.y + e2.z) - e2.x * (e1.y + e1.z)
  //   
  //                               -e2.x
  //    du/dt = -------------------------------------------
  //            e1.x * (e2.y + e2.z) - e2.x * (e1.y + e1.z)
  //   
  //                            e1.y + e1.z
  //    dv/ds = -------------------------------------------
  //            e2.x * (e1.y + e1.z) - e1.x * (e2.y + e2.z)
  //   
  //                               -e1.x
  //    dv/dt = -------------------------------------------
  //            e2.x * (e1.y + e1.z) - e1.x * (e2.y + e2.z)
  //
  // and dT/du and dT/dv are basically the deltas of texture coordinates
  // for edges 1 and 2.
  //

  // Edge coordinate deltas in normal space.
  //
  Vec e1_w = triangle.v(1) - triangle.v(0); // triangle edge 1 in world space
  Vec e2_w = triangle.v(2) - triangle.v(0); // triangle edge 2 in world space
  Vec e1 = normal_frame.to (e1_w);	      // edge 1 in normal space
  Vec e2 = normal_frame.to (e2_w);	      // edge 2 in normal space

  // Calculate du/ds, du/dt, dv/ds, and dv/dt.
  //
  dist_t du_den = e1.x * (e2.y + e2.z) - e2.x * (e1.y + e1.z);
  dist_t inv_du_den = du_den == 0 ? 0 : 1 / du_den;
  dist_t duds = (e2.y + e2.z) * inv_du_den;
  dist_t dudt = -e2.x * inv_du_den;
  dist_t dv_den = e2.x * (e1.y + e1.z) - e1.x * (e2.y + e2.z);
  dist_t inv_dv_den = dv_den == 0 ? 0 : 1 / dv_den;
  dist_t dvds = (e1.y + e1.z) * inv_dv_den;
  dist_t dvdt = -e1.x * inv_dv_den;

  // Calculate texture coordinate partial derivatives, in normal space.
  //
  UV dTds = dTdu * duds + dTdv * dvds;
  UV dTdt = dTdu * dudt + dTdv * dvdt;

  // Make the intersect object.
  //
  return Intersect (ray, media, context, *triangle.part.material,
		    normal_frame, geom_frame, T, dTds,dTdt);
}

// Return the normal of this intersection (in the world frame).
//
Vec
Mesh::Part::Triangle::IsecInfo::normal () const
{
  // XXX is geometric normal enough?
  return triangle.raw_normal ();
}



// Mesh::Part::Triangle intersection

// If this surface intersects RAY, change RAY's maximum bound
// (Ray::t1) to reflect the point of intersection, and return a
// Surface::Renderable::IsecInfo object describing the intersection
// (which should be allocated using placement-new with CONTEXT);
// otherwise return zero.
//
const Surface::Renderable::IsecInfo *
Mesh::Part::Triangle::intersect (Ray &ray, RenderContext &context) const
{
  // We have to convert the types to match that of RAY first.
  //
  Pos corner = v(0);
  Vec edge1 = v(1) - corner, edge2 = v(2) - corner;

  dist_t t, u, v;
  if (triangle_intersects (corner, edge1, edge2, ray, t, u, v))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, *this, u, v);
    }

  return 0;
}

// Return true if this surface intersects RAY.
//
bool
Mesh::Part::Triangle::intersects (const Ray &ray, RenderContext &) const
{
  // We have to convert the types to match that of RAY first.
  //
  Pos corner = v(0);
  Vec edge1 = v(1) - corner, edge2 = v(2) - corner;

  dist_t t, u, v;
  return triangle_intersects (corner, edge1, edge2, ray, t, u, v);
}

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
bool
Mesh::Part::Triangle::occludes (const Ray &ray, const Medium &medium,
			  Color &total_transmittance,
			  RenderContext &)
  const
{
  // We have to convert the types to match that of RAY first.
  //
  Pos corner = v(0);
  Vec edge1 = v(1) - corner, edge2 = v(2) - corner;

  dist_t t, u, v;
  if (triangle_intersects (corner, edge1, edge2, ray, t, u, v))
    {
      // Avoid unnecessary calculation if possible.
      if (part.material->fully_occluding ())
	return true;

      IsecInfo isec_info (Ray (ray, t), *this, u, v);
      if (part.material->occlusion_requires_tex_coords ())
	{
	  UV T0, dTdu, dTdv;
	  get_texture_params (T0, dTdu, dTdv);
	  UV T = T0 + dTdu * u + dTdv * v;

	  TexCoords tex_coords (ray (t), T);

	  return part.material->occludes (isec_info, tex_coords, medium,
					  total_transmittance);
	}
      else
	return part.material->occludes (isec_info, medium, total_transmittance);
    }

  return false;
}



// misc Mesh::Part::Triangle methods

// Return a bounding box for this surface.
//
BBox
Mesh::Part::Triangle::bbox () const
{
  BBox bbox (v (0));
  bbox += v (1);
  bbox += v (2);
  return bbox;
}



// Mesh::Part::add_to_space


// Add Surface::Renderable objects associated with this mesh part to
// the space being built by SPACE_BUILDER.
//
void
Mesh::Part::add_to_space (SpaceBuilder &space_builder) const
{
  for (unsigned i = 0; i < triangles.size(); i++)
    {
      const Triangle &tri = triangles[i];

      // Degenerate triangles (those with a zero-length normal) can
      // cause a crash during rendering, so only add non-degenerate
      // triangles.
      //
      if (tri.raw_normal_unscaled().length_squared() > 0)
	space_builder.add (&tri);
    }
}



// Mesh part-related methods


// Add a new part to the mesh.  Each part has its own material and
// set of faces (all parts share the same vertices, normals, and
// uvs).
//
Mesh::part_index_t
Mesh::add_part (const Ref<const Material> &mat)
{
  part_index_t part = parts.size ();
  parts.push_back (new Part (*this, mat));
  return part;
}


// Add new triangles to mesh part PART, using vertices from
// TRI_VERT_INDICES.  TRI_VERT_INDICES should contain three entries
// for each new triangle; the indices in TRI_VERT_INDICES are
// relative to BASE_VERT (which should be a value returned from an
// earlier call to Mesh::add_vertices).  If there is no part PART,
// an error is signaled.
//
void
Mesh::add_triangles (part_index_t part,
		     const std::vector<vert_index_t> &tri_vert_indices,
		     vert_index_t base_vert)
  const
{
  if (part > parts.size ())
    throw std::runtime_error ("Invalid mesh part index");

  parts[part]->add_triangles (tri_vert_indices, base_vert);
}


// Return a pointer to the material for mesh part PART.
//
// Note that this doesn't add a reference to the material, and is
// only valid while the mesh still exists (as the mesh holds
// references to all of its materials).
//
const Material *
Mesh::material (part_index_t part) const
{
  return &*parts[part]->material;
}




namespace { // keep local to file

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
  std::vector<Group> groups;
};

} // namespace


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
  float cos_fg;
  if (group.num_faces == 0
      || (cos_fg = cos_angle (face_normal, group.normal) >= min_cos))
    //
    // It fits, add FACE_NORMAL to GROUP, and return VERTEX.
    {
      group.num_faces++;
      group.normal_sum += face_normal;
      group.normal = group.normal_sum.unit ();
      return vertex;
    }
  else if (cos_fg < 0)
    {
      throw std::runtime_error (
		   "Face orientation mismatch while smoothing mesh");
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
  unsigned num_parts = parts.size ();
  unsigned num_old_norms = vertex_normals.size();

  if (num_old_norms < num_verts)
    {
      VertNormGroups norm_groups (*this, max_angle, num_old_norms);

      for (part_index_t part = 0; part < num_parts; part++)
	{
	  std::vector<Part::Triangle> &triangles = parts[part]->triangles;
	  unsigned num_triangles = triangles.size ();

	  for (unsigned t = 0; t < num_triangles; t++)
	    {
	      Part::Triangle &triang = triangles[t];
	      const MVec norm (triang.raw_normal ());

	      // Find a vertex normal group for this triangle's normal.
	      // We just update each vertex index to refer to the vertex to
	      // which the normal was added.
	      //
	      for (unsigned num = 0; num < 3; num++)
		if (triang.vi[num] >= num_old_norms)
		  triang.vi[num] = norm_groups.add_face (norm, triang.vi[num]);
	    }
	}

      // The number of vertices may have increased due to vertex splitting.
      //
      num_verts = vertices.size ();

      vertex_normals.resize (num_verts);

      for (vert_index_t v = num_old_norms; v < num_verts; v++)
	vertex_normals[v] = norm_groups[v].normal;
    }
}



// misc Mesh methods


Mesh::~Mesh ()
{
  for (std::vector<Part *>::const_iterator pi = parts.begin ();
       pi != parts.end (); ++pi)
    delete *pi;
}


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
Mesh::add_to_space (SpaceBuilder &space_builder) const
{
  // Shrink the allocated space in the various vectors to the amount
  // actually needed.
  //

  if (!quiet && vertices.size () > 50000)
    std::cout << "* adding large mesh: "
	      << commify (vertices.size ()) << " vertices"
	      << ", " << commify (num_triangles ()) << " triangles"
	      << std::endl;

  for (std::vector<Part *>::const_iterator pi = parts.begin ();
       pi != parts.end (); ++pi)
    (*pi)->add_to_space (space_builder);
}


// Return the number of triangles in all mesh parts.
//
unsigned
Mesh::num_triangles () const
{
  unsigned num_tris = 0;

  for (std::vector<Part *>::const_iterator pi = parts.begin ();
       pi != parts.end (); ++pi)
    num_tris += (*pi)->triangles.size ();

  return num_tris;
}


// Add statistics about this surface to STATS (see the definition of
// Surface::Stats below for details).  CACHE is used internally for
// coordination amongst nested surfaces.
//
// This method is intended for internal use in the Surface class
// hierachy, but cannot be protected: due to pecularities in the way
// that is defined in C++.
//
void
Mesh::accum_stats (Stats &stats, StatsCache &) const
{
  unsigned num_tris = num_triangles ();
  stats.num_render_surfaces += num_tris;
  stats.num_real_surfaces += num_tris;
}

// Recalculate this mesh's bounding box.
//
void
Mesh::recalc_bbox ()
{
  unsigned num_verts = vertices.size ();

  if (num_verts > 0)
    {
      _bbox = vertex (0);

      for (vert_index_t v = 1; v < num_verts; v++)
	_bbox += vertex (v);
    }
  else
    _bbox = BBox ();		// empty (degenerate) bbox
}


// Transform the geometry of this surface by XFORM.
//
void
Mesh::transform (const Xform &xform)
{
  const SXform xf = SXform (xform);

  for (vert_index_t v = 0; v < vertices.size (); v++)
    vertices[v].transform (xf);

  if (vertex_normals.size() > 0)
    {
      // Calculate a variant of XFORM suitable for transforming
      // normals.
      //
      SXform norm_xf = xf.inverse().transpose();

      for (vert_index_t v = 0; v < vertex_normals.size (); v++)
	vertex_normals[v].transform (norm_xf);
    }

  if (xform.reverses_handedness ())
    left_handed = !left_handed;

  // Recalculate the bounding-box based on the transformed vertices.
  //
  // We could also just transform the old bounding-box by XFORM, but
  // recalculating it from scratch will probably give a tighter
  // bounding box (because transforming the bounding-box has less
  // information, and so needs to be more conservative).
  //
  recalc_bbox ();
}


// arch-tag: 3090c323-f2dd-48ef-b8fc-20ce5d687c66
