// load-lua.h -- Load lua scene file
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
class MaterialDict;


// If FILENAME is a format that has a Lua scene loader, load the file named
// FILENAME into SCENE and CAMERA using Lua, and return true; if FILENAME
// is unrecogized, return false.  If an error occurs during loading, an
// exception is thrown.  Loads all parts of the scene, including lights and
// the first camera position.
//
extern bool load_lua_file (const std::string &filename, const std::string &fmt,
			   Scene &scene, Camera &camera);

// If FILENAME is a format that has a Lua mesh loader, load the file named
// FILENAME into MESH using Lua, and return true; if FILENAME is
// unrecogized, return false.  If an error occurs during loading, an
// exception is thrown.
//
extern bool load_lua_file (const std::string &filename, const std::string &fmt,
			   Mesh &mesh, const MaterialDict &mat_dict);

}


#endif /* __LOAD_LUA_H__ */

// arch-tag: 3952dbec-b73a-40ac-92a9-38382f1e720e
