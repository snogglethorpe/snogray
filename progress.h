// progress.h -- Progress indicator
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PROGRESS_H__
#define __PROGRESS_H__

#include <iostream>
#include <string>

#include "timeval.h"

namespace snogray {

class Progress
{
public:

  enum Verbosity { QUIET, MINIMAL, CHATTY };

  static const float default_update_interval = 10;
  static const float startup_interval = 20;

  // Initialize with the desired bounds.  Note that nothing actually
  // happens until the `start' method is called.
  //
  Progress (std::ostream &stream, const std::string &_unit_name,
	    unsigned _start_pos, unsigned num,
	    Verbosity _verbosity = CHATTY,
	    float _update_interval = default_update_interval)
    : unit_name (_unit_name),
      start_pos (_start_pos), end_pos (_start_pos + num),
      last_pos (0), last_update_time (0), update_pos (0), start_time (0),
      verbosity (_verbosity), os (stream), pos_width (5),
      update_interval (_update_interval)
  { }
      
  void start ();
  void update (unsigned pos);
  void end ();

  // The name of the unit that "pos" refers to.
  //
  std::string unit_name;

  // Overall rendering bounds
  //
  unsigned start_pos, end_pos;

  // When we last updated the progress indicator
  //
  unsigned last_pos;
  Timeval last_update_time;

  // When we will next update it
  //
  unsigned update_pos;

  // When we started
  //
  Timeval start_time;

  // How chatty to be
  //
  Verbosity verbosity;

  // Where to send progress reports to
  //
  std::ostream &os;

  // The number of digits required to output the position.
  //
  unsigned pos_width;

  // How often (approximately) to update (in seconds)
  //
  Timeval update_interval;
};

}

#endif /* __PROGRESS_H__ */

// arch-tag: 54fddbd6-cd1a-4b36-81b6-8260d700dd24
