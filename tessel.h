// tessel.h -- Surface tessellation
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_H__
#define __TESSEL_H__

#include <map>
#include <utility>		// for std::pair
#include <vector>

#include "pos.h"
#include "vec.h"
#include "uv.h"
#include "llist.h"
#include "freelist.h"


namespace snogray {

class Mesh;


// State during tessellation of some function
//
class Tessel
{
public:

  // A Function is a class that defines a surface for tessellation
  //
  class Function;
  friend class Function;

  // Distance of a point on the tessellation from the actual point on
  // the curve.
  //
  typedef dist_t err_t;

  // This type should be overridden to supply as an argument to the constructor.
  //
  struct MaxErrCalc
  {
    virtual err_t max_err (const Pos &pos) const = 0;
    virtual ~MaxErrCalc ();	// make gcc shut up
  };


  // A tessellation of FUN, where MAX_ERR_CALC is used to calculate the
  // permissible error at a given location.
  //
  Tessel (const Function &fun, const MaxErrCalc &_max_err_calc);


  // A simple version of MaxErrCalc that always returns a constant error
  //
  struct ConstMaxErr : MaxErrCalc
  {
    ConstMaxErr (err_t _err) : err (_err) { }
    virtual err_t max_err (const Pos &pos) const;
    err_t err;
  };


  // Add the results of this tessellation to MESH.
  //
  void add_to_mesh (Mesh *mesh);


  // A vertex is a point that's actually on the curve, and can be used
  // as a vertex in the final tessellation.
  //
  // Vertices are completely managed by subclasses of Tessel -- the
  // algorithms in Tessel only ever handle pointers or references to
  // them (the Vertex constructor is private to enforce that
  // constraint), and all vertices are created by the subclass.  Thus
  // Tessel subclasses may assume that any vertex pointer or reference
  // points to a vertex created by them.  In particular this means it's
  // safe to use static_cast to cast a Vertex pointer/reference to
  // whatever subclass of Vertex the Tessel subclass uses to create its
  // vertices.
  //
  class Vertex;

  //
  // For the convenience of Functions in defining the basis, we don't
  // directly export the Edge datatype (which is annoying because you
  // have to worry about direction), but add a layer that allows
  // directly specifying cells in terms of vertices.
  //

  // Add a triangular cell with the given vertices
  //
  void add_cell (const Vertex *v1, const Vertex *v2, const Vertex *v3)
  {
    Edge *e1 = get_edge (v1, v2);
    Edge *e2 = get_edge (v2, v3);
    Edge *e3 = get_edge (v3, v1);

    add_cell (e1, e2, e3);
  }


private:

  // Return the maximum permissible error at VERT (the error may be
  // position dependent).
  //
  err_t max_err (const Pos &pos) const
  {
    return max_err_calc.max_err (pos);
  }


  //
  // Vertices
  //

  // Add and return a new vertex.
  //
  void remove_vertex (Vertex *v)
  {
    vertices.remove (v);
    free_vertices.put (v);
  }


  //
  // Edge subdivision trees
  //

  // A "subdivision" of an edge, yielding two smaller edges that more
  // closely approximate the underlying curve than the parent edge.
  //
  struct Subdiv;

  // Allocate and return a new subdiv.
  //
  Subdiv *add_subdiv (Vertex *mid, dist_t corr, Subdiv *bef, Subdiv *aft,
		      err_t err);

  void remove_subdiv (Subdiv *subdiv);

  // Build a subdivision tree to full resolution between VERT1 and VERT2.
  //
  Subdiv *sample (const Vertex *vert1, const Vertex *vert2);

  // Prune the subdivision tree rooted at TREE, removing any levels that
  // are beneath the error threshold.
  //
  void simplify (Subdiv *&tree);

  // Return a reversed version of SUBDIV.
  //
  Subdiv *reverse (const Subdiv *subdiv);

  // Delete the subdiv tree TREE; if PRUNE_VERTICES is true, also free any
  // vertices it references.
  //
  void prune (Subdiv *tree, bool prune_vertices);


  //
  // Edges
  //

  // An edge of a triangle in the tessellated surface
  //
  struct Edge;

  // Add a new edge to the edge list.
  //
  Edge *add_edge (const Vertex *vert1, const Vertex *vert2,
		  Subdiv *_subdiv, Subdiv *_rev_subdiv, err_t _err);

  // Remove an edge.
  //
  void remove_edge (Edge *edge);

  // Add and return a new root edge from VERT1 to VERT2.  A root edge is
  // one which does not share subdiv structure with any previous edges.
  //
  Edge *add_root_edge (const Vertex *vert1, const Vertex *vert2);

  // Remove a root edge.  The only real difference from `remove_edge' is
  // that we also free the subdiv trees.
  //
  void remove_root_edge (Edge *edge);

  // Add and return a new edge which is the reverse of EDGE.
  //
  Edge *add_reverse_edge (const Edge *edge);

