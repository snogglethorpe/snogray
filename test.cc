#include <iostream>

#include "snogray.h"
#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "light.h"
#include "image.h"

using namespace std;

static unsigned
get_unsigned_opt_arg (char **&argv, const char *prog_name)
{
  if ((*argv)[2])
    return atoi (*argv + 2);
  else if (argv[1] && isdigit (*argv[1]))
    return atoi (*++argv);
  else
    {
      cerr << prog_name << ": " << *argv
	   << ": option requires a numeric argument" << endl;
      exit (2);
    }
}  

int main (int argc, char **argv)
{
  SnogRay snogray;
  const char *prog_name = *argv;
  unsigned final_width = 640, final_height = 480, aa_factor = 1;
  const char *output_file = 0;

  while (*++argv)
    if (**argv == '-')
      switch ((*argv)[1])
	{
	case 'a':
	  aa_factor = get_unsigned_opt_arg (argv, prog_name);
	  break;
	case 'w':
	  final_width = get_unsigned_opt_arg (argv, prog_name);
	  break;
	case 'h':
	  final_height = get_unsigned_opt_arg (argv, prog_name);
	  break;

	default:
	  cerr << prog_name << ": " << *argv << ": unknown option" << endl;
	  exit (1);
	}
    else
      break;

  if (*argv)
    output_file = *argv;
  else
    {
      cerr << "Usage: " << prog_name
	   << " [-a AA_FACTOR] [-w WIDTH] [-h HEIGHT]"
	   << " OUTPUT_IMAGE_FILE" << endl;
      exit (10);
    }

  const unsigned width = final_width * aa_factor;
  const unsigned height = final_height * aa_factor;
  snogray.camera.set_aspect_ratio ((float)width / (float)height);

//  Lambert mat1 (Color (1, 0.5, 0.2));
//  Lambert mat2 (Color (0.5, 0.5, 0));
//  Lambert mat3 (Color (1, 0.5, 1));
//  Lambert mat4 (Color (1, 0.5, 1));
  Lambert mat1 (Color (1, 0.5, 0.2));
  Phong mat2 (Color (0.8, 0.8, 0.8), 300);
//   Phong mat3 (Color (0.1, 0.1, 0.1), 400);
  Phong mat3 (Color (0.8, 0, 0), 400);
  Lambert mat4 (Color (0.2, 0.5, 0.1));

  // First test scene
  snogray.scene.add (new Light (Pos (0, 15, 0), 30));
  snogray.scene.add (new Light (Pos (0, 0, -5), 30));
  snogray.scene.add (new Light (Pos (-5, 10, 0), Color (0, 0, 1), 40));
  snogray.scene.add (new Light (Pos (-40, 15, -40), 300));
  snogray.scene.add (new Light (Pos (-40, 15,  40), 300));
  snogray.scene.add (new Light (Pos ( 40, 15, -40), 300));
  snogray.scene.add (new Light (Pos ( 40, 15,  40), 300));
  snogray.scene.add (new Sphere (&mat1, Pos (0, 2, 7), 5));
  snogray.scene.add (new Sphere (&mat2, Pos (-8, 0, 3), 3));
  snogray.scene.add (new Sphere (&mat3, Pos (-6, 5, 2), 1));
  snogray.scene.add (new Triangle (&mat4,
				   Pos (-100, -3, -100),
				   Pos (100, -3, -100),
				   Pos (100, -3, 100)));
  snogray.scene.add (new Triangle (&mat4,
				   Pos (-100, -3, -100),
				   Pos (100, -3, 100),
				   Pos (-100, -3, 100)));
//   snogray.camera.move (Pos (-3, 2, -18));
  snogray.camera.move (Pos (0, 50, 30));
  snogray.camera.point (Pos (0, 5, 0));

//   // from cs465 Test1.xml
//   const unsigned width = 512 * aa_factor;
//   const unsigned height = 512 * aa_factor;
//   snogray.camera.move (Pos (0, 3, -4));
//   snogray.camera.point (Pos (0, 0, 0), Vec (0, 1, 0));
//   snogray.camera.set_aspect_ratio (1);
//   snogray.scene.add (new Sphere (&mat1, Pos (0, 0, -0.866), 1));
//   snogray.scene.add (new Sphere (&mat2, Pos (1, 0, 0.866), 1));
//   snogray.scene.add (new Sphere (&mat3, Pos (-1, 0, 0.866), 1));
//   snogray.scene.add (new Light (Pos (0, 5, 0), 25));

  Image image (width, height);

  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++)
      {
	float u = (float)x / (float)width;
	float v = (float)(height - y) / (float)height;
	image (x, y) = snogray.render (u, v);
      }

  cout << "scene_closest_intersect_calls = " << snogray.scene.stats.scene_closest_intersect_calls << endl;
  cout << "obj_closest_intersect_calls = " << snogray.scene.stats.obj_closest_intersect_calls << endl;
  cout << "scene_intersects_calls = " << snogray.scene.stats.scene_intersects_calls << endl;
  cout << "obj_intersects_calls = " << snogray.scene.stats.obj_intersects_calls << endl;

  if (aa_factor > 1)
    {
      Image aa_image (image, aa_factor);
      aa_image.write_png_file (output_file);
    }
  else
    image.write_png_file (output_file);
}

// arch-tag: 2bd751cf-5474-4782-bee1-9e58ce38ab7d
