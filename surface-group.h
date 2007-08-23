// surface-group.h -- Group of surfaces
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SURFACE_GROUP_H__
#define __SURFACE_GROUP_H__


#include <list>

#include "surface.h"


namespace snogray {


// A group of surfaces represented as a single composite surface.
//
class SurfaceGroup : public Surface
{
public:

  SurfaceGroup () : Surface (0) { }

  // Add SURFACE to this group.
  //
  void add (Surface *surface)
  {
    surfaces.push_back (surface);
    _bbox += surface->bbox ();
  }

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const { return _bbox; }

  // Add this (or some other ...) surfaces to SPACE
  //
  virtual void add_to_space (Space *space);

private:

  // A list of the surfaces in this group.
  //
  std::list<Surface *> surfaces;

  // Cached bounding box for the entire group.
  //
  BBox _bbox;
};


}


#endif /* __SURFACE_GROUP_H__ */
