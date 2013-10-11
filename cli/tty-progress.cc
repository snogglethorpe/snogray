// tty-progress.h -- Progress indicator for terminals
//
//  Copyright (C) 2006-2007, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <ostream>
#include <iomanip>

#include "util/snogmath.h"

#include "tty-progress.h"


using namespace snogray;


// Set the start position of the progress range; positions before this
// are not counted as progress.  This is normally called before
// Progress::start, but may be called afterwards and should do
// something reasonable.
//
void
TtyProgress::set_start (int new_start)
{
  unsigned size = end_pos - start_pos;

  start_pos = new_start;
  end_pos = new_start + size;

  if (last_pos < start_pos)
    last_pos = start_pos;
  else if (last_pos > end_pos)
    last_pos = end_pos;

  update_pos = last_pos;
}

// Set the size of the progress range, following the start position.
// This is normally called before Progress::start, but may be called
// afterwards and should do something reasonable.
//
void
TtyProgress::set_size (unsigned size)
{
  end_pos = start_pos + size;

  if (last_pos > end_pos)
    last_pos = end_pos;

  update_pos = last_pos;
}

// Start displaying the progress indicator.
//
void
TtyProgress::start ()
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

// Update the progress indicator to position POS.
//
void
TtyProgress::update (int pos)
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
      if (elapsed > startup_interval())
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

	  os << std::setw (5) << std::fixed << std::setprecision (1)
	     << (progress * 100) << "%"
	     << "  ("
	     << std::setw (5) << (now - start_time) << " elapsed, "
	     << std::setw (5) << remaining_est << " rem"
	     << ")";

	  // Estimate which pos we will have reached after the desired
	  // update interval, and make that our next update pos.
	  //
	  update_pos = pos + int (cur_lps * float (update_interval));

	  int update_limit = int (pos + (end_pos - start_pos) * 0.02f);

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
	     << std::setw (5) << std::fixed << std::setprecision (1)
	     << (progress * 100)
	     << "%";

	  update_pos = int (pos + (end_pos - start_pos) * 0.001);
	}

      // This is a kludge to handle long periods where POS doesn't change.
      //
      ticks_until_forced_update = 200000;

      os.flush ();
    }
}

// Finish the progress indicator.
//
void
TtyProgress::end ()
{
  if (verbosity != QUIET)
    {
      if (verbosity == MINIMAL)
	os << "done" << std::endl;
      else
	os << "\r" << std::string (prefix.length() + 40, ' ')
	   << "\r" << prefix << "done" << std::endl;
    }
}


// arch-tag: 47323aa4-dbdc-42d9-ac12-09af4130ed3c
