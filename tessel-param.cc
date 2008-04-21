// tessel-param.cc -- Tessellation of parametric surfaces
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "tessel-param.h"

using namespace snogray;


ParamTesselFun::ParamTesselFun (const Xform &_xform)
  : xform (_xform), reversed_handedness (xform.reverses_handedness ())
{ }

// The size of vertex objects used by this Function (which should be a
// subclass of Tessel::Vertex).
//
size_t
ParamTesselFun::vertex_size () const
{
  return sizeof (Vertex);
}

// Add to TESSEL, and return, a vertex with parameter values U and V.
// The position of the new vertex is automatically calculated using the
// `surface_pos' method.
//
ParamTesselFun::Vertex *
ParamTesselFun::add_vertex (Tessel &tessel, param_t u, param_t v) const
{
  Pos pos = surface_pos (u, v) * xform;
  Vertex *vert = new (alloc_vertex (tessel)) Vertex (u, v, pos);
  Tessel::Function::add_vertex (tessel, vert);
  return vert;
}

// Add to TESSEL and return a new vertex which is on this function's
// surface midway between VERT1 and VERT2 (for some definition of
// "midway").  This is the basic operation used during tessellation.
// VERT1 and VERT2 are guaranteed to have come from either the original
// basis defined by `define_basis', or from a previous call to
// `midpoint'; thus it is safe for subclasses to down-cast them to
// whatever Vertex subclass they use.
//
Tessel::Vertex *
ParamTesselFun::midpoint (Tessel &tessel,
			  const Tessel::Vertex *tvert1,
			  const Tessel::Vertex *tvert2)
  const
{
  const Vertex *vert1 = static_cast<const Vertex *>(tvert1);
  const Vertex *vert2 = static_cast<const Vertex *>(tvert2);

  return
    add_vertex (tessel, (vert1->u + vert2->u) / 2, (vert1->v + vert2->v) / 2);
}

// Add normal vectors for the vertices in the list from VERTICES_BEG
// to VERTICES_END, to NORMALS.
//
void ParamTesselFun::get_vertex_normals (
		       LinkedList<Tessel::Vertex>::iterator vertices_beg,
		       LinkedList<Tessel::Vertex>::iterator vertices_end,
		       std::vector<SVec> &normals)
  const
{
  Xform norm_xform = xform.inverse ().transpose ();

  for (LinkedList<Tessel::Vertex>::iterator vi = vertices_beg;
       vi != vertices_end; vi++)
    {
      const Vertex &vert = static_cast<const Vertex &>(*vi);
      normals.push_back (SVec (vertex_normal (vert) * norm_xform).unit ());
    }
}

// Add UV values for the vertices in the list from VERTICES_BEG to
// VERTICES_END, to UVS.
//
void
ParamTesselFun::get_vertex_uvs (
	       LinkedList<Tessel::Vertex>::iterator vertices_beg,
	       LinkedList<Tessel::Vertex>::iterator vertices_end,
	       std::vector<UV> &uvs)
  const
{
  for (LinkedList<Tessel::Vertex>::iterator vi = vertices_beg;
       vi != vertices_end; vi++)
    {
      const Vertex &vert = static_cast<const Vertex &>(*vi);
      uvs.push_back (UV (vert.u, vert.v));
    }
}


// arch-tag: e33bedf6-51a0-4e6b-a25b-7238ee36da1a
