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
#include <iomanip>
#include <string>
#include <cstring>

#include "cmdlineparser.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "image.h"
#include "image-cmdline.h"

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
def_scene_miles_test1 (Scene &scene, Camera &camera)
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
}

// from cs465 Test1.xml
static void
def_scene_cs465_test1 (Scene &scene, Camera &camera)
{
  // First test scene, only uses spheres, Lambertian shading,
  // and one light directly above the center of the 3 spheres.

  Material *mat1 = scene.add (new Lambert (Color (1, 0.5, 0.2)));
  Material *mat2 = scene.add (new Phong (300, Color (0.8, 0.8, 0.8)));
//   Material *mat3 = scene.add (new Phong (400, Color (0.1, 0.1, 0.1)));
  Material *mat3 = scene.add (new Phong (400, Color (0.8, 0, 0)));
  Material *mat4 = scene.add (new Lambert (Color (0.2, 0.5, 0.1)));

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

  Material *sphereMat
    = scene.add (new Phong (100,
			    Color (0.249804, 0.218627, 0.0505882),
			    Color (0.3, 0.3, 0.3)));
  Material *grey
    = scene.add (new Lambert (Color (0.3, 0.3, 0.3)));

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

  Material *shinyBlack
    = scene.add (new Phong (300, Color (0.02, 0.02, 0.02), Color (2, 2, 2)));
  Material *shinyWhite
    = scene.add (new Phong (300, Color (0.6, 0.6, 0.6), Color (1, 1, 1)));
  Material *shinyGray
    = scene.add (new Phong (300, Color (0.2, 0.2, 0.2), Color (2, 2, 2)));
  Material *boxMat
    = scene.add (new Lambert (Color (0.3, 0.19, 0.09)));
  Material *gray
    = scene.add (new Lambert (Color (0.6, 0.6, 0.6)));
	
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

static void
define_scene (Scene &scene, Camera &camera)
{
  def_scene_miles_test1 (scene, camera);
//   def_scene_cs465_test1 (scene, camera);
//   def_scene_cs465_test2 (scene, camera);
//   def_scene_cs465_test3 (scene, camera);
}


// LimitSpec datatype

// A "limit spec" represents a user's specification of a single
// rendering limit (on x, y, width, or height), which may be absolute,
// relative, a proportion of the image size, or both.
struct LimitSpec
{
  LimitSpec (const char *_name, unsigned _abs_val)
    : name (_name), is_frac (false), abs_val (_abs_val), is_rel (false) { }
  LimitSpec (const char *_name, int _abs_val)
    : name (_name), is_frac (false), abs_val (_abs_val), is_rel (false) { }
  LimitSpec (const char *_name, float _frac_val)
    : name (_name), is_frac (true), frac_val (_frac_val), is_rel (false) { }
  LimitSpec (const char *_name, double _frac_val)
    : name (_name), is_frac (true), frac_val (_frac_val), is_rel (false) { }

  bool parse (const char *&str);
  unsigned apply (CmdLineParser &clp, unsigned range, unsigned base = 0) const;

  const char *name;

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
LimitSpec::apply (CmdLineParser &clp, unsigned range, unsigned base) const
{
  unsigned val = is_frac ? (unsigned)(frac_val * range) : abs_val;

  if (is_rel)
    val += base;

  if (val > range)
    {
      cerr << clp.err_pfx()
	   << val << ": " << name
	   << " limit out of range (0 - " << range << ")" << endl;
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


// Random string helper functions

// Return a string version of NUM
static string 
stringify (unsigned num)
{
  string str = (num > 9) ? stringify (num / 10) : "";
  char ch = (num % 10) + '0';
  str += ch;
  return str;
}

// Return a string version of NUM, with commas added every 3rd place
static string 
commify (unsigned long long num, unsigned sep_count = 1)
{
  string str = (num > 9) ? commify (num / 10, sep_count % 3 + 1) : "";
  char ch = (num % 10) + '0';
  if (sep_count == 3 && num > 9)
    str += ',';
  str += ch;
  return str;
}


// Main driver

static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << "[OPTION...] [OUTPUT_IMAGE_FILE]" << endl;
}

static void
help (CmdLineParser &clp, ostream &os)
{
  usage (clp, os);
  os << "Ray-trace an image" << endl
     << endl
<< "  -w, --width=PIXELS         Set image width to PIXELS wide" << endl
<< "  -h, --height=LINES         Set image height to LINES high" << endl
<< "  -s, --size=WIDTHxHEIGHT    Set image size to WIDTH x HEIGHT pixels/lines"
     << endl
<< "  -l, --limit=LIMIT_SPEC     Limit output to area defined by LIMIT_SPEC"
     << endl
     << endl
<< "  -q, --quiet                Do not output informational or progress messages"
     << endl
<< "  -P, --no-progress          Do not output progress indicator" << endl
<< "  -p, --progress             Output progress indicator despite --quiet"
     << endl
     << endl
     << IMAGE_OUTPUT_OPTIONS_HELP << endl
     << endl
     << CMDLINEPARSER_GENERAL_OPTIONS_HELP << endl
     << endl
     << "If no filename is given, standard output is used.  The output"  << endl
     << "image format is guessed using the output filename when possible"<< endl
     << "(using the file's extension)." << endl;
}

int main (int argc, char *const *argv)
{
  // Command-line option specs
  static struct option long_options[] = {
    { "width",		required_argument, 0, 'w' },
    { "height",		required_argument, 0, 'h' },
    { "size",		required_argument, 0, 's' },
    { "limit",		required_argument, 0, 'l' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    "w:h:s:l:qpP"
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  unsigned final_width = 640, final_height = 480;
  LimitSpec limit_x_spec ("min-x", 0), limit_y_spec ("min-y", 0);
  LimitSpec limit_max_x_spec ("max-x", 1.0), limit_max_y_spec ("max-y", 1.0);
  bool quiet = false, progress = true; // quiet mode, progress indicator
  bool progress_set = false;
  ImageCmdlineSinkParams image_sink_params (clp);

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

	IMAGE_OUTPUT_OPTION_CASES (clp, image_sink_params);

	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  // Final output file parameter
  if (clp.num_remaining_args() > 1)
    {
      usage (clp, cerr);
      cerr << "Try `" << clp.prog_name() << " --help' for more information"
	   << endl;
      exit (10);
    }
  image_sink_params.file_name = clp.get_arg();

  // We reference this a lot below, so make a local copy
  unsigned aa_factor = image_sink_params.aa_factor;
  if (aa_factor == 0)
    aa_factor = 1;

  // The size of the actual full final image
  const unsigned width = final_width * aa_factor;
  const unsigned height = final_height * aa_factor;

  // Set our drawing limits based on the scene size
  unsigned limit_x = limit_x_spec.apply (clp, final_width);
  unsigned limit_y = limit_y_spec.apply (clp, final_height);
  unsigned limit_width
    = limit_max_x_spec.apply (clp, final_width, limit_x) - limit_x;
  unsigned limit_height
    = limit_max_y_spec.apply (clp, final_height, limit_y) - limit_y;

  // The size of what we actually output is the same as the limit
  image_sink_params.width = limit_width;
  image_sink_params.height = limit_height;

  // Create output image
  ImageOutput image (image_sink_params);

  // Print image info
  if (! quiet)
    {
      cout << "Image:" << endl;
      cout << "   size:         "
	   << setw (11) << (stringify (final_width)
			    + " x " + stringify (final_height))
	   << endl;

      if (limit_x != 0 || limit_y != 0
	  || limit_width != final_width || limit_height != final_height)
	cout << "   limit:"
	     << setw (19) << (stringify (limit_x) + "," + stringify (limit_y)
			      + " - " + stringify (limit_x + limit_width)
			      + "," + stringify (limit_y + limit_height))
	     << " (" << limit_width << " x "  << limit_height << ")"
	     << endl;

      if (image_sink_params.target_gamma != 0)
        cout << "   target_gamma:        "
	     << setw (4) << image_sink_params.target_gamma << endl;

      // Anti-aliasing info
      if ((aa_factor + image_sink_params.aa_overlap) > 1)
	{
	  if (aa_factor > 1)
	    cout << "   aa_factor:           "
		 << setw (4) << aa_factor << endl;

	  if (image_sink_params.aa_overlap > 0)
	    cout << "   aa_kernel_size:      "
		 << setw (4)
		 << (aa_factor + image_sink_params.aa_overlap*2)
		 << " (overlap = " << image_sink_params.aa_overlap << ")"
		 << endl;
	  else
	    cout << "   aa_kernel_size:      "
		 << setw (4) << aa_factor << endl;

	  cout << "   aa_filter:       " << setw (8);
	  if (image_sink_params.aa_filter == ImageOutput::aa_box_filter)
	    cout << "box";
	  else if (image_sink_params.aa_filter == ImageOutput::aa_triang_filter)
	    cout << "triang";
	  else if (image_sink_params.aa_filter == ImageOutput::aa_gauss_filter
		   || !image_sink_params.aa_filter)
	    cout << "gauss";
	  else
	    cout << "???";
	  cout << endl;
	}
    }

  // 
  Scene scene;
  Camera camera;

  // Set camera aspect ratio to give pixels a 1:1 aspect ratio
  camera.set_aspect_ratio ((float)width / (float)height);

  // Define our scene!
  define_scene (scene, camera);

  // Print scene info
  if (! quiet)
    {
      cout << "Scene:" << endl;
      cout << "   objects:       "
	   << setw (10) << commify (scene.objs.size ()) << endl;
      cout << "   lights:        "
	   << setw (10) << commify (scene.lights.size ()) << endl;
      cout << "   materials:     "
	   << setw (10) << commify (scene.materials.size ()) << endl;
      cout << "   voxtree nodes: "
	   << setw (10) << commify (scene.obj_voxtree.num_nodes ()) << endl;
      cout << "   voxtree max depth:"
	   << setw (7) << commify (scene.obj_voxtree.max_depth ()) << endl;
    }

  // Limits in terms of higher-resolution pre-AA image
  unsigned hr_limit_x = limit_x * aa_factor;
  unsigned hr_limit_y = limit_y * aa_factor;
  unsigned hr_limit_max_x = hr_limit_x + limit_width * aa_factor;
  unsigned hr_limit_max_y = hr_limit_y + limit_height * aa_factor;

  if (! quiet)
    cout << endl;

  // Main ray-tracing loop
  for (unsigned y = hr_limit_y; y < hr_limit_max_y; y++)
    {
      ImageRow &output_row = image.next_row ();

      // Progress indicator
      if (progress)
	{
	  if (aa_factor > 1)
	    cout << "\rrendering: line "
		 << setw (5) << y / aa_factor
		 << "_" << (y - (y / aa_factor) * aa_factor);
	  else
	    cout << "\rrendering: line "
		 << setw (5) << y;
	  cout << " (" << (y - hr_limit_y) * 100 / (hr_limit_max_y - hr_limit_y)
	       << "%)";
	  cout.flush ();
	}

      for (unsigned x = hr_limit_x; x < hr_limit_max_x; x++)
	{
	  float u = (float)x / (float)width;
	  float v = (float)(height - y) / (float)height;
	  Ray camera_ray = camera.get_ray (u, v);

	  camera_ray.set_len (10000);

	  output_row[x - hr_limit_x] = scene.render (camera_ray);
	}
    }

  if (progress)
    {
      cout << "\rrendering: done              " << endl;
      if (! quiet)
	cout << endl;
    }

  // Print render stats
  if (! quiet)
    {
      Scene::Stats &sstats = scene.stats;
      Voxtree::Stats &vstats1 = sstats.voxtree_closest_intersect;
      Voxtree::Stats &vstats2 = sstats.voxtree_shadowed;

      cout << "Rendering stats:" << endl;
      cout << "  closest_intersect:" << endl;
      cout << "     scene calls:       "
	   << setw (14) << commify (sstats.scene_closest_intersect_calls)
	   << endl;
      cout << "     voxtree node calls:"
	   << setw (14) << commify (vstats1.node_intersect_calls) << endl;
      cout << "     obj calls:         "
	   << setw (14) << commify (sstats.obj_closest_intersect_calls) << endl;

      cout << "  shadowed:" << endl;
      cout << "     scene tests:       "
	   << setw (14) << commify (sstats.scene_shadowed_tests) << endl;
      cout << "     shadow hint hits:  "
	   << setw (14) << commify (sstats.shadow_hint_hits) << endl;
      cout << "     shadow hint misses:"
	   << setw (14) << commify (sstats.shadow_hint_misses) << endl;
      cout << "     voxtree node tests:"
	   << setw (14) << commify (vstats2.node_intersect_calls) << endl;
      cout << "     obj tests:         "
	   << setw (14) << commify (sstats.obj_intersects_tests) << endl;
    }
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
