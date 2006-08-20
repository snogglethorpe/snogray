// load-ply.cc -- Load a .ply format mesh file
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <csetjmp>

#include "excepts.h"
#include "xform.h"
#include "mesh.h"

extern "C" {
#include "rply.h"
}

# include "load-ply.h"


using namespace Snogray;


// Error handling

static jmp_buf err_jmpbuf;
static const char *err_str = 0;

static void
rply_error_cb (const char *str)
{
  // We can't use c++ exceptions as this is used by a C file
  //
  err_str = str;
  longjmp (err_jmpbuf, 1);
}



struct RplyState
{
  RplyState (Mesh &_mesh, const Xform &_xform)
    : mesh (_mesh), base_vert_index (mesh.num_vertices ()),
      xform (_xform), norm_xform (xform.inverse().transpose()),
      last_vert_index (0)
  { }

  Mesh &mesh;

  Mesh::vert_index_t base_vert_index;

  Xform xform;

  // A variant of XFORM suitable for transforming normals.
  //
  Xform norm_xform;

  double vals[3];

  Mesh::vert_index_t last_vert_index;
};


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
      = s->mesh.add_vertex (Pos (s->vals[0], s->vals[1], s->vals[2])
			    * s->xform);

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
			      Vec (s->vals[0], s->vals[1], s->vals[2])
			      * s->norm_xform);

      if (norm_index != s->last_vert_index)
	rply_error_cb ("Normal consistency error");
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
    rply_error_cb ("Invalid number of indices in face");
  else if (n >= 0)
    s->vals[n] = ply_get_argument_value (arg);

  if (n == 2)
    s->mesh.add_triangle (Mesh::vert_index_t (s->vals[0]) + s->base_vert_index,
			  Mesh::vert_index_t (s->vals[1]) + s->base_vert_index,
			  Mesh::vert_index_t (s->vals[2]) + s->base_vert_index);

  return 1;
}


// Main loading function

// Load mesh from a .ply format mesh file into MESH.  Geometry is first
// transformed by XFORM.
//
void
Snogray::load_ply_file (const std::string &filename, Mesh &mesh,
			const Xform &xform)
{
  volatile p_ply ply = 0;

  if (setjmp (err_jmpbuf) != 0)
    {
      if (ply)
	ply_close (ply);

      throw std::runtime_error (err_str);
    }

  ply = ply_open (filename.c_str(), rply_error_cb);

  ply_read_header (ply);

  RplyState state (mesh, xform);
  void *cookie = (void *)&state;

  unsigned nverts
    = ply_set_read_cb (ply, "vertex", "x", vert_cb, cookie, 0);
  ply_set_read_cb(ply, "vertex", "y", vert_cb, cookie, 1);
  ply_set_read_cb(ply, "vertex", "z", vert_cb, cookie, 2);
  ply_set_read_cb(ply, "vertex", "nx", norm_cb, cookie, 0);
  ply_set_read_cb(ply, "vertex", "ny", norm_cb, cookie, 1);
  ply_set_read_cb(ply, "vertex", "nz", norm_cb, cookie, 2);

  unsigned ntris
    = ply_set_read_cb (ply, "face", "vertex_indices", face_cb, cookie, 0);

  mesh.reserve (nverts, ntris);

  ply_read (ply);

  ply_close (ply);
}

// arch-tag: c40c2ae8-576b-4985-ad52-2582b3bb2fa0
