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

#include <getopt.h>

namespace Snogray {

class CmdLineParser
{
public:
  CmdLineParser (int _argc, char *const *_argv,
		 const char *_short_opts, const struct option *_long_opts)
    : argc (_argc), argv (_argv),
      short_opts (_short_opts), long_opts (_long_opts)
  { }

  int get_opt ();

  float float_opt_arg () const;
  unsigned unsigned_opt_arg () const;
  void opt_err (const char *phrase) const;

  const char *get_arg ();
  int num_remaining_args () const;

private:

  const char *short_opts;
  const struct option *long_opts;

  int argc;
  char *const *argv;

  // These are set after calling getopt
  int long_opt_index;
  int short_opt;
};

}

#endif /* __CMDLINEPARSER_H__ */

// arch-tag: 5852e7d6-c333-492e-8c89-0f6ef8f09056
