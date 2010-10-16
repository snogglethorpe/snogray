// bsdf.h -- Bi-directional scattering distribution function
//
//  Copyright (C) 2005, 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BSDF_H__
#define __BSDF_H__

#include "color.h"
#include "vec.h"
#include "uv.h"


namespace snogray {

class Intersect;


// A Bsdf object represents the state of a Material object at an
// intersection (a particular point on the surface, viewed from a
// particular direction), and is used to calculate how light scatters
// from the surface.
//
// The Bsdf may be reference-counted using Ref<Bsdf>.  However Bsdf
// objects are not deleted when the reference-count goes to zero, only
// the destructor is called.  This is because Bsdf objects are not
// allocated using the standard allocator.
//
class Bsdf
{
public:

  enum Flags {
    //
    // BSDF sample classification flags:  These classify BSDF samples
    // into a number of different categories.
    //
    // They are used both descriptively, e.g., in the
    // Bsdf::Sample::flags field, and as input arguments for various
    // Bsdf methods (e.g. Bsdf::sample and Bsdf::eval), for describing
    // what sorts of samples are to be considered.
    //

    // Sample directions: reflection, or transmission (through the surface)
    //
    REFLECTIVE		= 0x10,
    TRANSMISSIVE 	= 0x20,
    // Mask for all sample directions
    ALL_DIRECTIONS	= REFLECTIVE|TRANSMISSIVE,

    // BSDF "layers"; these are basically broad classes of BSDF
    // response.  Many BSDFs will actually implement multiple layers in
    // parallel (typically a diffuse layer and a glossy layer).
    //
    SPECULAR		= 0x01,	// perfectly specular (infinitely narrow spike)
    GLOSSY		= 0x02,	// sharp glossy lobe
    DIFFUSE		= 0x04,	// very broad response, no sharp peak
    // Mask for all surface layers
    ALL_LAYERS		= SPECULAR|GLOSSY|DIFFUSE,

    // Mask of all flags
    ALL			= ALL_DIRECTIONS|ALL_LAYERS
  };

  struct Sample
  {
    Sample (const Color &_val, float _pdf, const Vec &_dir,
	    unsigned _flags = 0)
      : val (_val), pdf (_pdf), dir (_dir), flags (_flags)
    { }
    Sample () : val (0), pdf (0), flags (0) { }

    // The value of the BSDF for this sample.
    //
    Color val;

    // The value of the "probability density function" for this
    // sample in the BSDF's sample distribution.
    //
    // However, if this is a specular sample (with the SPECULAR flag
    // set), the value is not defined (theoretically the value is
    // infinity for specular samples).
    //
    float pdf;

    // The sample direction (the origin is implicit), in the
    // surface-normal coordinate system (where the surface normal is
    // (0,0,1)).
    //
    Vec dir;

    // Flags applying to this sample (see the Flags enum for various
    // values).
    //
    unsigned flags;
  };

  struct Value
  {
    Value (const Color &_val, float _pdf)
      : val (_val), pdf (_pdf)
    { }
    Value () : val (0), pdf (0) { }

    // The value of the BSDF for this value.
    //
    Color val;

    // The value of the "probability density function" for this value in the
    // BSDF's value distribution.
    //
    // However, if this is a purely specular bsdf the pdf is not defined
    // (theoretically the value is infinity for specular values).
    //
    float pdf;
  };

  Bsdf (const Intersect &_isec) : isec (_isec), ref_count (0) { }
  virtual ~Bsdf () {}

  // Methods implementing the reference-counting protocol used by Ref<>.
  //
  // Unlike the RefCounted class most reference-counted classes use as a
  // superclass, Bsdf objects are not deleted when the reference-count
  // goes to zero, only the destructor is called.  This is because Bsdf
  // objects are not allocated using the standard allocator.
  //
  void ref () const { ++ref_count; }
  void deref () const { if (--ref_count <= 0) this->~Bsdf (); }

  // Return a sample of this BSDF, based on the parameter PARAM.
  // FLAGS is the types of samples we'd like.
  //
  virtual Sample sample (const UV &param, unsigned flags = ALL) const = 0;

  // Evaluate this BSDF in direction DIR (in the surface-normal
  // coordinate system of the intersection where this BSDF was created),
  // and return its value and pdf.  If FLAGS is specified, then only the
  // given types of surface interaction are considered.
  //
  virtual Value eval (const Vec &dir, unsigned flags = ALL) const = 0;

  // Return a bitmask of flags from Bsdf::Flags, describing what
  // types of scatting this BSDF supports.  The returned value will
  // include only flags in LIMIT (default, all flags).
  //
  // The various fields (Bsdf::ALL_LAYERS, Bsdf::ALL_DIRECTIONS) in
  // the returned value should be consistent -- a layer flag like
  // Bsdf::DIFFUSE should only be included if that layer is
  // supported by one of the sample-directions
  // (e.g. Bsdf::REFLECTIVE) in return value, and vice-versa.
  //
  virtual unsigned supports (unsigned limit = ALL) const = 0;

  // The intersection where this Bsdf was created.
  //
  const Intersect &isec;

private:

  // Reference-count.
  //
  mutable int ref_count;
};


}

#endif // __BSDF_H__


// arch-tag: 8360ddd7-dc17-40b8-8319-8f6d61fe62bf
