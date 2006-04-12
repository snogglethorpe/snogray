// load-3ds.h -- Load 3ds scene file
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LOAD_3DS_H__
#define __LOAD_3DS_H__

#include <string>

#include "xform.h"

namespace Snogray {

class Scene;
class Camera;
class Mesh;

// Load a 3ds scene file into SCENE and CAMERA; loads all parts of the
// scene, including lights and the first camera position.
//
extern void load_3ds_file (const std::string &filename,
			   Scene &scene, Camera &camera);

// Load meshes (and any materials they use) from a 3ds scene file into
// MESH.  Geometry is first transformed by XFORM.
//
extern void load_3ds_file (const std::string &filename,
			   Mesh &mesh, const Xform &xform);

}

#endif /* __LOAD_3DS_H__ */

// arch-tag: e4ac5abf-2695-4f3f-99c3-16f723ec0bef
