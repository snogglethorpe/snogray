// scene-def.h -- Scene definition object
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SCENE_DEF_H__
#define __SCENE_DEF_H__

#include <iostream>
#include <iomanip>

#include "test-scenes.h"

#define SCENE_DEF_OPTIONS_HELP "\
 Scene options:\n\
  -b, --background=BG        Use BG as a background; BG may be a color or a\n\
                               cube-map specification\n\
  -I, --scene-format=FMT     Scene is in format FMT (one of: test, aff, nff)\n\
  -G, --assumed-gamma=GAMMA  Reverse implicit gamma correction of GAMMA\n\
  -L, --light-scale=SCALE    Scale all scene lighting by SCALE\n\
\n\
  -T, --tessel-accur=ERR     Set tessellation accuracy for test:tessel scenes\n\
                             (default 0.001; prepend `@' to toggle smoothing)\n\
\n\
      --list-test-scenes     Ouput a list of builtin test-scenes to stdout\n\
\n\
 Camera options:\n\
  -c, --camera=COMMANDS      Move/point the camera according to COMMANDS:\n\
                               g X,Y,Z     Goto absolute location X, Y, Z\n\
                               t X,Y,Z     Point at target X, Y, Z\n\
                               m[rludfb] D Move distance D in the given dir\n\
                                           (right, left, up, down, fwd, back)\n\
                               m[xyz] D    Move distance D on the given axis\n\
                               r[rlud] A   Rotate A deg in the given dir\n\
                               ra A        Rotate A deg around center axis\n\
                               r[xyz] A    Rotate A degrees around [xyz]-axis\n\
                               o[xyz] A    Orbit A degrees around [xyz]-axis"
//
#define SCENE_DEF_SHORT_OPTIONS		"b:I:G:L:T:c:"
//
#define SCENE_DEF_LONG_OPTIONS						\
    { "background",     required_argument, 0, 'b' },			\
    { "scene-format", 	required_argument, 0, 'I' },			\
    { "assumed-gamma", 	required_argument, 0, 'G' },			\
    { "light-scale", 	required_argument, 0, 'L' },			\
    { "tessel-accur",   required_argument, 0, 'T' },			\
    { "camera",		required_argument, 0, 'c' },			\
    { "list-test-scenes", no_argument,     0, SCENE_DEF_OPT_LIST_TEST_SCENES }
//
#define SCENE_DEF_OPT_BASE		(('S'<<24)+('C'<<16)+('N'<<8))
#define SCENE_DEF_OPT_LIST_TEST_SCENES	(SCENE_DEF_OPT_BASE + 1)
//
#define SCENE_DEF_EXTRA_HELP "\
The \"test\" scene type is special, as no scene file is actually read;\n\
instead, a built in test-scene with the given name is used.  As a shortcut\n\
a prefix of `test:' maybe be used instead of the `-Itest' option;\n\
e.g. `test:cbox1' refers to the built-in test-scene `cbox1'.\n\
\n\
For a full list of test-scenes, use the `--list-test-scenes' option."

#define SCENE_DEF_OPTION_CASES(clp, scene_def)				      \
  case 'b':								      \
    scene_def.bg_spec = clp.opt_arg ();					      \
    break;								      \
									      \
  case 'I':								      \
    scene_def.explicit_fmt = clp.opt_arg ();				      \
    break;								      \
  case 'G':								      \
    scene_def.assumed_gamma = clp.float_opt_arg ();			      \
    break;								      \
  case 'L':								      \
    scene_def.light_scale = clp.float_opt_arg ();			      \
    break;								      \
									      \
  case 'c':								      \
    scene_def.camera_cmds += clp.opt_arg ();				      \
    break;								      \
									      \
  case 'T':								      \
    {									      \
      const char *arg = clp.opt_arg();					      \
      if (*arg == '@')							      \
	{								      \
	  tessel_smooth = !tessel_smooth;				      \
	  arg++;							      \
	}								      \
      if (*arg)								      \
	tessel_accur = atof (arg);					      \
    }									      \
    break;								      \
									      \
  case SCENE_DEF_OPT_LIST_TEST_SCENES:					      \
    {									      \
      std::vector<TestSceneDesc> descs = list_test_scenes ();		      \
									      \
      std::cout << "Built-in test scenes:" << std::endl << std::endl;	      \
      std::cout.setf (std::ios::left);					      \
									      \
      for (std::vector<TestSceneDesc>::const_iterator di = descs.begin();     \
	   di != descs.end(); di++)					      \
	{								      \
	  unsigned nlen = 15;						      \
	  while (di->name.length() > nlen - 3)				      \
	    nlen += 12;							      \
	  std::cout << "   " << std::setw(nlen) << di->name << di->desc	      \
		    << std::endl;					      \
	}								      \
    }									      \
    exit (0);

namespace Snogray {

class CmdLineParser;
class Scene;
class Camera;

class SceneDef
{
public:

  SceneDef ()
    : assumed_gamma (1), light_scale (1), bg_spec (0)
  { }

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

  // A list of scene specs to load.
  //
  std::vector<Spec> specs;

  // An explicit scene fmt specified with the -I option.  Overrides
  // automatic guessing of format.
  //
  std::string explicit_fmt;

  // User commands for the camera (applied following scene-definition)
  //
  std::string camera_cmds;

  float assumed_gamma, light_scale;

  const char *bg_spec;
};

}

#endif /* __SCENE_DEF_H__ */

// arch-tag: 4628d634-58c1-4054-b89d-2b88618e6a9f
