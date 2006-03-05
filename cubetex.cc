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
#include "image-io.h"

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
  Texture2::param_t u = dot (dir, face.u_dir) / axis_val;
  Texture2::param_t v = dot (dir, face.v_dir) / axis_val;

  // Translate [-1, 1] params into [0, 1] for texture lookup
  //
  u = (u + 1) * 0.5;
  v = (v + 1) * 0.5;

  // Lookup the value
  //
  return face.tex->map (u, v);
}


// Cubetex general loading interface

void
Cubetex::load (const string &filename)
{
  if (ImageInput::recognized_filename (filename))
    //
    // Load from a single image file
    {
      Image image (filename);

      try
	{
	  load (image);
	}
      catch (runtime_error &err)
	{
	  throw file_error (filename + ": Error loading cubetex image: "
			    + err.what ());
	}
    }

  else
    // Load from a "descriptor" file
    {
      ifstream stream (filename.c_str ());

      if (stream)
	try
	  { 
	    // Compute filename prefix used for individual image files from
	    // the path used to open the cubetex file.
	    //
	    string filename_pfx;
	    unsigned pfx_end = filename.find_last_of ("/");
	    if (pfx_end > 0)
	      filename_pfx = filename.substr (0, pfx_end + 1);

	    load (stream, filename_pfx);
	  }
	catch (runtime_error &err)
	  {
	    throw file_error (filename + ": Error loading cubetex file: "
			      + err.what ());
	  }
      else
	throw file_error (filename + ": Cannot open cubetex file");
    }
}


// Loading of a .ctx "descriptor" file

void
Cubetex::load (istream &stream, const string &filename_pfx)
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

      if (tex_filename[0] != '/' && filename_pfx.length() > 0)
	tex_filename.insert (0, filename_pfx);

      try
	{ 
	  face.tex = new Texture2 (tex_filename);
	}
      catch (runtime_error &err)
	{
	  throw file_error (string ("Error loading texture: ") + err.what ());
	}
    }
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


// Loading of a single background image

void
Cubetex::load (const Image &image)
{
  unsigned size;
  unsigned w = image.width, h = image.height;

  if ((size = w / 3) * 3 == w && size * 4 == h)
    //
    // "vertical cross" format
    {
      // Back
      faces[5].tex = new Texture2 (image, size, size * 3, size, size);
      faces[5].u_dir = Vec (-1, 0, 0);
      faces[5].v_dir = Vec (0, 1, 0);
    }
  else if ((size = w / 4) * 4 == w && size * 3 == h)
    //
    // "horizontal cross" format
    {
      // Back
      faces[5].tex = new Texture2 (image, size * 3, size, size, size);
      faces[5].u_dir = Vec (1, 0, 0);
      faces[5].v_dir = Vec (0, -1, 0);
    }
  else
    throw bad_format ("unrecognized cube-texture image size");

  // Common parts of the two "cross" formats

  // Right
  faces[0].tex = new Texture2 (image, size * 2, size, size, size);
  faces[0].u_dir = Vec (0, 0, -1);
  faces[0].v_dir = Vec (0, 1, 0);

  // Left
  faces[1].tex = new Texture2 (image, 0, size, size, size);
  faces[1].u_dir = Vec (0, 0, -1);
  faces[1].v_dir = Vec (0, -1, 0);

  // Top
  faces[2].tex = new Texture2 (image, size, 0, size, size);
  faces[2].u_dir = Vec (1, 0, 0);
  faces[2].v_dir = Vec (0, 0, -1);

  // Bottom
  faces[3].tex = new Texture2 (image, size, size * 2, size, size);
  faces[3].u_dir = Vec (-1, 0, 0);
  faces[3].v_dir = Vec (0, 0, -1);

  // Front
  faces[4].tex = new Texture2 (image, size, size, size, size);
  faces[4].u_dir = Vec (1, 0, 0);
  faces[4].v_dir = Vec (0, 1, 0);
}

// arch-tag: 6f62ca7f-6a3e-47d7-a558-3f321b11fd70
