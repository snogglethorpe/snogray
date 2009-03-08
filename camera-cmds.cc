// camera-cmds.cc -- Parsing of camera command strings
//
//  Copyright (C) 2005, 2006, 2007, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <sstream>
#include <iomanip>

#include "excepts.h"
#include "val-table.h"
#include "scene.h"
#include "camera.h"
#include "trace-params.h"
#include "trace-context.h"
#include "trace-cache.h"

#include "camera-cmds.h"


using namespace snogray;
using namespace std;


// Primitive stream character parsing functions

static char
looking_at (istream &stream, const char *choices)
{
  stream >> ws;

  char ch = stream.eof() ? 0 : stream.peek();

  for (const char *p = choices; *p; p++)
    if (ch == *p)
      return ch;

  return 0;
}

static char
eat (istream &stream, const char *choices, const char *req_desc = 0)
{
  char ch = looking_at (stream, choices);
  if (ch)
    {
      stream.get ();		// eat it
      return ch;
    }

  if (req_desc)
    {
      string msg;

      if (ch)
	msg += "Invalid ";
      else
	msg += "Missing ";

      msg += req_desc;

      if (ch)
	{
	  msg += " `";
	  msg += ch;
	  msg += "'";
	}

      msg += "; expected one of ";

      for (const char *p = choices; *p; p++)
	{
	  msg += "`";
	  msg += *p;
	  msg += "'";
	  if (p[1])
	    {
	      if (p > choices || p[2])
		msg += ",";
	      if (! p[2])
		msg += " or";
	      msg += " ";
	    }
	}
      
      throw (runtime_error (msg));
    }

  return 0;  
}

static void
eat_close (istream &stream, char open)
{
  if (open)
    {
      char close_delims[2] = { open, 0 };
      switch (open)
	{
	case '(': close_delims[0] = ')'; break;
	case '[': close_delims[0] = ']'; break;
	case '{': close_delims[0] = '}'; break;
	case '<': close_delims[0] = '>'; break;
	}
      eat (stream, close_delims, "close bracket");
    }
}


// Stream parsing functions

static double
read_float (istream &stream, const char *desc)
{
  double val;
  if (stream >> val)
    return val;
  else
    throw runtime_error (string ("Missing/invalid ") + desc);
}

static double
read_angle (istream &stream, const char *desc)
{
  return read_float (stream, desc) * PI / 180;
}

static dist_t
read_dist (istream &stream, const char *desc)
{
  return read_float (stream, desc);
}

static Pos
read_pos (istream &stream)
{
  Pos pos;
  char open = eat (stream, "(<[{");
  pos.x = read_float (stream, "x coord");
  eat (stream, ",", "comma");
  pos.y = read_float (stream, "y coord");
  eat (stream, ",", "comma");
  pos.z = read_float (stream, "z coord");
  eat_close (stream, open);
  return pos;
}

static Xform
read_rot_xform (istream &stream, const Camera &camera)
{
  char dir = eat (stream, "udlraxyz", "direction/axis");
  float angle = read_angle (stream, "angle");
  Xform xform;

  if (dir == 'u')
    xform.rotate (camera.right, -angle);
  else if (dir == 'd')
    xform.rotate (camera.right, angle);
  else if (dir == 'l')
    xform.rotate (camera.up, -angle);
  else if (dir == 'r')
    xform.rotate (camera.up, angle);
  else if (dir == 'a')
    xform.rotate (camera.forward, angle);
  else if (dir == 'x')
    xform.rotate_x (angle);
  else if (dir == 'y')
    xform.rotate_y (angle);
  else if (dir == 'z')
    xform.rotate_z (angle);

  return xform;
}

static float
read_fraction (istream &stream, const char *desc)
{
  float val = read_float (stream, desc);
  if (eat (stream, "%"))
    val *= 0.01;		// percentage
  if (val < 0 || val > 1)
    throw runtime_error (string ("Invalid ") + desc);
  return val;
}


// Autofocusing

static Vec
probe_scene (float u, float v, Camera &camera, const Scene &scene)
{
  // Dummy values to make renderer happy
  //
  ValTable render_params;
  TraceParams trace_params (render_params);
  TraceContext trace_context (scene, trace_params);
  TraceCache root_cache (trace_context);

  Ray probe (camera.eye_ray (u, v), Scene::DEFAULT_HORIZON);

  Surface::IsecCtx isec_ctx (trace_context, root_cache);
  if (scene.intersect (probe, isec_ctx))
    return probe.dir * probe.length ();
  else
    return Vec (0,0,0);
}

