// scene-load-nff.cc -- Load .nff scene file
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <fstream>
#include <string>

#include "scene.h"
#include "mesh.h"
#include "excepts.h"
#include "mirror.h"
#include "sphere.h"

using namespace Snogray;
using namespace std;

#define E 0.00001

// How bright we make lights
//
#define NFF_LIGHT_INTENS 100

// The .nff files we have seen all use wacky "gamma adjusted" lighting,
// So try to compensate for that here.
//
#define NFF_ASSUMED_GAMMA 2.2


// Low-level input functions

static void
read_required_kw (istream &stream, const char *kw)
{
  char buf[10];
  stream >> buf;
  if (stream.fail ())
    throw bad_format ("Unexpected EOF");
  if (strcmp (buf, kw) != 0)
    throw bad_format (string ("Missing keyword \"") + kw + "\"; found: " + buf);
}

static unsigned
read_unsigned (istream &stream)
{
  unsigned val;
  stream >> val;
  if (stream.fail ())
    throw bad_format ("Expected unsigned");
  return val;
}

static float
read_float (istream &stream)
{
  float val;
  stream >> val;
  if (stream.fail ())
    throw bad_format ("Expected unsigned");
  return val;
}

static Pos
read_pos (istream &stream)
{
  coord_t x = read_float (stream);
  coord_t y = read_float (stream);
  coord_t z = read_float (stream);
  return Pos (x, y, z);
}

static Vec
read_vec (istream &stream)
{
  dist_t x = read_float (stream);
  dist_t y = read_float (stream);
  dist_t z = read_float (stream);
  return Vec (x, y, z);
}

static Color
read_color (istream &stream)
{
  Color::component_t r = read_float (stream);
  Color::component_t g = read_float (stream);
  Color::component_t b = read_float (stream);
  return Color (r, g, b);
}


// Mesh loading

// The currently defined mesh
//
struct MeshState
{
  MeshState (Scene &_scene) : scene (_scene), mesh (0) { }
  ~MeshState () { finish (); }

  void read_polygon (istream &stream, unsigned num_vertices,
		     const Material *material);
  void read_polygon_with_normals (istream &stream, unsigned num_vertices,
				  const Material *material);

  unsigned read_vertex (istream &stream)
  {
    Pos pos = read_pos (stream);
    return mesh->add_vertex (pos, vertex_group);
  }

  unsigned read_vertex_and_normal (istream &stream)
  {
    Pos pos = read_pos (stream);
    Vec normal = read_vec (stream);
    return mesh->add_vertex (pos, normal, vertex_and_normals_group);
  }

  void finish ();

  Scene &scene;

  Mesh *mesh;

  Mesh::VertexGroup vertex_group;
  Mesh::VertexNormalGroup vertex_and_normals_group;
};

void
MeshState::read_polygon (istream &stream, unsigned num_vertices,
			 const Material *mat)
{
  if (mesh && mat != mesh->material ())
    finish ();

  if (! mesh)
    mesh = new Mesh (mat);

  unsigned v0i = read_vertex (stream);
  unsigned v1i = read_vertex (stream);

  num_vertices -= 2;

  while (num_vertices > 0)
    {
      unsigned v2i = read_vertex (stream);

      mesh->add_triangle (v0i, v1i, v2i);

      v1i = v2i;

      num_vertices--;
    }
}

void
MeshState::read_polygon_with_normals (istream &stream, unsigned num_vertices,
				      const Material *mat)
{
  if (mesh && mat != mesh->material ())
    finish ();

  if (! mesh)
    mesh = new Mesh (mat);

  unsigned v0i = read_vertex_and_normal (stream);
  unsigned v1i = read_vertex_and_normal (stream);

  num_vertices -= 2;

  while (num_vertices > 0)
    {
      unsigned v2i = read_vertex_and_normal (stream);

      mesh->add_triangle (v0i, v1i, v2i);

      v1i = v2i;

      num_vertices--;
    }
}

#include <iostream>

void
MeshState::finish ()
{
  if (mesh)
    {
      cout << "Adding mesh... nverts = " << mesh->vertices.size()
	   << ", ntriangles = " << mesh->triangles.size()
	   << endl;

      scene.add (mesh);

      vertex_group.clear ();
      vertex_and_normals_group.clear ();

      mesh = 0;
    }
}



static void
barf_if_no_material (const Material *mat, const char *op)
{
  if (! mat)
    throw bad_format (string ("no material defined for operator \"")
		      + op + "\""); 
}

