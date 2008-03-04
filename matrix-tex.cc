// matrix-tex.h -- 2d texture based on discrete matrix of values
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "matrix-tex.h"


using namespace snogray;


// If the compiler supports "extern template" syntax, we can define some
// commonly used instantiations out-of-line here, which saves a lot of
// space.
//
// These instantiations should be synchronized with the "extern template class"
// declarations at the end of "matrix-tex.tcc".
// 
#if HAVE_EXTERN_TEMPLATE
template class MatrixTex<Color>;
template class MatrixTex<float>;
#endif


// arch-tag: 96027c82-fed0-4df9-8fc7-079cef7114bb
