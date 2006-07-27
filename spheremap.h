// spheremap.h -- Texture wrapped around a sphere
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHEREMAP_H__
#define __SPHEREMAP_H__

#include <memory>

#include "color.h"
#include "matrix-tex2.h"
#include "envmap.h"


//
// This class maps a single texture onto a sphere.  It supports two
// different mappings from directions to texture coordinates:
//
// + The MAPPING_DEBEVEC type is used by Paul Debevec's light-probe
//   images, using square source images.  As described on his web
//   page:
//
//      "The coordinate mapping of these images is such that the
//      center of the image is straight forward, the circumference
//      of the image is straight backwards, and the horizontal line
//      through the center linearly maps azimuthal angle to pixel
//      coordinate.
//      
//      Thus, if we consider the images to be normalized to have
//      coordinates u=[-1,1], v=[-1,1], we have theta=atan2(v,u),
//      phi=pi*sqrt(u*u+v*v).  The unit vector pointing in the
//      corresponding direction is obtained by rotating (0,0,-1) by
//      phi degrees around the y (up) axis and then theta degrees
//      around the -z (forward) axis.  If for a direction vector in
//      the world (Dx, Dy, Dz), the corresponding (u,v) coordinate
//      in the light probe image is (Dx*r,Dy*r) where
//      r=(1/pi)*acos(Dz)/sqrt(Dx^2 + Dy^2)."
//
// + The MAPPING_LL is a simple longitude/latitude mapping using 2x1
//   source images.
//


namespace Snogray {

class Spheremap : public Envmap
{
public:

  // Methods for mapping directional coordinate to texture coordinates.
  //
  enum mapping_t { MAPPING_LL, MAPPING_DEBEVEC };

  Spheremap (const Image &image)
    : tex (image), mapping (guess_mapping (image))
  { }

  virtual Color map (const Vec &dir) const
  {
    // First calculate u,v in range -PI - PI
    //
    tparam_t u, v;
    if (mapping == MAPPING_LL)
      {
	u = dir.longitude();
	v = dir.latitude() * 2;
      }
    else
      {
	tparam_t d = sqrt (dir.x * dir.x + dir.y * dir.y);
	tparam_t rpi = (d == 0) ? 0 : acos (dir.z) / d;
	u = dir.x * rpi;
	v = dir.y * rpi;
      }

    // Convert to range 0-1 for texture lookup
    //
    u = (u + tparam_t (M_PI)) * tparam_t (M_1_PI * 0.5);
    v = (v + tparam_t (M_PI)) * tparam_t (M_1_PI * 0.5);

    return tex.map (u, v);
  }

private:

  mapping_t guess_mapping (const Image &image)
  {
    if (image.height == image.width)
      return MAPPING_DEBEVEC;
    else if (image.width == image.height * 2)
      return MAPPING_LL;
    else
      throw bad_format ("Unrecognized sphere-map image size");
  }

  // The texture wrapped around the sphere.
  //
  MatrixTex2<Color> tex;

  // How directional coordinates should be mapped to texture
  // coordinates.
  //
  mapping_t mapping;
};

}

#endif /* __SPHEREMAP_H__ */

// arch-tag: 13f3c349-9a3e-4337-ba26-cf083997542f
