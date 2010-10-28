// matrix-io.h -- Debugging output for Matrix type
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX_IO_H__
#define __MATRIX_IO_H__

#include <fstream>

#include "nice-io.h"

#include "matrix.h"


namespace snogray {


template<typename T>
std::ostream&
operator<< (std::ostream &os, const Matrix<T> &matrix)
{
  // this syntax isn't the prettiest, but is kind of compatible with
  // emacs calc, Lua, and C

  os << "matrix{{";

  for (unsigned row = 0; row < matrix.rows(); row++)
    {
      if (row > 0)
	os << "}, {";
      for (unsigned col = 0; col < matrix.columns(); col++)
	{
	  if (col > 0)
	    os << ", ";
	  output_nicely (os, matrix (col, row));
	}
    }

  os << "}}";

  return os;
}


}

#endif // __MATRIX_IO_H__
