// render.h -- Main rendering loop
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDER_H__
#define __RENDER_H__

#include <ostream>

#include "val-table.h"

namespace snogray {

class Scene;
class Camera;
class ImageOutput;
class TraceStats;

extern void
render (const Scene &scene, const Camera &camera,
	unsigned width, unsigned height,
	ImageOutput &output, unsigned offs_x, unsigned offs_y,
	const ValTable &render_params, TraceStats &stats,
	std::ostream &progress_stream, Progress::Verbosity verbosity);

}

#endif /* __RENDER_H__ */

// arch-tag: 4893a4e7-a67c-4c9c-9182-6d774fb18812
