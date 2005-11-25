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

extern float tessel_accur;

#include <sstream>

#include "test-scenes.h"

#include "scene.h"
#include "camera.h"
#include "point-light.h"
#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "glow.h"
#include "mirror.h"
#include "glass.h"
#include "mesh.h"
#include "tessel-param.h"
#include "string-funs.h"

using namespace Snogray;
using namespace std;


// Define the scene, since can't read any kind of scene file yet

static void
add_bulb (Scene &scene, const Pos &pos, float radius,
	  const Color &col = Color::white)
{
  const Material *bulb_mat = scene.add (new Glow (col));
  scene.add (new PointLight (pos, col));
  scene.add (new Sphere (bulb_mat, pos, radius));
}

static void
def_scene_miles (const string &name, unsigned num,
		       Scene &scene, Camera &camera)
{
//  Material *mat1 = scene.add (new Lambert (Color (1, 0.5, 0.2)));
//  Material *mat2 = scene.add (new Lambert (Color (0.5, 0.5, 0)));
//  Material *mat3 = scene.add (new Lambert (Color (1, 0.5, 1)));
//  Material *mat4 = scene.add (new Lambert (Color (1, 0.5, 1)));
//   Material *mat3 = scene.add (Material::phong (400, Color (0.1, 0.1, 0.1)));
//   const Material *mat1
//     = scene.add (new Mirror (0.5, Color (1, 0.5, 0.2) * 0.5, 5));
  const Material *crystal
    = scene.add (new Glass (Medium (0.99, 1.8), 0.1, 0.01,
			    Material::phong (2000, 1.5)));
  const Material *gold
    = scene.add (new Mirror (0.9 * Color (0.71, 0.63, 0.1),
			     0.1 * Color (0.80, 0.80, 0.05),
			     Material::phong (500, Color (1, 1, 0.2))));
  const Material *mat1 = crystal, *mat2 = gold;
//   const Material *mat1
//     = scene.add (new Glass (Medium (1, 1.1), 0.1, 0, 300));
//   const Material *mat2
//     = scene.add (new Mirror (0.8, 0.2, 100));
  const Material *mat3
    = scene.add (new Material (Color (0.8, 0, 0), Material::phong (400)));
  const Material *mat4
    = scene.add (new Material (Color (0.2, 0.5, 0.1)));

  // First test scene
  add_bulb (scene, Pos (0, 15, 0), 0.06, 30);
  add_bulb (scene, Pos (0, 0, -5), 0.06, 30);
  add_bulb (scene, Pos (-5, 10, 0), 0.06, 40 * Color (0, 0, 1));
  add_bulb (scene, Pos (-40, 15, -40), 0.06, 300);
  add_bulb (scene, Pos (-40, 15,  40), 0.06, 300);
  add_bulb (scene, Pos ( 40, 15, -40), 0.06, 300);
  add_bulb (scene, Pos ( 40, 15,  40), 0.06, 300);

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

  switch (num)
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
add_scene_descs_miles (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("miles0", "Lots of spheres and triangles, low angle"));
  descs.push_back (TestSceneDesc ("miles1", "Lots of spheres and triangles, square angle"));
  descs.push_back (TestSceneDesc ("miles2", "Lots of spheres and triangles, high angle"));
  descs.push_back (TestSceneDesc ("miles3", "Lots of spheres and triangles, slightly wider angle"));
}

