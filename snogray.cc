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
#include <cstring>

#include "cmdlineparser.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "image.h"
#include "image-png.h"
#include "image-exr.h"

#include "sphere.h"
#include "triangle.h"
#include "lambert.h"
#include "phong.h"
#include "glow.h"

using namespace Snogray;
using namespace std;


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


// LimitSpec datatype

// A "limit spec" represents a user's specification of a single
// rendering limit (on x, y, width, or height), which may be absolute,
// relative, a proportion of the image size, or both.
struct LimitSpec
{
  LimitSpec (unsigned _abs_val)
    : abs_val (_abs_val), is_frac (false), is_rel (false) { }
  LimitSpec (int _abs_val)
    : abs_val (_abs_val), is_frac (false), is_rel (false) { }
  LimitSpec (float _frac_val)
    : frac_val (_frac_val), is_frac (true), is_rel (false) { }
  LimitSpec (double _frac_val)
    : frac_val (_frac_val), is_frac (true), is_rel (false) { }

  bool parse (const char *&str);
  unsigned apply (unsigned range, unsigned base = 0) const;

  bool is_frac;
  unsigned abs_val;
  float frac_val;

  bool is_rel;
};

bool
LimitSpec::parse (const char *&str)
{
  const char *skip = str + strspn (str, "0123456789");
  char *end = 0;

  is_frac = (*skip == '.' || *skip == '%');

  if (is_frac)
    // floating-point fractional spec
    {
      frac_val = strtof (str, &end);
      if (!end || end == str)
	return false;

      if (*end == '%')
	{
	  end++;
	  frac_val /= 100;
	}

      if (frac_val < 0 || frac_val > 1.0)
	return false;
    }
  else
    // integer absolute spec
    {
      abs_val = strtoul (str, &end, 10);
      if (!end || end == str)
	return false;
    }

  str = end;

  return true;
}

unsigned
LimitSpec::apply (unsigned range, unsigned base) const
{
  unsigned val = is_frac ? (unsigned)(frac_val * range) : abs_val;

  if (is_rel)
    val += base;

  if (val > range)
    {
      cerr << val << ": limit out of range (0 - " << range << ")" << endl;
      exit (5);
    }

  return val;
}


// Parsers for --size and --limit command-line option arguments

static void
parse_size_opt_arg (CmdLineParser &clp, unsigned &width, unsigned &height)
{
  const char *size = clp.opt_arg ();
  char *end = 0;

  width = strtoul (size, &end, 10);

  if (end && end != size)
    {
      size = end + strspn (end, " ,x");

      height = strtoul (size, &end, 10);

      if (end && end != size && *end == '\0')
	return;
    }

  clp.opt_err ("requires a size specification (WIDTHxHEIGHT)");
}

static void
parse_limit_opt_arg (CmdLineParser &clp,
		     LimitSpec &limit_x_spec, LimitSpec &limit_y_spec,
		     LimitSpec &limit_max_x_spec, LimitSpec &limit_max_y_spec)
{
  const char *spec = clp.opt_arg ();

  bool ok = limit_x_spec.parse (spec);
  if (ok)
    {
      spec += strspn (spec, ", ");

      ok = limit_y_spec.parse (spec);
      if (ok)
	{
	  spec += strspn (spec, " ");

	  limit_max_x_spec.is_rel = limit_max_y_spec.is_rel = (*spec == '+');

	  spec += strspn (spec, "+-");
	  spec += strspn (spec, " ");

	  ok = limit_max_x_spec.parse (spec);
	  if (ok)
	    {
	      spec += strspn (spec, ", ");
	      ok = limit_max_y_spec.parse (spec);
	    }
	}
    }

  if (!ok || *spec != '\0')
    clp.opt_err ("requires a limit specification (X,Y[+-]W,H)");
}


// Main driver

