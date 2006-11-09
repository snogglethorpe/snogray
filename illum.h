// illum.h -- Illumination interface
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ILLUM_H__
#define __ILLUM_H__

#include "color.h"


namespace Snogray {


class Intersect;
class Trace;


class Illum
{
public:
  
  virtual ~Illum () { }

  // Illuminate the intersection ISEC.
  //
  virtual Color illum (const Intersect &isec) = 0;
};


class IllumGlobalState
{
public:
  
  virtual ~IllumGlobalState () { }

  virtual Illum *get_illum (Trace &trace) = 0;
  virtual void put_illum (Illum *ill) = 0;
};


}

#endif // __ILLUM_H__

// arch-tag: 628a8d08-8766-4ca4-befb-257d020b1dae
