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
#include "uv.h"
#include "matrix-tex2.h"
#include "envmap.h"


namespace Snogray {


//
// Various direction -> texture-coordinate mappings used for environment maps
//

// Linear mapping of longitude/latitude to u, v
//
struct LatLongMapping
{
  UV map (const Vec &dir) const
  {
    return UV ((dir.longitude () + M_PI) * M_1_PI * 0.5,
	       (dir.latitude () + M_PI_2) / M_PI);
  }
};

// Linear mapping of longitude to u, and y to v
//
struct MercatorMapping
{
  UV map (const Vec &dir) const
  {
    return UV ((dir.longitude () + M_PI) * M_1_PI * 0.5, (dir.y + 1) * 0.5);
  }
};

// Mapping used by Paul Debevec's light-probe images.  As described on
// his web page:
//
//   "The coordinate mapping of these images is such that the
//   center of the image is straight forward, the circumference
//   of the image is straight backwards, and the horizontal line
//   through the center linearly maps azimuthal angle to pixel
//   coordinate.
//   
//   Thus, if we consider the images to be normalized to have
//   coordinates u=[-1,1], v=[-1,1], we have theta=atan2(v,u),
//   phi=pi*sqrt(u*u+v*v).  The unit vector pointing in the
//   corresponding direction is obtained by rotating (0,0,-1) by
//   phi degrees around the y (up) axis and then theta degrees
//   around the -z (forward) axis.  If for a direction vector in
//   the world (Dx, Dy, Dz), the corresponding (u,v) coordinate
//   in the light probe image is (Dx*r,Dy*r) where
//   r=(1/pi)*acos(Dz)/sqrt(Dx^2 + Dy^2)."
//
// We slightly change the above formula because our texture coordinates
// have a range of 0-1.
//
struct DebevecMapping
{
  UV map (const Vec &dir) const
  {
    tparam_t x = dir.x, y = dir.y, z = dir.z;
    tparam_t d = sqrt (x * x + y * y);
    tparam_t rpi = (d == 0) ? 0 : M_1_PIf * 0.5f * acos (z) / d;

    return UV (x * rpi + 0.5f, y * rpi + 0.5f);
  }
};

// Mapping used by a raw mirrored-ball light-probe image.  Similar to
// the Debevec mapping, but the radius to z mapping is a cosine function
// rather than linear.
//
struct MirrorBallMapping
{
  UV map (const Vec &dir) const
  {
    tparam_t x = dir.x, y = dir.y, z = dir.z;
    tparam_t d = sqrt (x * x + y * y);
    tparam_t rpi = (d == 0) ? 0 : 0.5f * sqrt (0.5f * (1 - z)) / d;

    return UV (x * rpi + 0.5f, y * rpi + 0.5f);
  }
};



// An environment map using a single image.  Takes the "mapping" (from
// direction to texture coordinates) as a template arg.
//
template<class Mapping>
class Spheremap : public Envmap
{
public:

  Spheremap (const Image &image) : tex (image) { }

  virtual Color map (const Vec &dir) const
  {
    UV uv = mapping.map (dir);
    return tex.map (uv.u, uv.v);
  }

private:

  // The texture wrapped around the sphere.
  //
  MatrixTex2<Color> tex;

  Mapping mapping;
};


}

#endif /* __SPHEREMAP_H__ */


// arch-tag: 13f3c349-9a3e-4337-ba26-cf083997542f