int main (int argc, char *const *argv)
{
  Scene scene;
  Camera camera;
  const char *prog_name = *argv;
  unsigned final_width = 640, final_height = 480;
  unsigned aa_factor = 1, aa_overlap = 0;
  float (*aa_filter) (int offs, unsigned size) = ImageOutput::aa_gauss_filter;
  LimitSpec limit_x_spec (0), limit_y_spec (0);
  LimitSpec limit_max_x_spec (1.0), limit_max_y_spec (1.0);
  const char *limit = 0;
  float target_gamma = 2.2;	// Only applies to formats that understand it
  const char *output_fmt = 0;	// 0 means auto-detect
  bool quiet = false, progress = true; // quiet mode, progress indicator
  bool progress_set = false;

  // Command-line option specs
  static struct option long_options[] = {
    { "width",		required_argument, 0, 'w' },
    { "height",		required_argument, 0, 'h' },
    { "aa-factor",	required_argument, 0, 'a' },
    { "aa-overlap",	required_argument, 0, 'A' },
    { "aa-filter",	required_argument, 0, 'F' },
    { "gamma",		required_argument, 0, 'g' },
    { "size",		required_argument, 0, 's' },
    { "limit",		required_argument, 0, 'l' },
    { "output-format",	required_argument, 0, 'O' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    { 0, 0, 0, 0 }
  };
  CmdLineParser clp (argc, argv, "a:A:F:w:h:g:s:l:O:qpP", long_options);

  // Parse command-line options
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	// Verbosity options
      case 'q':
	quiet = true;
	if (! progress_set)
	  progress = false;
	break;
      case 'p':
	progress = true;
	progress_set = true;
	break;
      case 'P':
	progress = false;
	progress_set = true;
	break;

	// Size options
      case 's':
	parse_size_opt_arg (clp, final_width, final_height);
	break;
      case 'l':
	parse_limit_opt_arg (clp, limit_x_spec, limit_y_spec,
			     limit_max_x_spec, limit_max_y_spec);
	break;
      case 'w':
	final_width = clp.unsigned_opt_arg ();
	break;
      case 'h':
	final_height = clp.unsigned_opt_arg ();
	break;

	// Anti-aliasing options
      case 'a':
	aa_factor = clp.unsigned_opt_arg ();
	break;
      case 'A':
	aa_overlap = clp.unsigned_opt_arg ();
	break;
      case 'F':
	{
	  const char *filt_name = clp.opt_arg ();
	  if (strcmp (filt_name, "box") == 0)
	    aa_filter = ImageOutput::aa_box_filter;
	  else if (strcmp (filt_name, "triang") == 0)
	    aa_filter = ImageOutput::aa_triang_filter;
	  else if (strcmp (filt_name, "gauss") == 0)
	    aa_filter = ImageOutput::aa_gauss_filter;
	  else
	    clp.opt_err ("requires an anti-aliasing filter name"
			 " (box, triang, gauss)");
	}
	break;

	// Output image options
      case 'g':
	target_gamma = clp.float_opt_arg ();
	break;
      case 'O':
	output_fmt = clp.opt_arg ();
	break;
      }

  // Final output file parameter
  if (clp.num_remaining_args() != 1)
    {
      cerr << "Usage: " << prog_name
	   << " [-a AA_FACTOR] [-w WIDTH] [-h HEIGHT] [-g GAMMA]"
	   << " OUTPUT_IMAGE_FILE" << endl;
      exit (10);
    }
  const char *output_file = clp.get_arg();

  // The size of the actual full final image
  const unsigned width = final_width * aa_factor;
  const unsigned height = final_height * aa_factor;

  // Set our drawing limits based on the scene size
  unsigned limit_x = limit_x_spec.apply (final_width);
  unsigned limit_y = limit_y_spec.apply (final_height);
  unsigned limit_width
    = limit_max_x_spec.apply (final_width, limit_x) - limit_x;
  unsigned limit_height
    = limit_max_y_spec.apply (final_height, limit_y) - limit_y;

  // Set output format automatically if necessary
  if  (! output_fmt)
    {
      const char *output_ext = rindex (output_file, '.');

      if (! output_ext)
	{
	  cerr << output_file
	       << ": No filename extension to determine output type"
	       << endl;
	  exit (25);
	}

      output_fmt = ++output_ext;
    }

  // Make the output-format-specific parameter block
  ImageSinkParams *image_params = 0;
  bool target_gamma_used = false;
  if (strcmp (output_fmt, "png") == 0)
    {
      image_params
	= new PngImageSinkParams (output_file, limit_width, limit_height,
				  target_gamma);
      target_gamma_used = true;
    }
  else if (strcmp (output_fmt, "exr") == 0)
    image_params
      = new ExrImageSinkParams (output_file, limit_width, limit_height);
  else
    {
      cerr << output_file << ": Unknown output type" << endl;
      exit (26);
    }

  // Print image info
  if (! quiet)
    {
      cout << "image.size = " << final_width << " x " << final_height << endl;
      if (limit_x != 0 || limit_y != 0
	  || limit_width != final_width || limit_height != final_height)
	cout << "image.limit = "
	     << limit_x << "," << limit_y
	     << " - " << limit_x + limit_width << "," << limit_y + limit_height
	     << " (" << limit_width << " x "  << limit_height << ")"
	     << endl;

      if (target_gamma_used)
	cout << "image.target_gamma = " << target_gamma << endl;

      // Anti-aliasing info
      if ((aa_factor + aa_overlap) > 1)
	{
	  if (aa_factor > 1)
	    cout << "image.aa_factor = " << aa_factor << endl;

	  if (aa_overlap > 0)
	    cout << "image.aa_kernel_size = " << (aa_factor + aa_overlap*2)
		 << " (overlap = " << aa_overlap << ")" << endl;
	  else
	    cout << "image.aa_kernel_size = " << aa_factor << endl;

	  cout << "image.aa_filter = ";
	  if (aa_filter == ImageOutput::aa_box_filter)
	    cout << "box";
	  else if (aa_filter == ImageOutput::aa_triang_filter)
	    cout << "triang";
	  else if (aa_filter == ImageOutput::aa_gauss_filter)
	    cout << "gauss";
	  else
	    cout << "???";
	  cout << endl;
	}
    }

  // Set camera aspect ratio to give pixels a 1:1 aspect ratio
  camera.set_aspect_ratio ((float)width / (float)height);

  // Define our scene!
  define_scene (scene, camera);

  // Print scene info
  if (! quiet)
    {
      cout << "scene.num_objects = " << scene.objs.size () << endl;
      cout << "scene.num_lights = " << scene.lights.size () << endl;
      cout << "scene.num_materials = " << scene.materials.size () << endl;
    }

  // The image we're creating
  ImageOutput image (*image_params, aa_factor, aa_overlap, aa_filter);

  // Limits in terms of higher-resolution pre-AA image
  unsigned hr_limit_x = limit_x * aa_factor;
  unsigned hr_limit_y = limit_y * aa_factor;
  unsigned hr_limit_max_x = hr_limit_x + limit_width * aa_factor;
  unsigned hr_limit_max_y = hr_limit_y + limit_height * aa_factor;

  // Main ray-tracing loop
  for (unsigned y = hr_limit_y; y < hr_limit_max_y; y++)
    {
      ImageRow &output_row = image.next_row ();

      // Progress indicator
      if (progress)
	{
	  if (aa_factor > 1)
	    cout << "\rrendering: line " << y / aa_factor
		 << "_" << (y - (y / aa_factor) * aa_factor);
	  else
	    cout << "\rrendering: line " << y;
	  cout << " (" << (y - hr_limit_y) * 100 / (hr_limit_max_y - hr_limit_y)
	       << "%)";
	  cout.flush ();
	}

      for (unsigned x = hr_limit_x; x < hr_limit_max_x; x++)
	{
	  float u = (float)x / (float)width;
	  float v = (float)(height - y) / (float)height;
	  Ray camera_ray = camera.get_ray (u, v);

	  output_row[x - hr_limit_x] = scene.render (camera_ray);
	}
    }

  if (progress)
    cout << endl;

  // Print render stats
  if (! quiet)
    {
      Scene::Stats &stats = scene.stats;
      cout << "stats.scene_closest_intersect_calls = "
	   << stats.scene_closest_intersect_calls << endl;
      cout << "stats.obj_closest_intersect_calls = "
	   << stats.obj_closest_intersect_calls << endl;
      cout << "stats.scene_intersects_calls = "
	   << stats.scene_intersects_calls << endl;
      cout << "stats.obj_intersects_calls = "
	   << stats.obj_intersects_calls << endl;
    }
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