static void 
def_scene_pretty_bunny (const string &name, unsigned num,
			Scene &scene, Camera &camera)
{
  // This is a mutation of test:cs465-4

  camera.move (Pos (-1, 0.7, 2.3)); // y=0.5
  camera.point (Pos (-0.75, -0.07, 0), Vec (0, 1, 0));
  camera.set_vert_fov (M_PI_4);
  camera.set_z_mode (Camera::Z_DECREASES_FORWARD);

  const Material *gray
    = scene.add (new Material (0.6));
  const Material *red
    = scene.add (new Material (Color (1, 0, 0), Material::phong (500)));
  const Material *yellow
    = scene.add (new Material (Color (1, 1, 0), Material::phong (500)));
  const Material *green
    = scene.add (new Material (Color (0, 1, 0), Material::phong (500)));
  const Material *crystal
    = scene.add (new Glass (Medium (0.9, 1.8), 0.2, 0.01,
			    Material::phong (2000, 1.5)));
  const Material *gold
    = scene.add (new Mirror (0.9 * Color (0.71, 0.63, 0.1),
			     0.05 * Color (0.80, 0.80, 0.05),
			     Material::phong (500, Color (1, 1, 0.2))));

  bool goldbunny = ends_in (name, "gold");

  Mesh *bunny = new Mesh (goldbunny ? gold : crystal);
  bunny->load ("bunny500.msh");
  bunny->compute_vertex_normals ();
  scene.add (bunny);

  scene.add (new Sphere (goldbunny ? crystal : gold, Pos (-3, 0, -3), 1.5));

  scene.add (new Sphere (red,    Pos (3.5, 0.65 - 0.65, -5.0), 0.65));
  scene.add (new Sphere (green,  Pos (2.5, 0.40 - 0.65, -7.0), 0.40));
  scene.add (new Sphere (yellow, Pos (0.3, 0.40 - 0.65, -2.5), 0.40));

  // ground
  scene.add (new Triangle (gray,
			   Pos (-10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, -10)));
  scene.add (new Triangle (gray,
			   Pos (10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, 10)));
  
  add_bulb (scene, Pos (0, 10, 0), 0.06, 100);
  add_bulb (scene, Pos (15, 2, 0), 0.06, 100);
  add_bulb (scene, Pos (0, 1, 15), 0.06, 100);
}

static void
add_scene_descs_pretty_bunny (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("pretty-bunny", "Crystal Stanford bunny with some spheres"));
  descs.push_back (TestSceneDesc ("goldbunny", "Gold Stanford bunny with some spheres"));
}



static Surface *
mottle_ball (const Material *mat, const Pos &pos, dist_t radius, dist_t max_err)
{
  return
    new Mesh (mat,
	      SphereTesselFun (pos, radius, radius * 0.002),
	      Tessel::ConstMaxErr (max_err), true);
}

