// cmdlineparser.cc -- Command-line parser
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
#include <string>

#include "cmdlineparser.h"

using namespace Snogray;
using namespace std;


// General error handling

string
CmdLineParser::err_pfx () const
{
  return string (argv[0]) + ": ";
}

void
CmdLineParser::err (const char *phrase) const
{
  cerr << err_pfx() << phrase << endl;
  exit (1);
}

void
CmdLineParser::err (const string &phrase) const
{
  err (phrase.c_str ());
}


// Option handling

int
CmdLineParser::get_opt ()
{
  long_opt_index = -1;		// not guaranteed set by getopt_long
  short_opt = getopt_long (argc, argv, short_opts, long_opts, &long_opt_index);
  if (short_opt == '?')
    exit (1);
  return short_opt;
}

int
CmdLineParser::num_remaining_args () const
{
  return argc - optind;
}

const char *
CmdLineParser::get_arg ()
{
  if (argc == optind)
    return 0;
  else
    return argv[optind++];
}

string
CmdLineParser::opt_err_pfx () const
{
  string pfx = err_pfx ();
  pfx += "Option `-";
  if (long_opt_index >= 0)
    {
      pfx += "-";
      pfx += long_opts[long_opt_index].name;
    }
  else
    pfx += (char)short_opt;
  pfx += "'";
  return pfx;
}

void
CmdLineParser::opt_err (const char *phrase) const
{
  cerr << opt_err_pfx() << " " << phrase << endl;
  exit (2);
}

const char *
CmdLineParser::opt_arg () const
{
  return optarg;
}  

unsigned
CmdLineParser::unsigned_opt_arg () const
{
  if (isdigit (optarg[0]))
    return atoi (optarg);
  else
    opt_err ("requires a numeric argument");
}  

float
CmdLineParser::float_opt_arg () const
{
  if (isdigit (optarg[0])
      || (optarg[0] == '.' && isdigit (optarg[1]))
      || ((optarg[0] == '-' || optarg[0] == '+')
	  && (isdigit (optarg[1])
	      || (optarg[1] == '.' && isdigit (optarg[2])))))
    return atof (optarg);
  else
    opt_err ("requires a numeric argument");
}

// arch-tag: 2a1d984b-5816-4d7d-975b-31a39ed3b5d8
