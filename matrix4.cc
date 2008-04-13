// matrix4.cc -- 4 x 4 matrices
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include "matrix4.h"


using namespace snogray;


// If the compiler supports "extern template" syntax, we can define some
// commonly used instantiations out-of-line here, which saves a lot of
// space.
//
// These instantiations should be synchronized with the "extern template
// class" declarations at the end of "matrix4.tcc".
// 
#if HAVE_EXTERN_TEMPLATE
template class Matrix4<dist_t>;
#endif
