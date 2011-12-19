# util.swg -- SWIG interfaces for miscellaneous utility functions/data
#
#  Copyright (C) 2011  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include <iostream>
%}


// SWIG-exported interfaces to stuff in the std namespace.
//
namespace std {

  // Tell SWIG std::cout and std::cerr are const so that it doesn't try
  // to provide setters for them.
  //
  class iostream;
  extern const iostream cout, cerr;

}
