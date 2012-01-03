// snogray.cc -- Main driver for snogray ray tracer
//
//  Copyright (C) 2005-2008, 2010-2012  Miles Bader <miles@gnu.org>
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

#include "cmdlineparser.h"
#include "rusage.h"
#include "string-funs.h"
#include "file-funs.h"
#include "num-cores.h"
#include "tty-progress.h"
#include "globals.h"

#include "scene.h"
#include "camera.h"
#include "light.h"
#include "render-mgr.h"
#include "recover.h"
#include "image-input.h"
#include "image-cmdline.h"
#include "render-cmdline.h"
#include "scene-def.h"
#include "camera-cmds.h"
#include "render-stats.h"
#include "octree.h"
#include "pos-io.h"
#include "vec-io.h"


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


// Parser for the --limit command-line option argument

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
s " Rendering options:"
n
s RENDER_OPTIONS_HELP
n
s "  -c, --camera=COMMANDS      Move/point the camera according to COMMANDS:"
s "                               g X,Y,Z     Goto absolute location X, Y, Z"
s "                               t X,Y,Z     Point at target X, Y, Z"
s "                               m[rludfb] D Move distance D in the given dir"
s "                                           (right, left, up, down, fwd, back)"
s "                               m[xyz] D    Move distance D on the given axis"
s "                               r[rlud] A   Rotate A deg in the given dir"
s "                               ra A        Rotate A deg around center axis"
s "                               r[xyz] A    Rotate A degrees around [xyz]-axis"
s "                               o[xyz] A    Orbit A degrees around [xyz]-axis"
s "                               z SCALE     Zoom by SCALE"
s "                               l FOC_LEN   Set lens focal-length to FOCLEN"
s "                               f F_STOP    Set lens aperture to F_STOP"
s "                               d DIST      Set focus distance to DIST"
s "                               a X,Y       Auto-focus at point X,Y on image"
s "                               u SIZE      Set scene unit to SIZE, in mm"
s "                               h           Set camera orientation to horizontal"
s "                               v           Set camera orientation to vertic"
n
s " Scene options:"
n
s SCENE_DEF_OPTIONS_HELP
n
s " Output image options:"
n
s IMAGE_OUTPUT_OPTIONS_HELP
n
s "  -L, --limit=X,Y+W,H        Limit output to area X,Y - X+W,Y+H"
s "  -L, --limit=X1,Y1-X2,Y2    Limit output to area X1,Y1 - X2,Y2"
s "                               Limit coordinates/sizes can be percentages"
s "                               (with \"%\") or fractions (with \".\") of the"
s "                               \"nominal\" output image, or integer numbers"
s "                               of pixels (note: the -s/--size option always"
s "                               specifies the size of the nominal image)"
n
s " Misc options:"
n
#if USE_THREADS
s "  -j, --threads=NUM          Use NUM threads for rendering (default all cores)"
n
#endif
s "  -C, --continue             Continue a previously aborted render"
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
    { "limit",		required_argument, 0, 'L' },
    { "quiet",		no_argument,	   0, 'q' },
    { "progress",	no_argument,	   0, 'p' },
    { "no-progress",	no_argument,	   0, 'P' },
    { "continue",	no_argument,	   0, 'C' },
    { "camera",		required_argument, 0, 'c' },
#if USE_THREADS
    { "threads",	required_argument, 0, 'j' },
#endif

    RENDER_LONG_OPTIONS,
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,

    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "L:qpPCc:"
#if USE_THREADS
    "j:"
#endif
    SCENE_DEF_SHORT_OPTIONS
    RENDER_SHORT_OPTIONS
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  LimitSpec limit_x_spec ("min-x", 0), limit_y_spec ("min-y", 0);
  LimitSpec limit_max_x_spec ("max-x", 1.0), limit_max_y_spec ("max-y", 1.0);
  unsigned num_threads = 0;	// autodetect
  bool recover = false;
  TtyProgress::Verbosity verbosity = TtyProgress::CHATTY;
  bool progress_set = false;
  ValTable output_params, render_params;
  SceneDef scene_def;
  std::string camera_cmds;	// User commands for the camera


  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'L':
	parse_limit_opt_arg (clp, limit_x_spec, limit_y_spec,
			     limit_max_x_spec, limit_max_y_spec);
	break;

