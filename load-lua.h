// load-lua.h -- Load lua scene file
//
//  Copyright (C) 2007-2008, 2010-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LOAD_LUA_H
#define SNOGRAY_LOAD_LUA_H

#include <string>


namespace snogray {

class Scene;
class Camera;
class Mesh;
class ValTable;


// If FILENAME is a format that has a Lua scene loader, load the file named
// FILENAME into SCENE and CAMERA using Lua, and return true; if FILENAME
// is unrecogized, return false.  If an error occurs during loading, an
// exception is thrown.  Loads all parts of the scene, including lights and
// the first camera position.
//
extern bool load_lua_file (const std::string &filename,
			   Scene &scene, Camera &camera, ValTable &params);


// Cleanup and free all global Lua state.
//
extern void cleanup_load_lua_state ();


}

#endif // SNOGRAY_LOAD_LUA_H


// arch-tag: 3952dbec-b73a-40ac-92a9-38382f1e720e
