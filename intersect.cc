// intersect.cc -- Datatype for recording surface-ray intersection results
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

#include "surface.h"
#include "material.h"
#include "brdf.h"
#include "trace.h"
#include "scene.h"
#include "trace-context.h"

#include "intersect.h"

using namespace snogray;



// Use the bump-map TEX to perturb NORMAL_FRAME.
//
static void
bump_map (Frame &normal_frame, const Ref<const Tex<float> > &tex,
	  const TexCoords &tex_coords, const UV &dTds, const UV &dTdt)
{
  // Perturbation amounts (is there a better way to pick these?).
  //
  dist_t ds = 0.001f, dt = 0.001f;
      
  // Non-perturbed bump-map value.
  //
  float origin_depth = tex->eval (tex_coords);

  // Evaluate bump-map in s direction.
  //
  Pos ds_pos = tex_coords.pos + normal_frame.x * ds;
  UV ds_uv = tex_coords.uv + dTds * ds;
  TexCoords ds_tex_coords (ds_pos, ds_uv);
  float ds_delta = tex->eval (ds_tex_coords) - origin_depth;

  // Evaluate bump-map in t direction.
  //
  Pos dt_pos = tex_coords.pos + normal_frame.y * dt;
  UV dt_uv = tex_coords.uv + dTdt * dt;
  TexCoords dt_tex_coords (dt_pos, dt_uv);
  float dt_delta = tex->eval (dt_tex_coords) - origin_depth;

  if (ds_delta != 0 || dt_delta != 0)
    {
      // New tangent vectors, in the old normal frame.
      //
      Vec new_os (1, 0, ds_delta / ds);
      Vec new_ot (0, 1, dt_delta / dt);

      // Calculate a new normal frame with an appropriately
      // perturbed normal.
      //
      Vec new_s = normal_frame.from (new_os).unit ();
      Vec new_t = normal_frame.from (new_ot).unit ();
      Vec new_norm = cross (new_t, new_s).unit ();

      // Make sure the new frame is orthogonal.
      //
      new_t = cross (new_s, new_norm);

      // Overwrite the old normal frame.
      //
      normal_frame.x = new_s;
      normal_frame.y = new_t;
      normal_frame.z = new_norm;
    }
}



// Finish initialization.  This method is called by all constructors.
//
void
Intersect::finish_init (const UV &dTds, const UV &dTdt)
{
  if (material->bump_map)
    bump_map (normal_frame, material->bump_map, tex_coords, dTds, dTdt);

  // Eye ray in the world frame.
  //
  Vec wv = -ray.dir.unit ();

  // Now that bump-mapping has been done, calculate stuff that depends
  // on on the normal frame.

  // Eye ("view") vector.
  //
  v = normal_frame.to (wv);

  // Eye vector in the geometric frame.
  //
  Vec gv = geom_frame.to (wv);

  // back-face flag.
  //
  back = (gv.z < 0);

  // Make sure V (in the normal frame of reference) always has a
  // positive Z component.
  //
  if (back)
    {
      v.z = -v.z;
      normal_frame.z = -normal_frame.z;
    }

  // Now that NORMAL_FRAME is completely set up, calculate the geometric
  // normal in that frame, GEOM_N.  Unlike GEOM_FRAME, GEOM_N is flipped so
  // that it is always in the same hemisphere as the lighting normal (i.e.,
  // GEOM_N.z is always positive).
  //
  geom_n = normal_frame.to (geom_frame.z);
  geom_n.z = abs (geom_n.z);	// flip GEOM_N if necessary

  // Set up the "brdf" field by calling Intersect::get_brdf.  This is done
  // separately from the constructor initialization, because we pass the
  // intersect object as argument to Material::get_brdf, and we want it to
  // be in a consistent state.
  //
  brdf = material->get_brdf (*this);
}



Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Frame &_normal_frame, const UV &_tex_coords,
		      const UV &dTds, const UV &dTdt, Trace &_trace)
  : ray (_ray), surface (_surface),
    normal_frame (_normal_frame), geom_frame (_normal_frame),
    // v and back are initialized by Intersect::finish_init
    material (&*_surface->material), brdf (0),
    smoothing_group (0), no_self_shadowing (false),
    tex_coords (normal_frame.origin, _tex_coords),
    trace (_trace)
{
  finish_init (dTds, dTdt);
}

Intersect::Intersect (const Ray &_ray, const Surface *_surface,
		      const Frame &_normal_frame, const Frame &_geom_frame,
		      const UV &_tex_coords,
		      const UV &dTds, const UV &dTdt, Trace &_trace)
  : ray (_ray), surface (_surface),
    normal_frame (_normal_frame), geom_frame (_geom_frame),
    // v, geom_n, and back are initialized by Intersect::finish_init
    material (&*_surface->material), brdf (0),
    smoothing_group (0), no_self_shadowing (false),
    tex_coords (normal_frame.origin, _tex_coords),
    trace (_trace)
{
  finish_init (dTds, dTdt);
}

Intersect::~Intersect ()
{
  // Destroy the BRDF object.  The memory will be implicitly freed later.
  //
  if (brdf)
    brdf->~Brdf ();
}


// arch-tag: 4e2a9676-9a81-4f69-8702-194e8b9158a9
