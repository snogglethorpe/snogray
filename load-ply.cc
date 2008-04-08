// load-ply.cc -- Load a .ply format mesh file
//
//  Copyright (C) 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <csetjmp>

#include "excepts.h"
#include "mesh.h"

extern "C" {
#include "rply.h"
}

#include "load-ply.h"


using namespace snogray;



struct RplyState
{
  RplyState (Mesh &_mesh, const Ref<const Material> &_mat)
    : mesh (_mesh), base_vert_index (mesh.num_vertices ()), mat (_mat),
      last_vert_index (0)
  { }

  Mesh &mesh;

  Mesh::vert_index_t base_vert_index;

  Ref<const Material> mat;

  double vals[3];

  Mesh::vert_index_t last_vert_index;

  // Set if an error occurs in the rply library.
  //
  std::string err_str;
};


// Error callback for rply library

static void
error_cb (p_ply ply, const char *str)
{
  // We can't use c++ exceptions as this is used by a C file, so just save
  // the msg and rely on the rest of the code to fail gracefully.

  void *pdata;
  if (ply_get_user_data (ply, &pdata, 0))
    {
      RplyState *s = (RplyState *)pdata;
      s->err_str = str;
    }
}


// RPly callbacks

static int
vert_cb (p_ply_argument arg)
{
  long n;
  void *cookie;
  ply_get_argument_user_data (arg, &cookie, &n);

  RplyState *s = (RplyState *)cookie;

  s->vals[n] = ply_get_argument_value (arg);

  if (n == 2)
    s->last_vert_index
      = s->mesh.add_vertex (Pos (s->vals[0], s->vals[1], s->vals[2]));

  return 1;
}

static int
norm_cb (p_ply_argument arg)
{
  long n;
  void *cookie;
  ply_get_argument_user_data (arg, &cookie, &n);

  RplyState *s = (RplyState *)cookie;

  s->vals[n] = ply_get_argument_value (arg);
  
  // Some .ply files have bogus (null) normals, so we check for such and
  // only add non-null normals.
  //
  if (n == 2 && (s->vals[0] > Eps || s->vals[1] > Eps || s->vals[2] > Eps))
    {
      Mesh::vert_index_t norm_index
	= s->mesh.add_normal (s->last_vert_index,
			      Vec (s->vals[0], s->vals[1], s->vals[2]));

      if (norm_index != s->last_vert_index)
	{
	  s->err_str = "Normal consistency error";
	  return 0;
	}
    }

  return 1;
}

static int
face_cb (p_ply_argument arg)
{
  void *cookie;
  ply_get_argument_user_data (arg, &cookie, 0);

  RplyState *s = (RplyState *)cookie;

  long n;
  ply_get_argument_property (arg, NULL, 0, &n);

  if (n >= 3)
    {
      s->err_str = "Invalid number of indices in face";
      return 0;
    }

  if (n >= 0)
    s->vals[n] = ply_get_argument_value (arg);

  if (n == 2)
    s->mesh.add_triangle (Mesh::vert_index_t (s->vals[0]) + s->base_vert_index,
			  Mesh::vert_index_t (s->vals[1]) + s->base_vert_index,
			  Mesh::vert_index_t (s->vals[2]) + s->base_vert_index,
			  s->mat);

  return 1;
}


// Main loading function

// Load mesh from a .ply format mesh file into MESH.  If non-zero the
// material MAT will be used for triangles loaded (otherwise MESH's
// default material will be used).
//
void
snogray::load_ply_file (const std::string &filename, Mesh &mesh,
			const Ref<const Material> &mat)
{
  // State used by all our call back functions.
  //
  RplyState state (mesh, mat);

  // The rply library uses a void* value to store client state, so keep a
  // pointer to STATE in that form.
  //
  void *cookie = (void *)&state;

  p_ply ply = ply_open (filename.c_str(), error_cb, 0, cookie);

  if (ply)
    {
      if (ply_read_header (ply))
	{
	  unsigned nverts
	    = ply_set_read_cb (ply, "vertex", "x", vert_cb, cookie, 0);
	  ply_set_read_cb (ply, "vertex", "y", vert_cb, cookie, 1);
	  ply_set_read_cb (ply, "vertex", "z", vert_cb, cookie, 2);
	  ply_set_read_cb (ply, "vertex", "nx", norm_cb, cookie, 0);
	  ply_set_read_cb (ply, "vertex", "ny", norm_cb, cookie, 1);
	  ply_set_read_cb (ply, "vertex", "nz", norm_cb, cookie, 2);

	  unsigned ntris
	    = ply_set_read_cb (ply, "face", "vertex_indices",
			       face_cb, cookie, 0);

	  mesh.reserve (nverts, ntris);

	  // Ply files use a right-handed coordinate system by convention.
	  //
	  mesh.left_handed = false;

	  ply_read (ply);
	}

      ply_close (ply);
    }

  if (! state.err_str.empty ())
    throw std::runtime_error (state.err_str);
}

// arch-tag: c40c2ae8-576b-4985-ad52-2582b3bb2fa0
