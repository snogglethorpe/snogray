// load-3ds.h -- Load 3ds scene file
//
//  Copyright (C) 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LOAD_3DS_H
#define SNOGRAY_LOAD_3DS_H

#include <string>

namespace snogray {

class Scene;
class Camera;
class Mesh;
class MaterialDict;

// Load a 3ds scene file into SCENE and CAMERA; loads all parts of the
// scene, including lights and the first camera position.
//
extern void load_3ds_file (const std::string &filename,
			   Scene &scene, Camera &camera);

// Load meshes (and any materials they use) from a 3ds scene file into MESH.
//
extern void load_3ds_file (const std::string &filename, Mesh &mesh);

}

#endif /* SNOGRAY_LOAD_3DS_H */

// arch-tag: e4ac5abf-2695-4f3f-99c3-16f723ec0bef