static void
def_scene_teapot (const string &name, unsigned num,
		  Scene &scene, Camera &camera)
{
  // Teapot mesh and coords come from .nff file
  //
  camera.set_z_mode (Camera::Z_DECREASES_FORWARD);

  // Note that the coordinates in this scene are weird -- it uses Z as
  // "height" rather than depth.

  const Material *silver
    = scene.add (new Mirror (0.3, Color (0.7, 0.8, 0.7), 10, 5));
  const Material *gloss_black
    = scene.add (new Mirror (0.3, 0.02, 10));
  const Material *ivory
    = scene.add (new Mirror (0.2, 2 * Color (1.1, 1, 0.8), 5, 2));
  const Material *brown
    = scene.add (new Material (Color (0.3, 0.2, 0.05)));

  scene.add (new Mesh (silver, name + ".msh", true));
  scene.add (new Mesh (gloss_black, "board1.msh"));
  scene.add (new Mesh (ivory, "board2.msh"));
  scene.add (new Mesh (brown, "board3.msh"));

  if ((num & 1) == 0)
    /* night-time teapot */
    {
      scene.add (new PointLight (Pos (-3.1, 9.8, 12.1), 100));
      //scene.add (new PointLight (Pos (11.3, 5.1, 8.8), 5));
      add_bulb (scene, Pos (4.7, 2, 3), 0.2, 4 * Color (1, 1, 0.3));
      add_bulb (scene, Pos (-1, -2, 4), 0.2, 4 * Color (1, 1, 0.3));
      scene.set_background (Color (0.01, 0.01, 0.02));
    }
  else
    /* day-time teapot */
    {
      scene.add (new PointLight (Pos (-3.1, 9.8, 12.1), 90));
      scene.add (new PointLight (Pos (11.3, 5.1, 8.8), 50));
      scene.set_background (Color (0.078, 0.361, 0.753));
    }

  if (num >= 2)
    {
      num -= 2;
      num /= 2;

      const Material *orange
	= scene.add (new Material (Color (0.6,0.5,0.05),
				   Material::phong (250)));
      const Material *glass
	= scene.add (new Glass (Medium (0.95, 1.5), 0.1, 0.01,
				Material::phong (2000, 1.5)));
      const Material *gold
	= scene.add (new Mirror (Color (0.852, 0.756, 0.12), 0, 
				 Material::phong (800, Color (1, 1, 0.3))));

      const Material *ball_mat = orange;
      dist_t max_err = 0.0002;
      dist_t radius = 1;

      switch (num % 3)
	{
	case 0: ball_mat = orange; break;
	case 1: ball_mat = glass;  radius = 0.5; break;
	case 2: ball_mat = gold;   radius = 0.6; break;
	}

      scene.add (mottle_ball (ball_mat, Pos (3, 2, radius), radius, max_err));

      const Material *red
	= scene.add (new Material (Color (1, 0, 0), Material::phong (500)));
      const Material *yellow
	= scene.add (new Material (Color (1.5, 1.5, 0.1), Material::phong (500)));
      const Material *green
	= scene.add (new Material (Color (0, 1, 0), Material::phong (500)));
      const Material *blue
	= scene.add (new Material (Color (0.3, 0.3, 1.2), Material::phong (500)));

      dist_t r1 = 0.65, r2 = 0.40;
      scene.add (mottle_ball (blue,   Pos ( 1.5,  3.3, r2), r2, max_err));
      scene.add (mottle_ball (green,  Pos (-3,    1.2, r2), r2, max_err));
      scene.add (mottle_ball (yellow, Pos (-2.2,  3.1, r1), r1, max_err));
      scene.add (mottle_ball (red,    Pos ( 2.3, -1.7, r1), r1, max_err));
    }

  camera.set_vert_fov (M_PI_4 * 0.9);
  camera.move (Pos (4.86, 7.2, 5.4));
  camera.point (Pos (0, -0.2, 0), Vec (0, 0, 1));
}

static void
add_scene_descs_teapot (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("teapot", "Classic teapot at night"));
  descs.push_back (TestSceneDesc ("teapot1", "Classic teapot in day"));
  descs.push_back (TestSceneDesc ("teapot2", "Teapot at night with orange"));
  descs.push_back (TestSceneDesc ("teapot3", "Teapot in day with orange"));
  descs.push_back (TestSceneDesc ("teapot4", "Teapot at night with glass ball"));
  descs.push_back (TestSceneDesc ("teapot5", "Teapot in day with glass ball"));
  descs.push_back (TestSceneDesc ("teapot6", "Teapot at night with gold ball"));
  descs.push_back (TestSceneDesc ("teapot7", "Teapot in day with gold ball"));
}



