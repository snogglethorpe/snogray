// test-scenes.h -- Test scenes for snogray ray tracer
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEST_SCENES_H__
#define __TEST_SCENES_H__

#include <vector>
#include <string>

#include "scene.h"
#include "camera.h"

namespace snogray {

extern void def_test_scene (const std::string &name,
			    Scene &scene, Camera &camera);

struct TestSceneDesc
{
  TestSceneDesc (const std::string &_name, const std::string &_desc)
    : name (_name), desc (_desc)
  { }

  std::string name, desc;
};

extern std::vector<TestSceneDesc> list_test_scenes ();

// Blerg
//
extern float tessel_accur;
extern bool tessel_smooth;

}

#endif /* __TEST_SCENES_H__ */

// arch-tag: c5d7b6ea-cabf-4249-847b-9e584435e58c
