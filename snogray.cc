// snogray.cc -- Main driver for snogray ray tracer
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "cmdlineparser.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "image.h"

#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "glow.h"

using namespace std;

static void
add_bulb (Scene &scene, const Pos &pos, float intens,
	  const Color &col = Color::white)
{
  Material *bulb_mat = scene.add (new Glow (intens * col));
  scene.add (new Light (pos, intens, col));
  scene.add (new Sphere (bulb_mat, pos, 0.06))->no_shadow = true;
}

static void
define_scene (Scene &scene, Camera &camera)
{
//  Material *mat1 = scene.add (new Lambert (Color (1, 0.5, 0.2)));
//  Material *mat2 = scene.add (new Lambert (Color (0.5, 0.5, 0)));
//  Material *mat3 = scene.add (new Lambert (Color (1, 0.5, 1)));
//  Material *mat4 = scene.add (new Lambert (Color (1, 0.5, 1)));
  Material *mat1 = scene.add (new Lambert (Color (1, 0.5, 0.2)));
  Material *mat2 = scene.add (new Phong (300, Color (0.8, 0.8, 0.8)));
//   Material *mat3 = scene.add (new Phong (400, Color (0.1, 0.1, 0.1)));
  Material *mat3 = scene.add (new Phong (400, Color (0.8, 0, 0)));
  Material *mat4 = scene.add (new Lambert (Color (0.2, 0.5, 0.1)));
  Material *mat5 = scene.add (new Lambert (Color (1, 0.5, 1)));

  Material *bulb_mat = scene.add (new Glow (25 * Color::white));

  // First test scene
  add_bulb (scene, Pos (0, 15, 0), 30);
  add_bulb (scene, Pos (0, 0, -5), 30);
  add_bulb (scene, Pos (-5, 10, 0), 40, Color (0, 0, 1));
  add_bulb (scene, Pos (-40, 15, -40), 300);
  add_bulb (scene, Pos (-40, 15,  40), 300);
  add_bulb (scene, Pos ( 40, 15, -40), 300);
  add_bulb (scene, Pos ( 40, 15,  40), 300);

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
  // (1)
//   camera.move (Pos (-3, 2, -18));
//   // (2)
//   camera.move (Pos (0, 50, 30));
//   camera.point (Pos (0, 5, 0));
//   // (3)
//   camera.move (Pos (-3, 1.5, -25));
  // (4)
  camera.move (Pos (-6.5, -0.4, -19));
  camera.point (Pos (0, -2, 5));

  const unsigned gsize = 10;
  const unsigned gsep = 4;
  const Pos gpos (-20, -1, -20);
  for (unsigned i = 0; i < gsize; i++)
    for (unsigned j = 0; j < gsize; j++)
      {
	Color color ((float)i / (float)gsize + 0.2,
		     0.5,
		     (float)j / (float)gsize / 2 + 0.2);
	Pos pos = gpos + Vec (i * gsep, 0, j * gsep);
	Material *mat = scene.add (new Phong (500, color));
	scene.add (new Sphere (mat, pos, 0.5));
	scene.add (new Triangle (mat,
				 pos + Vec(1.5,-0.2,0),
				 pos + Vec(-0.5,-0.2,-1.1),
				 pos + Vec(-0.5,-0.2,1.1)));
      }

//   // from cs465 Test1.xml
//   const unsigned width = 512 * aa_factor;
//   const unsigned height = 512 * aa_factor;
//   camera.move (Pos (0, 3, -4));
//   camera.point (Pos (0, 0, 0), Vec (0, 1, 0));
//   camera.set_aspect_ratio (1);
//   scene.add (new Sphere (mat1, Pos (0, 0, -0.866), 1));
//   scene.add (new Sphere (mat2, Pos (1, 0, 0.866), 1));
//   scene.add (new Sphere (mat3, Pos (-1, 0, 0.866), 1));
//   scene.add (new Light (Pos (0, 5, 0), 25));
}

int main (int argc, char *const *argv)
{
  Scene scene;
  Camera camera;
  const char *prog_name = *argv;
  unsigned final_width = 640, final_height = 480, aa_factor = 1;
  float target_gamma = 2.2;
  const char *output_file = 0;

  static struct option long_options[] = {
    { "width",		required_argument, 0, 'w' },
    { "height",		required_argument, 0, 'h' },
    { "aa-factor",	required_argument, 0, 'a' },
    { "gamma",		required_argument, 0, 'g' },
    { 0, 0, 0, 0 }
  };
  CmdLineParser clp (argc, argv, "a:w:h:g:", long_options);

  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'a':
	aa_factor = clp.unsigned_opt_arg ();
	break;
      case 'w':
	final_width = clp.unsigned_opt_arg ();
	break;
      case 'h':
	final_height = clp.unsigned_opt_arg ();
	break;
      case 'g':
	target_gamma = clp.float_opt_arg ();
	break;
      }

  if (clp.num_remaining_args() != 1)
    {
      cerr << "Usage: " << prog_name
	   << " [-a AA_FACTOR] [-w WIDTH] [-h HEIGHT] [-g GAMMA]"
	   << " OUTPUT_IMAGE_FILE" << endl;
      exit (10);
    }

  output_file = clp.get_arg ();


  const unsigned width = final_width * aa_factor;
  const unsigned height = final_height * aa_factor;
  camera.set_aspect_ratio ((float)width / (float)height);

  define_scene (scene, camera);


  cout << "image.width = " << final_width << endl;
  cout << "image.height = " << final_height << endl;
  cout << "image.target_gamma = " << target_gamma << endl;
  if (aa_factor > 1)
    cout << "image.aa_factor = " << aa_factor << endl;

  cout << "scene.num_objects = " << scene.objs.size () << endl;
  cout << "scene.num_lights = " << scene.lights.size () << endl;
  cout << "scene.num_materials = " << scene.materials.size () << endl;


  Image image (width, height, target_gamma);

  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++)
      {
	float u = (float)x / (float)width;
	float v = (float)(height - y) / (float)height;

	Ray camera_ray = camera.get_ray (u, v);

	Intersect isec = scene.closest_intersect (camera_ray);

	image (x, y) = isec.obj ? scene.render (isec) : Color::black;
      }


  Scene::Stats &stats = scene.stats;
  cout << "stats.scene_closest_intersect_calls = "
       << stats.scene_closest_intersect_calls << endl;
  cout << "stats.obj_closest_intersect_calls = "
       << stats.obj_closest_intersect_calls << endl;
  cout << "stats.scene_intersects_calls = "
       << stats.scene_intersects_calls << endl;
  cout << "stats.obj_intersects_calls = "
       << stats.obj_intersects_calls << endl;


  if (aa_factor > 1)
    {
      Image aa_image (image, aa_factor);
      aa_image.write_png_file (output_file);
    }
  else
    image.write_png_file (output_file);
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
