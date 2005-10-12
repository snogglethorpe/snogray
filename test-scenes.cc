// test-scenes.cc -- Test scenes for snogray ray tracer
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "test-scenes.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "glow.h"
#include "mirror.h"
#include "glass.h"
#include "mesh.h"

using namespace Snogray;
using namespace std;


// Define the scene, since can't read any kind of scene file yet

static void
add_bulb (Scene &scene, const Pos &pos, const Color &col = Color::white)
{
  const Material *bulb_mat = scene.add (new Glow (col));
  scene.add (new Light (pos, col));
  scene.add (new Sphere (bulb_mat, pos, 0.06))->no_shadow = true;
}

static void
def_scene_miles_test1 (Scene &scene, Camera &camera, unsigned camera_pos)
{
//  Material *mat1 = scene.add (new Lambert (Color (1, 0.5, 0.2)));
//  Material *mat2 = scene.add (new Lambert (Color (0.5, 0.5, 0)));
//  Material *mat3 = scene.add (new Lambert (Color (1, 0.5, 1)));
//  Material *mat4 = scene.add (new Lambert (Color (1, 0.5, 1)));
//   Material *mat3 = scene.add (Material::phong (400, Color (0.1, 0.1, 0.1)));
//   const Material *mat1
//     = scene.add (new Mirror (0.5, Color (1, 0.5, 0.2) * 0.5, 5));
  const Material *mat1
    = scene.add (new Glass (0.8, 1.1, 0.2, 0.2, 500));
  const Material *mat2
    = scene.add (new Mirror (0.8, 0.2, 100));
  const Material *mat3
    = scene.add (new Material (Color (0.8, 0, 0), Material::phong (400)));
  const Material *mat4
    = scene.add (new Material (Color (0.2, 0.5, 0.1)));

  // First test scene
  add_bulb (scene, Pos (0, 15, 0), 30);
  add_bulb (scene, Pos (0, 0, -5), 30);
  add_bulb (scene, Pos (-5, 10, 0), 40 * Color (0, 0, 1));
  add_bulb (scene, Pos (-40, 15, -40), 300);
  add_bulb (scene, Pos (-40, 15,  40), 300);
  add_bulb (scene, Pos ( 40, 15, -40), 300);
  add_bulb (scene, Pos ( 40, 15,  40), 300);

//   // xxx
//   scene.add (new Sphere (mat1, Pos (-2, -2, -8), 0.5));
//   scene.add (new Sphere (mat3, Pos (1, -2, -8), 0.5));

  scene.add (new Sphere (mat1, Pos (0, 2, 7), 5));
  scene.add (new Sphere (mat2, Pos (-8, 0, 3), 3));
  scene.add (new Sphere (mat3, Pos (-6, 5, 2), 1));

  scene.add (new Triangle (mat4,
			   Pos (-100, -3, -100),
			   Pos (100, -3, -100),
			   Pos (100, -3, 100)));
  scene.add (new Triangle (mat4,
			   Pos (-100, -3, -100),
			   Pos (100, -3, 100),
			   Pos (-100, -3, 100)));

  switch (camera_pos)
    {
    case 0:
    default:
      camera.move (Pos (-6.5, -0.4, -19));
      camera.point (Pos (0, -2, 5));
      break;

    case 2:			// overhead
      camera.move (Pos (0, 50, 30));
      camera.point (Pos (0, 5, 0));
      break;

    case 1:
      camera.move (Pos (-3, 2, -18));
      break;
    case 3:
      camera.move (Pos (-3, 1.5, -25));
      break;
    }

  
  const unsigned gsize = 10;
  const unsigned gsep = 4;
  const Pos gpos (-20, -1, -20);
  for (unsigned i = 0; i < gsize; i++)
    for (unsigned j = 0; j < gsize; j++)
      {
	Color color (0,
		     (float)j / (float)gsize,
		     (float)i / (float)gsize);
	color *= 0.3;
	Pos pos = gpos + Vec (i * gsep, 0, j * gsep);
	const Material *mat
	  = scene.add (new Material (color, Material::phong (500)));
	scene.add (new Sphere (mat, pos, 0.5));
	scene.add (new Triangle (mat,
				 pos + Vec(1.5,-0.2,0),
				 pos + Vec(-0.5,-0.2,-1.1),
				 pos + Vec(-0.5,-0.2,1.1)));
      }
}



