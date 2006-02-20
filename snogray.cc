// snogray.cc -- Main driver for snogray ray tracer
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <sstream>
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
#include "image-io.h"
#include "image-cmdline.h"
#include "wire-frame.h"
#include "scene-def.h"
#include "scene-stats.h"

using namespace Snogray;
using namespace std;



// Print useful scene info
//
void
print_scene_info (const Scene &scene, const SceneDef &scene_def)
{
  Space::Stats tstats = scene.space.stats ();

  cout << "Scene:" << endl;

  string name = scene_def.user_name;
  if (name.empty ())
    name = "<standard input>";
      
  cout << "   scene:   " << setw (20) << name << endl;
  cout << "   top-level surfaces:  "
       << setw (8) << commify (scene.surfaces.size ()) << endl;
  cout << "   lights:          "
       << setw (12) << commify (scene.lights.size ()) << endl;
  if (scene.assumed_gamma != 1)
    cout << "   assumed gamma:   "
	 << setw (12) << scene.assumed_gamma << endl;
  if (scene_def.light_scale != 1)
    cout << "   light scale:     "
	 << setw (12) << scene_def.light_scale << endl;
  cout << "   materials:       "
       << setw (12) << commify (scene.materials.size ()) << endl;

  cout << "   tree surfaces:     "
       << setw (10) << commify (tstats.num_surfaces)
       << " (" << (tstats.num_dup_surfaces * 100 / tstats.num_surfaces)
       << "% duplicates)" << endl;
  cout << "   tree nodes:      "
       << setw (12) << commify (tstats.num_nodes)
       << " (" << (tstats.num_leaf_nodes * 100 / tstats.num_nodes)
       << "% leaves)" << endl;
  cout << "   tree avg depth:      "
       << setw (8) << int (tstats.avg_depth) << endl;
  cout << "   tree max depth:     "
       << setw (9) << tstats.max_depth << endl;
}



