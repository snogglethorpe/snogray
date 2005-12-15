// mesh.h -- Mesh surface
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

#include "primary-surface.h"
#include "tessel.h"
#include "pos.h"

namespace Snogray {

class Tessel;

class Mesh : public PrimarySurface
{
public:

  // A vertex group can be used to group vertices together.
  //
  typedef std::map<SPos, unsigned> VertexGroup;
  typedef std::map<std::pair<SPos, SVec>, unsigned> VertexNormalGroup;

  Mesh (const Material *mat) : PrimarySurface (mat), triangles (0, *this) { }

  // All-in-one constructor for loading a mesh from a file.
  //
  Mesh (const Material *mat, const std::string file_name, bool smooth = false)
    : PrimarySurface (mat), triangles (0, *this)
  {
    load (file_name);
    if (smooth)
      compute_vertex_normals ();
  }

  // All-in-one constructor for a tessellated mesh.
  //
  Mesh (const Material *mat, const Tessel::Function &tessel_fun,
	const Tessel::MaxErrCalc &max_err, bool smooth = false)
    : PrimarySurface (mat), triangles (0, *this)
  {
    add (tessel_fun, max_err, smooth);
  }

  // Add a triangle to the mesh
  //
  void add_triangle (unsigned v0i, unsigned v1i, unsigned v2i);
  void add_triangle (const SPos &v0, const SPos &v1, const SPos &v2);
  void add_triangle (const SPos &v0, const SPos &v1, const SPos &v2,
		     VertexGroup &vgroup);

  // Add a vertex to the mesh
  //
  unsigned add_vertex (const SPos &pos);
  unsigned add_vertex (const SPos &pos, VertexGroup &vgroup);

  // Add a vertex with normal to the mesh
  //
  unsigned add_vertex (const SPos &pos, const SVec &normal);
  unsigned add_vertex (const SPos &pos, const SVec &normal,
		       VertexNormalGroup &vgroup);

  // Add the results of tessellating TESSEL_FUN with MAX_ERR.
  //
  void add (const Tessel::Function &tessel_fun,
	    const Tessel::MaxErrCalc &max_err,
	    bool smooth = true);

  // For loading mesh from any file-type (automatically determined)
  //
  void load (const char *file_name);
  void load (const std::string &file_name) { load (file_name.c_str ()); }

  // For loading mesh from .msh file
  //
  void load_msh_file (std::istream &stream);

  // Add this (or some other ...) surfaces to SPACE
  //
  virtual void add_to_space (Voxtree &space);

  //
  void compute_vertex_normals ();

  SPos vertex (unsigned index) { return vertices[index]; }
  SPos vertex_normal (unsigned index) { return vertex_normals[index]; }

  //private:

  class Triangle : public Surface
  {
  public:

    Triangle (const Mesh &_mesh)
      : Surface (_mesh.material()->shadow_type ()), mesh (_mesh)
    { }
    Triangle (const Mesh &_mesh, unsigned v0i, unsigned v1i, unsigned v2i)
      : Surface (_mesh.material()->shadow_type ()), mesh (_mesh)
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
				      const IsecParams &isec_params)
      const;

    // Return true if RAY would hit the back of this surface.
    //
    virtual bool back (const Ray &ray) const;

    // Return a bounding box for this surface.
    //
    virtual BBox bbox () const;

    // Returns the material this surface is made from
    //
    virtual const Material *material () const;

    // The "smoothing group" this surface belongs to, or zero if it belongs
    // to none.  The smoothing group affects shadow-casting: if two objects
    // are in the same smoothing group, they will not be shadowed by
    // back-surface shadows from each other; typically all triangles in a
    // mesh are in the same smoothing group.
    //
    virtual const void *smoothing_group () const;

    // Vertex NUM of this triangle
    //
    const SPos &v (unsigned num) const { return mesh.vertices[vi[num]]; }

    // Normal of vertex NUM (assuming this mesh contains vertex normals!)
    //
    const SVec &vnorm (unsigned num) const { return mesh.vertex_normals[vi[num]];}

    // These both return the "raw" normal of this triangle, not doing
    // any normal interpolation.
    //
    const SVec raw_normal_unscaled () const
    {
      return (v(1) - v(0)).cross (v(2) - v(1));
    }
    const SVec raw_normal () const
    {
      return raw_normal_unscaled().unit ();
    }

    const Mesh &mesh;

    // Indices into mesh vertices array
    //
    unsigned vi[3];
  };

  // A list of vertices used in this part.
  std::vector<SPos> vertices;
  std::vector<SVec> vertex_normals;

  // A vector of Mesh::Triangle surfaces that use this part.
  std::vector<Triangle> triangles;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
