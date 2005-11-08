// freelist.h -- Memory freelists
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FREELIST_H__
#define __FREELIST_H__

#include <new>

namespace Snogray {

// A list of fixed-size blocks of memory.
//
class BlockFreelist
{
public:

  static const unsigned DEFAULT_CHUNK_LENGTH = 16;

  BlockFreelist (size_t _size, unsigned _chunk_length = DEFAULT_CHUNK_LENGTH)
    : block_size (_size), chunk_length (_chunk_length), blocks (0), allocs (0)
  { }
  ~BlockFreelist () { clear (); }

  // Return a new block of memory from this freelist.
  //
  void *get ()
  {
    if (! blocks)
      refill ();

    void *block = static_cast<void *>(blocks);

    blocks =  blocks->next;

    return block;
  }

  // Put BLOCK, previously allocated using `get', back in this freelist.
  //
  void put (void *block)
  {
    blocks = new (block) Block (blocks);
  }

  // This completely deallocates everything ever allocated from this
  // freelist (including blocks not yet returned to it with put!).
  //
  void clear ();

private:

  // A link in the list of available individual blocks.
  //
  struct Block
  {
    Block (Block *_next) : next (_next) { }
    Block *next;
  };

  // A chunk of memory allocated from the OS.
  //
  struct Alloc
  {
    Alloc (char *_mem, Alloc *_next) : mem (_mem), next (_next) { }
    char *mem;
    Alloc *next;
  };

  void refill ();

  // The size of one "block" -- `get' returns this much memory.
  //
  size_t block_size;

  // How many blocks at a time we allocate when we ask the OS for more memory.
  //
  unsigned chunk_length;

  // Individual blocks ready for allocation via `get'.
  //
  Block *blocks;

  // The actual (large) chunks of memory we allocated from the OS.
  //
  Alloc *allocs;
};

// A freelist holding objects of type T.  The only difference from
// BlockFreelist is that type casting is done for you -- no constructors
// or destructors are ever called by the free list.
//
template<class T>
class Freelist : public BlockFreelist
{
public:

  Freelist () : BlockFreelist (sizeof (T)) { }

  // Return a pointer to a block of memory large enough for an instance of
  // class T, suitable for using with placement new.  Does not call any
  // constructor (even the default one).
  //
  T *get ()
  {
    return static_cast<T *>(BlockFreelist::get ());
  }

  // Put T back in the freelist.  This does not invoke T's destructor,
  // that should have already been done by the caller!
  //
  void put (T *t)
  {
    BlockFreelist::put (static_cast<void *> (t));
  }
  
};

}

// The user can use this via placement new: "new (FREELIST) T (...)".
// The resulting object cannot be deleted using delete, but should be
// destructed (if necessary) explicitly -- "OBJ->~T()" -- and then
// returned to the freelist using "FREELIST.put(OBJ)".
//
template<class T>
inline void *operator new (size_t s, Snogray::Freelist<T> &fl)
{
  return static_cast<void *>(fl.get ());
}

// There's no syntax for user to use this, but the compiler may call it
// during exception handling.
//
template<class T>
inline void operator delete (void *m, Snogray::Freelist<T> &fl)
{
  fl.put (static_cast<T *>(m));
}

#endif /* __FREELIST_H__ */

// arch-tag: f5bdeaf5-d075-4221-adc1-3a48826ef92d
