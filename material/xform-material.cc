// xform-material.cc -- Material that transform texture coordinates
//
//  Copyright (C) 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "texture/xform-tex.h"

#include "xform-material.h"


using namespace snogray;



// XformMaterialBase

XformMaterialBase::XformMaterialBase (const Xform &_xform,
				      const Ref<const Material> &_material)
  : MaterialWrapper (_material), xform (_xform)
{
}



// XformMaterial

XformMaterial::XformMaterial (const Xform &_xform,
				  const Ref<const Material> &_material)
  : XformMaterialBase (_xform, _material)
{
  if (material->bump_map)
    bump_map
      = new XformTex<float> (xform, TexVal<float> (material->bump_map));
}

// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
XformMaterial::get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
  const
{
  TexCoords xf_tex_coords (xform (tex_coords.pos), xform (tex_coords.uv));
  return material->get_bsdf (isec, xf_tex_coords);
}

// Return the transmittance of this material at the intersection
// described by ISEC_INFO, with texture-coordinates TEX_COORDS, in
// medium MEDIUM.
//
// Note that this method only applies to "simple"
// transparency/translucency, where transmitted rays don't change
// direction; materials that are conceptually "transparent," but
// which exhibit more complex effects like refraction (which change
// the direction) may return zero from this method.
//
Color
XformMaterial::transmittance (const Surface::Renderable::IsecInfo &isec_info,
			      const TexCoords &tex_coords,
			      const Medium &medium)
  const
{
  TexCoords xf_tex_coords (xform (tex_coords.pos), xform (tex_coords.uv));
  return material->transmittance (isec_info, xf_tex_coords, medium);
}



// XformMaterialUV

XformMaterialUV::XformMaterialUV (const Xform &_xform,
				  const Ref<const Material> &_material)
  : XformMaterialBase (_xform, _material)
{
  if (material->bump_map)
    bump_map
      = new XformTexUV<float> (xform, TexVal<float> (material->bump_map));
}

// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
XformMaterialUV::get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
  const
{
  TexCoords xf_tex_coords (tex_coords.pos, xform (tex_coords.uv));
  return material->get_bsdf (isec, xf_tex_coords);
}

// Return the transmittance of this material at the intersection
// described by ISEC_INFO, with texture-coordinates TEX_COORDS, in
// medium MEDIUM.
//
// Note that this method only applies to "simple"
// transparency/translucency, where transmitted rays don't change
// direction; materials that are conceptually "transparent," but
// which exhibit more complex effects like refraction (which change
// the direction) may return zero from this method.
//
Color
XformMaterialUV::transmittance (const Surface::Renderable::IsecInfo &isec_info,
				const TexCoords &tex_coords,
				const Medium &medium)
  const
{
  TexCoords xf_tex_coords (tex_coords.pos, xform (tex_coords.uv));
  return material->transmittance (isec_info, xf_tex_coords, medium);
}



// XformMaterialPos

XformMaterialPos::XformMaterialPos (const Xform &_xform,
				    const Ref<const Material> &_material)
  : XformMaterialBase (_xform, _material)
{
  if (material->bump_map)
    bump_map
      = new XformTexPos<float> (xform, TexVal<float> (material->bump_map));
}

// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
XformMaterialPos::get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
  const
{
  TexCoords xf_tex_coords (xform (tex_coords.pos), tex_coords.uv);
  return material->get_bsdf (isec, xf_tex_coords);
}

// Return the transmittance of this material at the intersection
// described by ISEC_INFO, with texture-coordinates TEX_COORDS, in
// medium MEDIUM.
//
// Note that this method only applies to "simple"
// transparency/translucency, where transmitted rays don't change
// direction; materials that are conceptually "transparent," but
// which exhibit more complex effects like refraction (which change
// the direction) may return zero from this method.
//
Color
XformMaterialPos::transmittance (const Surface::Renderable::IsecInfo &isec_info,
				 const TexCoords &tex_coords,
				 const Medium &medium)
  const
{
  TexCoords xf_tex_coords (xform (tex_coords.pos), tex_coords.uv);
  return material->transmittance (isec_info, xf_tex_coords, medium);
}
