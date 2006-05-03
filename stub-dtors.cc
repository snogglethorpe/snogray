// stub-dtors.cc -- Dumping ground for virtual destructors
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


// We accumulate currently useless virtual destructors here, in cases where
// no proper source file is available.


#include "filter.h"
#include "sample2-gen.h"
#include "brdf.h"
#include "light.h"
#include "space.h"


using namespace Snogray;

Filter::~Filter () { }

Sample2Gen::~Sample2Gen () { }

Brdf::~Brdf () { }

Light::~Light () { }

Space::~Space () { }
Space::IntersectCallback::~IntersectCallback () { }


// arch-tag: 7a5f3914-e339-4d52-b400-e3c713c5aa9a
