# geometry.swg -- SWIG interfaces for snogray geometry operators/types
#
#  Copyright (C) 2011-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "geometry/coords.h"
#include "geometry/bbox-io.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class BBox
  {
  public:

    BBox ();
    BBox (const Pos &_min, const Pos &_max);
    BBox (const Pos &_pos);
    BBox (const Pos &_min, dist_t size);

    %rename(add) operator+= (const Pos &pos);
    BBox &operator+= (const Pos &pos);
    %rename(add) operator+= (const BBox &bbox);
    BBox &operator+= (const BBox &bbox);

    Vec extent () const;

    dist_t max_size () const;
    dist_t min_size () const;
    dist_t avg_size () const;
    dist_t median_size () const;    

    Pos min, max;
  };
  %extend BBox
  {
    BBox operator+ (const BBox &bbox) { return *$self + bbox; }
    BBox operator+ (const Pos &pos) { return *$self + pos; }

    const char* __str__() { return static_rep (*$self); }
  }

  class Frame
  {
  public:

    Frame (const Pos &org, const Vec &_x, const Vec &_y, const Vec &_z);
    Frame ();
    Frame (const Pos &org);
    Frame (const Vec &_x, const Vec &_y, const Vec &_z);
    Frame (const Vec &_z);
    Frame (const Pos &org, const Vec &_z);
    Frame (const Matrix4<dist_t> &M);

    Vec to (const Vec &vec) const;
    Vec to (const Pos &pos) const;
    Vec from (const Vec &vec) const;
    Pos from (const Pos &pos) const;

    Frame transformed (const Matrix4<dist_t> &xform) const;
    void transform (const Matrix4<dist_t> &xform);

    Pos origin;
    Vec x, y, z;
  };


} // namespace snogray


// The following are object-creation wrappers, which we use in place of
// swig-generated constructors for any classes that we want to manage via
// reference-counting.  This is because swig will do its own memory
// management for object which it constructs, and we want to avoid that.
//
// These wrapper functions simply create the object using new and return a
// reference to it; swig happily uses the reference instead, with almost
// exactly the same functionality as the underlying object pointer.
//
%inline %{
  namespace snogray {


    // This is basically an Xform constructor that takes an array
    // argument.  Using a separate function shouldn't actually be
    // necessary -- we should be able to just use overload the TXform
    // constructor -- but SWIG has bugs in handling overloads in
    // conjunction with table-to-array conversion (it correctly
    // generates code to do the conversion, but the type-checking code
    // for distinguishing the various overload cases checks the wrong
    // type).  Using a separate named function for this case works
    // around the bug.
    //
    static Xform raw_xform (const float els[16])
    {
      return Xform (els);
    }


  }
%}
