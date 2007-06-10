// isec_cache.h -- Simple mailboxing cache for intersection testing
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ISEC_CACHE_H__
#define __ISEC_CACHE_H__

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

  // Add an up-to-date entry for SURF.
  //
  void add (const Surface *surf)
  {
    Mbox &mbox = lookup (surf);
    mbox.surf = surf;
    mbox.gen = gen;
  }

private:

  typedef unsigned hash_t;
  typedef unsigned gen_t;

  static const unsigned TABLE_SIZE = 512;

  struct Mbox
  {
    gen_t gen;
    const Surface *surf;
  };

  hash_t hash (const Surface *surf) const
  {
    return ((unsigned long)surf) >> 4;
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


#endif // __ISEC_CACHE_H__

// arch-tag: 6342f4d6-028b-40f8-a7ff-836a7e1bdbe9
