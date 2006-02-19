// color.h -- Color datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iomanip>

#include "color.h"
#include "coords.h" // for lim()

using namespace Snogray;

const Color Color::black (0,0,0);
const Color Color::white (1,1,1);
const Color Color::funny (0,10,0);

std::ostream&
Snogray::operator<< (std::ostream &os, const Color &col)
{
  os << "rgb<" << std::setprecision (5) << lim (col.r)
     << ", " << std::setprecision (5) << lim (col.g)
     << ", " << std::setprecision (5) << lim (col.b) << ">";
  return os;
}

// arch-tag: 11e71f8e-3323-473e-95ce-e3e07e6197d8
