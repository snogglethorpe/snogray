// mesh.h -- Mesh object
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

#include "primary-obj.h"
#include "pos.h"

namespace Snogray {

class Mesh : public PrimaryObj
{
public:

  // A vertex group can be used to group vertices together.
  //
  typedef std::map<Pos, unsigned> VertexGroup;
  typedef std::map<std::pair<Pos, Vec>, unsigned> VertexNormalGroup;

  Mesh (const Material *mat) : PrimaryObj (mat), triangles (0, *this) { }
  Mesh (const Material *mat, const char *file_name)
    : PrimaryObj (mat), triangles (0, *this)
  { load (file_name); }

  // Add a triangle to the mesh
  //
  void add_triangle (unsigned v0i, unsigned v1i, unsigned v2i);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2);
  void add_triangle (const Pos &v0, const Pos &v1, const Pos &v2,
		     VertexGroup &vgroup);

  // Add a vertex to the mesh
  //
  unsigned add_vertex (const Pos &pos);
  unsigned add_vertex (const Pos &pos, VertexGroup &vgroup);

  // Add a vertex with normal to the mesh
  //
  unsigned add_vertex (const Pos &pos, const Vec &normal);
  unsigned add_vertex (const Pos &pos, const Vec &normal,
		       VertexNormalGroup &vgroup);

  // For loading mesh from any file-type (automatically determined)
  //
  void load (const char *file_name);
  void load (const std::string &file_name) { load (file_name.c_str ()); }

  // For loading mesh from .msh file
  //
  void load_msh_file (std::istream &stream);

  // Add this (or some other ...) objects to SPACE
  //
  virtual void add_to_space (Voxtree &space);

  //
  void compute_vertex_normals ();

  //private:

  class Triangle : public Obj
  {
  public:

    Triangle (const Mesh &_mesh) : mesh (_mesh) { }
    Triangle (const Mesh &_mesh, unsigned v0i, unsigned v1i, unsigned v2i)
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

    virtual dist_t intersection_distance (const Ray &ray) const;

    // Returns the normal vector for this surface at POINT.
    // INCOMING is the direction of the incoming ray that has hit POINT;
    // this can be used by dual-sided objects to decide which side's
    // normal to return.
    //
    virtual Vec normal (const Pos &point, const Vec &incoming) const;

    // Return a bounding box for this object.
    //
    virtual BBox bbox () const;

    // Returns the material this object is made from
    //
    virtual const Material *material () const;

    // Vertex NUM of this triangle
    //
    const Pos &v (unsigned num) const { return mesh.vertices[vi[num]]; }

    // Normal of vertex NUM (assuming this mesh contains vertex normals!)
    //
    const Vec &vnorm (unsigned num) const { return mesh.vertex_normals[vi[num]];}

    const Vec raw_normal () const
    {
      return ((v(1) - v(0)).cross (v(2) - v(1))).unit ();
    }

    const Mesh &mesh;

    // Indices into mesh vertices array
    //
    unsigned vi[3];
  };

  // A list of vertices used in this part.
  std::vector<Pos> vertices;
  std::vector<Vec> vertex_normals;

  // A vector of Mesh::Triangle objects that use this part.
  std::vector<Triangle> triangles;
};

}

#endif /* __MESH_H__ */

// arch-tag: e9987003-cf70-4bd5-b30c-90620e2317ad
