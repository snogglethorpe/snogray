// tessel.h -- Surface tessellation
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_H__
#define __TESSEL_H__

#include <map>
#include <utility>		// for std::pair

#include "pos.h"
#include "vec.h"
#include "llist.h"
#include "freelist.h"

namespace Snogray {

// State during tessellation of some function
//
class Tessel
{
public:

  class Function;
  class MaxErrCalc;

  // A tessellation of FUN, where MAX_ERR_CALC is used to calculate the
  // permissible error at a given location.
  //
  Tessel (const Function &fun, const MaxErrCalc &_max_err_calc);

  // Distance of a point on the tessellation from the actual point on
  // the curve.
  //
  typedef double err_t;

  // This type should be overridden to supply as an argument to the constructor.
  //
  struct MaxErrCalc
  {
    virtual err_t max_err (const Pos &pos) const = 0;
    virtual ~MaxErrCalc ();	// make gcc shut up
  };

  // A simple version of MaxErrCalc that always returns a constant error
  //
  struct ConstMaxErr : MaxErrCalc
  {
    ConstMaxErr (err_t _err) : err (_err) { }
    virtual err_t max_err (const Pos &pos) const;
    err_t err;
  };

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
  class Vertex : public LinkedList<Vertex>::Node
  {
  public:

    Vertex (const Pos &_pos) : pos (_pos) { }
    Vertex (coord_t x, coord_t y, coord_t z) : pos (Pos (x, y, z)) { }

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

//     // Only subclasses of Vertex can be constructed!  We guarantee
//     // to subclasses of Tessel that all vertices will be those it
//     // supplies, which allows the Tessel subclass to freely subclass
//     // Vertex, and use static_cast<VertexSubClass &>(vert) to convert
//     // any vertex it sees to that type.
//     //
//     Vertex (const Pos &_pos) : pos (_pos) { }
  };

  // A Function is a class that defines a surface for tessellation
  //
  class Function
  {
  public:

    virtual ~Function ();	// make gcc shut up

    // If the function can compute vertex normals too, it may use these
    // methods to communicate them.
    //
    virtual bool has_vertex_normals () const;
    virtual Vec vertex_normal (const Vertex &vert) const;

  protected:

    friend class Tessel;

    // Define the initial basis edges in TESSEL.
    //
    virtual void define_basis (Tessel &tessel) const = 0;

    // Return a position on the surface close to POS.  This is the basic
    // operation used during tessellation.
    //
    virtual Pos surface_pos (const Pos &pos) const = 0;

    // Returns the desired sample resolution needed, given a certain error
    // limit.
    //
    virtual dist_t sample_resolution (err_t max_err) const = 0;

    //
    // The following methods export private Tessel methods for the use of
    // Function subclasses.
    //

    // For subclasses to use to add new vertices they've constructed.
    //
    Vertex *add_vertex (Tessel &tessel, const Pos &pos) const
    {
      return tessel.add_vertex (pos);
    }
    Vertex *add_vertex (Tessel &tessel, coord_t x, coord_t y, coord_t z) const
    {
      return tessel.add_vertex (x, y, z);
    }

    // Add a triangular cell with the given vertices
    //
    void add_cell (Tessel &tessel,
		   const Vertex *v1, const Vertex *v2, const Vertex *v3)
      const
    {
      tessel.add_cell (v1, v2, v3);
    }
  };
  friend class Function;

  //
  // The following methods define iterators for retrieving the results
  // of tessellation.
  //

  // Return the total number of vertices in the tessellation; vertex
  // `index' fields will be in the range 0 - (NUM_VERTICES - 1).
  //
  unsigned num_vertices () const { return vertices.size (); }

  // Return the total number of triangles in the tessellation.
  //
  unsigned num_triangles () const { return cells.size (); }

  // Iterators for retrieving vertices
  //
  LinkedList<Vertex>::iterator vertices_begin () const { return vertices.begin (); }
  LinkedList<Vertex>::iterator vertices_end () const { return vertices.end (); }

private: // forward decl for Cell must have same access as real decl
  class Cell;
public:

  // An iterator for iterating over every triangle
  //
  class TriangleIter
  {
  public:

    TriangleIter (const LinkedList<Cell>::iterator &_cell_iter)
      : cell_iter (_cell_iter)
    { }

    bool operator== (const TriangleIter &ti) const
    { return cell_iter == ti.cell_iter; }
    bool operator!= (const TriangleIter &ti) const
    { return cell_iter != ti.cell_iter; }

    TriangleIter &operator++ () { ++cell_iter; return *this; }
    TriangleIter &operator++ (int) { cell_iter++; return *this; }

    const Vertex &vert1 () { return *cell_iter->e1->beg; }
    const Vertex &vert2 () { return *cell_iter->e2->beg; }
    const Vertex &vert3 () { return *cell_iter->e3->beg; }

  private:

    LinkedList<Cell>::iterator cell_iter;
  };
  friend class TriangleIter;

  TriangleIter triangles_begin () const { return cells.begin(); }
  TriangleIter triangles_end () const { return cells.end(); }


private:

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
  Vertex *add_vertex (const Pos &pos)
  {
    Vertex *v = new (free_vertices) Vertex (pos);
    return vertices.append (v);
  }
  Vertex *add_vertex (coord_t x, coord_t y, coord_t z)
  {
    return add_vertex (Pos (x, y, z));
  }
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
  struct Subdiv : public LinkedList<Subdiv>::Node
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

  // Delete the subdiv tree TREE; if FREE_VERTICES is true, also free any
  // vertices it references.
  //
  void prune (Subdiv *tree, bool free_vertices);


  //
  // Edges
  //

  // An edge of a triangle in the tessellated surface
  //
  struct Edge : public LinkedList<Edge>::Node
  {
    Edge (const Vertex *vert1, const Vertex *vert2,
	  Subdiv *_subdiv, Subdiv *_rev_subdiv, err_t _err);

    // Returns true if this edge has no further subdivisions.
    //
    bool simple () const { return !subdiv; }

    // Returns the midpoint of this edge; the edge must be non-simple.
    //
    const Vertex *midpoint () const;

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
  struct Cell : public LinkedList<Cell>::Node
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
  Freelist<Vertex> free_vertices;

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

}

#endif /* __TESSEL_H__ */

// arch-tag: 8c445032-6d14-4ff7-a415-a1ecf2683d7c
