// progress.h -- Progress indicator
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iomanip>
#include <cmath>

#include "progress.h"

using namespace Snogray;
using namespace std;

void
Progress::start ()
{
  if (verbosity == MINIMAL)
    {
      os << "rendering...";	// if no progress indicator, print _something_
      os.flush ();
    }

  last_pos = start_pos;

  start_time = last_update_time = Timeval (Timeval::TIME_OF_DAY);

  update_pos = start_pos + 1;

  pos_width = unsigned (log10 (end_pos)) + 1;
}

void
Progress::update (unsigned pos)
{
  if (verbosity > MINIMAL && pos >= update_pos)
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
	  float cur_lps = (pos - last_pos) / last_interval;
	  float overall_lps = (pos - start_pos) / elapsed;
	  float est_lps = (cur_lps + overall_lps) / 2;
	  
	  Timeval remaining_est = (end_pos - pos) / est_lps;

	  // Output progress
	  //
	  os << "\rrendering: " << unit_name << " " << setw (pos_width) << pos
	     << "  (" << setw (3) << unsigned (progress * 100) << "%, "
	     << setw (7) << (now - start_time).fmt(0) << " elapsed, "
	     << setw (7) << remaining_est.fmt(0) << " remaining) ...";

	  // Estimate which pos we will have reached after the desired
	  // update interval, and make that our next update pos.
	  //
	  update_pos = pos + unsigned (cur_lps * update_interval);

	  // Always wait until the next line
	  //
	  if (update_pos == pos)
	    update_pos++;

	  last_pos = pos;
	  last_update_time = now;
	}
      else
	{
	  // Output progress
	  //
	  os << "\rrendering: " << unit_name << " " << setw (pos_width) << pos
	     << "  (" << setw (3) << unsigned (progress * 100) << "%) ...";

	  update_pos = pos + 1;
	}

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
	os << "\rrendering: done"
	   << string (unit_name.length() - 4 + pos_width + 49, ' ')
	   << endl;
    }
}

// arch-tag: 47323aa4-dbdc-42d9-ac12-09af4130ed3c
