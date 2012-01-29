// photopic-glare-psf.h -- Photopic glare point-spread-function
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

#ifndef SNOGRAY_PHOTOPIC_GLARE_PSF_H
#define SNOGRAY_PHOTOPIC_GLARE_PSF_H

#include "glare-psf.h"


namespace snogray {


// Glare point-spread-function (PSF) based on the human visual system
// in non-dark-adapted ("photopic") state.
//
class PhotopicGlarePsf : public GlarePsf
{
public:

  PhotopicGlarePsf () { }

  // Return the value of the PSF at an angle of THETA radians from the
  // central axis.
  //
  virtual float operator() (float theta) const;
};


}

#endif // SNOGRAY_PHOTOPIC_GLARE_PSF_H