static void
def_scene_teapot (Scene &scene, Camera &camera, const char *teapot_mesh_name)
{
  // We copied various params from a .nff file with bogus gamma
  scene.set_assumed_gamma (2.2);

  const Material *teapot_mat
    = scene.add (new Mirror (0.25, Color (1, 0.5, 0.1) * 0.75,
			     Material::phong (3.0827, 0.25)));
  const Material *board1_mat = scene.add (new Mirror (0.3, 0.1, 10));
  const Material *board2_mat = scene.add (new Mirror (0.3, 0.8, 10));
    
  Mesh *teapot_mesh = new Mesh (teapot_mat);

  string teapot_mesh_file_name (teapot_mesh_name);
  teapot_mesh_file_name += ".msh";
  teapot_mesh->load (teapot_mesh_file_name);
  teapot_mesh->compute_vertex_normals ();
  scene.add (teapot_mesh);

  Mesh *board1_mesh = new Mesh (board1_mat);
  board1_mesh->load ("board1.msh");
  scene.add (board1_mesh);

  Mesh *board2_mesh = new Mesh (board2_mat);
  board2_mesh->load ("board2.msh");
  scene.add (board2_mesh);

  scene.add (new Light (Pos (-3.1, 9.8, 12.1), 100));
  scene.add (new Light (Pos (11.3, 5.1, 8.8), 100));

  scene.set_background (Color (0.078, 0.361, 0.753));

  camera.move (Pos (4.86, 7.2, 5.4));
  camera.point (Pos (0, 0, 0), Vec (0, 0, 1));
}


// CS465 test scenes

// from cs465 Test1.xml
static void
def_scene_cs465_test1 (Scene &scene, Camera &camera)
{
  // First test scene, only uses spheres, Lambertian shading,
  // and one light directly above the center of the 3 spheres.

  const Phong *ph300 = Material::phong (300), *ph400 = Material::phong (400);
  const Material *mat1 = scene.add (new Material (Color (1, 0.5, 0.2)));
  const Material *mat2 = scene.add (new Material (Color (0.8, 0.8, 0.8), ph300));
  const Material *mat3 = scene.add (new Material (Color (0.8, 0, 0), ph400));

  camera.move (Pos (0, 3, -4));
  camera.point (Pos (0, 0, 0), Vec (0, 1, 0));
  scene.add (new Sphere (mat1, Pos (0, 0, -0.866), 1));
  scene.add (new Sphere (mat2, Pos (1, 0, 0.866), 1));
  scene.add (new Sphere (mat3, Pos (-1, 0, 0.866), 1));
  scene.add (new Light (Pos (0, 5, 0), 25));
}

