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

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "glow.h"
#include "mirror.h"
#include "mesh.h"
#include "test-scene.h"

using namespace Snogray;



// Define the scene, since can't read any kind of scene file yet

static void
add_bulb (Scene &scene, const Pos &pos, float intens,
	  const Color &col = Color::white)
{
  Material *bulb_mat = scene.add (new Glow (intens * col));
  scene.add (new Light (pos, intens, col));
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
  Material *mat1
    = scene.add (new Mirror (0.5, Color (1, 0.5, 0.2) * 0.5, 5));
  Material *mat2
    = scene.add (new Mirror (0.8, Color::white * 0.2, 100));
  Material *mat3
    = scene.add (new Material (Color (0.8, 0, 0), Material::phong (400)));
  Material *mat4 = scene.add (new Material (Color (0.2, 0.5, 0.1)));

  // First test scene
  add_bulb (scene, Pos (0, 15, 0), 30);
  add_bulb (scene, Pos (0, 0, -5), 30);
  add_bulb (scene, Pos (-5, 10, 0), 40, Color (0, 0, 1));
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
	Material *mat = scene.add (new Material (color, Material::phong (500)));
	scene.add (new Sphere (mat, pos, 0.5));
	scene.add (new Triangle (mat,
				 pos + Vec(1.5,-0.2,0),
				 pos + Vec(-0.5,-0.2,-1.1),
				 pos + Vec(-0.5,-0.2,1.1)));
      }
}



static void
def_scene_miles_test2 (Scene &scene, Camera &camera)
{
  Material *mat
    = scene.add (new Mirror (0.8, Color::white * 0.2, 100));

  Mesh *mesh = new Mesh (mat);
  mesh->load ("teapot-30.msh");
  scene.add (mesh);

  scene.add (new Light (Pos (-3.1, 9.8, 12.1), 100));
  scene.add (new Light (Pos (11.3, 5.1, 8.8), 100));

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
  Material *mat1 = scene.add (new Material (Color (1, 0.5, 0.2)));
  Material *mat2 = scene.add (new Material (Color (0.8, 0.8, 0.8), ph300));
  Material *mat3 = scene.add (new Material (Color (0.8, 0, 0), ph400));

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

  Material *sphereMat
    = scene.add (new Material (Color (0.249804, 0.218627, 0.0505882), ph100));
  Material *grey
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

  Material *shinyBlack
    = scene.add (new Material (Color (0.02, 0.02, 0.02), ph300_2));
  Material *shinyWhite
    = scene.add (new Material (Color (0.6, 0.6, 0.6), ph300_1));
  Material *shinyGray
    = scene.add (new Material (Color (0.2, 0.2, 0.2), ph300_2));
  Material *boxMat
    = scene.add (new Material (Color (0.3, 0.19, 0.09)));
  Material *gray
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
test_scene (Scene &scene, Camera &camera, unsigned scene_num)
{
  switch (scene_num)
    {
    case 0:
    default:
      def_scene_miles_test1 (scene, camera, 0); break;

    case 1:
      def_scene_cs465_test1 (scene, camera); break;
    case 2:
      def_scene_cs465_test2 (scene, camera); break;
    case 3:
      def_scene_cs465_test3 (scene, camera); break;

    case 4:
      def_scene_miles_test1 (scene, camera, 1); break;
    case 5:
      def_scene_miles_test1 (scene, camera, 2); break;
    case 6:
      def_scene_miles_test1 (scene, camera, 3); break;

    case 7:
      def_scene_miles_test2 (scene, camera); break;

    case 10:
      extern TestSceneParams teapot_test_scene_params;
      define_test_scene (&teapot_test_scene_params, 115, scene, camera);
      break;
    case 11:
      extern TestSceneParams balls_test_scene_params;
      define_test_scene (&balls_test_scene_params, 18, scene, camera);
      break;
    case 12:
      extern TestSceneParams rings_test_scene_params;
      define_test_scene (&rings_test_scene_params, 25, scene, camera);
      break;
    case 13:
      extern TestSceneParams tetra_test_scene_params;
      define_test_scene (&tetra_test_scene_params, 25, scene, camera);
      break;
    case 14:
      extern TestSceneParams mount_test_scene_params;
      define_test_scene (&mount_test_scene_params, 35000, scene, camera);
      break;
    case 15:
      extern TestSceneParams tree_test_scene_params;
      define_test_scene (&tree_test_scene_params, 100, scene, camera);
      break;
    case 16:
      extern TestSceneParams gears_test_scene_params;
      define_test_scene (&gears_test_scene_params, 5, scene, camera);
      break;
    case 17:
      extern TestSceneParams sample_test_scene_params;
      define_test_scene (&sample_test_scene_params, 700, scene, camera);
      break;
    case 18:
      extern TestSceneParams nurbtst_test_scene_params;
      define_test_scene (&nurbtst_test_scene_params, 2500, scene, camera);
      break;
    case 19:
      extern TestSceneParams lattice_test_scene_params;
      define_test_scene (&lattice_test_scene_params, 0.5, scene, camera);
      break;
    case 20:
      extern TestSceneParams jacks_test_scene_params;
      define_test_scene (&jacks_test_scene_params, 25, scene, camera);
      break;
    case 21:
      extern TestSceneParams sombrero_test_scene_params;
      define_test_scene (&sombrero_test_scene_params, 500, scene, camera);
      break;
    case 22:
      extern TestSceneParams shells_test_scene_params;
      define_test_scene (&shells_test_scene_params, 50000, scene, camera);
      break;
    case 23:
      extern TestSceneParams f117_test_scene_params;
      define_test_scene (&f117_test_scene_params, 3000, scene, camera);
      break;
    case 24:
      extern TestSceneParams skull_test_scene_params;
      define_test_scene (&skull_test_scene_params, 1000, scene, camera);
      break;
    case 25:
      extern TestSceneParams f15_test_scene_params;
      define_test_scene (&f15_test_scene_params, 2500, scene, camera);
      break;

//     case 26:
//       extern TestSceneParams balls_5_test_scene_params;
//       define_test_scene (&balls_5_test_scene_params, 18, scene, camera);
//       break;
    case 27:
      extern TestSceneParams teapot_14_test_scene_params;
      define_test_scene (&teapot_14_test_scene_params, 115, scene, camera);
      break;
//     case 28:
//       extern TestSceneParams teapot_22_test_scene_params;
//       define_test_scene (&teapot_22_test_scene_params, 115, scene, camera);
//       break;
    case 29:
      extern TestSceneParams teapot_30_test_scene_params;
      define_test_scene (&teapot_30_test_scene_params, 115, scene, camera);
      break;
    }
}

// arch-tag: 307938a9-c663-4949-a58b-fb51040a6529