  // Add and return a new edge from EDGE's curve midpoint to its end
  // (EDGE must not be a leaf edge).
  //
  Edge *add_edge_after_midpoint (const Edge *edge);

  // Add and return a new edge from EDGE's beginning to its curve midpoint
  // (EDGE must not be a leaf edge).
  //
  Edge *add_edge_before_midpoint (const Edge *edge);

  // Return an edge from VERT1 to VERT2, creating it if necessary.
  //
  // The vertex->edge mapping is only for the convenience of Functions
  // while defining the basis, and is not maintained at other times
  // (e.g., during structuring).
  //
  Edge *get_edge (const Vertex *vert1, const Vertex *vert2);


  //
  // Cells
  //

  // A triangular "cell" on the surface
  //
  struct Cell;

  // "structure" CELL by recursively subdividing it into sub-cells; if
  // CELL is subdivided, its contents is replaced by that of some
  // (arbitrary) sub-cell.  Thus only leaf cells actually exist.
  //
  void structure (Cell &cell);

  // "structure" all cells by recursively subdividing them into sub-cells.
  // Note that the number of cells may grow during structuring; any newly
  // added cells will also be handled.
  //
  void structure ();

  // Add a new triangular cell with the given edges.
  //
  void add_cell (Edge *e1, Edge *e2, Edge *e3);


  //
  // Data
  //

  // The function that defines the surface being tessellated.
  //
  const Function &fun;

  // Active components of this tessellation.
  //
  LinkedList<Cell> cells;
  LinkedList<Vertex> vertices;

  // Freelists
  //
  Freelist<Edge> free_edges;
  Freelist<Subdiv> free_subdivs;

  // Freelist for vertices.  As the actual vertex type is defined by
  // subclasses of Function, we use a generic freelist here; the subclass
  // should define a "vertex_size" method which returns the appropriate
  // size to initialize this freelist with.
  //
  BlockFreelist free_vertices;

  // This refers to an object supplied by the user, which is used to
  // calculate the permissible error at a given location.
  //
  const MaxErrCalc &max_err_calc;

  // Map vertices into edges, for `get_edge'; this mapping is only for
  // the convenience of Functions while defining the basis, and is not
  // maintained at other times (e.g., during structuring).
  //
  typedef std::map<std::pair<const Vertex *, const Vertex *>, Edge *> VertexEdgeMap;
  VertexEdgeMap edge_map;
};


// Tessel::Vertex

// A vertex is a point that's actually on the curve, and can be used
// as a vertex in the final tessellation.
//
// Vertices are completely managed by subclasses of Tessel -- the
// algorithms in Tessel only ever handle pointers or references to
// them (the Vertex constructor is private to enforce that
// constraint), and all vertices are created by the subclass.  Thus
// Tessel subclasses may assume that any vertex pointer or reference
// points to a vertex created by them.  In particular this means it's
// safe to use static_cast to cast a Vertex pointer/reference to
// whatever subclass of Vertex the Tessel subclass uses to create its
// vertices.
//
class Tessel::Vertex : public LinkedList<Tessel::Vertex>::Node
{
public:

  // Mainly so we can use Vertex as a map key
  //
  bool operator< (const Vertex &v2) const { return pos < v2.pos; }

  // Vertex's position in space
  //
  Pos pos;

  // An integer index in the range 0 - (NUM_VERTICES - 1).  This is soley
  // for the use of external users, so may not be valid until after
  // tessellation is over; as subclasses of Tessel manage vertices, it
  // is their responsibility to manage index numbers as well.
  //
  unsigned index;

protected:

  // Only subclasses of Vertex can be constructed!  We guarantee
  // to subclasses of Tessel that all vertices will be those it
  // supplies, which allows the Tessel subclass to freely subclass
  // Vertex, and use static_cast<VertexSubClass &>(vert) to convert
  // any vertex it sees to that type.
  //
  Vertex (const Pos &_pos) : pos (_pos) { }
};


// Tessel::Function

// A Function is a class that defines a surface for tessellation
//
class Tessel::Function
{
public:

  virtual ~Function () { }	// make gcc shut up

  // Tesselate this function and add the results to MESH, using
  // _MAX_ERR_CALC to calculate the maximum allowable error.
  //
  void tessellate (Mesh *mesh, const MaxErrCalc &_max_err_calc) const;

  // Tesselate this function and add the results to MESH, with a maximum
  // allowable error of MAX_ERR.
  //
  void tessellate (Mesh *mesh, err_t max_err) const
  {
    tessellate (mesh, ConstMaxErr (max_err));
  }

  // Add normal vectors for the vertices in the list from VERTICES_BEG
  // to VERTICES_END, to NORMALS.
  //
  virtual void get_vertex_normals (LinkedList<Vertex>::iterator vertices_beg,
				   LinkedList<Vertex>::iterator vertices_end,
				   std::vector<SVec> &normals)
    const = 0;

