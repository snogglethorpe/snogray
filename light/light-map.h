// light-map.h -- An image-based map of light for LmapAnalyzer to analyze
//
//  Copyright (C) 2006, 2007, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LIGHT_MAP_H
#define SNOGRAY_LIGHT_MAP_H


#include "image/image.h"
#include "geometry/uv.h"


namespace snogray {


class LightMap
{
public:
  
  LightMap (Ref<Image> _map)
    : map (_map), width (map->width), height (map->height)
  { }
  virtual ~LightMap () { }

  virtual bool too_small (float, float, float w, float h) const
  {
    return w <= 2 || h <= 2;
  }

  virtual bool too_big (float, float, float w, float h) const
  {
    return w > width / 4 || h > height / 4;
  }

  virtual float aspect_ratio (float, float, float w, float h) const
  {
    return w / h;
  }

  virtual float area (float, float, float w, float h) const
  {
    return w * h;
  }

  Ref<Image> map;

  float width, height;
};


}


#endif // SNOGRAY_LIGHT_MAP_H

// arch-tag: 7cccd114-cb70-43a0-91e1-7b451d43a899
