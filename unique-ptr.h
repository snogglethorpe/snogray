// unique-ptr.h -- Portability layer for std::unique_ptr/auto_ptr
//
//  Copyright (C) 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// UniquePtr is a simple thin wrapper for std::unique_ptr or std::auto_ptr,
// to allow either to be used.
//
// It does not implement the full functionality of std::unique_ptr,
// just enough to support uses in snogray.
//


#ifndef __UNIQUE_PTR_H__
#define __UNIQUE_PTR_H__

#include <memory>

#include "config.h"


namespace snogray {

#if HAVE_STD_UNIQUE_PTR

// Wrapper around std::unique_ptr.
//
template<typename T>
class UniquePtr : public std::unique_ptr<T>
{
public:

  UniquePtr () { }
  explicit UniquePtr (T *v) : std::unique_ptr<T> (v) { }
  UniquePtr (UniquePtr &&v) : std::unique_ptr<T> (std::move (v)) { }

  UniquePtr &operator= (UniquePtr &&v)
  {
    std::unique_ptr<T>::operator= (std::move (v));
    return *this;
  }
};

#else // ! HAVE_STD_UNIQUE_PTR

// Wrapper around std::auto_ptr.
//
template<typename T>
class UniquePtr : public std::auto_ptr<T>
{
public:

  UniquePtr () { }
  explicit UniquePtr (T *v) : std::auto_ptr<T> (v) { }
  UniquePtr (UniquePtr &v) : std::auto_ptr<T> (v) { }
  UniquePtr (const UniquePtr &v) : std::auto_ptr<T> (const_cast<UniquePtr &> (v)) { }

  UniquePtr &operator= (UniquePtr &v)
  {
    std::auto_ptr<T>::operator= (v);
    return *this;
  }
};

#endif // ! HAVE_STD_UNIQUE_PTR

}


#endif // __UNIQUE_PTR_H__
