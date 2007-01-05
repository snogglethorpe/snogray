// matrix-tex2.h -- 2d texture based on discrete matrix of values
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "matrix-tex2.h"


using namespace snogray;


// If the compiler supports "extern template" syntax, we can define some
// commonly used instantiations out-of-line here, which saves a lot of
// space.
//
// These instantiations should be synchronized with the "extern template class"
// declarations at the end of "matrix-tex2.tcc".
// 
#if HAVE_EXTERN_TEMPLATE
template class MatrixTex2<Color>;
#endif


// arch-tag: 96027c82-fed0-4df9-8fc7-079cef7114bb