static void
def_scene_cs465_test2 (Scene &scene, Camera &camera)
{
  // Sphere on plane.  Sphere has greenish phong material.

  camera.move (Pos (0, 4, 4));
  camera.point (Pos (-0.5, 0, 0.5), Vec (0, 1, 0));

  const Phong *ph100 = Material::phong (100, Color (0.3, 0.3, 0.3));

  const Material *sphereMat
    = scene.add (new Material (Color (0.249804, 0.218627, 0.0505882), ph100));
  const Material *grey
    = scene.add (new Material (Color (0.3, 0.3, 0.3)));

  scene.add (new Sphere (sphereMat, Pos (0, 0, 0), 1));

  // ground
  scene.add (new Triangle (grey,
			   Pos (-10, -1, -10),
			   Pos (-10, -1, 10),
			   Pos (10, -1, -10)));
  scene.add (new Triangle (grey,
			   Pos (10, -1, -10),
			   Pos (-10, -1, 10),
			   Pos (10, -1, 10)));

  // Small Area type light
  scene.add (new Light (Pos (5, 5, 0), 8));
  scene.add (new Light (Pos (5.1, 5, 0), 8));
  scene.add (new Light (Pos (5.2, 5, 0), 8));
  scene.add (new Light (Pos (5.3, 5, 0), 8));
  scene.add (new Light (Pos (5, 5.1, 0), 8));
  scene.add (new Light (Pos (5.1, 5.1, 0), 8));
  scene.add (new Light (Pos (5.2, 5.1, 0), 8));
  scene.add (new Light (Pos (5.3, 5.1, 0), 8));
  scene.add (new Light (Pos (5, 5.2, 0), 8));
  scene.add (new Light (Pos (5.1, 5.2, 0), 8));
  scene.add (new Light (Pos (5.2, 5.2, 0), 8));
  scene.add (new Light (Pos (5.3, 5.2, 0), 8));
  scene.add (new Light (Pos (5, 5.3, 0), 8));
  scene.add (new Light (Pos (5.1, 5.3, 0), 8));
  scene.add (new Light (Pos (5.2, 5.3, 0), 8));
  scene.add (new Light (Pos (5.3, 5.3, 0), 8));

  // fill light
  scene.add (new Light (Pos (-5, 1, -22), 100));
}

static void
def_scene_cs465_test3 (Scene &scene, Camera &camera)
{
  // Three spheres and a box on a plane.  Mix of Lambertian
  // and Phong materials.

  camera.move (Pos (6, 6, 6));
  camera.point (Pos (0, 0, 0), Vec (0, 1, 0));

  const Phong *ph300_1 = Material::phong (300, Color (1, 1, 1));
  const Phong *ph300_2 = Material::phong (300, Color (2, 2, 2));

  const Material *shinyBlack
    = scene.add (new Material (Color (0.02, 0.02, 0.02), ph300_2));
  const Material *shinyWhite
    = scene.add (new Material (Color (0.6, 0.6, 0.6), ph300_1));
  const Material *shinyGray
    = scene.add (new Material (Color (0.2, 0.2, 0.2), ph300_2));
  const Material *boxMat
    = scene.add (new Material (Color (0.3, 0.19, 0.09)));
  const Material *gray
    = scene.add (new Material (Color (0.6, 0.6, 0.6)));
	
  // box
  // front
  scene.add (new Triangle (boxMat,
			   Pos (1, -1, 1),
			   Pos (1, -1, -1),
			   Pos (1, 1, -1)));
  scene.add (new Triangle (boxMat,
			   Pos (1, 1, 1),
			   Pos (1, -1, 1),
			   Pos (1, 1, -1)));

  // back
  scene.add (new Triangle (boxMat,
			   Pos (-1, -1, -1),
			   Pos (-1, -1, 1),
			   Pos (-1, 1, -1)));
  scene.add (new Triangle (boxMat,
			   Pos (-1, 1, 1),
			   Pos (-1, 1, -1),
			   Pos (-1, -1, 1)));

  // top
  scene.add (new Triangle (boxMat,
			   Pos (-1, 1, 1),
			   Pos (1, 1, -1),
			   Pos (-1, 1, -1)));
  scene.add (new Triangle (boxMat,
			   Pos (1, 1, 1),
			   Pos (1, 1, -1),
			   Pos (-1, 1, 1)));

  // bottom
  scene.add (new Triangle (boxMat,
			   Pos (-1, -1, 1),
			   Pos (-1, -1, -1),
			   Pos (1, -1, -1)));
  scene.add (new Triangle (boxMat,
			   Pos (1, -1, 1),
			   Pos (-1, -1, 1),
			   Pos (1, -1, -1)));

  // left
  scene.add (new Triangle (boxMat,
			   Pos (1, -1, -1),
			   Pos (-1, -1, -1),
			   Pos (-1, 1, -1)));
  scene.add (new Triangle (boxMat,
			   Pos (1, -1, -1),
			   Pos (-1, 1, -1),
			   Pos (1, 1, -1)));

  // right
  scene.add (new Triangle (boxMat,
			   Pos (-1, -1, 1),
			   Pos (1, -1, 1),
			   Pos (-1, 1, 1)));
  scene.add (new Triangle (boxMat,
			   Pos (1, -1, 1),
			   Pos (1, 1, 1),
			   Pos (-1, 1, 1)));

  // ground
  scene.add (new Triangle (gray,
			   Pos (-10, -1, -10),
			   Pos (-10, -1, 10),
			   Pos (10, -1, -10)));
  scene.add (new Triangle (gray,
			   Pos (10, -1, -10),
			   Pos (-10, -1, 10),
			   Pos (10, -1, 10)));

  // spheres	
  scene.add (new Sphere (shinyBlack, Pos (0, 2, 0), 1));
  scene.add (new Sphere (shinyGray, Pos (0, 0, 2.5), 1));
  scene.add (new Sphere (shinyWhite, Pos (2.5, 0, 0), 1));

  scene.add (new Light (Pos (0, 10, 5), Color (50, 30, 30)));
  scene.add (new Light (Pos (5, 10, 0), Color (30, 30, 50)));
  scene.add (new Light (Pos (5, 10, 5), Color (30, 50, 30)));
  scene.add (new Light (Pos (6, 6, 6), Color (25, 25, 25)));
}