static void
def_scene_orange (const string &name, unsigned num,
		  Scene &scene, Camera &camera)
{
  // Orange mesh and coords come from .nff file
  //
  camera.set_z_mode (Camera::Z_DECREASES_FORWARD);

  // Note that the coordinates in this scene are weird -- it uses Z as
  // "height" rather than depth.

  const Material *silver
    = scene.add (new Mirror (0.3, Color (0.7, 0.8, 0.7), 10, 5));
  const Material *orange
    = scene.add (new Material (Color (0.6,0.5,0.05), Material::phong (250)));
  const Material *glass
    = scene.add (new Glass (Medium (0.95, 1.5), 0.1, 0.01,
			    Material::phong (2000, 1.5)));
  const Material *gloss_black
    = scene.add (new Mirror (0.3, 0.02, 10));
  const Material *ivory
    = scene.add (new Mirror (0.2, 2 * Color (1.1, 1, 0.8), 5, 2));
  const Material *brown
    = scene.add (new Material (Color (0.3, 0.2, 0.05)));

  scene.add (new Mesh (gloss_black, "board1.msh"));
  scene.add (new Mesh (ivory, "board2.msh"));
  scene.add (new Mesh (brown, "board3.msh"));

  if ((num & 1) == 0)
    /* night-time orange */
    {
      scene.add (new PointLight (Pos (-3.1, 9.8, 12.1), 100));
      add_bulb (scene, Pos (4.7, 2, 3), 0.2, 4 * Color (1, 1, 0.3));
      add_bulb (scene, Pos (-1, -2, 4), 0.2, 4 * Color (1, 1, 0.3));
      scene.set_background (Color (0.01, 0.01, 0.02));
    }
  else
    /* day-time orange */
    {
      scene.add (new PointLight (Pos (-3.1, 9.8, 12.1), 90));
      scene.add (new PointLight (Pos (11.3, 5.1, 8.8), 50));
      scene.set_background (Color (0.078, 0.361, 0.753));
    }
  num /= 2;

  dist_t max_err = 0.0002;
  bool smooth = true;

  const Material *mat;
  switch (num)
    {
    default:
    case 0: mat = orange; break;
    case 1: mat = silver; break;
    case 2: mat = glass; max_err = 0.001; break;
    }

  scene.add (new Mesh (mat, SphereTesselFun (Pos (0, 0, 3), 3, 0.002),
		       Tessel::ConstMaxErr (max_err), smooth));

  camera.set_vert_fov (M_PI_4 * 0.9);
  camera.move (Pos (4.86, 7.2, 5.4));
  camera.point (Pos (0, -0.2, 0), Vec (0, 0, 1));
}

static void
add_scene_descs_orange (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("orange", "Giant orange on a chessboard"));
  descs.push_back (TestSceneDesc ("orange1", "Big rough silver ball on a chessboard"));
  descs.push_back (TestSceneDesc ("orange2", "Big rough glass ball on a chessboard"));
}



const void
add_rect (Scene &scene, const Material *mat,
	  const Pos &corner_0, const Pos &mid_corner_0, const Pos &corner_1)
{
  Triangle *t0
    = new Triangle (mat, corner_0, mid_corner_0, corner_1);
  Triangle *t1
    = new Triangle (mat,
		    corner_1, corner_1 + (corner_0 - mid_corner_0), corner_0);

  scene.add (t0);
  scene.add (t1);
}

const void
add_cube (Scene &scene, const Material *mat,
	  const Pos &corner, const Vec &up, const Vec &right, const Vec &fwd)
{
  add_rect (scene, mat, corner, corner + up, corner + right + up);
  add_rect (scene, mat, corner, corner + fwd, corner + fwd + up);
  add_rect (scene, mat, corner + up, corner + up + fwd, corner + up + fwd + right);
}

