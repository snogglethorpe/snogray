// mempool.h -- Object mempools
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"

#include "mempool.h"

using namespace snogray;

// Make sure that BEG and END are set up to allocate at least SIZE
// bytes of memory.  If SIZE is greater than BLOCK_SIZE, an error may
// be signalled.
//
void
Mempool::refill (size_t size)
{
  if (size > block_size)
    throw std::runtime_error ("allocation size too large for mempool");

  // If we have no more blocks remaining, get some from the system.
  //
  if (avail)
    {
      beg = avail->mem;
      end = beg + block_size;
      avail = avail->next;
    }
  else
    {
      blocks = new Block (new char [block_size], blocks);
      beg = blocks->mem;
      end = beg + block_size;
    }
}

// Return all memory allocate from this pool to the pool.  This is the
// only way to reclaim memory allocated with Mempool::get.
//
void
Mempool::reset ()
{
  avail = blocks;
  beg = end = 0;
}

// Return all allocated and available memory to the system.
//
void
Mempool::clear ()
{
  // Free all memory ever allocated.
  //
  while (blocks)
    {
      Block *bl = blocks;
      blocks = bl->next;
      delete[] bl->mem;
      delete bl;
    }

  // Stuff in `blocks' is now invalid.
  //
  blocks = 0;
}


// arch-tag: 0c723483-5280-4860-b702-5a0f233aa9e1