// Print image info
//
void
print_image_info (const ImageOutput &image,
		  const ImageSinkParams &image_sink_params,
		  unsigned width, unsigned height, unsigned multiple,
		  unsigned limit_x, unsigned limit_y,
		  unsigned limit_width, unsigned limit_height)
{
  cout << "Image:" << endl;

  if (multiple == 1)
    cout << "   size:    "
	 << setw (20) << (stringify (width)
			  + " x " + stringify (height))
	 << endl;
  else
    cout << "   size:    "
	 << setw (20) << (stringify (width) + " x " + stringify (height)
			  + " x " + stringify (multiple))
	 <<" (" << (width * multiple) << " x " << (height * multiple) << ")"
	 << endl;

  if (limit_x != 0 || limit_y != 0
      || limit_width != width || limit_height != height)
    cout << "   limit:  "
	 << setw (20) << (stringify (limit_x) + "," + stringify (limit_y)
			  + " - " + stringify (limit_x + limit_width)
			  + "," + stringify (limit_y + limit_height))
	 << " (" << limit_width << " x "  << limit_height << ")"
	 << endl;

  if (image_sink_params.target_gamma != 0)
    cout << "   target_gamma:           "
	 << setw (5) << image_sink_params.target_gamma << endl;

  // Anti-aliasing info
  //

  if ((image.aa_factor + image_sink_params.aa_overlap) > 1)
    {
      if (image.aa_factor > 1)
	cout << "   aa_factor:               "
	     << setw (4) << image.aa_factor << endl;

      if (image_sink_params.aa_overlap > 0)
	cout << "   aa_kernel_size:          "
	     << setw (4)
	     << (image.aa_factor + image_sink_params.aa_overlap*2)
	     << " (overlap = " << image_sink_params.aa_overlap << ")"
	     << endl;
      else
	cout << "   aa_kernel_size:          "
	     << setw (4) << image.aa_factor << endl;

      cout << "   aa_filter:           " << setw (8);
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
s "  -w, --wire-frame[=PARAMS]  Output in \"wire-frame\" mode; PARAMS has the form"
s "                               [TINT][/COLOR][:FILL] (default: 0.7/1:0)"
s "                               TINT is how much object color affects wires"
s "                               COLOR is the base color of wires"
s "                               FILL is the intensity of the scene between wires"
n
s "  -j, --jitter[=REPEAT]      Randomly jitter eye-rays; if REPEAT is specified,"
s "                               each eye-ray is repeated that many times"
n
s SCENE_DEF_OPTIONS_HELP
n
s IMAGE_OUTPUT_OPTIONS_HELP
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "If no input/output filenames are given, standard input/output are used"
s "respectively.  When no explicit scene/image formats are specified, the"
s "filename extensions are used to guess the format (so an explicit format"
s "must be specified when standard input/output are used)."
n
s SCENE_DEF_EXTRA_HELP
n;

#undef s
#undef n
}


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "jitter",		optional_argument, 0, 'j' },
    { "size",		required_argument, 0, 's' },
    { "multiple",	required_argument, 0, 'm' },
    { "limit",		required_argument, 0, 'l' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    { "wire-frame",     optional_argument, 0, 'w' },
    { "dump-samples",	required_argument, 0, 'S' },

    SCENE_DEF_LONG_OPTIONS,
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,

    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "j::w::s:m:l:qpPS:"
    SCENE_DEF_SHORT_OPTIONS
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
  bool wire_frame = false;
  WireFrameParams wire_frame_params;
  unsigned multiple = 1;
  unsigned jitter = 0;
  ImageCmdlineSinkParams image_sink_params (clp);
  SceneDef scene_def;

  // This speeds up I/O on cin/cout by not syncing with C stdio.
  //
  ios::sync_with_stdio (false);

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'w':
	wire_frame = true;
	if (clp.opt_arg ())
	  wire_frame_params.parse (clp);
	break;

      case 'j':
	if (clp.opt_arg ())
	  jitter = clp.unsigned_opt_arg ();
	else
	  jitter = 1;
	break;

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

	//
	// Scene options
	//
	SCENE_DEF_OPTION_CASES (clp, scene_def);

	// Image options
	//
	IMAGE_OUTPUT_OPTION_CASES (clp, image_sink_params);

	// Generic options
	//
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  // Parse scene spec (filename or test-scene name)
  //
  CMDLINEPARSER_CATCH (clp, scene_def.parse (clp));

  // Output filename
  //
  image_sink_params.file_name = clp.get_arg();


  // Start of "overall elapsed" time
  //
  Timeval beg_time (Timeval::TIME_OF_DAY);


  //
  // Define the scene
  //

  Rusage scene_beg_ru;		// start timing scene definition

  Scene scene;
  Camera camera;

  // Default camera aspect ratio to give pixels a 1:1 aspect ratio
  //
  camera.set_aspect_ratio ((float)width / (float)height);

  // Read in the scene/camera definitions
  //
  CMDLINEPARSER_CATCH (clp, scene_def.load (scene, camera));

  Rusage scene_end_ru;		// stop timing scene definition


  // To avoid annoying artifacts in cases where the camera is looking
  // exactly along an axis, always perturb the camera position just a
  // litttle bit.
  //
  camera.move (Vec (Eps, Eps, Eps));


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
  image_sink_params.width = limit_width * multiple;
  image_sink_params.height = limit_height * multiple;
  ImageOutput image (image_sink_params);

  // Maybe print lots of useful information
  //
  if (! quiet)
    {
      print_scene_info (scene, scene_def);
      print_image_info (image, image_sink_params, width, height, multiple,
			limit_x, limit_y, limit_width, limit_height);
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

  // We create this object regardless of whether we're doing wire-frame
  // output or not; in the latter case it simply isn't used.
  //
  WireFrameRendering
    wire_frame_rendering (scene, camera, hr_width, hr_height,
			  hr_limit_x, hr_limit_y,
			  hr_limit_max_x, hr_limit_max_y,
			  wire_frame_params);

  long long num_eye_rays = 0;

  // Global R/W state during tracing.
  //
  GlobalTraceState global_tstate;

  // Main ray-tracing loop
  //
  Rusage render_beg_ru;
  for (unsigned y = hr_limit_y; y < hr_limit_max_y; y++)
    {
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
      TraceState tstate (scene, global_tstate);

      // Process and (maybe) output one image row.  In wire-frame mode,
      // we actually output one row behind the row being calculated, so
      // we supress output the first time.
      //
      if (wire_frame)
	{
	  // Wire-frame rendering mode

	  wire_frame_rendering.render_row (tstate);

	  if (y > hr_limit_y)
	    wire_frame_rendering.get_prev_row (image.next_row ());

	  wire_frame_rendering.advance_row ();
	}
      else
	{
	  // Normal rendering mode

	  ImageRow &output_row = image.next_row ();

	  for (unsigned x = hr_limit_x; x < hr_limit_max_x; x++)
	    {
	      // Translate the image position X, Y into a ray radiating from
	      // the camera.
	      //
	      Ray camera_ray
		= camera.get_ray (x, y, hr_width, hr_height, jitter);

	      // Cast the camera ray and calculate image color at that point.
	      //
	      Color pix = scene.render (camera_ray, tstate);

	      // If oversampling, send out more rays for this pixel, and
	      // average the result.
	      //
	      if (jitter > 1)
		{
		  for (unsigned j = 1; j < jitter; j++)
		    {
		      camera_ray
			= camera.get_ray (x, y, hr_width, hr_height, true);
		      pix += scene.render (camera_ray, tstate);
		    }

		  pix /= jitter;
		}

	      // If necessary undo any bogus gamma-correction embedded in
	      // the scene lighting.  We'll do proper gamma correct later.
	      //
	      if (scene.assumed_gamma != 1)
		pix = pix.pow (scene.assumed_gamma);

	      output_row[x - hr_limit_x] = pix;
	    }
	}

      num_eye_rays += (hr_limit_max_x - hr_limit_x);
    }

  // In wire-frame mode we output one row behind the calcuation, so in
  // that case output the final row.
  //
  if (wire_frame)
    wire_frame_rendering.get_prev_row (image.next_row ());

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
      print_scene_stats (scene, cout);

      // a field width of 14 is enough for over a year of time...
      cout << "Time:" << endl;
      Timeval scene_def_time
	= ((scene_end_ru.utime() - scene_beg_ru.utime())
	   + (scene_end_ru.stime() - scene_beg_ru.stime()));
      cout << "  scene def cpu:" << setw (14) << scene_def_time.fmt() << endl;
      Timeval render_time = render_end_ru.utime() - render_beg_ru.utime();
      cout << "  rendering cpu:" << setw (14) << render_time.fmt() << endl;
      Timeval elapsed_time = end_time - beg_time;
      cout << "  total elapsed:" << setw (14) << elapsed_time.fmt() << endl;

      long long sc  = scene.stats.scene_intersect_calls;
      long long sst = scene.stats.scene_shadow_tests;
      double rps = (double)(sc + sst) / render_time;
      double erps = (double)num_eye_rays / render_time;
      cout << "  rays per second:    " << setw (8) << commify ((long long)rps)
	   << endl;
      cout << "  eye-rays per second:" << setw (8) << commify ((long long)erps)
	   << endl;
    }
}

// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
