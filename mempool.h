// mempool.h -- Quick but stupid memory allocator
//
//  Copyright (C) 2007, 2008, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_MEMPOOL_H
#define SNOGRAY_MEMPOOL_H

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
  static const size_t DEFAULT_BLOCK_SIZE = 16384;
  static const size_t DEFAULT_LARGE_SIZE = 8192;

  Mempool (size_t _block_size = DEFAULT_BLOCK_SIZE,
	   size_t _large_size = DEFAULT_LARGE_SIZE)
    : beg (0), end (0), blocks (0), avail (0), large_blocks (0),
      block_size (_block_size), large_size (_large_size)
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
      return _get (size);
    else
      {
	void *block = beg;
	beg += size;
	return block;
      }
  }

  // Return all memory allocate from this pool to the pool.  This is the
  // only way to reclaim memory allocated with Mempool::get.
  //
  void reset ();

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


  // Allocate a block of memory from this pool.  Unlike the Mempool::get
  // method, this method knows how to allocate large blocks or refill the
  // small-allocation arena.  If this is a small allocation it is assumed
  // that there is no more room in the small-allocation arena, so it is
  // refilled with a new block; this is because the easy case of allocating
  // small allocations form the arena is handled by Mempool::get.
  //
  void *_get (size_t size);

  // Return all blocks in BLOCK_LIST to the system, and set BLOCK_LIST to
  // zero.
  //
  void free_blocks (Block *&block_list);


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

  // Blocks too large to be allocated using the default mechanism, each
  // dedicated to a single user allocation.  These are returned the system
  // when reseting.
  //
  Block *large_blocks;

  // The size of the blocks used for normal allocations.
  //
  size_t block_size;

  // Allocations this size or larger are allocated individually from the
  // LARGE_BLOCKS list.
  //
  size_t large_size;
};


extern Mempool &anon_mempool_alloc_barf ();


// An STL allocator for allocating from an Intersect object.
//
template<typename T>
class MempoolAlloc
{
public:

  MempoolAlloc (Mempool &_mempool) : mempool (_mempool) { }

  // STL constructors need this, though using it would certainly be
  // incorrect.
  //
  MempoolAlloc () : mempool (anon_mempool_alloc_barf ()) { }

  typedef T		 value_type;
  typedef T		*pointer;
  typedef const T	*const_pointer;
  typedef T		&reference;
  typedef const T	&const_reference;

  template<typename T2>
  struct rebind
  {
    typedef MempoolAlloc<T2> other;
  };

  T *allocate (size_t n)
  {
    return static_cast<T *> (mempool.get (n * sizeof (T)));
  }

  void deallocate (void *, size_t)
  {
    // Nothing -- mempools cannot free data
  }

  void construct (T *obj, const T &from)
  {
    new (static_cast<void *> (obj)) T (from);
  }

  void destroy (T *obj)
  {
    obj->~T ();
  }

  size_t max_size() const
  {
    return size_t (-1) / sizeof (T);
  }

  Mempool &mempool;
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


#endif // SNOGRAY_MEMPOOL_H


// arch-tag: ce37e99d-810f-498b-ad0b-d98ccb5cdf2f