static void
def_scene_cornell_box (const string &name, unsigned num,
		       Scene &scene, Camera &camera)
{
  unsigned soft_shadow_count = 1; //5;
  float light_intens = 2.5;
  unsigned glow_mag = 1;
  bool fill_light = true;
  float scale = 1;

  coord_t rear   =  2   * scale, front = -3   * scale;
  coord_t left   = -1.2 * scale, right =  1.2 * scale;
  coord_t bottom =  0   * scale, top   =  2   * scale;

  dist_t width   = right - left;
  dist_t height  = top - bottom;
  coord_t mid_x  = left + width / 2;
  coord_t mid_z  = 0;

  dist_t light_width = width / 3;
  dist_t light_inset = 0.01 * scale;
  coord_t light_x    = left + width / 2;
  coord_t light_z    = 0;

  // Various spheres use this radius
  //
  dist_t rad = 0.4 * scale;

  // Appearance of left and right walls; set in ifs below
  //
  const Material *left_wall_mat, *right_wall_mat;

  // Corners of room (Left/Right + Bottom/Top + Rear/Front)
  //
  const Pos LBR (left, bottom, rear),   RBR (right, bottom, rear);
  const Pos RTR (right, top, rear),     LTR (left, top, rear);
  const Pos RBF (right, bottom, front), RTF (right, top, front);
  const Pos LBF (left, bottom, front),  LTF (left, top, front);

  const Material *wall_mat = scene.add (new Material (1));

  if (num == 1)
    {
      light_intens = 1.5;
      fill_light = false;
      light_z += scale * 0.2;

      const Material *crystal
	= scene.add (new Glass (Medium (Color (0.8, 0.8, 0.4), 1.35), 0.25, 0.1,
				Material::lambert));
      const Material *silver
	= scene.add (new Mirror (0.9, 0.05, Material::lambert));

      // silver sphere
      scene.add (new Sphere (silver, LBR + Vec (rad*1.55, rad, -rad*3), rad));
      // crystal sphere
      scene.add (new Sphere (crystal, Pos (right - rad*1.5, rad, -rad), rad));

      left_wall_mat = scene.add (new Material (Color (0.6, 0.1, 0.1)));
      right_wall_mat = scene.add (new Material (Color (0.1, 0.1, 0.6)));
    }
  else // default
    {
      soft_shadow_count = 5;

      // The reflected light doesn't look bright enough otherwise... XXX
      glow_mag = 4;

      Color light_blue (0.6, 0.6, 1);
//       const Material *gloss_blue
// 	= scene.add (new Material (light_blue, Material::phong (700, 5)));
      const Material *gloss_blue
	= scene.add (new Mirror (0.1, light_blue, Material::lambert));
      const Material *white
	= scene.add (new Material (1, Material::phong (50)));

      // blue sphere
      scene.add (new Sphere (gloss_blue, RBR + Vec (-rad*1.7, rad, -rad*4), rad));

      left_wall_mat = scene.add (new Material (Color (1, 0.35, 0.35)));
      right_wall_mat = scene.add (new Material (Color (0.35, 1, 0.35)));

      dist_t cube_sz = height * 0.4;
      float cube_angle = 50 * (M_PI / 180);
      Vec cube_up (0, cube_sz, 0);
      Vec cube_right (cube_sz * cos(cube_angle), 0, cube_sz * sin(cube_angle));
      Vec cube_fwd (cube_sz * -sin(cube_angle), 0, cube_sz * cos(cube_angle));
      add_cube (scene, white,
		Pos (mid_x - width / 4.5, bottom, mid_z - width / 3),
		cube_up, cube_right, cube_fwd);
    }

  // light

  const coord_t light_left  = light_x - light_width / 2;
  const coord_t light_right = light_x + light_width / 2;
  const coord_t light_front = light_z - light_width / 2;
  const coord_t light_back  = light_z + light_width / 2;

  add_rect (scene, scene.add (new Glow (light_intens * glow_mag)),
	    Pos (light_left, top, light_front),
	    Pos (light_left, top, light_back),
	    Pos (light_right, top, light_back));

  if (soft_shadow_count > 1)
    {
      unsigned num_sublights_per_dimen = soft_shadow_count;
      dist_t sub_light_edge_offs = light_width / 12;
      dist_t sub_light_spacing
	= ((light_width - sub_light_edge_offs * 2)
	   / (num_sublights_per_dimen - 1));
      float sub_light_intens
	= light_intens / (num_sublights_per_dimen * num_sublights_per_dimen);
  
      coord_t x = light_left + sub_light_edge_offs;
      for (unsigned i = 0; i < num_sublights_per_dimen; i++)
	{
	  coord_t z = light_front + sub_light_edge_offs;
	  for (unsigned j = 0; j < num_sublights_per_dimen; j++)
	    {
	      scene.add (new PointLight (Pos (x, top + light_inset, z),
					 sub_light_intens));
	      z += sub_light_spacing;
	    }
	  x += sub_light_spacing;
	}
    }
  else
    {
      scene.add (new PointLight (Pos (light_x, top + light_inset, light_z),
				 light_intens));
    }

  // Back wall
  add_rect (scene, wall_mat, LBR, LTR, RTR);
  // Right wall
  add_rect (scene, right_wall_mat, RBR, RTR, RTF);
  // Left wall
  add_rect (scene, left_wall_mat, LBR, LTR, LTF);
  // Floor
  add_rect (scene, wall_mat, LBF, LBR, RBR);
  // Ceiling
  add_rect (scene, wall_mat, LTF, LTR, Pos (light_left, top, rear));
  add_rect (scene, wall_mat, RTR, RTF, Pos (light_right, top, front));
  add_rect (scene, wall_mat,
	    Pos (light_left, top, front),
	    Pos (light_left, top, light_front),
	    Pos (light_right, top, light_front));
  add_rect (scene, wall_mat,
	    Pos (light_left, top, light_back),
	    Pos (light_left, top, rear),
	    Pos (light_right, top, rear));

  // for debugging
  if (fill_light)
    scene.add (new PointLight (Pos (left + 0.1, bottom + 0.1, front + 0.1),
			       light_intens / 4));

  camera.move (Pos  (mid_x, 0.525 * height + bottom, -6.6 * scale));
  camera.point (Pos (mid_x, 0.475 * height + bottom, 0), Vec (0, 1, 0));
  camera.set_horiz_fov (M_PI_4 * 0.7);
}

