// media.h -- Surrounding media
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MEDIA_H__
#define __MEDIA_H__


namespace snogray {

class Medium;
class Intersect;


// A node in a linked-list of surrounding media.
//
// Each time rendering enters a refractive object (i.e., via the
// front-face of a refractive surface), a Media node is added to the
// front of a linked list.  Each time rendering exits a refractive
// object (i.e., via the back-face of a refractive surface), a node is
// removed from the front.
//
// Because surfaces only directly refer to their _enclosed_ medium,
// having a history of surrounding media like this makes it possible to
// determine the correct Medium when a rendering path exits a refractive
// surface.
//
class Media
{
public:

  Media (const Medium &_medium, const Media *_surrounding_media = 0)
    : medium (_medium), surrounding_media (_surrounding_media)
  { }

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
  Media (const Intersect &isec, bool refraction);

  // Since in many contexts we actually have a reference to a Media
  // object, this is just a convenience constructor that adds the
  // appropriate address-of constructor.
  //
  Media (const Medium &_medium, const Media &_surrounding_media)
    : medium (_medium), surrounding_media (&_surrounding_media)
  { }

  // Return a reference to the enclosing medium, or DEFAULT_MEDIUM is
  // there is none recorded.
  //
  const Medium &enclosing_medium (const Medium &default_medium) const
  {
    return surrounding_media ? surrounding_media->medium : default_medium;
  }

  // Given the top-of-stack pointer INNERMOST for a media stack, update
  // it to reflect the results of entering or exiting a transmissive
  // surface at ISEC.
  //
  static void update_stack_for_transmission (const Media *&innermost,
					     const Intersect &isec);

  // The medium at this level.
  //
  const Medium &medium;

  // Surrounding layers of media.
  //
  const Media *surrounding_media;

private:

  // Return an appropriate medium for a refractive ray entering or leaving
  // a surface from ISEC.
  //
  static const Medium &refraction_medium (const Intersect &isec);
};

}


#endif /* __MEDIA_H__ */
