// scene-def.h -- Scene definition object
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SCENE_DEF_H__
#define __SCENE_DEF_H__

#include <iostream>
#include <iomanip>

#include "val-table.h"

#define SCENE_DEF_OPTIONS_HELP "\
 Scene options:\n\
  -b, --background=BG        Use BG as a background and light-source;\n\
                               BG may be a color or the name of an\n\
                               environment-map image file\n\
\
  -I, --scene-options=OPTS   Set scene options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"format\"    -- scene file type\n\
                                 \"background\"-- scene background\n\
                                 \"gamma\"     -- implied scene gamma correction"
                               
//
#define SCENE_DEF_SHORT_OPTIONS		"b:A:l:I:c:"
//
#define SCENE_DEF_LONG_OPTIONS						\
    { "background",     required_argument, 0, 'b' },			\
    { "scene-options", 	required_argument, 0, 'I' },			\

#define SCENE_DEF_OPTION_CASES(clp, scene_def)				\
  case 'b':								\
    scene_def.params.set ("background", clp.opt_arg ());		\
    break;								\
									\
  case 'I':								\
    clp.parse_opt_arg (scene_def.params);				\
    break;

namespace snogray {

class CmdLineParser;
class Scene;
class Camera;

class SceneDef
{
public:

  SceneDef () { }

  // Parse any scene-definition arguments necessary from CLP.
  // At most MAX_SPECS scene specifications will be consumed from CLP.
  // The exact aguments required may vary depending on previous options.
  //
  void parse (CmdLineParser &clp, unsigned max_specs = 1);

  // Load the scene into SCENE and CAMERA.
  //
  void load (Scene &scene, Camera &camera);

  // Returns a string containing the parsed scene specs.
  //
  std::string specs_rep () const;

  // A single scene specification to load
  //
  struct Spec
  {
    Spec (const std::string &_uname, const std::string &_name,
	  const std::string &_fmt)
      : user_name (_uname), name (_name), scene_fmt (_fmt)
    { }

    // The scene name specified by the user; zero-length if none.
    //
    std::string user_name;

    // The scene name possibly with the prefix removed.
    //
    std::string name;

    // The format of the scene; empty means "try to guess"
    //
    std::string scene_fmt;
  };

  // Returns a Spec for standard input.
  //
  Spec cin_spec ();

  // General scene parameters.
  //
  ValTable params;

  // A list of scene specs to load.
  //
  std::vector<Spec> specs;
};

}

#endif /* __SCENE_DEF_H__ */

// arch-tag: 4628d634-58c1-4054-b89d-2b88618e6a9f