static void
add_scene_descs_cornell_box (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("cbox0", "Cornell box, Henrik Jensen version 1 (simulated soft shadows)"));
  descs.push_back (TestSceneDesc ("cbox1", "Cornell box, Henrik Jensen version 0 (glass & mirror spheres)"));
}


// CS465 test scenes

// from cs465 Test1.xml
static void
def_scene_cs465_test1 (Scene &scene, Camera &camera)
{
  // First test scene, only uses spheres, Lambertian shading,
  // and one light directly above the center of the 3 spheres.

  const Phong &ph300 = Material::phong (300), &ph400 = Material::phong (400);
  const Material *mat1 = scene.add (new Material (Color (1, 0.5, 0.2)));
  const Material *mat2 = scene.add (new Material (Color (0.8, 0.8, 0.8), ph300));
  const Material *mat3 = scene.add (new Material (Color (0.8, 0, 0), ph400));

  camera.move (Pos (0, 3, -4));
  camera.point (Pos (0, 0, 0), Vec (0, 1, 0));
  scene.add (new Sphere (mat1, Pos (0, 0, -0.866), 1));
  scene.add (new Sphere (mat2, Pos (1, 0, 0.866), 1));
  scene.add (new Sphere (mat3, Pos (-1, 0, 0.866), 1));
  scene.add (new PointLight (Pos (0, 5, 0), 25));
}

static void
def_scene_cs465_test2 (Scene &scene, Camera &camera)
{
  // Sphere on plane.  Sphere has greenish phong material.

  camera.move (Pos (0, 4, 4));
  camera.point (Pos (-0.5, 0, 0.5), Vec (0, 1, 0));

  const Phong &ph100 = Material::phong (100, Color (0.3, 0.3, 0.3));

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
  scene.add (new PointLight (Pos (5, 5, 0), 8));
  scene.add (new PointLight (Pos (5.1, 5, 0), 8));
  scene.add (new PointLight (Pos (5.2, 5, 0), 8));
  scene.add (new PointLight (Pos (5.3, 5, 0), 8));
  scene.add (new PointLight (Pos (5, 5.1, 0), 8));
  scene.add (new PointLight (Pos (5.1, 5.1, 0), 8));
  scene.add (new PointLight (Pos (5.2, 5.1, 0), 8));
  scene.add (new PointLight (Pos (5.3, 5.1, 0), 8));
  scene.add (new PointLight (Pos (5, 5.2, 0), 8));
  scene.add (new PointLight (Pos (5.1, 5.2, 0), 8));
  scene.add (new PointLight (Pos (5.2, 5.2, 0), 8));
  scene.add (new PointLight (Pos (5.3, 5.2, 0), 8));
  scene.add (new PointLight (Pos (5, 5.3, 0), 8));
  scene.add (new PointLight (Pos (5.1, 5.3, 0), 8));
  scene.add (new PointLight (Pos (5.2, 5.3, 0), 8));
  scene.add (new PointLight (Pos (5.3, 5.3, 0), 8));

  // fill light
  scene.add (new PointLight (Pos (-5, 1, -22), 100));
}

