# light.swg -- SWIG interfaces for snogray lights
#
#  Copyright (C) 2011, 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "light/point-light.h"
#include "light/far-light.h"
#include "light/envmap-light.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  %ignore Light;
  class Light { };

  class PointLight : public Light
  {
  public:

    PointLight (const Pos &pos, const Color &col);
    PointLight (const Pos &pos, const Color &col, float angle, const Vec &dir,
    	        float fringe_angle = 0);
  };

  class FarLight : public Light
  {
  public:

    FarLight (const Vec &_dir, float _angle, const Color &_intensity);
  };

  %ignore Envmap;
  class Envmap { };
  %template(EnvmapRef) Ref<Envmap>;

  class EnvmapLight : public Light
  {
  public:

    EnvmapLight (const Ref<Envmap> &_envmap, const Frame &_frame = Frame ());
  };


} // namespace snogray
