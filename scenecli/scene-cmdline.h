// scene-cmdline.h -- Command-line options for scene parameters
//
//  Copyright (C) 2005-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SCENE_CMDLINE_H
#define SNOGRAY_SCENE_CMDLINE_H

#include "util/val-table.h"


#define SCENE_OPTIONS_HELP "\
  -b, --background=BG        Use BG as a background and light-source;\n\
                               BG may be a color or the name of an\n\
                               environment-map image file\n\
  -B, --background-orientation=[AXIS][HAND][ROTATION]\n\
                             Rotate the background and/or change its axis:\n\
                               AXIS is the vertical axis, with values\n\
                                  \"x\", \"y\", or \"z\" (default \"y\")\n\
                               HAND is the \"handedness\", with values\n\
                                  \"r\" or \"l\" (default \"r\")\n\
                               ROTATION is an amount to rotate the background\n\
                                 around the vertical axis, in degrees\n\
\n\
  -I, --scene-options=OPTS   Set scene options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"format\"    -- scene file type\n\
                                 \"background\"-- scene background"
                               
#define SCENE_SHORT_OPTIONS		"b:B:I:"

#define SCENE_LONG_OPTIONS						\
    { "background",     required_argument, 0, 'b' },			\
    { "background-orientation",     required_argument, 0, 'B' },	\
    { "scene-options", 	required_argument, 0, 'I' },

#define SCENE_OPTION_CASES(clp, scene_params)				\
  case 'b':								\
    scene_params.set ("background", clp.opt_arg ());			\
    break;								\
  case 'B':								\
    scene_params.set ("background-orientation", clp.opt_arg ());	\
    break;								\
  case 'I':								\
    clp.parse_opt_arg (scene_params);					\
    break;


namespace snogray {

// Handle any scene parameters specified in PARAMS, into SCENE.
//
extern void process_scene_params (const ValTable &params, Scene &scene);

}


#endif // SNOGRAY_SCENE_CMDLINE_H