static void
autofocus (istream &stream, Camera &camera, const Scene &scene)
{
  float u, v;

  stream >> ws;
  if (stream.eof() || looking_at (stream, ",;/"))
    u = v = 0.5f;
  else
    {
      char open = eat (stream, "(<[{");
      u = read_fraction (stream, "x position");
       eat (stream, ",", "comma");
      v = read_fraction (stream, "y position");
      eat_close (stream, open);
    }

  Vec focus_vec = probe_scene (u, v, camera, scene);

  if (focus_vec.length() > 0)
    camera.set_focus (focus_vec);
  else
    {
      ostringstream err_msg;
      err_msg << "Autofocus at (" << u << ", " << v << ") failed";
      throw runtime_error (err_msg.str ());
    }

  cout << "* autofocus distance: " << camera.focus << endl;
}


// User command-line camera-commands

void
snogray::interpret_camera_cmds (const string &cmds, Camera &camera,
				const Scene &scene)
{
  istringstream stream (cmds);

  try
    { 
      while (! stream.eof ())
	{
	  char cmd = eat (stream, "gtzlfduhvamro", "command");

	  if (cmd == 'g')	// goto position
	    camera.move (read_pos (stream));
	  else if (cmd == 't')	// target position
	    camera.point (read_pos (stream));
	  else if (cmd == 'z')	// zoom
	    camera.zoom (read_float (stream, "zoom factor"));
	  else if (cmd == 'l')	// lens focal length
	    {
	      float foclen = read_float (stream, "lens focal length");

	      // preserve the camera's current f-stop, if any (otherwise,
	      // the camera retains the old aperture setting, which
	      // corresponds to a different f-stop with the new focal
	      // length).
	      //
	      float f_stop = camera.f_stop ();

	      camera.set_focal_length (foclen);
	      camera.set_f_stop (f_stop);
	    }
	  else if (cmd == 'f')	// set f-stop
	    camera.set_f_stop (read_float (stream, "f-stop"));
	  else if (cmd == 'd')	// set focus distance
	    {
	      char mod = eat (stream, "+-");
	      float dist = read_float (stream, "focus distance");
	      if (mod)
		{
		  if (mod == '-')
		    dist = -dist;
		  dist += camera.focus_distance ();
		}
	      camera.set_focus (dist);
	    }
	  else if (cmd == 'a')	// auto-focus
	    autofocus (stream, camera, scene);
	  else if (cmd == 'u')	// define scene-unit size
	    camera.set_scene_unit (read_float (stream, "scene unit (in mm)"));
	  else if (cmd == 'h')	// use horizontal camera orientation
	    camera.set_orientation (Camera::ORIENT_HORIZ);
	  else if (cmd == 'v')	// use vertical camera orientation
	    camera.set_orientation (Camera::ORIENT_VERT);
	  else if (cmd == 'm')	// move in some direction
	    {
	      char dir = eat (stream, "udlrfbxyz", "movement direction/axis");
	      dist_t dist = read_dist (stream, "movement distance");

	      if (dir == 'r')
		camera.move (camera.right * dist);
	      else if (dir == 'l')
		camera.move (-camera.right * dist);
	      else if (dir == 'u')
		camera.move (camera.up * dist);
	      else if (dir == 'd')
		camera.move (-camera.up * dist);
	      else if (dir == 'f')
		camera.move (camera.forward * dist);
	      else if (dir == 'b')
		camera.move (-camera.forward * dist);
	      else if (dir == 'x')
		camera.move (Vec (dist, 0, 0));
	      else if (dir == 'y')
		camera.move (Vec (0, dist, 0));
	      else if (dir == 'z')
		camera.move (Vec (0, 0, dist));
	    }
	  else if (cmd == 'r')	// rotate camera
	    camera.rotate (read_rot_xform (stream, camera));
	  else if (cmd == 'o')	// "orbit" camera around target
	    camera.orbit (read_rot_xform (stream, camera).inverse ());

	  eat (stream, ",;/");	// eat delimiter
	}
    }
  catch (runtime_error &err)
    {
      throw runtime_error (cmds + ": Error interpreting camera commands: "
			   + err.what ());
    }
}

// arch-tag: c110f827-51e4-4915-91a5-c9383d3d3ae1