  // Add UV values for the vertices in the list from VERTICES_BEG
  // to VERTICES_END, to UVS.
  //
  virtual void get_vertex_uvs (LinkedList<Vertex>::iterator /*vertices_beg*/,
			       LinkedList<Vertex>::iterator /*vertices_end*/,
			       std::vector<UV> &/*uvs*/)
    const
  {
  }

protected:

  friend class Tessel;

  // Add the vertex VERT to TESSEL (and return it).
  //
  const Vertex *add_vertex (Tessel &tessel, Vertex *vert) const
  {
    return tessel.vertices.append (vert);
  }

  // Add a triangular cell with the given vertices to TESSEL.
  //
  void add_cell (Tessel &tessel,
		 const Vertex *v1, const Vertex *v2, const Vertex *v3)
  {
    tessel.add_cell (v1, v2, v3);
  }

  // Subclasses can call this to allocate a vertex from TESSEL's vertex
  // freelist.
  //
  void *alloc_vertex (Tessel &tessel) const
  {
    return tessel.free_vertices.get ();
  }

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const = 0;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  virtual dist_t sample_resolution (err_t max_err) const = 0;

  // Add to TESSEL and return a new vertex which is on this function's
  // surface midway between VERT1 and VERT2 (for some definition of
  // "midway").  This is the basic operation used during tessellation.
  // VERT1 and VERT2 are guaranteed to have come from either the original
  // basis defined by `define_basis', or from a previous call to
  // `midpoint'; thus it is safe for subclasses to down-cast them to
  // whatever Vertex subclass they use.
  //
  virtual Vertex *midpoint (Tessel &tessel,
			    const Vertex *vert1, const Vertex *vert2)
    const = 0;

  // The size of vertex objects used by this Function (which should be a
  // subclass of Tessel::Vertex).
  //
  virtual size_t vertex_size () const = 0;

  //
  // The following methods export private Tessel methods for the use of
  // Function subclasses.
  //

  // Add a triangular cell with the given vertices
  //
  void add_cell (Tessel &tessel,
		 const Vertex *v1, const Vertex *v2, const Vertex *v3)
    const
  {
    tessel.add_cell (v1, v2, v3);
  }
};


// Tessel::Edge

// An edge of a triangle in the tessellated surface
//
struct Tessel::Edge : public LinkedList<Tessel::Edge>::Node
{
  Edge (const Vertex *vert1, const Vertex *vert2,
	Subdiv *_subdiv, Subdiv *_rev_subdiv, err_t _err);

  // Returns true if this edge has no further subdivisions.
  //
  bool simple () const { return !subdiv; }

  // Returns the midpoint of this edge; the edge must be non-simple.
  //
  const Vertex *midpoint () const;

  dist_t length () const { return (end->pos - beg->pos).length(); }

  // Beginning and end points of this edge.
  //
  const Vertex *beg, *end;

  // A tree representing finer subdivisions of this edge to more closely
  // approximate the underlying curve.
  //
  Subdiv *subdiv;

  // SUBDIV in reverse order (from END to BEG).
  //
  Subdiv *reverse_subdiv;

  // Total error of the curve approximation
  //
  err_t err;

  //     // For ease in calculating triangle aspect ratios, keep tract of the
  //     // edge's length; it's obviously just (end - beg).length(), but
  //     // let's not keep calculating sqrt all the time!
  //     //
  //     dist_t length;
};


// Tessel::Cell

struct Tessel::Cell : public LinkedList<Tessel::Cell>::Node
{
  Cell (Edge *_e1, Edge *_e2, Edge *_e3)
    : e1 (_e1), e2 (_e2), e3 (_e3)
  { }

  // Replace all the edge pointers in this cell
  //
  void set_edges (Edge *_e1, Edge *_e2, Edge *_e3)
  {
    e1 = _e1; e2 = _e2; e3 = _e3;
  }

  Edge *e1, *e2, *e3;
};


// Tessel::Subdiv

struct Tessel::Subdiv : public LinkedList<Tessel::Subdiv>::Node
{
  Subdiv (Vertex *mid, dist_t corr, Subdiv *_bef_mid, Subdiv *_aft_mid,
	  err_t _err);

  // The actual point on the curve corresponding (roughly) to the
  // midpoint of the parent edge.
  //
  Vertex *curve_midpoint;

  // The distance between the parent edge's midpoint to CURVE_MIDPOINT
  // (i.e., from the "line midpoint" to the "curve midpoint".
  //
  dist_t midpoint_correction;

  // Finer subdivisions (if any) before and after CURVE_MIDPOINT.
  //
  Subdiv *bef_mid, *aft_mid;

  // Partial error of this branch of the subdivision tree, defined as
  // max (bef_mid.err, aft_mid.err)
  //
  err_t err;
};


}

#endif /* __TESSEL_H__ */


// arch-tag: 8c445032-6d14-4ff7-a415-a1ecf2683d7c
