// xform-material.h -- Material that transform texture coordinates
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

#ifndef SNOGRAY_XFORM_MATERIAL_H
#define SNOGRAY_XFORM_MATERIAL_H

#include "texture/tex.h"
#include "geometry/xform.h"

#include "material-wrapper.h"


namespace snogray {


// Common base class for XformMaterialUV and XformMaterialPos.
//
class XformMaterialBase : public MaterialWrapper
{
public:

  XformMaterialBase (const Xform &_xform, const Ref<const Material> &_material);

protected:

  // Transformation to use.  The same transform is used for both 2d and 3d
  // coordinates (the 2d coordinates are mapped to the x-y plane).
  //
  Xform xform;
};


// A material that transforms the texture coordinates of an underlying
// material, both UV and positional.
//
class XformMaterial : public XformMaterialBase
{
public:

  XformMaterial (const Xform &_xform, const Ref<const Material> &_material);

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

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
  virtual Color transmittance (const Surface::IsecInfo &isec_info,
			       const TexCoords &tex_coords,
			       const Medium &medium)
    const;
};


// A material that transforms the texture UV coordinates of an underlying
// material.
//
class XformMaterialUV : public XformMaterialBase
{
public:

  XformMaterialUV (const Xform &_xform, const Ref<const Material> &_material);

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

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
  virtual Color transmittance (const Surface::IsecInfo &isec_info,
			       const TexCoords &tex_coords,
			       const Medium &medium)
    const;
};


// A material that transforms the texture position coordinates of an
// underlying material.
//
class XformMaterialPos : public XformMaterialBase
{
public:

  XformMaterialPos (const Xform &_xform, const Ref<const Material> &_material);

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

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
  virtual Color transmittance (const Surface::IsecInfo &isec_info,
			       const TexCoords &tex_coords,
			       const Medium &medium)
    const;
};


}

#endif // SNOGRAY_XFORM_MATERIAL_H
