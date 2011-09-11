// photon.h -- Packet of light energy
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PHOTON_H
#define SNOGRAY_PHOTON_H

#include "pos.h"
#include "color.h"


namespace snogray {


class Photon
{
public:

  // The default constructor does nothing, to allow quickly allocating
  // vectors of photons.  (so be careful)
  //
  Photon () { }

  // Create a photon which arrived at position _POS, _from_ position
  // _DIR (i.e., _DIR points in the direction where it came from), and
  // has power _POWER.
  //
  Photon (const Pos &_pos, const Vec &_dir, const Color &_power)
    : pos (_pos), power (_power), dir (_dir)
  { }

  // Position in space.
  //
  Pos pos;

  // Photon power (light intensity).
  //
  Color power;

  // Direction from which this photon came.
  //
  Vec dir;
};


}

#endif // SNOGRAY_PHOTON_H
