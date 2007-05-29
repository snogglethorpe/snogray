// load-lua.h -- Load lua scene file
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LOAD_LUA_H__
#define __LOAD_LUA_H__

#include <string>

namespace snogray {

class Scene;
class Camera;
class Mesh;

// Load a lua scene file into SCENE and CAMERA.
//
extern void load_lua_file (const std::string &filename,
			   Scene &scene, Camera &camera);

}

#endif /* __LOAD_LUA_H__ */

// arch-tag: 3952dbec-b73a-40ac-92a9-38382f1e720e