static void
def_scene_cs465_test3 (Scene &scene, Camera &camera)
{
  // Three spheres and a box on a plane.  Mix of Lambertian
  // and Phong materials.

  camera.move (Pos (6, 6, 6));
  camera.point (Pos (0, 0, 0), Vec (0, 1, 0));

  const Phong &ph300_1 = Material::phong (300, Color (1, 1, 1));
  const Phong &ph300_2 = Material::phong (300, Color (2, 2, 2));

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

  scene.add (new PointLight (Pos (0, 10, 5), Color (50, 30, 30)));
  scene.add (new PointLight (Pos (5, 10, 0), Color (30, 30, 50)));
  scene.add (new PointLight (Pos (5, 10, 5), Color (30, 50, 30)));
  scene.add (new PointLight (Pos (6, 6, 6), Color (25, 25, 25)));
}

void 
def_scene_cs465_test4 (Scene &scene, Camera &camera)
{
  // Low resolution Stanford Bunny Mesh.

  camera.move (Pos (0, 0, 3));
  camera.point (Pos (-0.25, -0.07, 0), Vec (0, 1, 0));
  camera.set_vert_fov (M_PI_4);

  const Material *red = scene.add (new Material (Color (1, 0, 0)));
  const Material *gray = scene.add (new Material (Color (0.6, 0.6, 0.6)));

  Mesh *bunny = new Mesh (red);
  bunny->load ("bunny500.msh");
  scene.add (bunny);
  
  // ground
  scene.add (new Triangle (gray,
			   Pos (-10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, -10)));
  scene.add (new Triangle (gray,
			   Pos (10, -0.65, -10), Pos (-10, -0.65, 10),
			   Pos (10, -0.65, 10)));
  
  scene.add (new PointLight (Pos (0, 10, 0), 100));
  scene.add (new PointLight (Pos (15, 2, 0), 100));
  scene.add (new PointLight (Pos (0, 1, 15), 100));
}

static void
def_scene_cs465 (const string &name, unsigned num, Scene &scene, Camera &camera)
{
  switch (num)
    {
    case 1: def_scene_cs465_test1 (scene, camera); break;
    case 2: def_scene_cs465_test2 (scene, camera); break;
    case 3: def_scene_cs465_test3 (scene, camera); break;
    case 4: def_scene_cs465_test4 (scene, camera); break;
    default:
      throw runtime_error ("unknown cs465 test scene");
    }
}

static void
add_scene_descs_cs465 (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("cs465-1", "Cornell CS465 test-scene 1"));
  descs.push_back (TestSceneDesc ("cs465-2", "Cornell CS465 test-scene 2"));
  descs.push_back (TestSceneDesc ("cs465-3", "Cornell CS465 test-scene 3"));
  descs.push_back (TestSceneDesc ("cs465-4", "Cornell CS465 test-scene 4"));
}



