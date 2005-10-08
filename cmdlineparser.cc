#include <iostream>

#include "cmdlineparser.h"

using namespace std;

int
CmdLineParser::get_opt ()
{
  short_opt = getopt_long (argc, argv, short_opts, long_opts, &long_opt_index);
  if (short_opt == '?')
    exit (1);
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

void
CmdLineParser::opt_err (const char *phrase) const
{
  if (long_opt_index >= 0)
    cerr << argv[0] << ": option `--" << long_opts[long_opt_index].name
	 << "' " << phrase << endl;
  else
    cerr << argv[0] << ": option `-" << (char)short_opt
	 << "' " << phrase << endl;

  exit (2);
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
  if (isdigit (optarg[0]) || (optarg[0] == '.' && isdigit (optarg[1])))
    return atof (optarg);
  else
    opt_err ("requires a numeric argument");
}

// arch-tag: 2a1d984b-5816-4d7d-975b-31a39ed3b5d8
