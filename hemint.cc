// hemint.cc -- numerically integrate over the hemisphere
//
//  Copyright (C) 2006, 2007, 2010, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

// This program does a very simple 2d numerical integration of various
// functions over the hemisphere.  It is intended to test whether such
// things as pdfs and bsdfs properly meet certain constraints (pdfs must
// integrate to 1 over the hemisphere, bsdfs must integrate to something
// <= 1).  It assumes that the functions are "isotropic", so the actual
// iteration is only done over dimension.

#include <iostream>
#include <string>
#include <cmath>
#include <cstring>

#include <cmdlineparser.h>

#include "snogmath.h"

using namespace snogray;


#define LB	(-PI/2)
#define UB	(PI/2)
#define STEPS	10000


// Functions

// Constant 1 / 2pi
//
double f_inv_2pi (double, double)
{
  return 1 / (2 * PI);
}

// Phong (cos^n) distribution
//
double f_phong (double cos_theta, double p0)
{
  double exp = p0;
  return (exp + 1) * pow (cos_theta, exp) / (2 * PI);
}

// Cosine (lambertian) distribution
//
double f_cos (double cos_theta, double)
{
  return cos_theta / PI;
}

// Ward distribution
//
double f_ward (double cos_theta, double p0)
{
  if (cos_theta == 0)
    return 0;
  else
    {
      double inv_m2 = 1 / (p0 * p0);
      double inv_cos_theta = 1 / cos_theta;
      double inv_cos2_theta = inv_cos_theta * inv_cos_theta;
      double inv_cos3_theta = inv_cos2_theta * inv_cos_theta;
      double tan2_theta = inv_cos2_theta - 1;
      return inv_m2 * INV_PIf * inv_cos3_theta * exp (-tan2_theta * inv_m2);
    }
}


// List of functions

struct fun
{
  const char *name;
  double (*f) (double cos_theta, double p0);
  double p0;
};

struct fun funs[] = {
  { "inv2pi", f_inv_2pi, 0 },
  { "phong",  f_phong,  10 },
  { "cos",    f_cos,     0 },
  { "ward",   f_ward,   .1 },
  { 0, 0, 0 }
};


// Command-line help

static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name() << " [OPTION...] FUN [ARG...]" << std::endl;
}

static void
help (CmdLineParser &clp, ostream &os)
{
  usage (clp, os);

  // These macros just makes the source code for help output easier to line up
  //
#define s  << std::endl <<
#define n  << std::endl

  os <<
  "Numerically integrate over the hemisphere"
n
s "  -n, --steps=NUM_STEPS      Number of steps to use in integration"
n
s "  -v, --verbose              Show verbose output"
s "  -q, --quiet                Only show result"
n
s "  -f, --function             Output function values for plotting"
s "  -a, --area                 Output function value times area for plotting"
s "  -s, --sum                  Output cumulative value times area for plotting"
n
s "FUN is the name of the function to integrate; supported functions are:"
n
s "   inv2pi  -- Constant 1 / 2pi"
s "   cos     -- Cosine distribution:  cos (theta)"
s "   phong   -- Phong distribution:   (n+1) cos^n (theta) / 2pi"
s "   ward    -- Ward distribution"
n
s "Any following arguments are parameters to the chosen function."
n
    ;

#undef s
#undef n
}


// Main prog

int main (int argc, char *const argv[])
{
  static struct option long_options[] = {
    { "steps",	  required_argument, 0, 'n' },
    { "verbose",  no_argument, 	     0, 'v' },
    { "quiet",    no_argument, 	     0, 'q' },
    { "function", no_argument, 	     0, 'f' },
    { "area",     no_argument, 	     0, 'a' },
    { "sum",      no_argument, 	     0, 's' },
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };

  unsigned steps = STEPS;
  bool verbose = false, samps = false, quiet = false;
  bool val_samps = false, val_area_samps = false, sum_samps = false;

  CmdLineParser clp (argc, argv, "n:vqfas", long_options);

  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'n':
	steps = clp.unsigned_opt_arg (); break;
      case 'v':
	  verbose = true;
      case 'q':
	quiet = true; break;
      case 'f':
	val_samps = samps = true; break;
      case 'a':
	val_area_samps = samps = true; break;
      case 's':
	sum_samps = samps = true; break;

	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  const char *fun_name = clp.get_arg ();
  if (! fun_name)
    {
      usage (clp, std::cerr);
      std::cerr << "Try `" << clp.prog_name() << " --help' for more information"
		<< std::endl;
      exit (1);
    }

  struct fun *fun = 0;
  for (fun = funs; fun->name; fun++)
    if (strcmp (fun->name, fun_name) == 0)
      break;
  if (! fun->name)
    clp.err (std::string ("Unknown function: " + std::string (fun_name)));

  const char *p0_str = clp.get_arg ();
  double p0 = p0_str ? atof (p0_str) : fun->p0;

  double dt = (UB - LB) / steps;

  if (!samps && !quiet)
    {
      std::cout << "fun = " << fun->name << ", p0 = " << p0 << std::endl;
      std::cout << "steps = " << steps << std::endl;
    }

  double sum = 0;
  for (double theta = LB + dt / 2; theta < UB; theta += dt)
    {
      double cos_theta = cos (theta);

      // Value of function at THETA
      //
      double v = (*fun->f) (cos_theta, p0);

      // Half the area of a ring of width DT on the surface of
      // the hemisphere at THETA.
      //
      double a = dt * abs (sin (theta)) * PI;

      sum += v * a;

      if (val_samps)
	std::cout << theta << " " << v << std::endl;
      else if (val_area_samps)
	std::cout << theta << " " << v * a << std::endl;
      else if (sum_samps)
	std::cout << theta << " " << sum << std::endl;
      else if (verbose)
	std::cout << "f (" << theta << ") = " << v << ", a = " << a << std::endl;
    }

  if (! samps)
    {
      if (! quiet)
	std::cout << "integral = ";
      std::cout << sum << std::endl;
    }

  return 0;
}


// arch-tag: 65884dec-05d3-483b-baf7-b1c7d05f9a4f
