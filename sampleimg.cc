// sampleimg.cc -- Generate samples from an input image
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
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

#include "image.h"
#include "random.h"
#include "hist-2d.h"
#include "hist-2d-dist.h"
#include "grid.h"
#include "sample-set.h"
#include "radical-inverse.h"

using namespace snogray;

static struct option long_options[] = {
  { "meth",		required_argument, 0, 'm' },
  { "method",		required_argument, 0, 'm' },
  { "num",		required_argument, 0, 'n' },
  { "samples",		required_argument, 0, 'n' },
  { "num-samples",	required_argument, 0, 'n' },
  { "val",		required_argument, 0, 'v' },
  { "value",		required_argument, 0, 'v' },
  { 0, 0, 0, 0 }
};
static char short_options[] = "m:n:v:";


static void
usage (const char *prog_name)
{
  std::cerr << "Usage: " << prog_name
	    << " [OPTION...] INPUT_IMAGE OUTPUT_IMAGE" << std::endl;
  exit (1);
}


int main (int argc, char *argv[])
{
  const char *prog_name = basename (argv[0]);
  const char *meth_name = "radical";
  unsigned num_samples = 1000000;
  Color samp_color = 0.1;

  int opt;
  while ((opt = getopt_long (argc, argv, short_options, long_options, 0)) > 0)
    switch (opt)
      {
      case 'm':
	meth_name = optarg;
	break;
      case 'n':
	num_samples = atoi (optarg);
	break;
      case 'v':
	samp_color = atof (optarg);
	break;
      default:
	usage (prog_name);
      }

  enum { RADICAL, RANDOM, STRATIFIED } method;

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
  else
    {
      std::cerr << prog_name << ": " << meth_name
		<< ": Unknown sampling method" << std::endl;
      exit (2);
    }

  if (optind != argc - 2)
    usage (prog_name);

  Image inp_image (argv[optind]);

  unsigned w = inp_image.width, h = inp_image.height;

  Hist2d hist (w, h);

  for (unsigned row = 0; row < h; row++)
    for (unsigned col = 0; col < w; col++)
      {
	Color color = inp_image (col, row);
	float intens = color.intensity();;
	hist.add (col, row, intens);
      }

  Random rng;
  Hist2dDist dist (hist);

  double inv_pdf_sum = 0;
  unsigned zero_count = 0, nan_count = 0;

  Grid grid;
  SampleSet sample_set (num_samples, grid, rng);
  SampleSet::Channel<UV> chan = sample_set.add_channel<UV> ();

  if (method == STRATIFIED)
    sample_set.generate ();

  Image out_image (w,h);

  for (unsigned i = 0; i < num_samples; i++)
    {
      UV param;
      if (method == RADICAL)
	param = UV (radical_inverse (i+1,2), radical_inverse(i+1,3));
      else if (method == STRATIFIED)
	param = sample_set.get (chan, i);
      else
	param = UV (rng(), rng()); // RANDOM
	
      float pdf;
      UV pos = dist.sample (param, pdf);

      if (pdf == 0)
	zero_count++;
      else if (std::isnan (pdf))
	nan_count++;
      else
	inv_pdf_sum += 1 / pdf;

      unsigned col = min (unsigned (pos.u * w), w-1);
      unsigned row = min (unsigned (pos.v * h), h-1);

      Color old = out_image (col, row);

      out_image (col, row) = old + samp_color;
    }

  out_image.save (argv[optind + 1]);

  std::cout << "number of samples:  " << num_samples << std::endl;
  std::cout << "sampling method:    " << meth_name << std::endl;
  std::cout << "PDF reciprocal sum: " << inv_pdf_sum / num_samples << std::endl;

  if (zero_count != 0)
    std::cout << "number of zeroes:   " << zero_count << std::endl;
  if (nan_count != 0)
    std::cout << "number of NaNs:     " << nan_count << std::endl;

  return 0;
}
