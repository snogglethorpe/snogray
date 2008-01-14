// mempool.h -- Quick but stupid memory allocator
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
  static const size_t DEFAULT_MAX_SIZE = 16384;

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

  // Make sure that BEG and END are set up to allocate at least SIZE
  // bytes of memory.  If SIZE is greater than BLOCK_SIZE, an error may
  // be signalled.
  //
  void refill (size_t size);

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


extern void anon_mempool_alloc_barf ();


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
  MempoolAlloc ()
  {
    anon_mempool_alloc_barf ();
  }

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


#endif // __MEMPOOL_H__


// arch-tag: ce37e99d-810f-498b-ad0b-d98ccb5cdf2f
