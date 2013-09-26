// recursive-integ.h -- Superclass for simple recursive surface integrators
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RECURSIVE_INTEG_H
#define SNOGRAY_RECURSIVE_INTEG_H

#include "surface-integ.h"


namespace snogray {


// This is an abstract superclass for surface-integrators that use simple
// recursion to handle perfectly specular surfaces.  This class will handle
// the recursion for such surfaces and also any direct emission by
// surfaces, and call a subclass-provided method to handle other lighting.
//
class RecursiveInteg : public SurfaceInteg
{
public:

  // Return the light arriving at RAY's origin, from points up until
  // its end.  MEDIA is the media environment through which the ray
  // travels.
  //
  // This method also calls the volume-integrator's Li method, and
  // includes any light it returns for RAY as well.
  //
  // "Li" means "Light incoming".
  //
  virtual Tint Li (const Ray &ray, const Media &media,
		   const SampleSet::Sample &sample);

protected:

  // Integrator state for rendering a group of related samples.
  //
  RecursiveInteg (RenderContext &context) : SurfaceInteg (context) { }

  // This method should be provided by subclasses, and should return any
  // radiance not due to specular reflection/transmission or direct
  // emission.
  //
  virtual Color Lo (const Intersect &isec, const Media &media,
		    const SampleSet::Sample &sample)
    = 0;

private:

  // Return the light arriving at RAY's origin, from points up until
  // its end.  MEDIA is the media environment through which the ray
  // travels.
  //
  // This method also calls the volume-integrator's Li method, and
  // includes any light it returns for RAY as well.
  //
  // "Li" means "Light incoming".
  //
  // This an internal variant of Integ::lo which has an additional DEPTH
  // argument.  If DEPTH is greater than some limit, recursion will
  // stop.  It also returns a Color instead of a Tint, as alpha values
  // are only meaningful at the the top-level.
  //
  Color Li (const Ray &ray, const Media &media,
	    const SampleSet::Sample &sample,
	    unsigned depth);

  // Return the light emitted from ISEC.
  //
  Color Lo (const Intersect &isec, const Media &media,
	    const SampleSet::Sample &sample, unsigned depth);
};


}

#endif // SNOGRAY_RECURSIVE_INTEG_H
