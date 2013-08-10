// tessel-param.h -- Parametric tessellation functions
//
//  Copyright (C) 2005-2008, 2011-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TESSEL_PARAM_H
#define SNOGRAY_TESSEL_PARAM_H

#include "geometry/xform.h"

#include "tessel.h"


namespace snogray {


class ParamTesselFun : public Tessel::Function
{
public:

  typedef dist_t param_t;

  ParamTesselFun (const Xform &_xform);

protected:

  class Vertex : public Tessel::Vertex
  {
  public:

    Vertex (param_t _u, param_t _v, const Pos &pos)
      : Tessel::Vertex (pos), u (_u), v (_v)
    { }

    param_t u, v;
  };

  // Return a parameter value halfway between P1 and P2, given that
  // parameter values wrap around from 1 to 0.  The nearest of the two
  // possible midpoints chosen.
  //
  param_t wrapped_midpoint (param_t p1, param_t p2) const
  {
    param_t mid = (p1 + p2) * param_t (0.5);
    if (abs (p1 - p2) > param_t (0.5))
      mid += param_t (0.5);
    if (mid >= 1)
      mid -= 1;
    return mid;
  }

  // Add to TESSEL, and return, a vertex with parameter values U and V.
  // The position of the new vertex is automatically calculated using the
  // `surface_pos' method.
  //
  Vertex *add_vertex (Tessel &tessel, param_t u, param_t v) const;

  // Add a triangular cell with the given vertices to TESSEL.
  //
  void add_cell (Tessel &tessel,
		 const Vertex *v1, const Vertex *v2, const Vertex *v3)
    const
  {
    if (reversed_handedness)
      {
	const Vertex *tmp = v2;
	v2 = v3;
	v3 = tmp;
      }

    Tessel::Function::add_cell (tessel, v1, v2, v3);
  }

  // Add to TESSEL and return a new vertex which is on this function's
  // surface midway between VERT1 and VERT2 (for some definition of
  // "midway").  This is the basic operation used during tessellation.
  // VERT1 and VERT2 are guaranteed to have come from either the original
  // basis defined by `define_basis', or from a previous call to
  // `midpoint'; thus it is safe for subclasses to down-cast them to
  // whatever Vertex subclass they use.
  //
  virtual Tessel::Vertex *midpoint (Tessel &tessel,
				    const Tessel::Vertex *vert1,
				    const Tessel::Vertex *vert2)
    const;

  // The size of vertex objects used by this Function (which should be a
  // subclass of Tessel::Vertex).
  //
  virtual size_t vertex_size () const;

  // Add normal vectors for the vertices in the list from VERTICES_BEG
  // to VERTICES_END, to NORMALS.
  //
  virtual void get_vertex_normals (
		 LinkedList<Tessel::Vertex>::iterator vertices_beg,
		 LinkedList<Tessel::Vertex>::iterator vertices_end,
		 std::vector<SVec> &normals)
    const;

  // Add UV values for the vertices in the list from VERTICES_BEG to
  // VERTICES_END, to UVS.
  //
  virtual void get_vertex_uvs (
		 LinkedList<Tessel::Vertex>::iterator vertices_beg,
		 LinkedList<Tessel::Vertex>::iterator vertices_end,
		 std::vector<UV> &uvs)
    const;

  //
  // Methods to be overridden by subclasses
  //

  // Return the surface position corresponding to the parameters U, V.
  // Subclass should override this method.
  //
  virtual Pos surface_pos (param_t u, param_t v) const = 0;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const = 0;



  // Object to world-space transformation.
  //
  Xform xform;

  // True if XFORM reverses handedness.
  //
  bool reversed_handedness;
};


}

#endif /* SNOGRAY_TESSEL_PARAM_H */

// arch-tag: 9854c54e-f31a-43bb-9927-94bf75c8efaf
