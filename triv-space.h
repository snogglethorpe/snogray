// triv-space.h -- Trivial space search accelerator
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

#ifndef __TRIV_SPACE_H__
#define __TRIV_SPACE_H__

#include <vector>
#include <algorithm>

#include "space.h"


namespace snogray {


// A space search accelerator that just keeps a list of objects and
// always enumerates through the list for every operation.  This might
// be good for very small numbers of objects where the overhead of a
// more sophisticated accelerator is not justified by the gain.
//
class TrivSpace : public Space
{
public:

  // A class used for building a Space object.
  //
  class Builder;

  // Subclass of SpaceBuilderFactory for making TrivSpace builders.
  //
  class BuilderFactory;


  TrivSpace () { }


  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).  MEDIA is used to access
  // various cache data structures.  ISEC_STATS will be updated.
  //
  virtual void for_each_possible_intersector (const Ray &,
					      IntersectCallback &callback,
					      RenderContext &,
					      RenderStats::IsecStats &)
    const
  {
    for (std::vector<const Surface *>::const_iterator i = surfaces.begin();
	 i != surfaces.end(); ++i)
      callback (*i);
  }


private:  

  std::vector<const Surface *> surfaces;
};



// TrivSpace::Builder and TrivSpace::BuilderFactory

// A class used for building a Space object.
//
class TrivSpace::Builder : public SpaceBuilder
{
public:

  Builder () : space (new TrivSpace) { }

  // Add SURFACE to the space being built.
  //
  virtual void add (const Surface *surface)
  {
    space->surfaces.push_back (surface);
  }

  // Make the final space.  Note that this can only be done once.
  //
  virtual const Space *make_space ()
  {
    return space;
  }

private:

  TrivSpace *space;
};

// Subclass of SpaceBuilderFactory for making TrivSpace builders.
//
class TrivSpace::BuilderFactory : public SpaceBuilderFactory
{
public:

  // Return a new SpaceBuilder object.
  //
  virtual SpaceBuilder *make_space_builder () const
  {
    return new TrivSpace::Builder ();
  }
};


}

#endif /* __TRIV_SPACE_H__ */
