// funptr-cast.h -- Cast a function pointer to void*
//
//  Copyright (C) 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_FUNPTR_CAST_H
#define SNOGRAY_FUNPTR_CAST_H

#include "config.h"


namespace snogray {


// Return the function-pointer FUN_PTR cast to void*.
//
// Technically it's invalid, at least in the older C/C++ standards, to
// cast a function pointer to any data pointer, even void*, but some
// libraries require function pointers to be passed as void* anyway;
// in practice it works on most common architectures.  C++11 does
// allow this using reinterpret_cast.
//
// This function makes a best effort to do such a cast in a way that
// avoids compiler warnings/errors; however for architectures where
// this operation is truly invalid, even this should fail to compile.
//
template<typename FunPtrType>
static void *
cast_fun_ptr_to_void_ptr (FunPtrType fun_ptr)
{
  // For C++11, try to use reinterpret_cast; in other cases, use some
  // gross code to avoid compiler warnings/errors.
  //
#if REINTERP_CAST_FUN_PTR_TO_DATA_PTR_OK
  return reinterpret_cast<void *> (fun_ptr);
#else
  return *(void**)(void *)&(FunPtrType &)fun_ptr;
#endif
}  


}

#endif // SNOGRAY_FUNPTR_CAST_H
