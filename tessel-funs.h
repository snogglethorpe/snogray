// tessel-funs.h -- Tessellation surface functions
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TESSEL_FUNS_H__
#define __TESSEL_FUNS_H__

#include "tessel.h"

namespace Snogray {

class SphereTesselFun : public Tessel::Function
{
public:

  SphereTesselFun (Pos _origin, dist_t _radius, dist_t perturb = 0)
    : origin (_origin), radius (_radius), radius_perturb (perturb)
  { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Return a position on the surface close to POS.  This is the basic
  // operation used during tessellation.
  //
  Pos surface_pos (const Pos &pos) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  dist_t sample_resolution (Tessel::err_t max_err) const;

  // If the subclass can compute vertex normals too, it may use these
  // methods to communicate them.
  //
  virtual bool has_vertex_normals () const;
  virtual Vec vertex_normal (const Tessel::Vertex &vert) const;

private:

  Pos origin;
  dist_t radius;
  dist_t radius_perturb;
};

class SincTesselFun : public Tessel::Function
{
public:

  SincTesselFun (Pos _origin, dist_t _radius)
    : origin (_origin), radius (_radius)
  { }

protected:

  // Define the initial basis edges in TESSEL.
  //
  virtual void define_basis (Tessel &tessel) const;

  // Return a position on the surface close to POS.  This is the basic
  // operation used during tessellation.
  //
  Pos surface_pos (const Pos &pos) const;

  // Returns the desired sample resolution needed, given a certain error
  // limit.
  //
  dist_t sample_resolution (Tessel::err_t max_err) const;

private:

  Pos origin;
  dist_t radius;
};

}

#endif /* __TESSEL_FUNS_H__ */

// arch-tag: 9854c54e-f31a-43bb-9927-94bf75c8efaf
