// scene-def.h -- Scene definition object
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "envmap.h"
#include "envmap-light.h"
#include "excepts.h"
#include "glow.h"
#include "image-io.h"
#include "string-funs.h"
#include "camera-cmds.h"
#include "cmdlineparser.h"
#include "octree.h"
#if USE_LUA
# include "load-lua.h"
#endif

#include "scene-def.h"


using namespace snogray;
using namespace std;


// Command-line parsing

// Parse any scene-definition arguments necessary from CLP.
// At most MAX_SPECS scene specifications will be consumed from CLP.
// The exact aguments required may vary depending on previous options.
//
void
SceneDef::parse (CmdLineParser &clp, unsigned max_specs)
{
  unsigned num = clp.num_remaining_args();

  if (num > max_specs)
    num = max_specs;

  while (num > 0)
    {
      string user_name = clp.get_arg ();
      string name = user_name;
      string fmt = params.get_string ("format");

      specs.push_back (Spec (user_name, name, fmt));

      num--;
    }
}


// Scene loading

// Load a scene using arguments from CLP, into SCENE and CAMERA
//
void
SceneDef::load (Scene &scene, Camera &camera)
{
  // Set background (this is done before reading in the scene, so the scene
  // defining code can adjust for the presence of an environment map).
  //
  string bg_spec = params.get_string ("background");
  if (! bg_spec.empty ())
    {
      string fmt = strip_prefix (bg_spec, ":");

      if (fmt == "grey" || fmt == "g")
	scene.set_background (atof (bg_spec.c_str()));
      else if (fmt == "envmap")
	scene.set_background (load_envmap (bg_spec));
      else
	scene.set_background (load_envmap (bg_spec, fmt));
    }
  string lmap_spec = params.get_string ("light-map");
  if (! lmap_spec.empty ())
    {
      string fmt = strip_prefix (lmap_spec, ":");

      if (fmt == "envmap")
	scene.set_light_map (load_envmap (lmap_spec));
      else
	scene.set_light_map (load_envmap (lmap_spec, fmt));
    }

  float bg_alpha = params.get_float ("background-alpha", 1);
  scene.set_background_alpha (bg_alpha);

  // By default, use an environment map as a light-map too.
  //
  if (scene.env_map && !scene.light_map)
    scene.set_light_map (scene.env_map);

  // Read in scene file
  //
  for (vector<Spec>::iterator spec = specs.begin();
       spec != specs.end(); spec++)
    try
      {
	scene.load (spec->name, spec->scene_fmt, camera);
      }
    catch (runtime_error &err)
      {
	throw runtime_error (spec->user_name + ": Error reading scene: "
			     + err.what ());
      }

  if (scene.light_map)
    {
      EnvmapLight *env_light = new EnvmapLight (scene.light_map);

      scene.add (env_light);

      string env_light_dump_file = params.get_string ("envlight-dump-file");
      if (! env_light_dump_file.empty ())
	env_light->dump (env_light_dump_file, *scene.light_map);
    }

  // Make sure the space acceleration structures are built.
  //
  Octree::BuilderBuilder octree_builder_builder; // hardwired for now
  scene.build_space (&octree_builder_builder);

  if (camera_cmds.length () > 0)
    interpret_camera_cmds (camera_cmds, camera, scene);

  // Cleanup Lua loader state if necessary.
  //
  // Note that we can't do this if swig doesn't support the "disown"
  // feature, as without that, scene objects will still be considered
  // "owned" by swig, and deallocated if we cleanup Lua state.
  //
#if USE_LUA && HAVE_SWIG_DISOWN
  cleanup_load_lua_state ();
#endif
}



// Returns a string containing the parsed scene specs.
//
string
SceneDef::specs_rep () const
{
  string rep;

  for (vector<Spec>::const_iterator spec = specs.begin();
       spec != specs.end(); spec++)
    {
      if (spec != specs.begin ())
	rep += " ";
      rep += spec->user_name;
    }

  return rep;
}


// arch-tag: b48e19f8-8e7b-46bf-9812-03eeb57fef7e
