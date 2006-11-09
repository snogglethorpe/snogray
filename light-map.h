// light-map.h -- An image-based map of light for LmapAnalyzer to analyze
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_MAP_H__
#define __LIGHT_MAP_H__


#include "image.h"
#include "uv.h"


namespace Snogray {


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

  Ref<Image> map;

  float width, height;
};


}


#endif // __LIGHT_MAP_H__

// arch-tag: 7cccd114-cb70-43a0-91e1-7b451d43a899
