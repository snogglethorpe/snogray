// num-cores-var.h -- return the number of cores on this system
//
//  Copyright (C) 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#ifndef SNOGRAY_NUM_CORES_H
#define SNOGRAY_NUM_CORES_H


namespace snogray {


// If the number of available CPU cores on this system can be
// determined, return it, otherwise return DEFAULT_CORES.
//
extern int num_cores (int default_cores);


}


#endif // SNOGRAY_NUM_CORES_H
