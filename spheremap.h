// spheremap.h -- Texture wrapped around a sphere
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHEREMAP_H__
#define __SPHEREMAP_H__

#include "tex.h"
#include "snogmath.h"
#include "excepts.h"
#include "color.h"
#include "uv.h"
#include "matrix-tex.h"
#include "envmap.h"
#include "spherical-coords.h"


namespace snogray {


//
// Various direction -> texture-coordinate mappings used for environment maps
//

// Linear mapping of longitude/latitude to u, v
//
struct LatLongMapping
{
  static UV map (const Vec &dir)
  {
    return y_axis_latlong (dir);
  }
  static Vec map (const UV &uv)
  {
    return y_axis_latlong_to_vec (uv);
  }

  // Returns the area on the sphere corresponding to a one-unit area at
  // location UV in the texture.
  //
  static float sphere_area (UV uv)
  {
    return cos (uv.v * PIf - PIf/2);
  }
};

#if 0
// Linear mapping of longitude to u, and y to v
//
struct MercatorMapping
{
  static UV map (const Vec &dir) const
  {
    return UV (clamp ((dir.y_axis_longitude () + PIf) * INV_PIf / 2, 0.f, 1.f),
	       clamp ((dir.y + 1) / 2, 0.f, 1.f);
  }

  static Vec map (const UV &uv) const
  {
    float theta = ((uv.u * 2) - 1) * PI, phi = uv.v * PI;
    float sin_phi = sin (phi);
    return Vec (sin_phi * sin (theta), cos (phi), sin_phi * cos (theta));
  }
};
#endif

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
  static UV map (const Vec &dir)
  {
    tparam_t x = dir.x, y = dir.y, z = dir.z;
    tparam_t d = sqrt (x * x + y * y);
    tparam_t rpi = (d == 0) ? 0 : INV_PIf * acos (z) / 2 / d;

    return UV (clamp (x * rpi + 0.5f, 0.f, 1.f),
	       clamp (y * rpi + 0.5f, 0.f, 1.f));
  }

  static Vec map (const UV &uv)
  {
    dist_t u = uv.u * 2 - 1, v = uv.v * 2 - 1;
    dist_t theta = atan2 (v, u), phi = PI * sqrt (u * u + v * v);
    dist_t sin_phi = sin (phi);
    return Vec (sin_phi * sin (theta), cos (phi), sin_phi * cos (theta));
  }
};

// Mapping used by a raw mirrored-ball light-probe image.  Similar to
// the Debevec mapping, but the radius to z mapping is a cosine function
// rather than linear.
//
struct MirrorBallMapping
{
  static UV map (const Vec &dir)
  {
    tparam_t x = dir.x, y = dir.y, z = dir.z;
    tparam_t d = sqrt (x * x + y * y);
    tparam_t rpi = (d == 0) ? 0 : sqrt ((1 - z) / 2) / 2 / d;

    return UV (clamp (x * rpi + 0.5f, 0.f, 1.f),
	       clamp (y * rpi + 0.5f, 0.f, 1.f));
  }

  // XXXX this is not correct !!!!! XXXX
  // (right now it's just a copy of DebevecMapping::map(UV))
  //
  static Vec map (const UV &uv)
  {
    dist_t u = uv.u * 2 - 1, v = uv.v * 2 - 1;
    dist_t theta = atan2 (v, u), phi = PI * sqrt (u * u + v * v);
    dist_t sin_phi = sin (phi);
    return Vec (sin_phi * sin (theta), cos (phi), sin_phi * cos (theta));
  }
};



// An environment map using a single image.  Takes the "mapping" (from
// direction to texture coordinates) as a template arg.
//
template<class Mapping>
class Spheremap : public Envmap
{
public:

  Spheremap (const Ref<Image> &image) : tex (image) { }

  virtual Color map (const Vec &dir) const
  {
    return tex.eval (TexCoords (Pos (dir), mapping.map (dir)));
  }

  // Return a "light-map" -- a lat-long format spheremap image
  // containing light values of the environment map -- for this
  // environment map.
  //
  virtual Ref<Image> light_map () const
  {
    throw std::runtime_error ("Spheremap::light_map");
  }

private:

#if 0
  virtual void fill (FillDst &dst) const
  {
    for (MatrixTex<Color>::const_iterator p = tex.begin ();
	 p != tex.end (); ++p)
      dst.put (mapping.map (p.uv ()), p.val ());
  }
#endif

  // The texture wrapped around the sphere.
  //
  MatrixTex<Color> tex;

  Mapping mapping;
};


// Override the general definition of Spheremap::light_map for
// LatLongMapping, as it can be done more efficiently.
//
template<>
Ref<Image> Spheremap<LatLongMapping>::light_map () const;


}

#endif // __SPHEREMAP_H__


// arch-tag: 13f3c349-9a3e-4337-ba26-cf083997542f
