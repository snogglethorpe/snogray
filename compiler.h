// compiler.h -- compiler-specific definitions
//
//  Copyright (C) 2008, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_COMPILER_H
#define SNOGRAY_COMPILER_H

// unlikely(EXPR) returns EXPR (which is usually a boolean expression),
// and as a side-effect tries to tell the compiler that the expected
// value is zero (false).
//
#ifdef __GNUC__
# define unlikely(expr) __builtin_expect (expr, 0)
#else
# define unlikely(expr) (expr)
#endif

// likely(EXPR) returns EXPR converted to a boolean value, and as a
// side-effect tries to tell the compiler that the expected value is true.
//
#ifdef __GNUC__
# define likely(expr) __builtin_expect (!!(expr), true)
#else
# define likely(expr) (!!(expr))
#endif

#endif // SNOGRAY_COMPILER_H