static void
def_scene_tessel (const string &name, unsigned num,
		  Scene &scene, Camera &camera)
{
  switch (num / 10)
    {
    case 0:
      camera.move (Pos (2, 3, -4)); break;
    case 1:
      camera.move (Pos (4, 0.5, 0.001)); break;
    case 2:
      camera.move (Pos (4, 2, 0.001)); break;
    }
  camera.point (Pos (0, 0, 0), Vec (0, 1, 0));

  num %= 10;

  const Material *silver
    = scene.add (new Mirror (0.3, Color (0.7, 0.8, 0.7), 10, 5));
  const Material *green
    = scene.add (new Material (Color (0.1,1,0.1), Material::phong (250)));

  const Material *mat = ((num & 1) == 0) ? green : silver;

  num >>= 1;			// remove lowest bit

  const Material *orange
    = scene.add (new Material (Color (0.6,0.5,0.05), Material::phong (250)));
  const Material *ivory
    = scene.add (new Mirror (0.2, 2 * Color (1.1, 1, 0.8), 5, 2));

  coord_t base_rad = (num == 0) ? 1.5 : 1;
  coord_t base_height = (num == 0) ? -0.22 * base_rad : 0;

  scene.add (new Triangle (orange, Pos (base_rad, base_height, base_rad),
			   Pos (base_rad, base_height, -base_rad),
			   Pos (-base_rad, base_height, -base_rad)));
  scene.add (new Triangle (ivory, Pos (-base_rad, base_height, base_rad),
			   Pos (base_rad, base_height, base_rad),
			   Pos (-base_rad, base_height, -base_rad)));

  switch (num)
    {
      dist_t perturb;

    default:
    case 0:
      scene.add (new Mesh (mat, SincTesselFun (Pos (0, 0, 0), 1.5),
			   Tessel::ConstMaxErr (tessel_accur)));
      break;

    case 1:
    case 2:
    case 3:
    case 4:
      switch (num)
	{
	default: case 1: perturb = 0;     break;
	case 2: perturb = 0.001; break;
	case 3: perturb = 0.002; break;
	case 4: perturb = 0.01;  break;
	}

      scene.add (new Mesh (mat, SphereTesselFun (Pos (0, 0, 0), 1, perturb),
			   Tessel::ConstMaxErr (tessel_accur)));
      break;
    }

  scene.add (new PointLight (Pos (0, 5, 5), 50));
  scene.add (new PointLight (Pos (-5, 5, -5), 15));
  scene.add (new PointLight (Pos (10, -5, -15), 100));
}

static void
add_scene_descs_tessel (vector<TestSceneDesc> &descs)
{
  descs.push_back (TestSceneDesc ("tessel", "Tessellation test: green sinc function"));
  descs.push_back (TestSceneDesc ("tessel1", "Tessellation test: silver sinc function"));
  descs.push_back (TestSceneDesc ("tessel2", "Tessellation test: green ball"));
  descs.push_back (TestSceneDesc ("tessel3", "Tessellation test: silver ball"));
  descs.push_back (TestSceneDesc ("tessel4,6,8", "Tessellation test: rough green balls"));
  descs.push_back (TestSceneDesc ("tessel5,7,9", "Tessellation test: rough silver balls"));
}



void
Snogray::def_test_scene (const string &_name, Scene &scene, Camera &camera)
{
  string name (_name);		// make a local copy

  // Devide the name into a "base name" and "scene number" if possible
  //
  unsigned num = 0;
  unsigned base_end = name.find_last_not_of ("0123456789");
  if (base_end < name.length ())
    {
      istringstream idiots (name.substr (base_end + 1));
      idiots >> num;

      base_end = name.find_last_not_of ("-_ ", base_end);

      name = name.substr (0, base_end + 1);
    }

  if (name == "miles")
    def_scene_miles (name, num, scene, camera);
  else if (name == "teapot")
    def_scene_teapot (name, num, scene, camera);
  else if (name == "orange")
    def_scene_orange (name, num, scene, camera);
  else if (ends_in (name, "bunny"))
    def_scene_pretty_bunny (name, num, scene, camera);
  else if (name == "cornell-box" || name == "cbox")
    def_scene_cornell_box (name, num, scene, camera);
  else if (name == "cs465")
    def_scene_cs465 (name, num, scene, camera);
  else if (name == "tessel")
    def_scene_tessel (name, num, scene, camera);
  else
    throw (runtime_error ("Unknown test scene"));
}

vector<TestSceneDesc>
Snogray::list_test_scenes ()
{
  vector<TestSceneDesc> descs;

  add_scene_descs_miles (descs);
  add_scene_descs_teapot (descs);
  add_scene_descs_orange (descs);
  add_scene_descs_pretty_bunny (descs);
  add_scene_descs_cornell_box (descs);
  add_scene_descs_cs465 (descs);
  add_scene_descs_tessel (descs);

  return descs;
}

// arch-tag: 307938a9-c663-4949-a58b-fb51040a6529
