// illum-mgr.h -- Sample-based manager for illuminators
//
//  Copyright (C) 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ILLUM_MGR_H__
#define __ILLUM_MGR_H__

#include <list>

#include "ref.h"

#include "illum-sample.h"
#include "illum.h"


namespace snogray {

class Intersect;
class Trace;


class IllumMgr
{
public:

  IllumMgr () : any_uses (0) { }
  virtual ~IllumMgr () { }

  // Calculate the color incoming from the previously calculated
  // intersection described by ISEC_INFO.  "li" means "light incoming".
  //
  Color li (const Surface::IsecInfo *isec_info, Trace &trace) const;

  // Return the color emitted from the ray-surface intersection ISEC.
  // "lo" means "light outgoing".
  //
  Color lo (const Intersect &isec) const;

  // Add the illuminator ILLUM to the list of illuminators.  It will be
  // applied only to BRDF samples who have some flags in MASK which are
  // non-zero after being, xored with INVERT.  As a special case, if MASK
  // is zero, the illuminator will always be applied.
  //
  void add_illum (Illum *illum, unsigned mask, unsigned invert = 0);

private:

  struct IllumHandler
  {
    IllumHandler (Illum *_illum, unsigned mask, unsigned invert)
      : sample_flags_mask (mask), sample_flags_invert (invert),
	illum (_illum)
    { }

    // Implement "move semantics" for the copy constructor (which shouldn't
    // be used except when placing this object into an STL list).
    //
    IllumHandler (const IllumHandler &from)
      : sample_flags_mask (from.sample_flags_mask),
	sample_flags_invert (from.sample_flags_invert),
	illum (from.illum)
    {
      // Steal FROM's illum pointer.
      //
      const_cast<IllumHandler &> (from).illum = 0;
    }

    ~IllumHandler ();

    // Partition samples in the between BEG and END, so that all "matching"
    // samples come before all "non-matching" samples.  The partition point
    // (first non-matching sample) is returned.
    //
    IllumSampleVec::iterator
    partition_samples (IllumSampleVec::iterator beg,
		       IllumSampleVec::iterator end)
      const;

    // Flags used to determine which samples are passed to this handler.
    // This handler will be used if (FLAGS & MASK) ^ INVERT is non-zero.
    //
    unsigned sample_flags_mask;
    unsigned sample_flags_invert;

    // Actual illumination handler.
    //
    Illum *illum;
  };

  // A list of illuminators.
  //
  std::list<IllumHandler> illum_handlers;

  // The union of all USES fields for Illum objects in illum_handlers.
  //
  unsigned any_uses;
};


}

#endif /* __ILLUM_MGR_H__ */


// arch-tag: 8f0e4b8d-3a62-497d-8453-6dda9717342c
