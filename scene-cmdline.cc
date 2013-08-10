// scene-cmdline.h -- Command-line options for scene parameters
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstdlib>

#include "scene.h"
#include "texture/envmap.h"
#include "light/envmap-light.h"
#include "xform.h"
#include "load-envmap.h"
#include "light/far-light.h"
#include "excepts.h"
#include "image-io.h"
#include "string-funs.h"

#include "scene-cmdline.h"


using namespace snogray;


// Handle any scene parameters specified in PARAMS, into SCENE.
//
void
snogray::process_scene_params (const ValTable &params, Scene &scene)
{
  // Set scene background.
  //
  std::string bg_spec = params.get_string ("background");
  if (! bg_spec.empty ())
    {
      std::string fmt = strip_prefix (bg_spec, ":");

      Light *bg_light;
      if (fmt == "grey" || fmt == "g")
	{
	  // Light using a constant color.
	  
	  bg_light = new FarLight (Vec(0,1,0), 2*PIf, atof (bg_spec.c_str()));
	}
      else
	{
	  // Light using an environment map.

	  Ref<Envmap> envmap
	    = load_envmap (bg_spec, (fmt == "envmap") ? "" : fmt);

	  // A frame which defines a transformation from
	  // environment-map-coordinates to world coordinates.
	  //
	  Frame envmap_mapping_frame;

	  // Environment-map orientation parameters.
	  //
	  // We use left-handed coordinates by default, and most
	  // scenes have a vertical Y axis; most environment maps are
	  // right-handed, and the environment-mapping code assumes a
	  // vertical Z-axis.  The default rotation is chosen to
	  // preserve compatibility with old scenes.
	  //
	  char axis = 'y', handedness = 'r';
	  float rotation = 90;

	  // If the user specified some non-default options for the
	  // environment-map orientation, parse them.
	  //
	  std::string bg_orient = params.get_string ("background_orientation");
	  if (! bg_orient.empty ())
	    {
	      bg_orient = downcase (bg_orient);

	      const char *tail = bg_orient.c_str ();
	      if (*tail == 'x' || *tail == 'y' || *tail == 'z')
		axis = *tail++;
	      if (*tail == 'r' || *tail == 'l')
		handedness = *tail++;
	      if (*tail)
		{
		  // Read rotation amount; note that we _subtract_
		  // from the default (which is non-zero), as it seems
		  // more natural for a positive user angle argument
		  // to cause a counter-clockwise rotation (when
		  // viewed from the axis top, it would be a positive
		  // angle).

		  char *end;
		  rotation -= strtof (tail, &end);
		  if (end == tail)
		    throw bad_format ("Invalid background_orientation \""
				      + bg_orient + "\"");
		}
	    }

	  Xform xform;
 
	  // If the environment-map "handedness" isn't our default
	  // left-handedness, flip the transform, which will reverse
	  // handedness.
	  //
	  if (handedness == 'r')
	    xform.scale (-1, 1, 1);

	  // Do any desired about the vertical axis.
	  //
	  if (rotation != 0)
	    xform.rotate_z (rotation * PIf / 180);

	  // If the desired vertical axis isn't the default z-axis,
	  // rotate the desired axis into z's place.
	  //
	  if (axis == 'x')
	    xform.rotate_y (-PIf / 2);
	  else if (axis == 'y')
	    xform.rotate_x (-PIf / 2);

	  envmap_mapping_frame.transform (xform);

	  bg_light = new EnvmapLight (envmap, envmap_mapping_frame);
	}

      scene.add (bg_light);
    }
}
