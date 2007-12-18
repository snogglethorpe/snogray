// snogsamp.cc -- Dump a "sample map"
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
#include "render-cmdline.h"
#include "trace-params.h"
#include "envmap-light.h"

using namespace snogray;
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


// dump_bg

enum DumpBgKind { DUMP_BG_NONE, DUMP_BG_ENV_MAP, DUMP_BG_LIGHT_MAP, DUMP_BG_DIFF };

static void
dump_bg (const Scene &scene, Image &map, DumpBgKind what)
{
  const EnvmapLight *eml = 0;

  for (std::vector<const Light *>::const_iterator li = scene.lights.begin();
       !eml && li != scene.lights.end(); ++li)
    eml = dynamic_cast<const EnvmapLight *> (*li);

  float w = float (map.width), h = float (map.height);
  float iw = 1 / w, ih = 1 / h;

  for (float y = 0; y < h; y++)
    for (float x = 0; x < w; x++)
      {
	Color bg, lm;

	if (what != DUMP_BG_ENV_MAP)
	  {
	    float u = x * iw, v = (1 - y * ih);
	    float pdf;
	    lm = eml->intensity (u, v, pdf);
	  }

	if (what != DUMP_BG_LIGHT_MAP)
	  {
	    dist_t colat = -(y * ih * PI - PI/2);
	    dist_t lng = x * iw * PI * 2 - PI;
	    Vec dir = y_axis_latlong_to_vec (colat, lng);
	    bg = scene.background (dir);
	  }

	Color col;
	if (what == DUMP_BG_ENV_MAP)
	  col = bg;
	else if (what == DUMP_BG_LIGHT_MAP)
	  col = lm;
	else if (what == DUMP_BG_DIFF)
	  col = abs (bg - lm);

	map.put (x, y, col);
      }
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
s "  -i, --intensity            Indicate sample intensity too"
s "  -r, --radius=RADIUS        Draw samples with radius RADIUS"
n
s "  -x, --background[=WHAT]    Show background image according to WHAT:"
s "                               env   -- scene environment map (default)"
s "                               light -- scene light map"
s "                               diff  -- difference between `env' and `light'"
n
s "  -N, --no-normalize         Don't normalize sample values"
n
s SCENE_DEF_OPTIONS_HELP
n
s IMAGE_OUTPUT_OPTIONS_HELP
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
    { "background",	optional_argument, 0, 'x' },
    { "intensity",	no_argument,	   0, 'i' },
    { "no-normalize",	no_argument, 	   0, 'N' },
    { "map-size",	required_argument, 0, 'm' },
    { "brdf",		no_argument,	   0, OPT_BRDF },
    { "lights",		no_argument,	   0, OPT_LIGHTS },
    { "color",		required_argument, 0, 'C' },
    { "radius",		required_argument, 0, 'r' },
    SCENE_DEF_LONG_OPTIONS,
    IMAGE_OUTPUT_LONG_OPTIONS,
    RENDER_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  //
  char short_options[] =
    "s:m:Nr:iC:x::"
    SCENE_DEF_SHORT_OPTIONS
    IMAGE_OUTPUT_SHORT_OPTIONS
    RENDER_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  SceneDef scene_def;			      // scene to be probed
  ValTable image_params, render_params;
  unsigned width = 640, height = 480; // virtual "camera image"
  unsigned map_width = 800, map_height = 400; // sample map size
  bool no_normalize = false, show_intensity = false;
  DumpBgKind dump_bg_kind = DUMP_BG_NONE;
  // the following is the default, and is treated as "show both"
  bool use_light_samples = false, use_brdf_samples = false;
  unsigned sample_radius = 2;	// default
  Color sample_color (1, 0.2, 0.1);	// default

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
      case 'N':
	no_normalize = true;
	break;
      case 'x':
	{
	  const char *arg = clp.opt_arg ();
	  if (!arg || strcmp (arg, "env") == 0 || strcmp (arg, "e") == 0)
	    dump_bg_kind = DUMP_BG_ENV_MAP;
	  else if (strcmp (arg, "light") == 0 || strcmp (arg, "l") == 0)
	    dump_bg_kind = DUMP_BG_LIGHT_MAP;
	  else if (strcmp (arg, "diff") == 0 || strcmp (arg, "d") == 0)
	    dump_bg_kind = DUMP_BG_DIFF;
	}
	break;
      case 'i':
	show_intensity = true;
	break;
#if 0
      case 'C':
	sample_color = parse_color (clp.opt_arg ());
	break;
#endif
      case OPT_LIGHTS: 		// sample lights only
	use_light_samples = true;
	break;
      case OPT_BRDF: 		// sample brdf only
	use_brdf_samples = true;
	break;
      case 'r':			// set sample size
	sample_radius = clp.unsigned_opt_arg ();
	break;

	SCENE_DEF_OPTION_CASES (clp, scene_def);
	IMAGE_OUTPUT_OPTION_CASES (clp, image_params);
	RENDER_OPTION_CASES (clp, render_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  unsigned x = parse_coord (clp, "x-position", width);
  unsigned y = parse_coord (clp, "y-position", height);

  CMDLINEPARSER_CATCH
    (clp, scene_def.parse (clp, clp.num_remaining_args() - 1));

  std::string filename = clp.get_arg();

  if (!use_light_samples && !use_brdf_samples)
    use_light_samples = use_brdf_samples = true;
  if (! use_light_samples)
    render_params.set ("light-samples", 0);
  if (! use_brdf_samples)
    render_params.set ("brdf-samples", 0);

  // Define the scene.

  Scene scene;
  Camera camera;

  camera.set_aspect_ratio (float (width) / float (height));

  CMDLINEPARSER_CATCH (clp, scene_def.load (scene, camera));

  // Make a map

  TraceParams trace_params (render_params);

  SampleMap smap;

  float u = float (x) / float (width);
  float v = float (y) / float (height);
  unsigned num = smap.sample (camera.eye_ray (u, v), scene, trace_params);

  if (show_intensity && !no_normalize)
    smap.normalize ();

  cout << "sample map has " << smap.num_samples
       << " / " << num << " samples" << endl;

  if (show_intensity)
    {
      cout << "   min intensity = " << smap.min << endl;
      cout << "   max intensity = " << smap.max << endl;
      cout << "   avg intensity = " << smap.sum / num << endl;
    }

  Image map (map_width, map_height);

  if (dump_bg_kind != DUMP_BG_NONE)
    dump_bg (scene, map, dump_bg_kind);

  smap.draw (map, sample_radius, show_intensity ? -1 : sample_color);

  map.save (filename, image_params);

  return 0;
}


// arch-tag: dcd4d4d2-d3b4-43bc-8251-bfa7139ae20c
