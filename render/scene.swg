# scene.swg -- SWIG interfaces for snogray scenes
#
#  Copyright (C) 2011-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "render/scene.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  // fwd decls
  class SpaceBuilderFactory;

  // fwd decls
  class IsecInfo;
  class RenderContext;

  class Scene
  {
  public:

    Scene (const Surface &root_surface,
	   const SpaceBuilderFactory &space_builder_factory);;

    unsigned num_light_samplers () const;

    const IsecInfo *intersect (Ray &ray, RenderContext &context) const;
    bool intersects (const Ray &ray, RenderContext &context) const;
  };
  %extend Scene
  {
    const char* __str__() { return "<scene>"; }
  };


} // namespace snogray
