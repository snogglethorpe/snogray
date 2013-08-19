// sampleimg.cc -- Generate samples from an input image
//
//  Copyright (C) 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <getopt.h>
#include <iostream>

#include "cli/version.h"
#include "image/image.h"
#include "util/random.h"
#include "geometry/hist-2d.h"
#include "geometry/hist-2d-dist.h"
#include "render/grid.h"
#include "render/sample-set.h"
#include "util/radical-inverse.h"

using namespace snogray;


// usage/help messages

static void
usage (const char *prog_name, std::ostream &os)
{
  os << "Usage: " << prog_name
     << " [OPTION...] INPUT_IMAGE OUTPUT_IMAGE"
     << std::endl;
}

static void
try_help (const char *prog_name, std::ostream &os)
{
  os << "Try '" << prog_name << " --help' for more information."
     << std::endl;
}

static void
help (const char *prog_name, std::ostream &os)
{
  usage (prog_name, os);

  // These macros just makes the source code for help output easier to line up
  //
#define s  << std::endl <<
#define n  << std::endl

  os <<
  "Generate an output image using random samples with distribution based"
s "on an input image."
n
s "  -m, --method=METH          Use sampling method METH (default 'radical')"
s "  -n, --samples=NUM          Don't print image statistics (default 1000000)"
s "  -v, --value=VAL            Intensity of each sample"
n
s "      --help                 Output this help message"
s "      --version              Output program version"
n
s "Sampling methods are:"
s "   radical     Radical inverse"
s "   random      Completely random"
s "   stratified  Random within a grid pattern"
s "   grid        Rigid grid pattern"
n
s "If the -v/--value option is not given, a value will chosen"
s "automatically to try and approximate the input image's intensity."
s "If the -v/--value is given, but not the -s/--samples option,"
s "the number of samples used will be chosen similarly." 
n
    ;

#undef s
#undef n
}

#define OPT_HELP	-10
#define OPT_VERSION	-11

static struct option long_options[] = {
  { "meth",		required_argument, 0, 'm' },
  { "method",		required_argument, 0, 'm' },
  { "num",		required_argument, 0, 'n' },
  { "samples",		required_argument, 0, 'n' },
  { "num-samples",	required_argument, 0, 'n' },
  { "val",		required_argument, 0, 'v' },
  { "value",		required_argument, 0, 'v' },
  { "help",		no_argument, 	   0, OPT_HELP },
  { "version",		no_argument, 	   0, OPT_VERSION },
  { 0, 0, 0, 0 }
};
static char short_options[] = "m:n:v:";


