// cmdlineparser.h -- Command-line parser
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CMDLINEPARSER_H__
#define __CMDLINEPARSER_H__

#include <string>

#include <getopt.h>

// The following macros can be used in defining option parsers.
//
#define CMDLINEPARSER_GENERAL_OPTIONS_HELP "\
      --help                 Output this help message\n\
      --version              Output program version"
//
#define CMDLINEPARSER_GENERAL_SHORT_OPTIONS	""
//
#define CMDLINEPARSER_OPT_HELP		1
#define CMDLINEPARSER_OPT_VERSION	2
#define CMDLINEPARSER_GENERAL_LONG_OPTIONS				\
  { "help",	no_argument,	   0, CMDLINEPARSER_OPT_HELP },		\
  { "version",	no_argument,	   0, CMDLINEPARSER_OPT_VERSION }
//
#define CMDLINEPARSER_GENERAL_OPTION_CASES(clp)			\
  case CMDLINEPARSER_OPT_HELP:					\
    help (clp, cout);						\
    exit (0);							\
  case CMDLINEPARSER_OPT_VERSION:				\
    cout << clp.prog_name() << " (snogray) 1.0" << endl;	\
    exit (0);

namespace Snogray {

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
  unsigned unsigned_opt_arg () const;
  void opt_err (const char *phrase) const __attribute__ ((noreturn));

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

}

#endif /* __CMDLINEPARSER_H__ */

// arch-tag: 5852e7d6-c333-492e-8c89-0f6ef8f09056
