// light.cc -- Light
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "color.h"
#include "ray.h"
#include "intersect.h"
#include "brdf.h"
#include "trace-state.h"
#include "scene.h"

#include "light.h"

using namespace Snogray;

Light::~Light () { } // stop gcc bitching

// arch-tag: 3915e032-063a-4bbf-aa37-c4bbaba9f8b1
