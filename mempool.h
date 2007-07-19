// mempool.h -- Quick but stupid memory allocator
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <new>


namespace snogray {


// A mempool is a quick-but-stupid "allocate-only" memory allocator.
// Allocating from it is _extremely_ fast, but memory can only be freed
// in "bulk".
//
class Mempool
{
public:

  // The default maximum allocation size we support.
  //
  static const size_t DEFAULT_MAX_SIZE = 4096;

  Mempool (size_t max_size = DEFAULT_MAX_SIZE)
    : block_size (max_size), beg (0), end (0), blocks (0), avail (0)
  { }
  ~Mempool () { clear (); }

  // Allocate a block of memory from this pool.
  //
  // If alignment of the returned memory is relevant, it is up to the
  // user to make sure he only requests sizes which are multiples of the
  // desired minimum alignment.  Memory in the pool initially starts out
  // with typical system allocator alignment guarantees, but if the user
  // request a block with a size that is not a multiple of that
  // alignment, that may affect the alignment of subsequent blocks.
  //
  void *get (size_t size)
  {
    if (beg + size > end)
      refill (size);

    void *block = beg;

    beg += size;

    return block;
  }

  // Return all memory allocate from this pool to the pool.  This is the
  // only way to reclaim memory allocated with Mempool::get.
  //
  void reset ()
  {
    avail = blocks;
    if (avail)
      pop_avail ();
  }

  // Return all allocated and available memory to the system.
  //
  void clear ();

private:

  // A chunk of memory allocated from the OS.
  //
  struct Block
  {
    Block (char *_mem, Block *_next) : mem (_mem), next (_next) { }
    char *mem;
    Block *next;
  };

  // Make sure that BEG and END are set up to allocate at least SIZE
  // bytes of memory.  If SIZE is greater than BLOCK_SIZE, an error may
  // be signalled.
  //
  void refill (size_t size);

  // Pop the next block off the AVAIL list and make its contents
  // available for allocation.
  //
  void pop_avail ()
  {
    beg = avail->mem;
    end = beg + block_size;

    avail = avail->next;
  }

  // The size of each Block.  Nothing longer than this can be allocated.
  //
  size_t block_size;

  // The beginning and end of the current region of memory available for
  // allocation.  There are (END - BEG) bytes available for allocation.
  //
  char *beg, *end;

  // The actual (large) chunks of memory we allocated from the OS.
  //
  Block *blocks;

  // Blocks which are still available for allocation.  AVAIL is a tail
  // of the BLOCKS list.
  //
  Block *avail;
};


}


// The user can use this via placement new: "new (MEMPOOL) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly:  "OBJ->~T()".
//
inline void *operator new (size_t size, snogray::Mempool &pool)
{
  return pool.get (size);
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
inline void operator delete (void *, snogray::Mempool &)
{
  // Nothing can be done
}


#endif // __MEMPOOL_H__


// arch-tag: ce37e99d-810f-498b-ad0b-d98ccb5cdf2f
