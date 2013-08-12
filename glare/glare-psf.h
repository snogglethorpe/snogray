// glare-psf.h -- Glare point-spread-function
//
//  Copyright (C) 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_GLARE_PSF_H
#define SNOGRAY_GLARE_PSF_H


namespace snogray {


// Glare point-spread-function (PSF).
//
class GlarePsf
{
public:

  GlarePsf () { }

  // Return the value of the PSF at an angle of THETA radians from the
  // central axis.
  //
  virtual float operator() (float theta) const = 0;
};


}

#endif // SNOGRAY_GLARE_PSF_H
