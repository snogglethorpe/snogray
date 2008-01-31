// snogray.cc -- Main driver for snogray ray tracer
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <stdexcept>

#ifdef HAVE_FENV_H
#include <fenv.h>
#endif

#include "cmdlineparser.h"
#include "rusage.h"
#include "string-funs.h"
#include "file-funs.h"
#include "progress.h"
#include "globals.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "render.h"
#include "recover.h"
#include "image-output.h"
#include "image-input.h"
#include "image-cmdline.h"
#include "render-cmdline.h"
#include "scene-def.h"
#include "trace-stats.h"


using namespace snogray;
using namespace std;


// Default size used for the "long" dimension of output image.  This
// particular size is nice because it's divisible by a 2, 3, 5, 9, and 16,
// which allows it to naturally fit a fair number of film formats.
//
#define DEFAULT_IMAGE_SIZE 720

// The maximum number of "backup" files we make when recovering a partial image.
//
#define RECOVER_BACKUP_LIMIT 100


// Floating-point exceptions

// Try to enable those floating-point exceptions which don't interfere
// with normal calculations.  This is to help debugging (the program
// will die if an exception is hit).
//
static void
enable_fp_exceptions ()
{
#if defined(USE_FP_EXCEPTIONS) && defined(HAVE_FEENABLEEXCEPT)

  fexcept_t fexcepts = 0;

#ifdef FE_DIVBYZERO
  fexcepts |= FE_DIVBYZERO;
#endif
#ifdef FE_OVERFLOW
  fexcepts |= FE_OVERFLOW;
#endif
#ifdef FE_INVALID
  fexcepts |= FE_INVALID;
#endif

  if (fexcepts)
    feenableexcept (fexcepts);

#endif // USE_FP_EXCEPTIONS && HAVE_FEENABLEEXCEPT
}


// LimitSpec datatype

// A "limit spec" represents a user's specification of a single
// rendering limit (on x, y, width, or height), which may be absolute,
// relative, a proportion of the image size, or both.
//
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
  unsigned val = is_frac ? unsigned (frac_val * range) : abs_val;

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

