// isec_cache.h -- Simple mailboxing cache for intersection testing
//
//  Copyright (C) 2007, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_ISEC_CACHE_H
#define SNOGRAY_ISEC_CACHE_H

namespace snogray {


class Surface;


class IsecCache
{
public:

  IsecCache ()
    : gen (1)
  {
    for (unsigned i = 0; i < TABLE_SIZE; i++)
      mboxes[i].gen = 0;
  }

  // Mark all entries as out-of-date.  This is very fast.
  //
  void clear () { gen++; }

  // Return true if there's an up-to-date entry for SURF in the cache.
  //
  bool contains (const Surface *surf) const
  {
    const Mbox &mbox = lookup (surf);
    return mbox.gen == gen && mbox.surf == surf;
  }

  // Add an up-to-date entry for SURF.  Return true if a collision occurred
  // (the new entry removed an old one).
  //
  bool add (const Surface *surf)
  {
    Mbox &mbox = lookup (surf);
    bool collision = (mbox.gen == gen);
    mbox.surf = surf;
    mbox.gen = gen;
    return collision;
  }

  // Pool object protocol methods.
  //
  void acquire () { clear (); }
  void release () { }

private:

  typedef unsigned hash_t;
  typedef unsigned gen_t;

  static const unsigned TABLE_SIZE = 1024;

  struct Mbox
  {
    gen_t gen;
    const Surface *surf;
  };

  hash_t hash (const Surface *surf) const
  {
    return ((unsigned long)surf) >> 3;
  }

  Mbox &lookup (const Surface *surf)
  {
    return mboxes[hash (surf) % TABLE_SIZE];
  }
  const Mbox &lookup (const Surface *surf) const
  {
    return mboxes[hash (surf) % TABLE_SIZE];
  }

  gen_t gen;

  Mbox mboxes[TABLE_SIZE];
};


}


#endif // SNOGRAY_ISEC_CACHE_H

// arch-tag: 6342f4d6-028b-40f8-a7ff-836a7e1bdbe9
