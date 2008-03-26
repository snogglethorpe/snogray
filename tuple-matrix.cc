// tuple-matrix.cc -- Generic matrix storage type
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

#include "config.h"

#include "color.h"

#include "tuple-matrix.h"


using namespace snogray;


// If the compiler supports "extern template" syntax, we can define some
// commonly used instantiations out-of-line here, which saves a lot of
// space.
//
// These instantiations should be synchronized with the "extern template class"
// declarations at the end of "arith-tex.tcc".
// 
#if HAVE_EXTERN_TEMPLATE
template class TupleMatrixData<float>;
template class TupleMatrix<float>;
template class TupleMatrix<Color>;
#endif


// arch-tag: da22c1bc-101a-4b6e-a7e6-1db2676ea923
