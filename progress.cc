// progress.h -- Progress indicator
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iomanip>

#include "snogmath.h"

#include "progress.h"

using namespace snogray;
using namespace std;

void
Progress::start ()
{
  if (verbosity != QUIET)
    {
      os << prefix;	// if no progress indicator, print _something_
      os.flush ();
    }

  last_pos = start_pos;

  start_time = last_update_time = Timeval (Timeval::TIME_OF_DAY);

  update_pos = start_pos + 1;
}

void
Progress::update (unsigned pos)
{
  if (verbosity > MINIMAL
      && (pos >= update_pos
	  || (ticks_until_forced_update > 0
	      && --ticks_until_forced_update == 0)))
    {
      Timeval now (Timeval::TIME_OF_DAY);

      // How far we've gone
      //
      float progress = float (pos - start_pos) / (end_pos - start_pos);

      float elapsed = now - start_time;

      // We don't start trying to measure our speed until some time has
      // elapsed.
      //
      if (elapsed > startup_interval)
	{
	  float last_interval = now - last_update_time;

	  // How fast we seem to be progressing (poss per second)
	  //
	  float overall_lps = (pos - start_pos) / elapsed;
	  float cur_lps = (pos - last_pos) / last_interval;
	  
	  Timeval remaining_est = (end_pos - pos) / overall_lps;

	  // Output progress
	  //
	  os << "\r" << prefix;

	  os << setw (5) << fixed << setprecision (1) << (progress * 100) << "%"
	     << "  ("
	     << setw (5) << (now - start_time) << " elapsed, "
	     << setw (5) << remaining_est << " rem"
	     << ")";

	  // Estimate which pos we will have reached after the desired
	  // update interval, and make that our next update pos.
	  //
	  update_pos = pos + unsigned (cur_lps * update_interval);

	  unsigned update_limit = unsigned (pos + (end_pos - start_pos) * 0.02);

	  // Always wait until the next line, but never too long.
	  //
	  if (update_pos == pos)
	    update_pos++;
	  else if (update_pos > update_limit)
	    update_pos = update_limit;

	  last_pos = pos;
	  last_update_time = now;
	}
      else
	{
	  // Output progress
	  //
	  os << "\r" << prefix
	     << setw (5) << fixed << setprecision (1) << (progress * 100)
	     << "%";

	  update_pos = unsigned (pos + (end_pos - start_pos) * 0.001);
	}

      // This is a kludge to handle long periods where POS doesn't change.
      //
      ticks_until_forced_update = 200000;

      os.flush ();
    }
}

void
Progress::end ()
{
  if (verbosity != QUIET)
    {
      if (verbosity == MINIMAL)
	os << "done" << endl;
      else
	os << "\r" << string (prefix.length() + 40, ' ')
	   << "\r" << prefix << "done" << endl;
    }
}

// arch-tag: 47323aa4-dbdc-42d9-ac12-09af4130ed3c
