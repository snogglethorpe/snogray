# render.swg -- SWIG interfaces for snogray rendering types
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
#include "ray.h"
#include "ray-io.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  template<typename T>
  class TRay
  {
  public:

    TRay (TPos<T> _origin, TVec<T> _extent);
    TRay (TPos<T> _origin, TVec<T> _dir, T _t1);
    TRay (TPos<T> _origin, TVec<T> _dir, T _t0, T _t1);
    TRay (TPos<T> _origin, TPos<T> _targ);

    TPos<T> operator() (T t);
    TPos<T> begin () const;
    TPos<T> end () const;

    T length () const { return t1 - t0; }

    TPos<T> origin;
    TVec<T> dir;

    T t0, t1;
  };
  %extend TRay<dist_t>
  {
    const char* __str__() { return static_rep (*$self); }
  }
  %template(Ray) TRay<dist_t>;
  typedef TRay<dist_t> Ray;


} // namespace snogray
