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

namespace Snogray {

class Scene;
class Camera;

extern void load_3ds_file (const std::string &filename,
			   Scene &scene, Camera &camera);

}

#endif /* __LOAD_3DS_H__ */

// arch-tag: e4ac5abf-2695-4f3f-99c3-16f723ec0bef
