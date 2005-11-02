// space.cc -- Some miscellaneous functions for Vec and Pos datatypes
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iomanip>

#include "vec.h"
#include "pos.h"

using namespace Snogray;

inline coord_t lim (coord_t v) { return (v < Eps && v > -Eps) ? 0 : v; }

std::ostream&
Snogray::operator<< (std::ostream &os, const Vec &vec)
{
  os << "vec<" << std::setprecision (5) << lim (vec.x)
     << ", " << std::setprecision (5) << lim (vec.y)
     << ", " << std::setprecision (5) << lim (vec.z)
     << ">";
  return os;
}

std::ostream&
Snogray::operator<< (std::ostream &os, const Pos &pos)
{
  os << "pos<" << std::setprecision (5) << lim (pos.x)
     << ", " << std::setprecision (5) << lim (pos.y)
     << ", " << std::setprecision (5) << lim (pos.z)
     << ">";
  return os;
}

// arch-tag: 2ddaa14a-cfab-4e4e-a4e3-a0349f3eb748