void
Scene::load_nff_file (istream &stream, Camera &camera)
{
  MeshState cur_mesh (*this);
  const Material *cur_material = 0;

  set_assumed_gamma (NFF_ASSUMED_GAMMA);

  while (! stream.eof ())
    {
      char cmd_buf[10];

      stream >> cmd_buf;
      if (stream.fail ())
	break;			// probably EOF after a newline

      if (cmd_buf[0] == '#')
	// comment
	{
	  while (! stream.eof() && stream.get () != '\n')
	    ;
	  continue;
	}

      if (strcmp (cmd_buf, "v") == 0)
	//
	// Viewpoint location.  Description:
	//     "v"
	//     "from" Fx Fy Fz
	//     "at" Ax Ay Az
	//     "up" Ux Uy Uz
	//     "angle" angle
	//     "hither" hither
	//     "resolution" xres yres
	//
	{
	  read_required_kw (stream, "from");
	  Pos pos = read_pos (stream);

	  read_required_kw (stream, "at");
	  Pos targ = read_pos (stream);

	  read_required_kw (stream, "up");
	  Vec up = read_vec (stream);

	  read_required_kw (stream, "angle");
	  float fov_y = read_float (stream) * M_PI / 180;

	  read_required_kw (stream, "hither");
	  read_float (stream);

	  read_required_kw (stream, "resolution");
	  read_unsigned (stream);
	  read_unsigned (stream);

	  camera.move (pos);
	  camera.point (targ, up);
	  camera.set_vert_fov (fov_y);
	}

      else if (strcmp (cmd_buf, "b") == 0)
	//
	// Background color.  A color is simply RGB with values between 0 and 1:
	//     "b" R G B
	//
	{
	  set_background (read_color (stream));
	}

      else if (strcmp (cmd_buf, "l") == 0)
	//
	// Positional light.  A light is defined by XYZ position.  Description:
	//     "l" X Y Z [R G B]
	//
	{
	  Pos pos = read_pos (stream);

	  if (stream.peek () == '\n')
	    add (new Light (pos, NFF_LIGHT_INTENS));
	  else
	    add (new Light (pos, NFF_LIGHT_INTENS, read_color (stream)));
	}
      else if (strcmp (cmd_buf, "f") == 0)
	//
	// Fill color and shading parameters.  Description:
	//     "f" red green blue Kd Ks Shine T index_of_refraction
	//
	{
	  Color color = read_color (stream);

	  float diffuse = read_float (stream);
	  float specular = read_float (stream);
	  float phong_exp = read_float (stream);
	  float transmittance = read_float (stream);
	  float ior = read_float (stream);

	  color *= diffuse;

	  const LightModel *lmodel = 0;
	  if (phong_exp <= E || phong_exp > 1000)
	    lmodel = Material::lambert;
	  else
	    lmodel = Material::phong (phong_exp, specular);

	  if (specular > E)
	    cur_material = new Mirror (specular, color, lmodel);
	  else
	    cur_material = new Material (color, lmodel);

	  add (cur_material);
	}

      else if (strcmp (cmd_buf, "c") == 0)
	//
	// Cylinder or cone.  A cylinder is defined as having a radius and
	//     an axis defined by two points, which also define the top and
	//     bottom edge of the cylinder.  A cone is defined similarly,
	//     the difference being that the apex and base radii are
	//     different.  The apex radius is defined as being smaller than
	//     the base radius.  Note that the surface exists without
	//     endcaps.  The cone or cylinder description:
	//
	//     "c"
	//     base.x base.y base.z base_radius
	//     apex.x apex.y apex.z apex_radius
	//
	{
	  barf_if_no_material (cur_material, cmd_buf);
	}

      else if (strcmp (cmd_buf, "s") == 0)
	//
	// Sphere.  A sphere is defined by a radius and center position:
	//     "s" center.x center.y center.z radius
	//
	{
	  barf_if_no_material (cur_material, cmd_buf);
	  add (new Sphere (cur_material, read_pos(stream), read_float(stream)));
	}

      else if (strcmp (cmd_buf, "p") == 0)
	//
	// Polygon.  A polygon is defined by a set of vertices.  With these
	//     databases, a polygon is defined to have all points coplanar.
	//     A polygon has only one side, with the order of the vertices
	//     being counterclockwise as you face the polygon (right-handed
	//     coordinate system).  The first two edges must form a
	//     non-zero convex angle, so that the normal and side
	//     visibility can be determined by using just the first three
	//     vertices.  Description:
	//
	//     "p" total_vert_indexices
	//     vert1.x vert1.y vert1.z
	//     [etc. for total_vert_indexices vertices]
	//
	{
	  barf_if_no_material (cur_material, cmd_buf);
	  cur_mesh.read_polygon (stream, read_unsigned (stream), cur_material);
	}

      else if (strcmp (cmd_buf, "pp") == 0)
	//
	// Polygonal patch.  A patch is defined by a set of vertices and
	//     their normals.  With these databases, a patch is defined to
	//     have all points coplanar.  A patch has only one side, with
	//     the order of the vertices being counterclockwise as you face
	//     the patch (right-handed coordinate system).  The first two
	//     edges must form a non-zero convex angle, so that the normal
	//     and side visibility can be determined.  Description:
	//
	//     "pp" total_vert_indexices
	//     vert1.x vert1.y vert1.z norm1.x norm1.y norm1.z
	//     [etc. for total_vert_indexices vertices]
	//
	// Format:
	//     pp %d
	//     [ %g %g %g %g %g %g ] <-- for total_vert_indexices vertices
	//
	{
	  barf_if_no_material (cur_material, cmd_buf);
	  cur_mesh.read_polygon_with_normals (stream, read_unsigned (stream),
					      cur_material);
	}

      else
	throw runtime_error (string ("Unknown NFF operator: ") + cmd_buf);
    }

  cur_mesh.finish ();
}

// arch-tag: 3f073815-0f70-4721-8337-6edaced21495
