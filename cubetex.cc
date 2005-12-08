// cubetex.cc -- Texture wrapped around a cube
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

#include "excepts.h"

#include "cubetex.h"

using namespace Snogray;
using namespace std;

Cubetex::Cubetex (const string &filename)
{
  for (unsigned i = 0; i < 6; i++)
    faces[i].tex = 0;

  load (filename);
}

Cubetex::~Cubetex ()
{
  for (unsigned i = 0; i < 6; i++)
    delete faces[i].tex;
}

Color
Cubetex::map (const Vec &dir) const
{
  // Choose the main axis of view

  unsigned axis = 0;
  dist_t axis_val = dir.x;

  if (abs (dir.y) > abs (axis_val))
    {
      axis = 1;
      axis_val = dir.y;
    }

  if (abs (dir.z) > abs (axis_val))
    {
      axis = 2;
      axis_val = dir.z;
    }

  // Choose one of the six faces, depending on the axis and diretion
  //
  const Face &face = faces[axis * 2 + (axis_val < 0)];

  // Calculate u and v -- basically the non-axis components of DIR
  // divided by the axis component.
  //
  Texture2::param_t u = dir.dot (face.u_dir) / axis_val;
  Texture2::param_t v = dir.dot (face.v_dir) / axis_val;

  // Translate [-1, 1] params into [0, 1] for texture lookup
  //
  u = (u + 1) * 0.5;
  v = (v + 1) * 0.5;

  // Lookup the value
  //
  return face.tex->map (u, v);
}


// Cubetex loading

void
Cubetex::load (const string &filename)
{
  ifstream stream (filename.c_str ());

  if (stream)
    try
      { 
	load (stream, filename);
      }
    catch (runtime_error &err)
      {
	throw file_error (filename + ": Error loading cubetex file: "
			  + err.what ());
      }
  else
    throw file_error (filename + ": Cannot open cubetex file");
}

Vec
Cubetex::parse_axis_dir (const string &str)
{
  dist_t val = 1;
  bool bad = false;
  unsigned offs = 0;

  if (str[0] == '+')
    offs++;
  else if (str[0] == '-')
    {
      offs++;
      val = -val;
    }
  else
    bad = true;

  Vec vec;
  if (str[offs] == 'x')
    vec = Vec (val, 0, 0);
  else if (str[offs] == 'y')
    vec = Vec (0, val, 0);
  else if (str[offs] == 'z')
    vec = Vec (0, 0, val);
  else
    bad = true;

  if (str.length() - offs > 1)
    bad = true;

  if (bad)
    throw runtime_error (str + ": Illegal axis spec");

  return vec;
}

void
Cubetex::load (istream &stream, const string &filename)
{
  unsigned num_faces_loaded = 0;

  while (num_faces_loaded < 6)
    {
      bool skipped_comment;
      do
	{
	  skipped_comment = false;
	  stream >> ws;
	  if (stream.peek () == '#')
	    {
	      char ch;
	      while (stream.get (ch) && ch != '\n')
		/* nothing */;
	      skipped_comment = true;
	    }
	}
      while (skipped_comment);

      string kw;
      stream >> kw;

      unsigned face_num;
      if (kw == "right" || kw == "rgt")
	face_num = 0;
      else if (kw == "left" || kw == "lft")
	face_num = 1;
      else if (kw == "top" || kw == "up")
	face_num = 2;
      else if (kw == "bottom" || kw == "bot" || kw == "down")
	face_num = 3;
      else if (kw == "front" || kw == "fwd" || kw == "forward")
	face_num = 4;
      else if (kw == "back" || kw == "rear" || kw == "bwd" || kw == "backward")
	face_num = 5;
      else
	throw bad_format (kw + ": Unknown face name");

      Face &face = faces[face_num];

      if (face.tex)
	throw bad_format (kw + ": Face defined multiple times");
      else
	num_faces_loaded++;

      string u_spec, v_spec;
      stream >> u_spec;
      stream >> v_spec;

      face.u_dir = parse_axis_dir (u_spec);
      face.v_dir = parse_axis_dir (v_spec);

      string tex_filename;
      stream >> ws;
      getline (stream, tex_filename);

      if (tex_filename[0] != '/' && filename.length() > 0)
	{
	  // prepend the path used to open the cubetex file

	  unsigned pfx_end = filename.find_last_of ("/");

	  if (pfx_end > 0)
	    tex_filename.insert (0, filename.substr (0, pfx_end + 1));
	}

      try
	{ 
	  face.tex = new Texture2 (tex_filename);
	}
      catch (runtime_error &err)
	{
	  throw file_error (filename + ": Error loading texture: "
			    + err.what ());
	}
    }
}

// arch-tag: 6f62ca7f-6a3e-47d7-a558-3f321b11fd70
