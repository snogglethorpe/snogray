// snogsamp.cc -- Dump a "sample map"
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstring>
#include <iostream>
#include <string>

#include "scene.h"
#include "camera.h"
#include "scene-def.h"
#include "sample-map.h"
#include "cmdlineparser.h"
#include "image-cmdline.h"

using namespace Snogray;
using namespace std;

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



static unsigned
parse_coord (CmdLineParser &clp, const char *what, unsigned size)
{
  if (clp.num_remaining_args () == 0)
    clp.err (string ("missing ") + what);

  string arg = clp.get_arg ();
  unsigned len = arg.length ();

  if (len == 0 || arg.find_last_not_of ("0123456789.%") < len)
    clp.err (arg + ": invalid " + what);

  if (arg[len - 1] == '%')
    return atoi (arg.c_str ()) * size / 100;
  else if (arg.find ('.') < len)
    return unsigned (atof (arg.c_str ()) * size);
  else
    return atoi (arg.c_str ());
}


// Main driver

static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << " [OPTION...] X_POS Y_POS [SCENE_FILE... [MAP_IMAGE_FILE]]" << endl;
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
  "Shoot a ray at a scene and dump a light-sample map of the first intersection"
n
s "  -m, --map-size=WIDTHxHEIGHT  Set output map size (default 800 x 400)"
n
s "  -s, --size=WIDTHxHEIGHT    Set camera image size (default 640 x 480)"
n
s "      --brdf                 Only sample the BRDF"
s "      --lights               Only sample the lights"
n
s "  -w, --sample-width=N       Set width of displayed samples"
n
s "  -n, --no-normalize         Don't normalize sample values"
n
s SCENE_DEF_OPTIONS_HELP
n
s IMAGE_OUTPUT_FMT_OPTIONS_HELP
n
s IMAGE_OUTPUT_TRANSFORM_OPTIONS_HELP
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "X_POS and Y_POS determine the location in the virtual camera image of"
s "the ray to be shot.  They can be pixel row/column positions or a"
s "fractional location within the image.  The latter may be floating-"
s "point fractions (containing a `.') or percentages (with a `%' suffix)."
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

#define OPT_BRDF	1
#define OPT_LIGHTS	2


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "size",		required_argument, 0, 's' },
    { "map-size",	required_argument, 0, 'm' },
    { "no-normalize",	no_argument, 	   0, 'n' },
    { "brdf",		no_argument,	   0, OPT_BRDF },
    { "lights",		no_argument,	   0, OPT_LIGHTS },
    { "size",		required_argument, 0, 'r' },
    SCENE_DEF_LONG_OPTIONS,
    IMAGE_OUTPUT_FMT_LONG_OPTIONS,
    IMAGE_OUTPUT_TRANSFORM_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "s:m:nr:"
    SCENE_DEF_SHORT_OPTIONS
    IMAGE_OUTPUT_FMT_SHORT_OPTIONS
    IMAGE_OUTPUT_TRANSFORM_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  SceneDef scene_def;			      // scene to be probed
  ImageCmdlineSinkParams image_sink_params (clp);
  unsigned width = 640, height = 480; // virtual "camera image"
  unsigned map_width = 800, map_height = 400; // sample map size
  bool no_normalize = false;
  SampleMap::type map_type = SampleMap::FILTERED;
  unsigned sample_width = 5;	// default

  // This speeds up I/O on cin/cout by not syncing with C stdio.
  //
  ios::sync_with_stdio (false);

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 's':			// image size
	parse_size_opt_arg (clp, width, height);
	break;
      case 'm':			// map size
	parse_size_opt_arg (clp, map_width, map_height);
	break;
      case OPT_LIGHTS: 		// sample lights only
	map_type = SampleMap::LIGHTS;
	break;
      case OPT_BRDF: 		// sample brdf only
	map_type = SampleMap::BRDF;
	break;
      case 'r':			// set sample size
	sample_width = clp.unsigned_opt_arg ();
	break;

	SCENE_DEF_OPTION_CASES (clp, scene_def);
	IMAGE_OUTPUT_FMT_OPTION_CASES (clp, image_sink_params);
	IMAGE_OUTPUT_TRANSFORM_OPTION_CASES (clp, image_sink_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  unsigned x = parse_coord (clp, "x-position", width);
  unsigned y = parse_coord (clp, "y-position", height);

  CMDLINEPARSER_CATCH
    (clp, scene_def.parse (clp, clp.num_remaining_args() - 1));

  image_sink_params.file_name = clp.get_arg();

  // Define the scene.

  Scene scene;
  Camera camera;

  camera.set_aspect_ratio ((float)width / (float)height);

  CMDLINEPARSER_CATCH (clp, scene_def.load (scene, camera));

  // Make a map

  SampleMap map (map_width, map_height, map_type);

  map.sample (camera.get_ray (x, y, width, height), scene);

  if (! no_normalize)
    map.normalize ();

  if (sample_width > 0)
    {
      // We use the image backend's image-smoothing to make circular
      // "smears" for each sample.  The relationship between size,
      // brightness, and perceived size, is very ad-hoc, but this works
      // well enough.

      image_sink_params.aa_overlap = sample_width;
      image_sink_params.exposure += log (float (sample_width)) / 2 + 3;
    }

  map.save (image_sink_params);

  cout << "sample map has " << map.num_samples << " samples" << endl;
  cout << "   min = " << map.min << endl;
  cout << "   max = " << map.max << endl;
  cout << "   sum = " << map.sum << endl;
}

// arch-tag: dcd4d4d2-d3b4-43bc-8251-bfa7139ae20c
