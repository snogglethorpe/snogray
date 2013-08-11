// mempool.h -- Object mempools
//
//  Copyright (C) 2005, 2007, 2008  Miles Bader <miles@gnu.org>
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


// Allocate a block of memory from this pool.  Unlike the Mempool::get
// method, this method knows how to allocate large blocks or refill the
// small-allocation arena.  If this is a small allocation it is assumed
// that there is no more room in the small-allocation arena, so it is
// refilled with a new block; this is because the easy case of allocating
// small allocations from the arena is handled by Mempool::get.
//
void *
Mempool::_get (size_t size)
{
  if (size > large_size)
    {
      // A very large allocation; allocate an individual block from the system.

      large_blocks = new Block (new char [size], large_blocks);
      return large_blocks->mem;
    }
  else
    {
      // A small allocation.

      // Refill the small-allocation arena.
      //
      if (avail)
	{
	  // Use an existing block.

	  beg = avail->mem;
	  end = beg + block_size;
	  avail = avail->next;
	}
      else
	{
	  // No more blocks remaining, get some from the system.

	  blocks = new Block (new char [block_size], blocks);
	  beg = blocks->mem;
	  end = beg + block_size;
	}

      // Finally, allocate from the small-allocation arena, now guaranteed
      // to be useable.
      //
      void *block = beg;
      beg += size;
      return block;
    }
}



// Return all blocks in BLOCK_LIST to the system, and set BLOCK_LIST to
// zero.
//
void
Mempool::free_blocks (Block *&block_list)
{
  Block *tail = block_list;

  while (tail)
    {
      Block *bl = tail;
      tail = bl->next;
      delete[] bl->mem;
      delete bl;
    }

  block_list = 0;
}

// Return all memory allocate from this pool to the pool.  This is the
// only way to reclaim memory allocated with Mempool::get.
//
void
Mempool::reset ()
{
  avail = blocks;
  beg = end = 0;
  free_blocks (large_blocks);
}

// Return all allocated and available memory to the system.
//
void
Mempool::clear ()
{
  reset ();
  free_blocks (blocks);
}


// arch-tag: 0c723483-5280-4860-b702-5a0f233aa9e1
