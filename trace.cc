// trace.cc -- Tracing path
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

#include "material.h"
#include "intersect.h"

#include "trace.h"

using namespace snogray;


// Constructor for a sub-trace eminating from SOURCE_ISEC in the
// direction of RAY.  TRANSMISSIVE should be true if RAY is entering
// the surface rather than being reflected from it (this information is
// theoretically possible to calculate by looking at the dot-product of
// RAY's direction with SOURCE_ISEC's surface normal, but such a
// calculation can be unreliable in edge cases due to precision errors).
// If supplied, BRANCH_FACTOR gives the complexity of the sub-trace
// compared to the prevous trace.
//
Trace::Trace (const Intersect &source_isec, const Ray &_ray,
	      bool transmissive, float branch_factor)
  : source (&source_isec.trace),
    type (transmissive
	  ? (source_isec.back ? REFRACTION_OUT : REFRACTION_IN)
	  : REFLECTION),
    ray (_ray),
    complexity (source->complexity * branch_factor),
    medium (transmissive ? refraction_medium (source_isec) : source->medium)
{ }

// Searches back through the trace history to find the enclosing medium.
//
const Medium &
Trace::enclosing_medium (const Medium &default_medium) const
{
  const Trace *ts = this;

  int enclosure_level = 0;

  while (enclosure_level >= 0 && ts)
    {
      if (ts->type == REFRACTION_IN)
	enclosure_level--;
      else if (ts->type == REFRACTION_OUT)
	enclosure_level++;

      ts = ts->source;
    }

  return ts ? ts->medium : default_medium;
}

// Return an appropriate medium for a refractive ray entering or
// leaving a surface from SOURCE_ISEC.
//
const Medium &
Trace::refraction_medium (const Intersect &source_isec)
{
  const Medium &default_medium = source_isec.context.default_medium;

  if (source_isec.back)
    return source_isec.trace.enclosing_medium (default_medium);
  else
    {
      const Medium *medium = source_isec.material->medium ();
      if (medium)
	return *medium;
      else
	return default_medium;
    }
}


// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
