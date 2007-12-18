// cmdlineparser.h -- Command-line parser
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

#ifndef __CMDLINEPARSER_H__
#define __CMDLINEPARSER_H__

#include <string>
#include <stdexcept>
#include <cstdlib>

#include <getopt.h>

#include "config.h"

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
    help (clp, cout);						\
    exit (0);							\
  case CMDLINEPARSER_OPT_VERSION:				\
    cout << clp.prog_name() << " (snogray) " << PACKAGE_VERSION	\
         << endl;						\
    exit (0);

namespace snogray {

class CmdLineParser
{
public:

  CmdLineParser (int _argc, char *const *_argv,
		 const char *_short_opts, const struct option *_long_opts)
    : argc (_argc), argv (_argv),
      short_opts (_short_opts), long_opts (_long_opts),
      long_opt_index (-1), short_opt (0)
  { }

  const char *prog_name () const { return argv[0]; }

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

  const char *get_arg ();
  int num_remaining_args () const;

private:

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

#endif /* __CMDLINEPARSER_H__ */

// arch-tag: 5852e7d6-c333-492e-8c89-0f6ef8f09056
