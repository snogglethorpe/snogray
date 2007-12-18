// freelist.h -- Object freelists
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

#include "freelist.h"

static const size_t MAX_ALLOC_SIZE = 16*1024*1024;

using namespace snogray;

void
BlockFreelist::refill ()
{
  size_t chunk_alloc_size = block_size * chunk_length;

  if (chunk_alloc_size * 2 + sizeof (Alloc) <= MAX_ALLOC_SIZE)
    {
      chunk_alloc_size *= 2;
      chunk_length *= 2;
    }

  size_t base_alloc_size = chunk_alloc_size;

  size_t base_alloc_size_over = base_alloc_size % sizeof (Alloc);
  if (base_alloc_size_over != 0)
    base_alloc_size += sizeof (Alloc) - base_alloc_size_over;

  size_t alloc_size = base_alloc_size + sizeof (Alloc);
  char *mem = new char[alloc_size];

  void *alloc_link = static_cast<void *>(mem + base_alloc_size);
  allocs = new (alloc_link) Alloc (mem, allocs);

  char *end = mem + chunk_alloc_size;
  for (char *p = mem; p < end; p += block_size)
    put (static_cast<void *>(p));
}

// This completely deallocates everything ever allocated from this
// freelist (including blocks not yet returned to it with put!).
//
void
BlockFreelist::clear ()
{
  // Free all memory ever allocated.
  //
  while (allocs)
    {
      char *mem = allocs->mem;
      allocs = allocs->next;
      delete[] (mem);
    }

  // Stuff in `blocks' is now invalid.
  //
  blocks = 0;
}

// arch-tag: 82cb07cf-a7a5-4e1e-a241-5d84ffee6fab
