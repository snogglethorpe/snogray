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
#include <stdexcept>

#include "cmdlineparser.h"
#include "rusage.h"
#include "string-funs.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "image.h"
#include "image-cmdline.h"
#include "test-scenes.h"

using namespace Snogray;
using namespace std;



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


// Main driver

static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << " [OPTION...] [SCENE_FILE [OUTPUT_IMAGE_FILE]]" << endl;
}

static void
help (CmdLineParser &clp, ostream &os)
{
  usage (clp, os);

  // These macros just makes the source code for help output easier to line up
  //
#define s  << endl <<
#define n  << endl

  os <<
  "Ray-trace an image"
n
s "  -s, --size=WIDTHxHEIGHT    Set image size to WIDTH x HEIGHT pixels/lines"
s "  -m, --multiple=NUM         Make real output size NUM times specified size"
s "                               (useful if it will later be anti-aliased)"
s "  -l, --limit=LIMIT_SPEC     Limit output to area defined by LIMIT_SPEC"
n
s "  -q, --quiet                Do not output informational or progress messages"
s "  -P, --no-progress          Do not output progress indicator"
s "  -p, --progress             Output progress indicator despite --quiet"
n
s " Scene options:"
s "  -I, --scene-format=FMT     Scene is in format FMT (one of: test, aff, nff)"
s "  -G, --assumed-gamma=GAMMA  Reverse implicit gamma correction of GAMMA"
s "  -L, --light-scale=SCALE    Scale all scene lighting by SCALE"
n
s IMAGE_OUTPUT_OPTIONS_HELP
n
s "      --list-test-scenes     Ouput a list of builtin test-scenes to stdout"
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "If no input/output filenames are given, standard input/output are used"
s "respectively.  When no explicit scene/image formats are specified, the"
s "filename extensions are used to guess the format (so an explicit format"
s "must be specified when standard input/output are used)."
n
s "The \"test\" scene type is special, as no scene file is actually read;"
s "instead, a built in test-scene with the given name is used.  As a shortcut"
s "a prefix of `test:' maybe be used instead of the `-Itest' option;"
s "e.g. `test:cbox1' refers to the built-in test-scene `cbox1'." 
n
s "For a full list of test-scenes, use the `--list-test-scenes' option."
n
    ;

#undef s
#undef n
}


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
#define OPT_LIST_TEST_SCENES	10
  //
  static struct option long_options[] = {
    { "size",		required_argument, 0, 's' },
    { "multiple",	required_argument, 0, 'm' },
    { "limit",		required_argument, 0, 'l' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    { "scene-format", 	required_argument, 0, 'I' },
    { "assumed-gamma", 	required_argument, 0, 'G' },
    { "light-scale", 	required_argument, 0, 'L' },
    { "list-test-scenes", no_argument,     0, OPT_LIST_TEST_SCENES },

    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,

    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "s:m:w:h:l:qpPI:G:L:"
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  unsigned width = 640, height = 480;
  LimitSpec limit_x_spec ("min-x", 0), limit_y_spec ("min-y", 0);
  LimitSpec limit_max_x_spec ("max-x", 1.0), limit_max_y_spec ("max-y", 1.0);
  bool quiet = false, progress = true; // quiet mode, progress indicator
  bool progress_set = false;
  const char *scene_fmt = 0;
  unsigned multiple = 1;
  float scene_assumed_gamma = 1, scene_light_scale = 1;
  ImageCmdlineSinkParams image_sink_params (clp);

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	// Scene options
	//
      case 'I':
	scene_fmt = clp.opt_arg ();
	break;
      case 'G':
	scene_assumed_gamma = clp.float_opt_arg ();
	break;
      case 'L':
	scene_light_scale = clp.float_opt_arg ();
	break;

      case OPT_LIST_TEST_SCENES:
	{
	  vector<TestSceneDesc> descs = list_test_scenes ();

	  cout << "Built-in test scenes:" << endl << endl;
	  cout.setf (ios::left);

	  for (vector<TestSceneDesc>::const_iterator di = descs.begin();
	       di != descs.end(); di++)
	    cout << "   " << setw(15) << di->name << di->desc << endl;
	}
	exit (0);

	// Size options
	//
      case 's':
	parse_size_opt_arg (clp, width, height);
	break;
      case 'm':
	multiple = clp.unsigned_opt_arg ();
	break;
      case 'l':
	parse_limit_opt_arg (clp, limit_x_spec, limit_y_spec,
			     limit_max_x_spec, limit_max_y_spec);
	break;

	// Verbosity options
	//
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

	// Image options
	//
	IMAGE_OUTPUT_OPTION_CASES (clp, image_sink_params);

	// Generic options
	//
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  // Start of "overall elapsed" time
  //
  Timeval beg_time (Timeval::TIME_OF_DAY);


  //
  // Define our scene!
  //

  Scene scene;
  Camera camera;

  // Default camera aspect ratio to give pixels a 1:1 aspect ratio
  //
  camera.set_aspect_ratio ((float)width / (float)height);

  Rusage scene_beg_ru;		// start timing scene definition

  const char *scene_file_name = clp.get_arg ();
  if (strcmp (scene_file_name, "-") == 0)
    scene_file_name = 0;

  // Read in scene file (or built-in test scene)
  //
  try
    {
      if (scene_file_name && scene_fmt && strcmp (scene_fmt, "test") == 0)
	def_test_scene (scene_file_name, scene, camera);
      else if (scene_file_name && !scene_fmt
	       && strncmp (scene_file_name, "test:", 5) == 0)
	// The input filename "test:..." is the same as specifying -Itest
	//
	def_test_scene (scene_file_name + 5, scene, camera);
      else if (scene_file_name)
	scene.load (scene_file_name, scene_fmt, camera);
      else if (! scene_fmt)
	clp.err ("Scene format must be specified for stream input");
      else if (strcmp (scene_fmt, "test") == 0)
	clp.err ("No test scene name specified");
      else
	scene.load (cin, scene_fmt, camera);
    }
  catch (runtime_error &err)
    {
      clp.err (string (scene_file_name ? scene_file_name : "<standard input>")
	       + ": Error reading scene: " + err.what ());
    }

  // Correct for bogus "gamma correction in lighting"
  //
  if (scene_assumed_gamma != 1)
    scene.set_assumed_gamma (scene_assumed_gamma);

  // Correct scene lighting
  //
  if (scene_light_scale != 1)
    for (Scene::light_iterator_t li = scene.lights.begin();
	 li != scene.lights.end(); li++)
      {
	Light *light = *li;
	light->color *= scene_light_scale;
      }

  Rusage scene_end_ru;		// stop timing scene definition


  //
  // Init output image
  //

  // Set our drawing limits based on the scene size
  //
  unsigned limit_x = limit_x_spec.apply (clp, width);
  unsigned limit_y = limit_y_spec.apply (clp, height);
  unsigned limit_width
    = limit_max_x_spec.apply (clp, width, limit_x) - limit_x;
  unsigned limit_height
    = limit_max_y_spec.apply (clp, height, limit_y) - limit_y;

  // Create output image.  The size of what we output is the same as the
  // limit, and includes the user's multiple, but not aa_factor.
  //
  // This will produce diagnostics and exit if there's something wrong
  // with IMAGE_SINK_PARAMS or a problem creating the output image, so
  // we create the image before printing any normal output.
  //
  image_sink_params.file_name = clp.get_arg();
  image_sink_params.width = limit_width * multiple;
  image_sink_params.height = limit_height * multiple;
  ImageOutput image (image_sink_params);

  // Maybe print lots of useful information
  //
  if (! quiet)
    {
      // Print scene info

      cout << "Scene:" << endl;

      cout << "   scene: "
	   << setw (18)
	   << (scene_file_name ? scene_file_name : "<standard input>") << endl;
      cout << "   top-level surfaces:"
	   << setw (6) << commify (scene.surfaces.size ()) << endl;
      cout << "   lights:        "
	   << setw (10) << commify (scene.lights.size ()) << endl;
      if (scene.assumed_gamma != 1)
	cout << "   assumed gamma: "
	     << setw (10) << scene.assumed_gamma << endl;
      if (scene_light_scale != 1)
	cout << "   light scale:   "
	     << setw (10) << scene_light_scale << endl;
      cout << "   materials:     "
	   << setw (10) << commify (scene.materials.size ()) << endl;
      cout << "   voxtree surfaces:"
	   << setw (8) << commify (scene.surface_voxtree.num_surfaces ()) << endl;
      cout << "   voxtree nodes: "
	   << setw (10) << commify (scene.surface_voxtree.num_nodes ()) << endl;
      float vt_avg_depth = scene.surface_voxtree.avg_depth ();
      cout << "   voxtree avg depth: "
	   << setw (6) << int (vt_avg_depth) << endl;
      cout << "   voxtree max depth:"
	   << setw (7) << commify (scene.surface_voxtree.max_depth ()) << endl;

      // Print image info

      cout << "Image:" << endl;

      if (multiple == 1)
	cout << "   size:     "
	     << setw (15) << (stringify (width)
			      + " x " + stringify (height))
	     << endl;
      else
	cout << "   size:     "
	     << setw (15) << (stringify (width) + " x " + stringify (height)
			      + " x " + stringify (multiple))
	     <<" (" << (width * multiple) << " x " << (height * multiple) << ")"
	     << endl;

      if (limit_x != 0 || limit_y != 0
	  || limit_width != width || limit_height != height)
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
      //

      if ((image.aa_factor + image_sink_params.aa_overlap) > 1)
	{
	  if (image.aa_factor > 1)
	    cout << "   aa_factor:           "
		 << setw (4) << image.aa_factor << endl;

	  if (image_sink_params.aa_overlap > 0)
	    cout << "   aa_kernel_size:      "
		 << setw (4)
		 << (image.aa_factor + image_sink_params.aa_overlap*2)
		 << " (overlap = " << image_sink_params.aa_overlap << ")"
		 << endl;
	  else
	    cout << "   aa_kernel_size:      "
		 << setw (4) << image.aa_factor << endl;

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

      cout << endl;
    }

  // For convenience, we fold the size increase due to anti-aliasing into
  // the user's specified size multiple.
  //
  unsigned hr_multiple = multiple * image.aa_factor;

  // The size of the actual image we're calculating
  //
  const unsigned hr_width = width * hr_multiple;
  const unsigned hr_height = height * hr_multiple;

  // Limits in terms of higher-resolution pre-AA image
  //
  unsigned hr_limit_x = limit_x * hr_multiple;
  unsigned hr_limit_y = limit_y * hr_multiple;
  unsigned hr_limit_max_x = hr_limit_x + limit_width * hr_multiple;
  unsigned hr_limit_max_y = hr_limit_y + limit_height * hr_multiple;

  if (!progress && !quiet)
    {
      cout << "rendering...";	// if no progress indicator, print _something_
      cout.flush ();
    }

  // Main ray-tracing loop
  //
  Rusage render_beg_ru;
  for (unsigned y = hr_limit_y; y < hr_limit_max_y; y++)
    {
      ImageRow &output_row = image.next_row ();

      // Progress indicator
      if (progress)
	{
	  if (hr_multiple > 1)
	    cout << "\rrendering: line "
		 << setw (5) << y / hr_multiple
		 << "_" << (y - (y / hr_multiple) * hr_multiple);
	  else
	    cout << "\rrendering: line "
		 << setw (5) << y;
	  cout << " (" << (y - hr_limit_y) * 100 / (hr_limit_max_y - hr_limit_y)
	       << "%)";
	  cout.flush ();
	}

      // This is basically a cache to speed up tracing by holding hints
      // that take advantage of spatial coherency.  We create a new one
      // for each row as the state at the end of the previous row is
      // probably not too useful anyway.
      //
      TraceState tstate (scene);

      // Process one image row
      //
      for (unsigned x = hr_limit_x; x < hr_limit_max_x; x++)
	{
	  // Translate the image position X, Y into a ray radiating from
	  // the camera.
	  //
	  float u = (float)x / (float)hr_width;
	  float v = (float)(hr_height - y) / (float)hr_height;
	  Ray camera_ray = camera.get_ray (u, v);

	  // Cast the camera ray and calculate the image color at that point.
	  //
	  Color pix = scene.render (camera_ray, tstate);

	  // If necessary undo any bogus gamma-correction embedded in
	  // the scene lighting.  We'll do proper gamma correct later.
	  //
	  if (scene.assumed_gamma != 1)
	    pix = pix.pow (scene.assumed_gamma);

	  output_row[x - hr_limit_x] = pix;
	}
    }
  Rusage render_end_ru;

  Timeval end_time (Timeval::TIME_OF_DAY);

  if (progress)
    cout << "\rrendering: done              " << endl;
  else if (! quiet)
    cout << "done" << endl;

  // Print stats
  //
  if (! quiet)
    {
      Scene::Stats &sstats = scene.stats;
      Voxtree::Stats &vstats1 = sstats.voxtree_intersect;
      Voxtree::Stats &vstats2 = sstats.voxtree_shadow;

      long long sc  = sstats.scene_intersect_calls;
      long long vnc = vstats1.node_intersect_calls;
      long long ocic = sstats.surface_intersect_calls;
      long long hhh = sstats.horizon_hint_hits;
      long long hhm = sstats.horizon_hint_misses;

      unsigned vnn = scene.surface_voxtree.num_nodes ();
      unsigned vno  = scene.surface_voxtree.num_surfaces ();

      cout << endl;
      cout << "Rendering stats:" << endl;
      cout << "  intersect:" << endl;
      cout << "     scene calls:       " << setw (14) << commify (sc) << endl;
      cout << "     horizon hint hits: " << setw (14) << commify (hhh)
	   << " (" << setw(2) << (100 * hhh / sc) << "%)" << endl;
      cout << "     horizon hint misses:" << setw (13) << commify (hhm)
	   << " (" << setw(2) << (100 * hhm / sc) << "%)" << endl;
      if (vnn != 0)
	cout << "     voxtree node calls:" << setw (14) << commify (vnc)
	     << " (" << setw(2) << (100 * vnc / (sc * vnn)) << "%)" << endl;
      if (vno != 0)
	cout << "     surface calls:     " << setw (14) << commify (ocic)
	     << " (" << setw(2) << (100 * ocic / (sc * vno)) << "%)" << endl;

      long long sst = sstats.scene_shadow_tests;

      if (sst != 0)
	{
	  long long shh = sstats.shadow_hint_hits;
	  long long shm = sstats.shadow_hint_misses;
	  long long sss = sstats.scene_slow_shadow_traces;
	  long long oss = sstats.surface_slow_shadow_traces;
	  long long vnt = vstats2.node_intersect_calls;
	  long long ot  = sstats.surface_intersects_tests;

	  cout << "  shadow:" << endl;
	  cout << "     scene tests:       " << setw (14) << commify (sst)
	       << endl;
	  cout << "     shadow hint hits:  " << setw (14) << commify (shh)
	       << " (" << setw(2) << (100 * shh / sst) << "%)" << endl;
	  cout << "     shadow hint misses:" << setw (14) << commify (shm)
	       << " (" << setw(2) << (100 * shm / sst) << "%)" << endl;
	  if (sss != 0)
	    cout << "     non-opaque traces: " << setw (14) << commify (sss)
		 << " (" << setw(2) << (100 * sss / sst) << "%"
		 << "; average depth = " << (float (oss) / float (sss)) << ")"
		 << endl;
	  if (vnn != 0)
	    cout << "     voxtree node tests:" << setw (14) << commify (vnt)
		 << " (" <<setw(2) << (100 * vnt / (vnn * (sst - shh))) << "%)"
		 << endl;
	  if (vno != 0)
	    cout << "     surface tests:     " << setw (14) << commify (ot)
		 << " (" <<setw(2) << (100 * ot / (vno * (sst - shh))) << "%)"
		 << endl;
	}

      // a field width of 13 is enough for over a year of time...
      cout << "Time:" << endl;
      Timeval scene_def_time = scene_end_ru.utime() - scene_beg_ru.utime();
      cout << "  scene def cpu: " << setw (13) << scene_def_time.fmt() << endl;
      Timeval render_time = render_end_ru.utime() - render_beg_ru.utime();
      cout << "  rendering cpu: " << setw (13) << render_time.fmt() << endl;
      Timeval elapsed_time = end_time - beg_time;
      cout << "  total elapsed: " << setw (13) << elapsed_time.fmt() << endl;
    }
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
