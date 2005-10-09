// scene-load.cc -- Loading scene files
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <fstream>
#include <cerrno>

#include "scene.h"
#include "excepts.h"

using namespace Snogray;
using namespace std;

void
Scene::load (const char *scene_file_name, const char *fmt, Camera &camera)
{
  ifstream stream (scene_file_name);

  if (! stream)
    throw file_error (string ("Cannot open scene file: ") + strerror (errno));

  if (! fmt)
    { 
      const char *file_ext = rindex (scene_file_name, '.');

      if (! file_ext)
	throw
	  file_error ("No filename extension to determine scene file format");
      else
	fmt = file_ext + 1;
    }

  load (stream, fmt, camera);
}

void
Scene::load (istream &stream, const char *fmt, Camera &camera)
{
  if (strcasecmp (fmt, "nff") == 0 || strcasecmp (fmt, "aff") == 0)
    load_aff_file (stream, camera);
  else
    throw (runtime_error (string ("Unknown scene file format: ") + fmt));
}

// arch-tag: 6afb7868-7e1a-46de-815c-eed49bdd8609
