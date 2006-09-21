// camera-cmds.h -- Parsing of camera command strings
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CAMERA_CMDS_H__
#define __CAMERA_CMDS_H__

#include <string>

namespace Snogray {

class Camera;
class Scene;

extern void interpret_camera_cmds (const std::string &cmds, Camera &camera,
				   const Scene &scene);

}

#endif // __CAMERA_CMDS_H__

// arch-tag: ae7f1cd0-dd0d-4ca3-8cf1-b60b85ba218c
