// cmdlineparser.h -- Command-line parser
//
//  Copyright (C) 2005, 2006, 2007, 2010, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_CMDLINEPARSER_H
#define SNOGRAY_CMDLINEPARSER_H

#include <string>
#include <stdexcept>
#include <cstdlib>

#include <getopt.h>

#include "config.h"
#include "version.h"

// The following macros can be used in defining option parsers.
//
#define CMDLINEPARSER_GENERAL_OPTIONS_HELP "\
      --help                 Output this help message\n\
      --version              Output program version"
//
#define CMDLINEPARSER_GENERAL_SHORT_OPTIONS	""
//
#define CMDLINEPARSER_GENERAL_LONG_OPTIONS				\
  { "help",	no_argument,	   0, CMDLINEPARSER_OPT_HELP },		\
  { "version",	no_argument,	   0, CMDLINEPARSER_OPT_VERSION }
//
#define CMDLINEPARSER_OPT_BASE		(('C'<<24)+('M'<<16)+('D'<<8))
#define CMDLINEPARSER_OPT_HELP		(CMDLINEPARSER_OPT_BASE + 1)
#define CMDLINEPARSER_OPT_VERSION	(CMDLINEPARSER_OPT_BASE + 2)
//
#define CMDLINEPARSER_GENERAL_OPTION_CASES(clp)			\
  case CMDLINEPARSER_OPT_HELP:					\
    help (clp, std::cout);					\
    exit (0);							\
  case CMDLINEPARSER_OPT_VERSION:				\
    std::cout << clp.version_string () << std::endl;		\
    exit (0);


namespace snogray {

class ValTable;


class CmdLineParser
{
public:

  CmdLineParser (int _argc, char *const *_argv,
		 const char *_short_opts, const struct option *_long_opts)
    : argc (_argc), argv (_argv),
      short_opts (_short_opts), long_opts (_long_opts),
      long_opt_index (-1), short_opt (0)
  { }

  std::string prog_name () const;

  int get_opt ();

  const char *opt_arg () const;

  float float_opt_arg () const;
  float float_opt_arg (float default_val) const;
  unsigned unsigned_opt_arg () const;
  unsigned unsigned_opt_arg (unsigned default_val) const;

  void opt_err (const char *phrase) const __attribute__ ((noreturn));
  void opt_err (const std::string &phrase) const __attribute__ ((noreturn));

  void err (const char *phrase) const __attribute__ ((noreturn));
  void err (const std::string &phrase) const __attribute__ ((noreturn));
  std::string err_pfx () const;
  std::string opt_err_pfx () const;

  // Print a "Try prog --help for more help" message and exit
  //
  void try_help_err () const __attribute__ ((noreturn));

  const char *get_arg ();
  int num_remaining_args () const;

  // Parse the argument of the current option using "NAME=VALUE"
  // syntax, and store an entry with name NAME and value VALUE into
  // TABLE.  The syntax "NAME:VALUE" is also accepted.  The type of
  // the new value is always a string (which can be converted to
  // another type when the value is subsequently requested).
  //
  void parse_opt_arg (ValTable &table) { parse (opt_arg(), table); }

  // Split the argument of the current option into parts separated
  // by any character in MULTIPLE_SEPS, removing any whitespace
  // surrounding a separator, and then parse each part using
  // "NAME=VALUE" syntax, and store the resulting entries into
  // TABLE.  The syntax "NAME:VALUE" is also accepted.  The type of
  // the new value is always a string (which can be converted to
  // another type when the value is subsequently requested).
  //
  void parse_opt_arg (const std::string &multiple_seps, ValTable &table)
  {
    parse (opt_arg(), multiple_seps, table);
  }

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
  void store_opt_arg_with_sub_options (
	 const std::string &name,
	 ValTable &table,
	 const std::string &main_subkey,
	 const std::string &first_option_seps,
	 const std::string &multiple_option_seps = "");

  // Return a string containing the program name and version.
  //
  std::string version_string () const;

private:

  // Parse the named-value specification STR using "NAME=VALUE" syntax, and
  // store VALUE in TABLE under the name NAME.  The syntax "NAME:VALUE" is
  // also accepted.  The type of the new value is always a string (which
  // can be converted to another type when the value is subsequently
  // requested).  NAME_PREFIX is prepended to names before storing.
  //
  void parse (const std::string &str, ValTable &table,
	      const std::string &name_prefix = "");

  // First split STR option into parts separated by any character in
  // MULTIPLE_SEPS, removing any whitespace surrounding a separator, and
  // then parse each part using "NAME=VALUE" syntax, and store the
  // resulting entries into TABLE.  The syntax "NAME:VALUE" is also
  // accepted.  The type of the new value is always a string (which can
  // be converted to another type when the value is subsequently
  // requested).  NAME_PREFIX is prepended to names before storing.
  //
  void parse (const std::string &str, const std::string &multiple_seps,
	      ValTable &table, const std::string &name_prefix = "");

  int argc;
  char *const *argv;

  const char *short_opts;
  const struct option *long_opts;

  // These are set after calling getopt
  int long_opt_index;
  int short_opt;
};

#define CMDLINEPARSER_CATCH(clp, expr) \
  try { (expr); } catch (std::runtime_error &err) { clp.err (err.what ()); }


}

#endif /* SNOGRAY_CMDLINEPARSER_H */

// arch-tag: 5852e7d6-c333-492e-8c89-0f6ef8f09056
