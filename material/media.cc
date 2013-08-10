// media.cc -- Tracing path
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "material.h"
#include "intersect.h"

#include "media.h"

using namespace snogray;


// This is a convenient constructor for making a new media layer for a
// recursive trace originating from ISEC.  REFRACTION indicates
// whether the new trace goes through the surface (true) or is
// reflected from it (false).
//
// If REFRACTION is false, then ISEC's current media is just copied
// into the new media object.
//
// If REFRACTION is true, if the new trace is:
//
//   * Entering the surface:  The new media object will be a new media
//     layer, with the medium returned by ISEC's material's
//     Surface::medium method, and referring to ISEC's media as its
//     surrounding media.  I.e., a new layer is pushed on the media stack.
//
//   * Exiting the surface:  The new media object will be a copy of
//     ISEC's media's surrounding media object; i.e., a layer is
//     popped off the media stack.
//
Media::Media (const Intersect &isec, bool refraction)
  : medium (refraction ? refraction_medium (isec) : isec.media.medium),
    surrounding_media (refraction
		       ? (isec.back
			  ? isec.media.surrounding_media // pop
			  : &isec.media)		 // push
		       : isec.media.surrounding_media)	 // copy
{ }

// Return an appropriate medium for a refractive ray entering or leaving
// a surface from ISEC.
//
const Medium &
Media::refraction_medium (const Intersect &isec)
{
  const Medium &default_medium = isec.context.default_medium;

  if (isec.back)
    return isec.media.enclosing_medium (default_medium);
  else
    {
      const Medium *medium = isec.material.medium ();
      if (medium)
	return *medium;
      else
	return default_medium;
    }
}



// Given the top-of-stack pointer INNERMOST for a media stack, update
// it to reflect the results of entering or exiting a transmissive
// surface at ISEC.
//
void
Media::update_stack_for_transmission (const Media *&innermost,
				      const Intersect &isec)
{
  // Get the medium of the surface.  A transmissive surface
  // without a medium has no effect on the media stack (so it
  // acts like a thin shell, rather than a volume).
  //
  const Medium *medium = isec.material.medium ();

  if (medium)
    {
      if (isec.back)
	{
	  // Exiting refractive object, pop the innermost medium.
	  //
	  // We avoid popping the last element, as other places
	  // assume there's at least one present (ideally, this
	  // would never happen, because enter/exit events
	  // should be matched, but malformed scenes or
	  // degenerate conditions can cause it to happen
	  // sometimes).
	  //
	  // We do not need to deallocate popped Media objects,
	  // as they are allocated using the Mempool allocator
	  // in CONTEXT (everything allocated there is later
	  // bulk-freed in the main rendering loop).
	  //

	  if (innermost->surrounding_media)
	    innermost = innermost->surrounding_media;
	}
      else
	{
	  // Entering refractive object, push the new medium.
	  //
	  // Allocate a new Media object using CONTEXT's Mempool
	  // allocator, and make it the new top of the media
	  // stack.
	  //
	  innermost = new (isec) Media (*medium, innermost);
	}
    }
}
