// test-scenes.h -- Test scenes for snogray ray tracer
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEST_SCENES_H__
#define __TEST_SCENES_H__

#include <string>

#include "scene.h"
#include "camera.h"

namespace Snogray {

extern void def_test_scene (const std::string &name,
			    Scene &scene, Camera &camera);

}

#endif /* __TEST_SCENES_H__ */

// arch-tag: c5d7b6ea-cabf-4249-847b-9e584435e58c
