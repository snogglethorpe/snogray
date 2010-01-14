// render-context.cc -- "semi-global" information used during rendering
//
//  Copyright (C) 2006, 2007, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mis-illum.h"
#include "octree.h"

#include "render-context.h"


using namespace snogray;


RenderContext::RenderContext (const Scene &_scene, const RenderParams &_params)
  : scene (_scene), params (_params),
    space_builder_factory (new Octree::BuilderFactory) // hardwired for now
{ }

RenderContext::~RenderContext ()
{
}


// arch-tag: bacb1ae7-4350-408b-8746-cbcf77debc30
