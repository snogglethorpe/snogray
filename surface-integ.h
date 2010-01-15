// surface-integ.h -- Light integrator interface for surfaces
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SURFACE_INTEG_H__
#define __SURFACE_INTEG_H__

#include "integ.h"


namespace snogray {


class SurfaceInteg : public Integ
{
protected:

  SurfaceInteg (SampleSet &samples, RenderContext &_context)
    : Integ (samples, _context)
  { }
};


}

#endif // __SURFACE_INTEG_H__
