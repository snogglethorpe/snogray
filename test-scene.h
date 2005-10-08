// test-scene.h -- Simple test-scene importing
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEST_SCENE_H__
#define __TEST_SCENE_H__

#include "scene.h"
#include "camera.h"

namespace Snogray {

struct TestSceneMaterialParams
{
  float r, g, b;		// color
  float Kd, Ks;			// diffuse, specular reflection
  float exp;			// phong exponent
};

struct TestSceneVertexParams
{
  float x, y, z;
};

struct TestSceneTriangleParams
{
  unsigned mat_index;
  unsigned v0i, v1i, v2i;
};

struct TestSceneSphereParams
{
  unsigned mat_index;
  float x, y, z;
  float r;
};

struct TestSceneLightParams
{
  float x, y, z;
  float r, g, b;
};

struct TestSceneParams
{
  const TestSceneMaterialParams *material_params;
  unsigned num_materials;

  const TestSceneVertexParams   *vertex_params;
  unsigned num_vertices;

  const TestSceneTriangleParams *triangle_params;
  unsigned num_triangles;

  const TestSceneSphereParams   *sphere_params;
  unsigned num_spheres;

  const TestSceneLightParams   *light_params;
  unsigned num_lights;

  float bg_r, bg_g, bg_b;

  float cam_x, cam_y, cam_z;
  float cam_targ_x, cam_targ_y, cam_targ_z;
  float cam_up_x, cam_up_y, cam_up_z;
  float cam_fov_y;
};

extern void define_test_scene (const TestSceneParams *params,
			       float light_intens,
			       Scene &scene, Camera &camera);

}

#endif /* __TEST_SCENE_H__ */

// arch-tag: 45908baa-bb50-4140-ae1b-87df63f22863
