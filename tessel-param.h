// tessel-param.h -- Parametric tessellation functions
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

#ifndef __TESSEL_PARAM_H__
#define __TESSEL_PARAM_H__

#include "tessel.h"

namespace snogray {

class ParamTesselFun : public Tessel::Function
{
public:

  typedef dist_t param_t;

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
    param_t mid = (p1 + p2) * 0.5f;
    if (abs (p1 - p2) > 0.5f)
      mid += 0.5f;
    if (mid >= 1)
      mid -= 1;
    return mid;
  }

  // Add to TESSEL and return a vertex with parameter values U and V.
  // The position of the new vertex is automatically calculated using the
  // `surface_pos' method.
  //
  Vertex *add_vertex (Tessel &tessel, param_t u, param_t v) const
  {
    Pos pos = surface_pos (u, v);
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

  // Return true if this function can calculate vertex normals.
  // If so, the `vertex_normal' method will be called to get each vertex's
  // normal.  Note that the default is true, so this method need only be
  // overridden for subclasses which _can't_ calculate vertex normals.
  //
  virtual bool can_calc_vertex_normals () const;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const = 0;
};



class SphereTesselFun : public ParamTesselFun
{
public:

  SphereTesselFun (Pos _origin, dist_t _radius, dist_t perturb = 0)
    : origin (_origin), radius (_radius), radius_perturb (perturb)
  { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  virtual dist_t sample_resolution (Tessel::err_t max_err) const;

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

  // Return the surface position corresponding to the parameters U, V.
  //
  virtual Pos surface_pos (param_t u, param_t v) const;

  // Return true if this function can calculate vertex normals.
  // If so, the `vertex_normal' method will be called to get each vertex's
  // normal.  Note that the default is true, so this method need only be
  // overridden for subclasses which _can't_ calculate vertex normals.
  //
  virtual bool can_calc_vertex_normals () const;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const;

private:

  Pos origin;
  dist_t radius;
  dist_t radius_perturb;
};

// Simple interface to SphereTesselFun
//
inline void
tessel_sphere (Mesh *mesh, const Pos &origin, dist_t radius, dist_t max_err)
{
  SphereTesselFun fun (origin, radius);
  fun.tessellate (mesh, max_err);
}



class SincTesselFun : public ParamTesselFun
{
public:

  SincTesselFun (Pos _origin, dist_t _radius)
    : origin (_origin), radius (_radius)
  { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  virtual dist_t sample_resolution (Tessel::err_t max_err) const;

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

  // Return the surface position corresponding to the parameters U, V.
  //
  virtual Pos surface_pos (param_t u, param_t v) const;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const;

private:

  Pos origin;
  dist_t radius;
};

// Simple interface to SincTesselFun
//
inline void
tessel_sinc (Mesh *mesh, const Pos &origin, dist_t radius, dist_t max_err)
{
  SincTesselFun fun (origin, radius);
  fun.tessellate (mesh, max_err);
}



class TorusTesselFun : public ParamTesselFun
{
public:

  TorusTesselFun (Pos _origin, dist_t _radius, dist_t _hole_radius,
		  dist_t perturb = 0)
    : origin (_origin), radius (_radius), hole_radius (_hole_radius),
      radius_perturb (perturb)
  { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  virtual dist_t sample_resolution (Tessel::err_t max_err) const;

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

  // Return the surface position corresponding to the parameters U, V.
  //
  virtual Pos surface_pos (param_t u, param_t v) const;

  // Return true if this function can calculate vertex normals.
  // If so, the `vertex_normal' method will be called to get each vertex's
  // normal.  Note that the default is true, so this method need only be
  // overridden for subclasses which _can't_ calculate vertex normals.
  //
  virtual bool can_calc_vertex_normals () const;

  // Return the surface normal corresponding for VERTEX.
  // Subclass should override this method.
  //
  virtual Vec vertex_normal (const Vertex &vertex) const;

private:

  Pos origin;
  dist_t radius, hole_radius;
  dist_t radius_perturb;
};

// Simple interface to TorusTesselFun
//
inline void
tessel_torus (Mesh *mesh, const Pos &origin, dist_t radius, dist_t hole_radius,
	      dist_t max_err)
{
  TorusTesselFun fun (origin, radius, hole_radius);
  fun.tessellate (mesh, max_err);
}


}

#endif /* __TESSEL_PARAM_H__ */

// arch-tag: 9854c54e-f31a-43bb-9927-94bf75c8efaf
