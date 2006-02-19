// scene-stats.h -- Print post-rendering statistics
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SCENE_STATS_H__
#define __SCENE_STATS_H__

#include <ostream>

#include "scene.h"

namespace Snogray {

// Print post-rendering scene statistics
//
void print_scene_stats (const Scene &scene, std::ostream &os);

}

#endif /*__SCENE_STATS_H__ */

// arch-tag: b7800699-80ca-46da-9f30-732a78beb547
