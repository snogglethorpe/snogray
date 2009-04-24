// cubemap.h -- Texture wrapped around a cube
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CUBEMAP_H__
#define __CUBEMAP_H__

#include "color.h"
#include "tex.h"
#include "envmap.h"
#include "unique-ptr.h"


namespace snogray {


class Cubemap : public Envmap
{
public:

  Cubemap (const std::string &spec)
  {
    load (spec);
  }
  Cubemap (const Ref<Image> &image)
  {
    load (image);
  }

  void load (const std::string &filename);
  void load (std::istream &stream, const std::string &filename_pfx);
  void load (const Ref<Image> &image);

  // Return the color of this environment map in direction DIR.
  //
  virtual Color map (const Vec &dir) const;

  // Return a "light-map" -- a lat-long format spheremap image
  // containing light values of the environment map -- for this
  // environment map.
  //
  virtual Ref<Image> light_map () const;

private:

  Vec parse_axis_dir (const std::string &str);

  struct Face
  {
    UniquePtr<Tex<Color> > tex;

    Vec u_dir, v_dir;
  };

  // The faces in order are:  right, left, top, bottom, front, back
  //
  Face faces[6];
};


}

#endif // __CUBEMAP_H__


// arch-tag: dacaf2e7-e81c-4562-b7db-9917e9d55994
