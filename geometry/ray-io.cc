// ray-io.cc -- Debugging output for Ray type
//
//  Copyright (C) 2005, 2007, 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <ostream>

#include "util/nice-io.h"

#include "ray-io.h"


std::ostream&
snogray::operator<< (std::ostream &os, const Ray &ray)
{
  const Pos &o = ray.origin;
  const Vec &d = ray.dir;

  os << "ray{origin = {";
  output_nicely (os, o.x);
  os << ", ";
  output_nicely (os, o.y);
  os << ", ";
  output_nicely (os, o.z);
  os << "}, dir = {";
  output_nicely (os, d.x);
  os << ", ";
  output_nicely (os, d.y);
  os << ", ";
  output_nicely (os, d.z);
  os << "}, t0 = ";
  output_nicely (os, ray.t0);
  os << ", t1 = ";
  output_nicely (os, ray.t1);
  os << "}";

  return os;
}


// arch-tag: b1d9b9a4-ee16-451b-b341-20265628a715
