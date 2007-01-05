// image-dtors.cc -- Dumping ground for virtual destructors
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

using namespace snogray;

Filter::~Filter () { }

// arch-tag: 0e9eaf9e-23a8-4df7-a623-9b3b8818c3c5