void 
def_scene_cs465_test4 (Scene &scene, Camera &camera)
{
  // Low resolution Stanford Bunny Mesh.

  camera.move (Pos (0, 0, 3));
  camera.point (Pos (-0.25, -0.07, 0), Vec (0, 1, 0));
  camera.set_vert_fov (M_PI_4);

  const Material *red = scene.add (new Material (Color (0.5, 0.3, 0.05)));
  //Material *red = scene.add (new Mirror (0.05, Color (1, 0, 0), 500));
  const Material *gray = scene.add (new Material (Color (0.6, 0.6, 0.6)));

  Mesh *bunny = new Mesh (red);
  bunny->load ("bunny500.msh");
  bunny->compute_vertex_normals ();
  scene.add (bunny);
  
  // ground
  scene.add (new Triangle (gray,
			   Pos (-10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, -10)));
  scene.add (new Triangle (gray,
			   Pos (10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, 10)));
  
  scene.add (new Light (Pos (0, 10, 0), 100));
  scene.add (new Light (Pos (15, 2, 0), 100));
  scene.add (new Light (Pos (0, 1, 15), 100));
}



void
Snogray::def_test_scene (const char *name, Scene &scene, Camera &camera)
{
  if (strcmp (name, "miles-0") == 0 || strcmp (name, "0") == 0)
    def_scene_miles_test1 (scene, camera, 0);
  else if (strcmp (name, "miles-1") == 0 || strcmp (name, "7") == 0)
    def_scene_miles_test1 (scene, camera, 1);
  else if (strcmp (name, "miles-2") == 0 || strcmp (name, "5") == 0)
    def_scene_miles_test1 (scene, camera, 2);
  else if (strcmp (name, "miles-3") == 0 || strcmp (name, "6") == 0)
    def_scene_miles_test1 (scene, camera, 3);

  else if (strncmp (name, "teapot", 6) == 0)
    def_scene_teapot (scene, camera, name);

  else if (strcmp (name, "cs465-1") == 0 || strcmp (name, "1") == 0)
    def_scene_cs465_test1 (scene, camera);
  else if (strcmp (name, "cs465-2") == 0 || strcmp (name, "2") == 0)
    def_scene_cs465_test2 (scene, camera);
  else if (strcmp (name, "cs465-3") == 0 || strcmp (name, "3") == 0)
    def_scene_cs465_test3 (scene, camera);
  else if (strcmp (name, "cs465-4") == 0 || strcmp (name, "4") == 0)
    def_scene_cs465_test4 (scene, camera);

  else
    throw (runtime_error ("unknown test scene"));
}

// arch-tag: 307938a9-c663-4949-a58b-fb51040a6529
