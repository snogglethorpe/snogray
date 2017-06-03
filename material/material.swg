// material.swg -- SWIG interfaces for snogray materials
//
//  Copyright (C) 2011, 2012, 2013, 2017  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


%{
#include "material/material.h"
#include "material/material-dict.h"
#include "material/glow.h"
#include "material/norm-glow.h"
#include "material/glass.h"
#include "material/thin-glass.h"
#include "material/mirror.h"
#include "material/stencil.h"
#include "material/cook-torrance.h"
#include "material/lambert.h"
#include "material/xform-material.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  // Fwd decls
  class Color;
  template<typename T>
  class Tex;
  template<typename T>
  class TexVal;


  class Material
  {
  public:
    Ref<Tex<float> > bump_map;
  };
  %template(MatRef) Ref<Material>;

  class Ior
  {
  public:

    Ior (float _n, float _k = 0);

    bool operator== (const Ior &ior2);

    float n, k;
  };
  %extend Ior
  {
    const char* __str__()
    {
      std::ostringstream s;
      s << "ior<" << $self->n << ", " << $self->k << ">";
      strcpy (static_rep_buf, s.str().c_str());
      return static_rep_buf;
    }
  }

  class Medium
  {
  public:

    Medium (float _ior = 1, const Color &_absorb = 0);
  };


  class MaterialDict
  {
  public:

    MaterialDict (const Ref<Material> &default_material = 0);

    void add (const char *name, const Ref<Material> &mat);
    bool contains (const char *name) const;

    void set_default (const Ref<Material> &mat);

    // MaterialDict::get and MaterialDict::get_default are implemented
    // using %extend, so we can lie to swig about their return types.
  };
  %extend MaterialDict
  {
    Ref<Material> get (const char *name, const Ref<Material> &def_mat) const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get (name, def_mat);
      return const_cast<snogray::Material *> (&*cmat);
    }
    Ref<Material> get (const char *name) const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get (name);
      return const_cast<snogray::Material *> (&*cmat);
    }
    Ref<Material> get_default () const
    {
      snogray::Ref<const snogray::Material> cmat = $self->get_default ();
      return const_cast<snogray::Material *> (&*cmat);
    }

    const Ref<const Material> __getitem__ (const char *name) const
    {
      return $self->get (name);
    }
    void __setitem__ (const char *name, const Ref<Material> &mat)
    {
      $self->add (name, mat);
    }

    // Since a "nil" table value in lua conventionally means "no entry",
    // support true/false for use with "negative" entries (an entry
    // mapping to a zero pointer in the native MaterialDict type).
    //
    void __setitem__ (const char *name, bool)
    {
      $self->add (name, 0);
    }

    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"material-dict<nentries=%d%s>",
		$self->num_entries(),
		$self->get_default() ? "+1" : "");
      return static_rep_buf;
    }
  }

  // Try to make transforms appliable to textures too.. ugh... soooo
  // many SWIG bugs....
  //
  %extend TXform<dist_t>
  {
    snogray::Ref<snogray::Material> operator () (const snogray::Ref<snogray::Material> &obj) const
    {
      return new snogray::XformMaterial (*$self, obj);
    }
  }


} // namespace snogray


// The following are object-creation wrappers, which we use in place of
// swig-generated constructors for any classes that we want to manage via
// reference-counting.  This is because swig will do its own memory
// management for object which it constructs, and we want to avoid that.
//
// These wrapper functions simply create the object using new and return a
// reference to it; swig happily uses the reference instead, with almost
// exactly the same functionality as the underlying object pointer.
//
%inline %{
  namespace snogray {


    // Lambert
    static Ref<Material> lambert (const TexVal<Color> &col)
    {
      return new Lambert (col);
    }

    // CookTorrance
    static Ref<Material>
    cook_torrance (const TexVal<Color> &col, const TexVal<Color> &spec_col,
		   const TexVal<float> &m, const Ior &ior)
    {
      return new CookTorrance (col, spec_col, m, ior);
    }
    static Ref<Material>
    cook_torrance (const TexVal<Color> &col, const TexVal<Color> &spec_col,
		   const TexVal<float> &m, float ior)
    {
      return new CookTorrance (col, spec_col, m, ior);
    }
    
    // Mirror
    static Ref<Material>
    mirror (const Ior &_ior, const TexVal<Color> &_reflectance,
	    const Ref<Material> &underlying_material)
    {
      return new Mirror (_ior, _reflectance, underlying_material);
    }
    static Ref<Material>
    mirror (const Ior &_ior,
	    const TexVal<Color> &_reflectance,
	    const TexVal<Color> &col = Color(0))
    {
      return new Mirror (_ior, _reflectance, col);
    }
    static Ref<Material>
    mirror (float _ior, const TexVal<Color> &_reflectance,
    	    const Ref<Material> &underlying_material)
    {
      return new Mirror (_ior, _reflectance, underlying_material);
    }
    static Ref<Material>
    mirror (float _ior,
	    const TexVal<Color> &_reflectance,
	    const TexVal<Color> &col = Color(0))
    {
      return new Mirror (_ior, _reflectance, col);
    }

    // Stencil
    static Ref<Material>
    stencil (const TexVal<Color> &opacity,
	     const Ref<Material> &underlying_material)
    {
      return new Stencil (opacity, underlying_material);
    }

    // Glass
    static Ref<Material> glass (const Medium &medium)
    {
      return new Glass (medium);
    }      

    // ThinGlass
    static Ref<Material> thin_glass (const Color &col, float _ior = 1.5)
    {
      return new ThinGlass (col, _ior);
    }      
    static Ref<Material> thin_glass (float _ior = 1.5)
    {
      return new ThinGlass (_ior);
    }      

    // Glow, NormGlow
    static Ref<Material> glow (const TexVal<Color> &col)
    {
      return new Glow (col);
    }
    static Ref<Material> glow (const TexVal<Color> &col,
			       const Ref<Material> &underlying_material)
    {
      return new Glow (col, underlying_material);
    }
    static Ref<Material> norm_glow (intens_t intens)
    {
      return new NormGlow (intens);
    }

    // XformMaterial, XformMaterialUV, XformMaterialPos
    static Ref<Material>
    xform_material (const Xform &xform, const Ref<Material> &material)
    {
      return new XformMaterial (xform, material);
    }
    static Ref<Material>
    xform_material_uv (const Xform &xform, const Ref<Material> &material)
    {
      return new XformMaterialUV (xform, material);
    }
    static Ref<Material>
    xform_material_pos (const Xform &xform, const Ref<Material> &material)
    {
      return new XformMaterialPos (xform, material);
    }

  }
%}
