// space.swg -- SWIG interfaces for space-acceleration structures
//
//  Copyright (C) 2011, 2013, 2017  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


%{
#include "space/octree.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class SpaceBuilderFactory { };


  // A wrapper for Octree::BuilderFactory (SWIG can't handle nested
  // classes).
  //
  class OctreeBuilderFactory : public SpaceBuilderFactory
  {
  public:
    OctreeBuilderFactory ();
  };
  %{
  namespace snogray {
    class OctreeBuilderFactory : public Octree::BuilderFactory
    {
    public:
      OctreeBuilderFactory () { }
    };
  }
  %}


}
