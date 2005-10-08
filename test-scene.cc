// test-scene.cc -- Simple test-scene importing
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "test-scene.h"

#include "lambert.h"
#include "phong.h"
#include "material.h"
#include "mirror.h"
#include "triangle.h"
#include "sphere.h"

using namespace Snogray;

// epsilon
#define E 0.0001

// The lighting in SPD-produced scene files has screwed up gamma
#define ASSUMED_GAMMA 2.2

void
Snogray::define_test_scene (const TestSceneParams *p, float light_intens,
			    Scene &scene, Camera &camera)
{
  scene.set_assumed_gamma (2.2);

  Material **materials = 0;
  if (p->num_materials > 0)
    {
      materials = new Material*[p->num_materials];
      for (unsigned i = 0; i < p->num_materials; i++)
	{
	  const TestSceneMaterialParams *mp = &p->material_params[i];

	  // Diffuse color
	  Color diffuse = Color (mp->r, mp->g, mp->b) * mp->Kd;

	  // Specular color
	  const LightModel *lmodel = 0;
	  if (mp->exp <= E || mp->exp > 1000)
	    lmodel = Material::lambert;
	  else
	    lmodel = Material::phong (mp->exp, mp->Ks);

	  // Reflectance
	  if (mp->Ks > E)
	    materials[i] = new Mirror (mp->Ks, diffuse, lmodel);
	  else
	    materials[i] = new Material (diffuse, lmodel);

	  scene.add (materials[i]);
	}
    }

  Pos *vertices = 0;
  if (p->num_vertices > 0)
    {
      vertices = new Pos[p->num_vertices];
      for (unsigned i = 0; i < p->num_vertices; i++)
	{
	  const TestSceneVertexParams *vp = &p->vertex_params[i];
	  vertices[i].x = vp->x;
	  vertices[i].y = vp->y;
	  vertices[i].z = vp->z;
	}
    }

  if (p->num_triangles > 0)
    for (unsigned i = 0; i < p->num_triangles; i++)
      {
	const TestSceneTriangleParams *tp = &p->triangle_params[i];

	scene.add (new Triangle (materials[tp->mat_index],
				 vertices[tp->v0i],
				 vertices[tp->v1i],
				 vertices[tp->v2i]));
      }

  if (p->num_spheres > 0)
    for (unsigned i = 0; i < p->num_spheres; i++)
      {
	const TestSceneSphereParams *sp = &p->sphere_params[i];

	scene.add (new Sphere (materials[sp->mat_index],
			       Pos (sp->x, sp->y, sp->z),
			       sp->r));
      }

  if (p->num_lights > 0)
    for (unsigned i = 0; i < p->num_lights; i++)
      {
	const TestSceneLightParams *lp = &p->light_params[i];

	scene.add (new Light (Pos (lp->x, lp->y, lp->z),
			      light_intens,
			      Color (lp->r, lp->g, lp->b)));
      }

  scene.set_background (Color (p->bg_r, p->bg_g, p->bg_b));

  camera.move (Pos (p->cam_x, p->cam_y, p->cam_z));
  camera.point (Pos (p->cam_targ_x, p->cam_targ_y, p->cam_targ_z),
		Vec (p->cam_up_x, p->cam_up_y, p->cam_up_z));
  camera.set_vert_fov (p->cam_fov_y);

  delete[] materials;
  delete[] vertices;
}

// arch-tag: 2d1cf1ee-5f23-49c4-a3e9-61231a4089bc
