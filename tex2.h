// tex2.h -- 2d texture base class
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEX2_H__
#define __TEX2_H__

namespace snogray {

// Texture param
//
typedef float tparam_t;

template<typename T>
class Tex2
{
public:

  virtual ~Tex2 () {}

  virtual T map (tparam_t u, tparam_t v) const = 0;
};

}

#endif /* __TEX2_H__ */

// arch-tag: 67828d6a-2cd7-4839-8bd4-9bddec07c6a4
