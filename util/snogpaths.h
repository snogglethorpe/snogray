// snogpaths.h -- Handling of installation paths
//
//  Copyright (C) 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SNOGPATHS_H
#define SNOGRAY_SNOGPATHS_H

#include <string>


namespace snogray {

std::string installed_prefix ();
std::string installed_bindir ();
std::string installed_pkgdatadir ();

}

#endif // SNOGRAY_SNOGPATHS_H
