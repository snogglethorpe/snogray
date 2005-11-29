// tessel-param.h -- Parametric tessellation functions
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_PARAM_H__
#define __TESSEL_PARAM_H__

#include "tessel.h"

namespace Snogray {

class ParamTesselFun : public Tessel::Function
{
public:

  typedef double param_t;

protected:

  class Vertex : public Tessel::Vertex
  {
  public:

    Vertex (param_t _u, param_t _v, const Pos &pos)
      : Tessel::Vertex (pos), u (_u), v (_v)
    { }

    param_t u, v;
  };

  // Calculate a midpoint between two angular parameters.
  //
  param_t angular_midpoint (param_t p1, param_t p2) const
  {
    param_t mid = (p1 + p2) / 2;
    param_t diff = p1 - p2;
    if (diff >= M_PI || diff <= -M_PI)
      mid += M_PI;
    if (mid >= M_PI * 2)
      mid -= M_PI * 2;
    else if (mid <= -M_PI * 2)
      mid += M_PI * 2;
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
		 std::vector<Vec> &normals)
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

}

#endif /* __TESSEL_PARAM_H__ */

// arch-tag: 9854c54e-f31a-43bb-9927-94bf75c8efaf
