// camera-cmds.h -- Parsing of camera command strings
//
//  Copyright (C) 2005, 2006, 2007, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_CAMERA_CMDS_H
#define SNOGRAY_CAMERA_CMDS_H

#include <string>

namespace snogray {

class Camera;
class Scene;

extern void interpret_camera_cmds (const std::string &cmds, Camera &camera,
				   const Scene &scene);

}

#endif // SNOGRAY_CAMERA_CMDS_H

// arch-tag: ae7f1cd0-dd0d-4ca3-8cf1-b60b85ba218c
