# tex.swg -- SWIG interfaces for snogray textures
#
#  Copyright (C) 2011  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "tex.h"
#include "image.h"
#include "envmap.h"
#include "matrix-tex.h"
#include "arith-tex.h"
#include "grey-tex.h"
#include "intens-tex.h"
#include "xform-tex.h"
#include "check-tex.h"
#include "misc-map-tex.h"
#include "perlin-tex.h"
#include "worley-tex.h"
#include "coord-tex.h"
#include "interp-tex.h"
#include "cmp-tex.h"
#include "perturb-tex.h"
#include "rescale-tex.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class UV;


  class TexCoords
  {
  public:

    TexCoords (const Pos &_pos, const UV &_uv);

    Pos pos;
    UV uv;
  };

  template<typename T>
  class Tex
  {
  };
  %template(ColorTex) Tex<Color>;
  %template(FloatTex) Tex<float>;
  %template(ColorTexRef) Ref<Tex<Color> >;
  %template(FloatTexRef) Ref<Tex<float> >;

  template<typename T>
  class TexVal
  {
  public:

    TexVal (const Ref<Tex<T> > &_tex);
    TexVal (const T &val);

    %rename(set) operator= (const Ref<Tex<T> > &_tex);
    TexVal &operator= (const Ref<Tex<T> > &_tex);
    %rename(set) operator= (const T &val);
    TexVal &operator= (const T &val);
  };
  %template(ColorTexVal) TexVal<Color>;
  %template(FloatTexVal) TexVal<float>;

  // Swig fails to define %renamed methods in a template'd class, so define
  // these manually.
  //
  %extend TexVal<float>
  {
    void set (const Ref<Tex<float> > &_tex) { *$self = _tex; }
    void set (float val) { *$self = val; }
  }
  %extend TexVal<Color>
  {
    void set (const Ref<Tex<Color> > &_tex) { *$self = _tex; }
    void set (Color val) { *$self = val; }
  }

  // Fake-out SWIG
  class Image;

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


    static Ref<Envmap> envmap (const Ref<Image> &image, const char *fmt = "")
    {
      return make_envmap (image, fmt);
    }

    // ImageTex
    static Ref<Tex<Color> > image_tex (const char *filename,
    	   		  	       const ValTable &params = ValTable::NONE)
    {
      return new MatrixTex<Color> (filename, params);
    }
    static Ref<Tex<Color> > image_tex (const Ref<Image> &contents)
    {
      return new MatrixTex<Color> (contents);
    }

    // "mono" ImageTex (float-valued)
    static Ref<Tex<float> > mono_image_tex (const char *filename,
    	   		  	            const ValTable &params = ValTable::NONE)
    {
      return new MatrixTex<float> (filename, params);
    }
    static Ref<Tex<float> > mono_image_tex (const Ref<TupleMatrix<float> > &contents)
    {
      return new MatrixTex<float> (contents);
    }

    // ArithTex
    static Ref<Tex<Color> > arith_tex (unsigned op,
				  const TexVal<Color> &arg1,
				  const TexVal<Color> &arg2)
    {
      return new ArithTex<Color> (ArithTex<Color>::Op (op), arg1, arg2);
    }
    static Ref<Tex<float> > arith_tex (unsigned op,
				  const TexVal<float> &arg1,
				  const TexVal<float> &arg2)
    {
      return new ArithTex<float> (ArithTex<float>::Op (op), arg1, arg2);
    }

    // CmpTex
    static Ref<Tex<Color> > cmp_tex (unsigned op,
				     const TexVal<float> &cval1,
				     const TexVal<float> &cval2,
				     const TexVal<Color> &rval1,
				     const TexVal<Color> &rval2)
    {
      return new CmpTex<Color> (CmpTex<Color>::Op (op), cval1, cval2,
				rval1, rval2);
    }
    static Ref<Tex<float> > cmp_tex (unsigned op,
				     const TexVal<float> &cval1,
				     const TexVal<float> &cval2,
				     const TexVal<float> &rval1,
				     const TexVal<float> &rval2)
    {
      return new CmpTex<float> (CmpTex<float>::Op (op), cval1, cval2,
				rval1, rval2);
    }

    // GreyTex
    static Ref<Tex<Color> > grey_tex (const TexVal<float> &val)
    {
      return new GreyTex (val);
    }

    // IntensTex
    static Ref<Tex<float> > intens_tex (const TexVal<Color> &val)
    {
      return new IntensTex (val);
    }

    // XformTex
    static Ref<Tex<Color> > xform_tex (const Xform &xform,
				       const TexVal<Color> &tex)
    {
      return new XformTex<Color> (xform, tex);
    }
    static Ref<Tex<float> > xform_tex (const Xform &xform,
				       const TexVal<float> &tex)
    {
      return new XformTex<float> (xform, tex);
    }

    // CheckTex
    static Ref<Tex<Color> > check_tex (const TexVal<Color> &tex1,
				       const TexVal<Color> &tex2)
    {
      return new CheckTex<Color> (tex1, tex2);
    }
    static Ref<Tex<float> > check_tex (const TexVal<float> &tex1,
				       const TexVal<float> &tex2)
    {
      return new CheckTex<float> (tex1, tex2);
    }

    // CheckTex3d
    static Ref<Tex<Color> > check3d_tex (const TexVal<Color> &tex1,
					 const TexVal<Color> &tex2)
    {
      return new Check3dTex<Color> (tex1, tex2);
    }
    static Ref<Tex<float> > check3d_tex (const TexVal<float> &tex1,
					 const TexVal<float> &tex2)
    {
      return new Check3dTex<float> (tex1, tex2);
    }

    // PlaneMapTex
    static Ref<Tex<Color> > plane_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new PlaneMapTex<Color> (tex);
    }
    static Ref<Tex<float> > plane_map_tex (const Ref<Tex<float> > &tex)
    {
      return new PlaneMapTex<float> (tex);
    }

    // CylinderMapTex
    static Ref<Tex<Color> > cylinder_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new CylinderMapTex<Color> (tex);
    }
    static Ref<Tex<float> > cylinder_map_tex (const Ref<Tex<float> > &tex)
    {
      return new CylinderMapTex<float> (tex);
    }

    // LatLongMapTex
    static Ref<Tex<Color> > lat_long_map_tex (const Ref<Tex<Color> > &tex)
    {
      return new LatLongMapTex<Color> (tex);
    }
    static Ref<Tex<float> > lat_long_map_tex (const Ref<Tex<float> > &tex)
    {
      return new LatLongMapTex<float> (tex);
    }

    // PerlinTex
    static Ref<Tex<float> > perlin_tex ()
    {
      return new PerlinTex ();
    }

    // WorleyTex
    static Ref<Tex<float> > worley_tex (float C0, float C1 = 0,
					float C2 = 0, float C3 = 0)
    {
      float coefs[WorleyTex::MAX_N] = { C0, C1, C2, C3 };
      return new WorleyTex (coefs);
    }
    static Ref<Tex<float> > worley_id_tex (int kind, float min)
    {
      return new WorleyIdTex (WorleyIdTex::Kind (kind), min);
    }
    static Ref<Tex<float> > worley_id_tex (int kind, float min, float max)
    {
      return new WorleyIdTex (WorleyIdTex::Kind (kind), min, max);
    }

    // CoordTex
    static Ref<Tex<float> > coord_tex (unsigned kind)
    {
      return new CoordTex (CoordTex::Kind (kind));
    }

    // LinterpTex
    static Ref<Tex<Color> > linterp_tex (const TexVal<float> &control,
					 const TexVal<Color> &val1,
					 const TexVal<Color> &val2)
    {
      return new LinterpTex<Color> (control, val1, val2);
    }
    static Ref<Tex<float> > linterp_tex (const TexVal<float> &control,
					 const TexVal<float> &val1,
					 const TexVal<float> &val2)
    {
      return new LinterpTex<float> (control, val1, val2);
    }

    // SinterpTex
    static Ref<Tex<Color> > sinterp_tex (const TexVal<float> &control,
					 const TexVal<Color> &val1,
					 const TexVal<Color> &val2)
    {
      return new SinterpTex<Color> (control, val1, val2);
    }
    static Ref<Tex<float> > sinterp_tex (const TexVal<float> &control,
					 const TexVal<float> &val1,
					 const TexVal<float> &val2)
    {
      return new SinterpTex<float> (control, val1, val2);
    }

    // RescaleTex
    static Ref<Tex<Color> > rescale_tex (const TexVal<Color> &val,
					 const Color &in_min,
					 const Color &in_max,
					 const Color &out_min,
					 const Color &out_max)
    {
      return new RescaleTex<Color> (val, in_min, in_max, out_min, out_max);
    }
    static Ref<Tex<float> > rescale_tex (const TexVal<float> &val,
					 float in_min, float in_max,
					 float out_min, float out_max)
    {
      return new RescaleTex<float> (val, in_min, in_max, out_min, out_max);
    }

    // PerturbPosTex
    static Ref<Tex<Color> > perturb_pos_tex (const TexVal<Color> &source,
					     const TexVal<float> &x,
					     const TexVal<float> &y,
					     const TexVal<float> &z)
    {
      return new PerturbPosTex<Color> (source, x, y, z);
    }
    static Ref<Tex<float> > perturb_pos_tex (const TexVal<float> &source,
					     const TexVal<float> &x,
					     const TexVal<float> &y,
					     const TexVal<float> &z)
    {
      return new PerturbPosTex<float> (source, x, y, z);
    }

    // PerturbUvTex
    static Ref<Tex<Color> > perturb_uv_tex (const TexVal<Color> &source,
					    const TexVal<float> &u,
					    const TexVal<float> &v)
    {
      return new PerturbUvTex<Color> (source, u, v);
    }
    static Ref<Tex<float> > perturb_uv_tex (const TexVal<float> &source,
					    const TexVal<float> &u,
					    const TexVal<float> &v)
    {
      return new PerturbUvTex<float> (source, u, v);
    }


  }
%}
