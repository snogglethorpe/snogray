// lambert.cc -- Lambertian material
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "cos-dist.h"
#include "bsdf.h"

#include "lambert.h"

using namespace snogray;




// The details of lambertian evaluation are in this class.
//
class LambertBsdf : public Bsdf
{
public:

  LambertBsdf (const Lambert &_lambert, const Intersect &_isec)
    : Bsdf (_isec), color (_lambert.color.eval (_isec))
  { }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    if ((flags & (REFLECTIVE|DIFFUSE)) == (REFLECTIVE|DIFFUSE))
      {
	float pdf;
	Vec dir = dist.sample (param, pdf);
	if (isec.cos_n (dir) > 0 && isec.cos_geom_n (dir) > 0)
	  return Sample (color * INV_PIf, pdf, dir, REFLECTIVE|DIFFUSE);
      }
    return Sample ();
  }

  // Evaluate this BSDF in direction DIR, and return its value and pdf.
  //
  virtual Value eval (const Vec &dir) const
  {
    float pdf = dist.pdf (isec.cos_n (dir));
    return Value (color * INV_PIf, pdf);
  }

  // Return a bitmask of flags from Bsdf::Flags, describing what types
  // of scatting this BSDF supports.  The returned value will include
  // only flags in LIMIT (default, all flags).
  //
  // The various fields (Bsdf::SURFACE_CLASS, Bsdf::SAMPLE_DIR) in the
  // returned value should be consistent -- a surface-class like
  // Bsdf::DIFFUSE should be included if that surface-class is supported
  // by one of the sample-directions (e.g. Bsdf::REFLECTIVE) that's also
  // included in the returned value, and vice-versa.
  //
  virtual unsigned supports (unsigned limit) const
  {
    return (limit & REFLECTIVE) ? ((REFLECTIVE | DIFFUSE) & limit) : 0;
  }

private:

  Color color;

  CosDist dist;
};


// Make a BSDF object for this material instantiated at ISEC.
//
Bsdf *
Lambert::get_bsdf (const Intersect &isec) const
{
  return new (isec) LambertBsdf (*this, isec);
}


// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
