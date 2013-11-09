// sphere.cc -- Sphere surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "geometry/sphere-isec.h"
#include "geometry/sphere-sample.h"
#include "render/intersect.h"
#include "light/sphere-light-sampler.h"
#include "surface-sampler.h"

#include "sphere.h"


using namespace snogray;



// Sphere::IsecInfo

class Sphere::IsecInfo : public Surface::Renderable::IsecInfo
{
public:

  IsecInfo (const Ray &ray, const Sphere &_sphere)
    : Surface::Renderable::IsecInfo (ray), sphere (_sphere)
  { }

  virtual Intersect make_intersect (const Media &media, RenderContext &context)
    const;

  virtual Vec normal () const;

private:

  const Sphere &sphere;
};

// Create an Intersect object for this intersection.
//
Intersect
Sphere::IsecInfo::make_intersect (const Media &media, RenderContext &context) const
{
  Pos point = ray.end ();

  // Calculate the normal and tangent vectors.  Normally the tangent
  // vectors are oriented "naturally", with s pointing "around" the
  // sphere's axis, and t roughly in the same direction as the axis.
  // However, if the normal _exactly_ coincides with the axis, this
  // isn't possible; we use more arbitrary tangent vectors in that case.
  //
  Vec norm = (point - sphere.frame.origin).unit ();
  Vec s = cross (norm, sphere.frame.z);
  if (s.length_squared() < Eps)
    s = norm.perpendicular ();	// degenerate case where NORM == AXIS
  s = s.unit ();
  Vec t = cross (s, norm);

  // 1 divided by the radius/circumference of the sphere.
  //
  dist_t inv_radius = 1 / sphere.radius;
  dist_t inv_circum = inv_radius * dist_t (INV_PI / 2);

  // Intersection point in object space.
  //
  Vec opoint = sphere.frame.to (point);

  // 1 divided by the radius/circumference of a horizontal cut (in "object
  // space") through the sphere at the current location (height == z).
  //
  dist_t z_radius = sqrt (opoint.x*opoint.x + opoint.y*opoint.y);
  dist_t inv_z_radius = z_radius ? 1 / z_radius : 0;
  dist_t inv_z_circum = inv_z_radius * dist_t (INV_PI / 2);

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  UV dTds (inv_z_circum, 0), dTdt (0, inv_circum * 2);

  return Intersect (ray, media, context, *sphere.material,
		    Frame (point, s, t, norm),
		    sphere.tex_coords (opoint), dTds, dTdt);
}

// Return the normal of this intersection (in the world frame).
//
Vec
Sphere::IsecInfo::normal () const
{
  return (ray.end() - sphere.frame.origin).unit ();
}



// intersection

// If this surface intersects RAY, change RAY's maximum bound
// (Ray::t1) to reflect the point of intersection, and return a
// Surface::Renderable::IsecInfo object describing the intersection
// (which should be allocated using placement-new with CONTEXT);
// otherwise return zero.
//
const Surface::Renderable::IsecInfo *
Sphere::intersect (Ray &ray, RenderContext &context) const
{
  dist_t t;
  if (sphere_intersects (frame.origin, radius, ray, t))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, *this);
    }
  return 0;
}

// Return true if this surface intersects RAY.
//
bool
Sphere::intersects (const Ray &ray, RenderContext &) const
{
  dist_t t;
  return sphere_intersects (frame.origin, radius, ray, t);
}

// Return true if this surface completely occludes RAY.  If it does
// not completely occlude RAY, then return false, and multiply
// TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
// MEDIUM.
//
// Note that this method does not try to handle non-trivial forms of
// transparency/translucency (for instance, a "glass" material is
// probably considered opaque because it changes light direction as
// well as transmitting it).
//
bool
Sphere::occludes (const Ray &ray, const Medium &medium,
		  Color &total_transmittance, RenderContext &)
  const
{
  dist_t t;
  if (sphere_intersects (frame.origin, radius, ray, t))
    {
      // Avoid unnecessary calculation if possible.
      if (material->fully_occluding ())
	return true;

      IsecInfo isec_info (Ray (ray, t), *this);
      if (material->occlusion_requires_tex_coords ())
	{
	  Pos pos = ray (t);
	  UV tex_coords_uv = tex_coords (frame.to (pos));
	  TexCoords tex_coords (pos, tex_coords_uv);

	  return material->occludes (isec_info, tex_coords, medium,
				     total_transmittance);
	}
      else
	return material->occludes (isec_info, medium, total_transmittance);
    }

  return false;
}



// misc Sphere methods

// Return a bounding box for this surface.
BBox
Sphere::bbox () const
{
  const Pos &cent = frame.origin;
  return BBox (Pos (cent.x - radius, cent.y - radius, cent.z - radius),
	       Pos (cent.x + radius, cent.y + radius, cent.z + radius));
}

// Transform the geometry of this surface by XFORM.
//
void
Sphere::transform (const Xform &xform)
{
  frame.transform (xform);
}

// Add light-samplers for this primitive's shape, with with intensity
// INTENSITY, to SAMPLERS.  An error will be signaled if this
// primitive does not support lighting.
//
void
Sphere::add_light_samplers (const TexVal<Color> &intensity,
			    std::vector<const Light::Sampler *> &samplers)
  const
{
  samplers.push_back (new SphereLightSampler (frame.origin, radius, intensity));
}



// Sphere::Sampler

// Sphere Sampler interface.
//
class Sphere::Sampler : public Surface::Sampler
{
public:

  Sampler (const Sphere &_sphere) : sphere (_sphere) { }

  // Return a sample of this surface.
  //
  virtual AreaSample sample (const UV &param) const;

  // If a ray from VIEWPOINT in direction DIR intersects this
  // surface, return an AngularSample as if the
  // Surface::Sampler::sample_from_viewpoint method had returned a
  // sample at the intersection position.  Otherwise, return an
  // AngularSample with a PDF of zero.
  //
  virtual AngularSample eval_from_viewpoint (const Pos &viewpoint,
					     const Vec &dir)
    const;

private:

  const Sphere &sphere;
};

// Return a sample of this surface.
//
Surface::Sampler::AreaSample
Sphere::Sampler::sample (const UV &param) const
{
  Pos center = sphere.frame.origin;
  dist_t radius = sphere.radius;

  Vec norm = sphere_sample (param);
  Pos pos = center + norm * radius;
  float pdf = 1 / (float (radius*radius) * 4 * PIf); // 1 / area

  return AreaSample (pos, norm, pdf);
}

// If a ray from VIEWPOINT in direction DIR intersects this
// surface, return an AngularSample as if the
// Surface::Sampler::sample_from_viewpoint method had returned a
// sample at the intersection position.  Otherwise, return an
// AngularSample with a PDF of zero.
//
Surface::Sampler::AngularSample
Sphere::Sampler::eval_from_viewpoint (const Pos &viewpoint, const Vec &dir)
  const
{
  dist_t t;
  if (sphere_intersects (sphere.frame.origin, sphere.radius, viewpoint, dir, t))
    {
      Pos pos = viewpoint + t * dir;
      Vec vec = pos - sphere.frame.origin;
      Vec norm = vec.unit ();
      float area_pdf =		// 1 / area
	1 / (float (sphere.radius * sphere.radius) * 4 * PIf);
      return AngularSample (AreaSample (pos, norm, area_pdf), viewpoint);
    }
  return AngularSample ();
}


// Return a sampler for this surface, or zero if the surface doesn't
// support sampling.  The caller is responsible for destroying
// returned samplers.
//
Surface::Sampler *
Sphere::make_sampler () const
{
  return new Sampler (*this);
}


// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