int main (int argc, char *argv[])
{
  const char *prog_name = argv[0];
  const char *meth_name = "radical";
  unsigned num_samples = 1000000;
  bool num_samples_specified = false; // true if user specified it
  Color samp_color = 0;

  int opt;
  while ((opt = getopt_long (argc, argv, short_options, long_options, 0)) != -1)
    switch (opt)
      {
      case 'm':
	meth_name = optarg;
	break;
      case 'n':
	num_samples = atoi (optarg);
	num_samples_specified = true;
	break;
      case 'v':
	samp_color = atof (optarg);
	break;
      case OPT_HELP:
	help (prog_name, std::cout);
	exit (0);
      case OPT_VERSION:
	std::cout << prog_name << " (" << PACKAGE_NAME << ") "
		  << snogray_version << std::endl;
	exit (0);
      default:
	try_help (prog_name, std::cerr);
	exit (1);
      }

  enum { RADICAL, RANDOM, STRATIFIED, GRID } method;

  if (strcmp (meth_name, "radical-inverse") == 0
      || strcmp (meth_name, "radical") == 0
      || strcmp (meth_name, "rad") == 0)
    {
      method = RADICAL;
      meth_name = "radical-inverse";
    }
  else if (strcmp (meth_name, "random") == 0
	   || strcmp (meth_name, "rand") == 0)
    {
      method = RANDOM;
      meth_name = "random";
    }
  else if (strcmp (meth_name, "stratified") == 0
	   || strcmp (meth_name, "strat") == 0)
    {
      method = STRATIFIED;
      meth_name = "stratified";
    }
  else if (strcmp (meth_name, "grid") == 0)
    {
      method = GRID;
      meth_name = "grid";
    }
  else
    {
      std::cerr << prog_name << ": " << meth_name
		<< ": Unknown sampling method" << std::endl;
      exit (2);
    }

  if (optind != argc - 2)
    {
      usage (prog_name, std::cerr);
      try_help (prog_name, std::cerr);
      exit (1);
    }

  Image inp_image (argv[optind]);

  unsigned w = inp_image.width, h = inp_image.height;

  Hist2d hist (w, h);

  intens_t intens_sum = 0;
  for (unsigned row = 0; row < h; row++)
    for (unsigned col = 0; col < w; col++)
      {
	Color color = inp_image (col, row);
	intens_t intens = color.intensity();;
	intens_sum += intens;
	hist.add (col, row, intens);
      }

  // If the user didn't specify a value, choose one based on the image
  // brightness.
  //
  if (samp_color == 0)
    samp_color = min (intens_sum / num_samples, 1.f);
  else if (! num_samples_specified)
    num_samples = intens_sum / samp_color.intensity();

  Random rng;
  Hist2dDist dist (hist);

  double inv_pdf_sum = 0;
  unsigned zero_count = 0, nan_count = 0;

  Grid strat;
  SampleSet sample_set (num_samples, strat, rng);
  SampleSet::Channel<UV> chan = sample_set.add_channel<UV> ();
  std::vector<UV> grid;

  if (method == STRATIFIED)
    sample_set.generate ();
  else if (method == GRID)
    {
      double sqrt_num = sqrt (double (num_samples));
      double up = ceil (sqrt_num);
      double down = floor (sqrt_num + 0.5);

      num_samples = unsigned (up * down);

      unsigned u_steps = unsigned (up);
      unsigned v_steps = unsigned (down);

      float u_step = 1 / up;
      float v_step = 1 / down;

      float v_offs = 0;

      grid.resize (num_samples);

      std::vector<UV>::iterator samp = grid.begin ();
      for (unsigned i = 0; i < v_steps; i++)
	{
	  float u_offs = 0;

	  for (unsigned j = 0; j < u_steps; j++)
	    {
	      *samp++ = UV (clamp01 (u_offs + 0.5f * u_step),
			    clamp01 (v_offs + 0.5f * v_step));
	      u_offs += u_step;
	    }

	  v_offs += v_step;
	}
    }

  Image out_image (w,h);

  for (unsigned i = 0; i < num_samples; i++)
    {
      UV param;
      if (method == RADICAL)
	param = UV (radical_inverse (i+1,2), radical_inverse(i+1,3));
      else if (method == STRATIFIED)
	param = sample_set.get (chan, i);
      else if (method == GRID)
	param = grid[i];
      else
	param = UV (rng(), rng()); // RANDOM
	
      float pdf;
      UV pos = dist.sample (param, pdf);

      if (pdf == 0)
	zero_count++;
      else if (std::isnan (pdf))
	nan_count++;
      else
	inv_pdf_sum += double (1 / pdf);

      unsigned col = min (unsigned (pos.u * w), w-1);
      unsigned row = min (unsigned (pos.v * h), h-1);

      Color old = out_image (col, row);

      out_image (col, row) = old + samp_color;
    }

  out_image.save (argv[optind + 1]);

  std::cout << "number of samples:  " << num_samples << std::endl;
  std::cout << "sample value:       " << samp_color.intensity() << std::endl;
  std::cout << "sampling method:    " << meth_name << std::endl;
  std::cout << "PDF reciprocal sum: " << inv_pdf_sum / num_samples << std::endl;

  if (zero_count != 0)
    std::cout << "number of zeroes:   " << zero_count << std::endl;
  if (nan_count != 0)
    std::cout << "number of NaNs:     " << nan_count << std::endl;

  return 0;
}
