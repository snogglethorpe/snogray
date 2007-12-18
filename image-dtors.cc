// image-dtors.cc -- Dumping ground for virtual destructors
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


// We accumulate currently useless virtual destructors here, in cases where
// no proper source file is available.

#include "filter.h"

using namespace snogray;

Filter::~Filter () { }

// arch-tag: 0e9eaf9e-23a8-4df7-a623-9b3b8818c3c5
