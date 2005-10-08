#include <iostream>

#include "snogray.h"
#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "light.h"
#include "image.h"

using namespace std;

int main (int argc, char **argv)
{
  SnogRay snogray;
//  Lambert mat1 (Color (1, 0.5, 0.2));
//  Lambert mat2 (Color (0.5, 0.5, 0));
//  Lambert mat3 (Color (1, 0.5, 1));
//  Lambert mat4 (Color (1, 0.5, 1));
  Lambert mat1 (Color (1, 0.5, 0.2));
  Phong mat2 (Color (0.5, 0.5, 0), 400);
  Phong mat3 (Color (1, 0.5, 1), 400);
  Lambert mat4 (Color (0.2, 0.5, 0.1));
  const unsigned aa_factor = 3;

  if (argc != 2)
    {
      cerr << "Usage: " << argv[0] << " OUTPUT_IMAGE_FILE" << endl;
      exit (10);
    }

  // First test scene
  const unsigned width = 640 * aa_factor;
  const unsigned height = 480 * aa_factor;
  snogray.camera.set_aspect_ratio ((float)width / (float)height);
  snogray.camera.move (Pos (-3, 2, -18));
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

  cout << "snogray.scene.stats.scene_closest_intersect_calls = " << snogray.scene.stats.scene_closest_intersect_calls << endl;
  cout << "snogray.scene.stats.obj_closest_intersect_calls = " << snogray.scene.stats.obj_closest_intersect_calls << endl;
  cout << "snogray.scene.stats.scene_intersects_calls = " << snogray.scene.stats.scene_intersects_calls << endl;
  cout << "snogray.scene.stats.obj_intersects_calls = " << snogray.scene.stats.obj_intersects_calls << endl;

  if (aa_factor > 1)
    {
      Image aa_image (image, aa_factor);
      aa_image.write_png_file (argv[1]);
    }
  else
    image.write_png_file (argv[1]);
}

// arch-tag: 2bd751cf-5474-4782-bee1-9e58ce38ab7d
