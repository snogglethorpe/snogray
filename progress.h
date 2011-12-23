// progress.h -- Progress indicator interface
//
//  Copyright (C) 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PROGRESS_H
#define SNOGRAY_PROGRESS_H


namespace snogray {


// A progress-indicator.
//
class Progress
{
public:

  Progress () { }
  virtual ~Progress () { }

  // Set the start position of the progress range; positions before
  // this are not counted as progress.  This is normally called before
  // Progress::start, but may be called afterwards and should do
  // something reasonable.
  //
  virtual void set_start (int pos) = 0;

  // Set the size of the progress range, following the start position.
  // This is normally called before Progress::start, but may be called
  // afterwards and should do something reasonable.
  //
  virtual void set_size (unsigned size) = 0;

  // Start displaying the progress indicator.
  //
  virtual void start () = 0;

  // Update the progress indicator to position POS.
  //
  virtual void update (int pos) = 0;

  // Finish the progress indicator.
  //
  virtual void end () = 0;
};


}

#endif // SNOGRAY_PROGRESS_H