#if USE_THREADS
      case 'j':
	num_threads = clp.unsigned_opt_arg ();
	break;
#endif

      case 'c':
	camera_cmds += clp.opt_arg ();
	break;

      case 'C':
	recover = true;
	break;

	// Verbosity options
	//
      case 'q':
	quiet = true;
	if (! progress_set)
	  verbosity = TtyProgress::QUIET;
	break;
      case 'p':
	verbosity = TtyProgress::CHATTY;
	progress_set = true;
	break;
      case 'P':
	verbosity = TtyProgress::QUIET;
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
	IMAGE_OUTPUT_OPTION_CASES (clp, output_params);

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
  output_params.set ("filename", clp.get_arg ());


  // Start of "overall elapsed" time
  //
  Timeval beg_time (Timeval::TIME_OF_DAY);


  //
  // Define the scene
  //

  Rusage scene_beg_ru;		// start timing scene definition

  // Scene object.
  //
  Scene scene;
  Camera camera;

  // Height/width of output image (may not be set yet).
  //
  unsigned width = output_params.get_uint ("width", 0);
  unsigned height = output_params.get_uint ("height", 0);

  // If the user specified both a width and a height, set the camera aspect
  // ratio to maintain pixels with a 1:1 aspect ratio.  We do this before
  // loading the scene so that scene camera manipulation commands can see
  // the final aspect ratio.
  //
  if (width && height)
    camera.set_aspect_ratio (float (width) / float (height));

  // Read in the scene/camera definitions.  We copy our parameter
  // tables into SCENE_DEF's parameter table with appropriate
  // prefixes, so that they're available for scene-loaders to
  // examine/change.
  //
  scene_def.params.import (render_params, "render.");
  scene_def.params.import (output_params, "output.");
  CMDLINEPARSER_CATCH (clp, scene_def.load (scene, camera));

  // Now rewrite our parameter tables appropriately-prefixed
  // parameters from SCENE_DEF.  As we earlier stored them, that means
  // they'll be the same unless the scene-loader changed something.
  //
  render_params = scene_def.params.filter_by_prefix ("render.");
  output_params = scene_def.params.filter_by_prefix ("output.");


  // Do post-load scene setup (nothing can be added to scene after this).
  //
  Octree::BuilderFactory octree_builder_factory;
  scene.setup (octree_builder_factory);

  // Do camera manipulation specified on the command-line.
  //
  if (camera_cmds.length () > 0)
    interpret_camera_cmds (camera_cmds, camera, scene);

  Rusage scene_end_ru;		// stop timing scene definition


  // Get the final image size.
  //
  get_image_size (output_params, camera.aspect_ratio (), DEFAULT_IMAGE_SIZE,
		  width, height);

  // Set the camera's aspect ration to be consistent with the final
  // output image size.
  //
  camera.set_aspect_ratio (float (width) / float (height));


  // Output filename
  //
  std::string file_name = output_params.get_string ("filename");


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

  // Set the base-coordinates of the "output sample space" to LIMIT_X, LIMIT_Y.
  //
  if (limit_x != 0)
    output_params.set ("sample-base-x", limit_x);
  if (limit_y != 0)
    output_params.set ("sample-base-y", limit_y);

  // If the scene has a non-default background alpha set, then make sure
  // there's an alpha-channel in the output image.
  //
  if (render_params.get_float ("background-alpha", 1) != 1)
    output_params.set ("alpha-channel", true);

  // Create output image.  The size of what we output is the same as the
  // limit (which defaults to, but is not the same as the nominal output
  // image size).
  //
  // This will produce diagnostics and exit if there's something wrong
  // with IMAGE_SINK_PARAMS or a problem creating the output image, so
  // we create the image before printing any normal output.
  //
  ImageSampledOutput output (file_name, limit_width, limit_height,
			     output_params);

  if (output_params.get_bool ("alpha-channel,alpha")
      && !output.has_alpha_channel())
    {
      std::cerr << clp.err_pfx() << file_name
		<< ": alpha-channel not supported"
		<< std::endl;
      return 1;
    }

  if (recover_input)
    {
      unsigned num_rows_recovered = recover_image (recover_input, output);
      recover_input = 0;

      if (! quiet)
	cout << "* recover: " << file_name
	     << ": Recovered " << num_rows_recovered << " rows" << endl;

      if (num_rows_recovered == limit_height)
	{
	  cout << file_name << ": Entire image was recovered, not rendering"
	       << endl;
	  return 0; // We want destructors to run, so must not use exit!
	}

      // Remove the recovered rows from what we will render.
      //
      limit_y += num_rows_recovered;
      limit_height -= num_rows_recovered;
    }

  if (! quiet)
    {
      cout << "* scene: "
	   << commify_with_units (scene.num_surfaces(),
				  "top-level surface", "top-level surfaces")
	   << ", "
	   << commify_with_units (scene.num_lights (), "light", "lights")
	   << endl;
      cout << "* camera: at " << camera.pos
	   << ", pointing at "
	   << (camera.pos + camera.forward * camera.target_dist)
	   << " (up = " << camera.up << ", right = " << camera.right << ")"
	   << endl;
      if (camera.aperture)
	cout << "* camera: aperture " << camera.aperture
	     << ", focal distance " << camera.focus
	     << endl;
    }


  // If the user didn't specify how many threads to use, try to use as
  // many as there are CPU cores.
  //
  if (num_threads == 0)
    num_threads = num_cores ();

  if (num_threads != 1)
    std::cout << "* using " << num_threads << " threads" << std::endl;


  // Setup rendering state.  This can take a _very_ long time, because
  // some rendering methods do length pre-rendering computation, so time
  // it.
  //
  Rusage setup_beg_ru;
  GlobalRenderState global_render_state (scene, render_params);
  Rusage setup_end_ru;


  RenderStats render_stats;

  // Start rendering...
  //
  Rusage render_beg_ru;

  // The pattern of pixels we will render; we add a small margin around
  // the output image to keep the edges clean.
  //
  unsigned x_margin = output.filter_x_radius ();
  unsigned y_margin = output.filter_y_radius ();
  RenderPattern pattern (limit_x - x_margin, limit_y - y_margin,
			 limit_width + x_margin*2, limit_height + y_margin*2);

  // Start progress indicator
  //
  TtyProgress prog (std::cout, "rendering...", verbosity);

  // Do the actual rendering.
  //
  RenderMgr render_mgr (global_render_state, camera, width, height);
  render_mgr.render (num_threads, pattern, output, prog, render_stats);

  // Done rendering.
  //
  Rusage render_end_ru;


  Timeval end_time (Timeval::TIME_OF_DAY);


  // Print stats
  //
  if (! quiet)
    {
      render_stats.print (cout);

      long num_eye_rays = limit_width * limit_height;

      // a field width of 14 is enough for over a year of time...
      cout << "Time:" << endl;

      // Note because scene-loading often involves significant disk I/O, we
      // add system time as well (this usually isn't a factor for other
      // time periods we measure).
      //
      Timeval scene_def_time
	= ((scene_end_ru.utime() - scene_beg_ru.utime())
	   + (scene_end_ru.stime() - scene_beg_ru.stime()));
      if (scene_def_time > 1)
	cout << "  scene def cpu:" << setw (14) << scene_def_time << endl;

      Timeval setup_time = setup_end_ru.utime() - setup_beg_ru.utime();
      if (setup_time > 1)
	cout << "  setup cpu:    " << setw (14) << setup_time << endl;

      Timeval render_time = render_end_ru.utime() - render_beg_ru.utime();
      cout << "  rendering cpu:" << setw (14) << render_time << endl;

      Timeval elapsed_time = end_time - beg_time;
      cout << "  total elapsed:" << setw (14) << elapsed_time << endl;

      long long sc  = render_stats.scene_intersect_calls;
      long long sst = render_stats.scene_shadow_tests;
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