// Parse a size option argument.  If both a width and height are
// specified WIDTH and HEIGHT are set.  If only a single number is
// specified, SIZE is set instead.
//
static void
parse_size_opt_arg (CmdLineParser &clp,
		    unsigned &width, unsigned &height, unsigned &size)
{
  const char *arg = clp.opt_arg ();
  char *end = 0;

  unsigned num = strtoul (arg, &end, 10);

  if (end && end != arg)
    {
      // If no height is given, it will be set according to the camera's
      // aspect ratio
      //
      if (*end == '\0') 
	{
	  size = num;
	  return;
	}

      arg = end + strspn (end, " ,x");

      width = num;
      height = strtoul (arg, &end, 10);

      if (end && end != arg && *end == '\0')
	return;
    }

  clp.opt_err ("requires a size specification (WIDTHxHEIGHT, or SIZE)");
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
	      spec += strspn (spec, " ,x");
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
     << " [OPTION...] SCENE_FILE... OUTPUT_IMAGE_FILE" << endl;
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
s "  -w, --width=WIDTH          Set image width to WIDTH pixels"
s "  -h, --height=HEIGHT        Set image height to HEIGHT lines"
n
s RENDER_OPTIONS_HELP
n
s SCENE_DEF_OPTIONS_HELP
n
s IMAGE_OUTPUT_OPTIONS_HELP
n
s "  -C, --continue             Continue a previously aborted render"
n
s "  -L, --limit=LIMIT_SPEC     Limit output to area defined by LIMIT_SPEC"
n
s "  -q, --quiet                Do not output informational or progress messages"
s "  -P, --no-progress          Do not output progress indicator"
s "  -p, --progress             Output progress indicator despite --quiet"
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n;

#undef s
#undef n
}


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "size",		required_argument, 0, 's' },
    { "width",		required_argument, 0, 'w' },
    { "height",		required_argument, 0, 'h' },
    { "limit",		required_argument, 0, 'L' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    { "continue",	no_argument,	   0, 'C' },

    RENDER_LONG_OPTIONS,
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,

    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "s:w:h:l:qpP"
    SCENE_DEF_SHORT_OPTIONS
    RENDER_SHORT_OPTIONS
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  unsigned width = 0, height = 0, size = DEFAULT_IMAGE_SIZE;
  LimitSpec limit_x_spec ("min-x", 0), limit_y_spec ("min-y", 0);
  LimitSpec limit_max_x_spec ("max-x", 1.0), limit_max_y_spec ("max-y", 1.0);
  bool recover = false;
  Progress::Verbosity verbosity = Progress::CHATTY;
  bool progress_set = false;
  ValTable image_params, render_params;
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
	// Size options
	//
      case 's':
	parse_size_opt_arg (clp, width, height, size);
	break;
      case 'w':
	width = clp.unsigned_opt_arg ();
	break;
      case 'h':
	height = clp.unsigned_opt_arg ();
	break;
      case 'L':
	parse_limit_opt_arg (clp, limit_x_spec, limit_y_spec,
			     limit_max_x_spec, limit_max_y_spec);
	break;

      case 'C':
	recover = true;
	break;

	// Verbosity options
	//
      case 'q':
	quiet = true;
	if (! progress_set)
	  verbosity = Progress::QUIET;
	break;
      case 'p':
	verbosity = Progress::CHATTY;
	progress_set = true;
	break;
      case 'P':
	verbosity = Progress::QUIET;
	progress_set = true;
	break;

	// Rendering options
	//
	RENDER_OPTION_CASES (clp, render_params);

	// Scene options
	//
	SCENE_DEF_OPTION_CASES (clp, scene_def);

	// Image options
	//
	IMAGE_OUTPUT_OPTION_CASES (clp, image_params);

	// Generic options
	//
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() < 2)
    {
      usage (clp, cerr);
      exit (1);
    }

  // Parse scene filename
  //
  CMDLINEPARSER_CATCH
    (clp, scene_def.parse (clp, clp.num_remaining_args() - 1));

  // Output filename
  //
  string file_name = clp.get_arg();


  // Start of "overall elapsed" time
  //
  Timeval beg_time (Timeval::TIME_OF_DAY);


  //
  // Define the scene
  //

  Rusage scene_beg_ru;		// start timing scene definition

  Scene scene;
  Camera camera;


  // If the user specified both a width and a height, set the camera aspect
  // ratio to maintain pixels with a 1:1 aspect ratio.  We do this before
  // loading the scene so that scene camera manipulation commands can see
  // the final aspect ratio.
  //
  if (width && height)
    camera.set_aspect_ratio (float (width) / float (height));


  // Read in the scene/camera definitions
  //
  CMDLINEPARSER_CATCH (clp, scene_def.load (scene, camera));


  // Enable floating-point exceptions if possible, which can help debugging.
  // Note that we do this _after_ reading the scene which helps avoid
  // problems with libraries that are not expecting floating-point exceptions
  // to be enabled (code must be compiled with -ftrapping-math to inform the
  // compiler that floating-point operations may trap).
  //
  enable_fp_exceptions ();


  // If the user specified both a width and a height, set the camera aspect
  // ratio _again_ to maintain pixels with a 1:1 aspect ratio, just in case
  // the scene file tried to muck with it.
  //
  if (width && height)
    camera.set_aspect_ratio (float (width) / float (height));
  else
    // Otherwise, the output image size was not fully specified, so use
    // the camera's aspect ration to deduce the missing dimension from
    // the provided one, or from SIZE, which sets the longest dimension
    // rather than a specific one.
    {
      float ar = camera.aspect_ratio ();

      if (!width && !height)
	{
	  if (ar > 1)
	    width = size;
	  else
	    height = size;
	}

      if (width)
	height = unsigned (width / ar);
      else
	width = unsigned (height * ar);
    }


  Rusage scene_end_ru;		// stop timing scene definition


  // Set our drawing limits based on the scene size
  //
  unsigned limit_x = limit_x_spec.apply (clp, width);
  unsigned limit_y = limit_y_spec.apply (clp, height);
  unsigned limit_width
    = limit_max_x_spec.apply (clp, width, limit_x) - limit_x;
  unsigned limit_height
    = limit_max_y_spec.apply (clp, height, limit_y) - limit_y;

  // If possible, try to recover a previously aborted render.
  //
  ImageInput *recover_input = 0;
  if (file_exists (file_name))
    {
      if (recover)
	//
	// Recover a previous aborted render
	try
	  {
	    recover_input = new ImageInput (file_name);

	    string recover_backup
	      = rename_to_backup_file (file_name, RECOVER_BACKUP_LIMIT);

	    if (! quiet)
	      cout << "* recover: " << file_name
		   << ": Backup in " << recover_backup << endl;
	  }
	catch (runtime_error &err)
	  {
	    clp.err (err.what ());
	  }
      else
	{
	  cerr << clp.err_pfx() << file_name << ": Output file already exists"
	       << endl;
	  cerr << clp.err_pfx()
	       << "To continue a previously aborted render"
	       << ", use the `--continue' option"
	       << endl;
	  exit (23);
	}
    }

  // Create output image.  The size of what we output is the same as the
  // limit (which defaults to, but is not the same as the nominal output
  // image size).
  //
  // This will produce diagnostics and exit if there's something wrong
  // with IMAGE_SINK_PARAMS or a problem creating the output image, so
  // we create the image before printing any normal output.
  //
  ImageOutput output (file_name, limit_width, limit_height, image_params);

  if (recover_input)
    {
      unsigned num_recovered = recover_image (recover_input, output);
      recover_input = 0;

      if (! quiet)
	cout << "* recover: " << file_name
	     << ": Recovered " << num_recovered << " rows" << endl;

      if (num_recovered == limit_height)
	{
	  cout << file_name << ": Entire image was recovered, not rendering"
	       << endl;
	  return 0; // We want destructors to run, so must not use exit!
	}
    }

  if (! quiet)
    cout << "* scene: "
	 << commify_with_units (scene.surfaces.size (),
				"top-level surface", "top-level surfaces")
	 << ", " << commify_with_units (scene.lights.size (),
					"light", "lights")
	 << ", " << commify_with_units (scene.materials.size (),
					"material", "materials")
	 << endl;


  TraceStats trace_stats;

  // Create the image.
  //
  Rusage render_beg_ru;
  render (scene, camera, width, height, output, limit_x, limit_y,
	  render_params, trace_stats, cout, verbosity);
  Rusage render_end_ru;


  Timeval end_time (Timeval::TIME_OF_DAY);


  // Print stats
  //
  if (! quiet)
    {
      trace_stats.print (cout);

      long num_eye_rays = limit_width * limit_height;

      // a field width of 14 is enough for over a year of time...
      cout << "Time:" << endl;
      Timeval scene_def_time
	= ((scene_end_ru.utime() - scene_beg_ru.utime())
	   + (scene_end_ru.stime() - scene_beg_ru.stime()));
      cout << "  scene def cpu:" << setw (14) << scene_def_time << endl;
      Timeval render_time = render_end_ru.utime() - render_beg_ru.utime();
      cout << "  rendering cpu:" << setw (14) << render_time << endl;
      Timeval elapsed_time = end_time - beg_time;
      cout << "  total elapsed:" << setw (14) << elapsed_time << endl;

      long long sc  = trace_stats.scene_intersect_calls;
      long long sst = trace_stats.scene_shadow_tests;
      double rps = render_time == 0 ? 0 : double (sc + sst) / render_time;
      double erps = render_time == 0 ? 0 : double (num_eye_rays) / render_time;
      cout << "  rays per second:    "
	   << setw (8) << commify (static_cast<long long> (rps))
	   << endl;
      cout << "  eye-rays per second:"
	   << setw (8) << commify (static_cast<long long> (erps))
	   << endl;
    }
}


// arch-tag: adea1df7-f224-4a25-9856-7959d7674435
