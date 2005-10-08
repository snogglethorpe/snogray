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

#include "vec.h"
#include "pos.h"

std::ostream&
operator<< (std::ostream &os, const Vec &vec)
{
  os << "vec<" << vec.x << ", " << vec.y << ", " << vec.z << ">";
  return os;
}

std::ostream&
operator<< (std::ostream &os, const Pos &pos)
{
  os << "pos<" << pos.x << ", " << pos.y << ", " << pos.z << ">";
  return os;
}

// arch-tag: 2ddaa14a-cfab-4e4e-a4e3-a0349f3eb748
