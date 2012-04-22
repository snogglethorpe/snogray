// cmdlineparser.cc -- Command-line parser
//
//  Copyright (C) 2005-2007, 2010, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <string>
#include <cstring>

#include "val-table.h"
#include "string-funs.h"

#include "cmdlineparser.h"

using namespace snogray;
using namespace std;


// misc

std::string
CmdLineParser::prog_name () const
{
  const char *basename = strrchr (argv[0], '/');
  if (basename)
    basename++;
  else
    basename = argv[0];
  return basename;
}


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

// Print a "Try prog --help for more help" message and exit
//
void
CmdLineParser::try_help_err () const
{
  cerr << "Try `" << prog_name() << " --help' for more information" << endl;
  exit (10);
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

unsigned
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
    pfx += char (short_opt);
  pfx += "'";
  return pfx;
}

void
CmdLineParser::opt_err (const char *phrase) const
{
  cerr << opt_err_pfx() << " " << phrase << endl;
  exit (2);
}

void
CmdLineParser::opt_err (const string &phrase) const
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

unsigned
CmdLineParser::unsigned_opt_arg (unsigned default_val) const
{
  if (optarg)
    return unsigned_opt_arg ();
  else
    return default_val;
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

float
CmdLineParser::float_opt_arg (float default_val) const
{
  if (optarg)
    return float_opt_arg ();
  else
    return default_val;
}


// Parsing and ValTable storage

// Return a copy of STR with all dashes ("-") replaced by underscores
// ("-").
//
static std::string
dashes_to_underscores (const std::string &str)
{
  std::string::size_type pos;
  std::string copy = str;
  while ((pos = str.find_first_of ("-", pos)) != std::string::npos)
    copy[pos] = '_';
  return copy;
}

// Parse the named-value specification STR using "NAME=VALUE" syntax,
// and store VALUE in TABLE under the name NAME, with any dashes ("-")
// in NAME replaced with underscores ("_").  The syntax "NAME:VALUE"
// is also accepted.  The type of the new value is always a string
// (which can be converted to another type when the value is
// subsequently requested).
//
void
CmdLineParser::parse (const std::string &str, ValTable &table)
{
  std::string::size_type inp_len = str.length ();
  std::string::size_type p_assn = str.find_first_of ("=:");

  if (p_assn < inp_len)
    table.set (dashes_to_underscores (str.substr (0, p_assn)),
	       str.substr (p_assn + 1));
  else if (str[0] == '!')
    table.set (dashes_to_underscores (str.substr (1)), false);
  else if (begins_with (str, "no-"))
    table.set (dashes_to_underscores (str.substr (3)), false);
  else
    table.set (dashes_to_underscores (str), true);
}

// First split STR option into parts separated by any character in
// MULTIPLE_SEPS, removing any whitespace surrounding a separator, and
// then parse each part using "NAME=VALUE" syntax, and store the
// resulting entries into TABLE, with any dashes ("-") in NAME
// replaced with underscores ("_").  The syntax "NAME:VALUE" is also
// accepted.  The type of the new value is always a string (which can
// be converted to another type when the value is subsequently
// requested).
//
void
CmdLineParser::parse (const std::string &str, const std::string &multiple_seps,
		      ValTable &table)
{
  std::string::size_type p_end = str.find_first_of (multiple_seps);

  if (p_end == std::string::npos)
    {
      parse (str, table);
    }
  else
    {
      std::string::size_type p_start = 0;
      do
	{
	  parse (str.substr (p_start, p_end - p_start), table);

	  p_start = str.find_first_not_of (multiple_seps, p_end);
	  p_end = str.find_first_of (multiple_seps, p_start);
	}
      while (p_end != std::string::npos);

      if (p_start != std::string::npos)
	parse (str.substr (p_start), table);
    }
}


// ValTable::set_with_options

// First, split the current option-argument into a "main value"
// MAIN_VAL, and "optional values", at any character in
// FIRST_OPTION_SEPS (removing any surrounding whitespace).  Then,
// store MAIN_VAL into PARAMS with the key (NAME + "." + MAIN_SUBKEY).
// The optional values will be further split apart using
// MULTIPLE_OPTION_SEPS, as if with CmdLineParser::parse_opt_arg, and
// each OPT_NAME=OPT_VAL pair will be stored as well, into entries
// with keys (NAME + "." OPT_NAME).
//
// If MULTIPLE_OPTION_SEPS is "" (the default), FIRST_OPTION_SEPS
// will be used for splitting further options instead.
//
// For example: if the current option argument is
// "oink/bar=zoo,zing=3", NAME is "plugh", MAIN_SUBKEY is "type",
// FIRST_OPTION_SEPS is "/", and MULTIPLE_OPTION_SEPS is ",/", then
// MAIN_VAL will be "oink", and the following entries will be stored
// into TABLE:
//
//   "plugh.type" => "oink"
//   "plugh.bar"  => "zoo"
//   "plugh.zing" => 3
//
void
CmdLineParser::store_opt_arg_with_sub_options (
		 const std::string &name,
		 ValTable &table,
		 const std::string &main_subkey,
		 const std::string &first_option_seps,
		 const std::string &multiple_option_seps)
{
  std::string val = opt_arg ();
  unsigned val_end = val.find_first_of (first_option_seps);

  std::string sep2 = multiple_option_seps;
  if (sep2 == "")
    sep2 = first_option_seps;

  ValTable &sub_table = table.writable_subtable (name);

  std::string main_val = val;

  if (val_end < val.length ())
    {
      unsigned options_start
	= val.find_first_not_of (first_option_seps + " \t", val_end + 1);

      parse (val.substr (options_start), sep2, sub_table);

      main_val = val.substr (0, val_end);
    }

  sub_table.set (main_subkey, main_val);
}

// Return a string containing the program name and version.
//
std::string
CmdLineParser::version_string () const
{
  std::string name = prog_name ();
  if (name != PACKAGE_NAME)
    name = name + " (" + PACKAGE_NAME + ")";
  name = name + " " + snogray_version;
  return name;
}


// arch-tag: 2a1d984b-5816-4d7d-975b-31a39ed3b5d8
