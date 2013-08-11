# snograw.swg -- Swig interface specification for snogray
#
#  Copyright (C) 2007-2008, 2010-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

//
// Swig's understanding of C++ is imperfect, so in various cases we
// lie to it, giving different source code for the swig input than is
// actually compiled elsewhere:
//
//   (1) For function arguments with a type of "const std::string &",
// 	 we use "const char *" in the declarations below (and rely on
// 	 the automatic C++ conversion to translate), because swig is
// 	 more functional with char pointers (e.g. it automatically
// 	 converts to/from the client language's string type).
//
//   (2) We generally omit "const" from types used in Ref's, where
//       const is used in the real code; e.g., if the real code says
//       "Ref<const T>", we tell swig it's "Ref<T>" instead.  The
//       reason is that whereas the compiler understands that a value
//       with type "Ref<T>" can be assigned to or used to initialize a
//       variable of type "Ref<const T>" (there are appropriate
//       overloads in the Ref definition to allow this), swig doesn't
//       understand this, and generally mucks up type-checking as a
//       result.  To avoid these problems we basically just always use
//       Ref<T> and let the compiler deal with making everything right
//       (which it does).
//

# This is the "raw" interface to snogray
#
%module snograw


// Tell swig to only make consturctor when we specifically ask for them.
//
%nodefaultctor;


%include "config.h"


%{
#include <sstream>
#include <cstring>

  static char static_rep_buf[255];

  template<typename T>
  const char *
  static_rep(const T &v)
  {
    std::ostringstream s;
    s << v;
    strcpy (static_rep_buf, s.str().c_str());
    return static_rep_buf;
  }

%}


%include "geometry/coords.h"

// For handling fixed-length array arguments
//
%include <typemaps.i>
%apply (float INPUT[ANY]) {(const float els[16])};
%apply (float INPUT[ANY]) {(const dist_t els[16])};


%include "vector.swg"
SWIG_TYPEMAP_NUM_VECTOR(float)
SWIG_TYPEMAP_NUM_VECTOR(unsigned)


%include "val-table.swg"


%{
#include "ref.h"
%}


// Some common stuff that's used by all included .swg files.  None of
// this should actually generate any interface code, it just informs
// SWIG about things.
//
namespace snogray {


  // Ref is our intrusive reference-counting smart-pointer class.
  //
  // We expose only enough of it to SWIG so that it knows it's a
  // pointer.  SWIG will then export operations on Ref<T> mirroring
  // those on T.
  //
  template<class T>
  class Ref
  {
  public:

    T &operator* ();
    T *operator-> ();
  };


  // Lots of places use ValTable::NONE, so tell SWIG about it.
  // But we don't want to actually export ValTable itself, instead
  // doing conversion between Lua tables and ValTables for those
  // interfaces which require/return a ValTable
  //
  %ignore ValTable;
  class ValTable
  {
  public:
    static ValTable NONE;
  };


#if HAVE_SWIG_DISOWN
  // The only time we pass Surface or Light pointers is when handing
  // over control of such objects to C++, in which case we want to
  // avoid any GC by Lua.
  //
  %apply SWIGTYPE* DISOWN {Surface *}
  %apply SWIGTYPE* DISOWN {Light *}
#endif


  typedef float intens_t;
}


// These includes export various subsets of snogray functionality.
//
%include "common.swg"
%include "image/image.swg"
%include "surface/surface.swg"
%include "material/material.swg"
%include "light/light.swg"
%include "scene.swg"
%include "texture/texture.swg"
%include "camera.swg"
%include "rdrive.swg"
%include "render.swg"
%include "space/space.swg"
%include "color/color.swg"
%include "load.swg"
%include "util.swg"


# arch-tag: ced00636-fe46-45bd-b738-1949800e26f1
