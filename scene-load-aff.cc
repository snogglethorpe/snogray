// scene-load-aff.cc -- Load AFF or NFF scene file
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// "NFF" (Neutral File Format) is the scene file format used by Eric Haines'
// "Standard Procedural Databases" (SPD) project.
//
// "AFF" (Animated File Format) is an extension of AFF used by the
// "BART: A Benchmark for Animated Ray Tracing" project, by Jonas Lext,
// Ulf Assarsson, and Tomas Möller.
//
// AFF is strictly upward compatible with NFF, so we use the same code
// to load both.
//

#include <fstream>
#include <string>

#include "snogmath.h"
#include "excepts.h"
#include "scene.h"
#include "mirror.h"
#include "glass.h"
#include "sphere.h"
#include "mesh.h"
#include "phong.h"
#include "sphere-light.h"
#include "glow.h"

using namespace snogray;
using namespace std;

// How bright we make lights
//
#define AFF_LIGHT_INTENS		100

// The .aff files we have seen all use wacky "gamma adjusted" lighting,
// So try to compensate for that here.
//
#define AFF_ASSUMED_GAMMA		2.2

// We scale phong highlights this much (over the Ks parameter) -- .nff
// files don't have a separate "phong intensity" parameter in material
// descriptions, whereas other SPD output formats do (and SPD actually
// uses it), so we just pick something arbitrary.
//
#define AFF_PHONG_ADJ			1

// Snogray defines the filtering effect of transparent surfaces in terms of
// absorption of the contained volume, rather than a surface filtering term
// (as is more typical, and used by NFF files).  This constant defines a
// scaling factor for the absorption calculated from the NFF transmittance
// value.
//
#define AFF_MEDIUM_ABSORPTION		10

// The index of refraction we use for reflective objects.
//
#define AFF_MIRROR_IOR			Ior (0.25, 3)

// How many sides cones have
//
#define AFF_CONE_SIDES			64
#define AFF_CONE_MAX_CIRC_DIFF		0.2


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


// Cone/cylinder creation

// Returns the position of a point on a disk's edge at rotation THETA
// (0 - 2*PI).  AXIS1 and AXIS2 are two perpendicular axes of the disk.
//
static Pos
disk_edge_point (float theta, const Pos &center,
		 const Vec &axis1, const Vec &axis2)
{
  return center + axis1 * cos (theta) + axis2 * sin (theta);
}

static void
add_cone_sides (Mesh *mesh,
	       const Pos &base_pos, dist_t base_radius,
	       const Pos &apex_pos, dist_t apex_radius)
{
  float base_circ = 2 * M_PIf * base_radius;
  float apex_circ = 2 * M_PIf * apex_radius;

  Vec axis = apex_pos - base_pos;

  if (abs (base_circ - apex_circ) / base_circ > AFF_CONE_MAX_CIRC_DIFF)
    {
      Pos mid_pos = base_pos + axis / 2;
      dist_t mid_radius = (base_radius + apex_radius) / 2;
      add_cone_sides (mesh, base_pos, base_radius, mid_pos, mid_radius);
      add_cone_sides (mesh, mid_pos, mid_radius, apex_pos, apex_radius);
      return;
    }

  float theta_step = 2 * M_PIf / AFF_CONE_SIDES;

  Vec base_axis1 = base_radius * axis.perpendicular().unit ();
  Vec base_axis2 = base_radius * cross (axis, base_axis1).unit ();
  Vec apex_axis1 = apex_radius * axis.perpendicular().unit ();
  Vec apex_axis2 = apex_radius * cross (axis, apex_axis1).unit ();

  Mesh::vert_index_t prev_base_vert = 0, first_base_vert = 0;
  Mesh::vert_index_t prev_apex_vert = 0, first_apex_vert = 0;

  for (float i = 0; i < AFF_CONE_SIDES; i++)
    {
      float theta = i * theta_step;

      Pos base_point
	= disk_edge_point (theta, base_pos, base_axis1, base_axis2);
      Pos apex_point
	= disk_edge_point (theta, // + theta_step / 2,
			   apex_pos, apex_axis1, apex_axis2);

      Vec surf_vec = apex_point - base_point;

      Vec base_vec = base_point - base_pos;
      Vec base_tangent = cross (surf_vec, base_vec).unit ();
      Vec base_normal = cross (surf_vec, base_tangent).unit ();

      Mesh::vert_index_t base_vert = mesh->add_vertex (base_point, base_normal);

      Vec apex_vec = apex_point - apex_pos;
      Vec apex_tangent = cross (surf_vec, apex_vec).unit ();
      Vec apex_normal = cross (surf_vec, apex_tangent).unit ();

      Mesh::vert_index_t apex_vert = mesh->add_vertex (apex_point, apex_normal);

      if (i == 0)
	{
	  first_base_vert = base_vert;
	  first_apex_vert = apex_vert;
	}
      else
	{
	  mesh->add_triangle (prev_base_vert, base_vert, prev_apex_vert);
	  mesh->add_triangle (base_vert, apex_vert, prev_apex_vert);
	}

      prev_base_vert = base_vert;
      prev_apex_vert = apex_vert;
    }

  mesh->add_triangle (prev_base_vert, first_base_vert, prev_apex_vert);
  mesh->add_triangle (first_base_vert, first_apex_vert, prev_apex_vert);
}

static Mesh *
make_cone (const Material *mat,
	       const Pos &base_pos, dist_t base_radius,
	       const Pos &apex_pos, dist_t apex_radius)
{
  Mesh *mesh = new Mesh (mat);
  add_cone_sides (mesh, base_pos, base_radius, apex_pos, apex_radius);
  return mesh;
}


// Mesh loading

// The currently defined mesh
//
struct MeshState
{
  MeshState (Scene &_scene) : scene (_scene), mesh (0) { }
  ~MeshState () { finish (); }

  void read_polygon (istream &stream, const Material *material,
		     unsigned num_vertices, bool read_normals = false);

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
MeshState::read_polygon (istream &stream, const Material *mat,
			 unsigned num_vertices, bool read_normals)
{
  if (! mesh)
    mesh = new Mesh ();

  // NFF files use a right-handed coordinate system.
  //
  mesh->left_handed = false;

  vector<Mesh::vert_index_t> verts;

  for (unsigned i = 0; i < num_vertices; i++)
    if (read_normals)
      verts.push_back (read_vertex_and_normal (stream));
    else
      verts.push_back (read_vertex (stream));

  if (num_vertices == 3)
    mesh->add_triangle (verts[0], verts[1], verts[2], mat);
  else if (num_vertices == 4)
    {
      mesh->add_triangle (verts[0], verts[1], verts[2], mat);
      mesh->add_triangle (verts[2], verts[3], verts[0], mat);
    }
  else
    {
      Pos center;

      for (unsigned i = 0; i < num_vertices; i++)
	center += Vec (mesh->vertex (verts[i]));

      unsigned center_vert
	= mesh->add_vertex (center / num_vertices, vertex_group);

      for (unsigned i = 0; i < num_vertices; i++)
	mesh->add_triangle (center_vert,
			    verts[i], verts[(i + 1) % num_vertices], mat);

    }
}

#include <iostream>

void
MeshState::finish ()
{
  if (mesh)
    {
      // mesh->compute_vertex_normals ();

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
Scene::load_aff_file (istream &stream, Camera &camera)
{
  MeshState cur_mesh (*this);
  const Material *cur_material = 0;

  set_assumed_gamma (AFF_ASSUMED_GAMMA);
  camera.set_z_mode (Camera::Z_DECREASES_FORWARD);

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
	  Color col = read_color (stream);
	  if (! bg_set)
	    set_background (col);
	}

      else if (strcmp (cmd_buf, "l") == 0)
	//
	// Positional light.  A light is defined by XYZ position.
	//
	// Description:
	//     "l" X Y Z [R G B]
	//
	{
	  Pos pos = read_pos (stream);

	  Color intens (AFF_LIGHT_INTENS);
	  if (stream.peek () != '\n')
	    intens *= read_color (stream);

	  dist_t radius = 1;
	  dist_t area = 4 * M_PI * radius * radius;

	  intens /= area;

	  add (new SphereLight (pos, radius, intens));
	  add (new Sphere (add (new Glow (intens)), pos, radius));
	}
      else if (strcmp (cmd_buf, "f") == 0 || strcmp (cmd_buf, "fm") == 0)
	//
	// Fill color and shading parameters.
	//
        // Description:
	//     "f" red green blue Kd Ks Shine T index_of_refraction
	//	"fm" amb_r amb_g amb_b diff_r diff_g diff_b spec_r spec_g spec_b Shine T index_of_refraction
	//
	// Format:
	//	f %g %g %g %g %g %g %g %g
	//	fm %g %g %g %g %g %g %g %g %g %g %g %g
	//
	{
	  Color diffuse, specular, ambient;

	  if (strcmp (cmd_buf, "f") == 0)
	    {
	      Color color = read_color (stream);
	      diffuse = color * read_float (stream);
	      specular = read_float (stream);
	    }
	  else
	    {
	      diffuse = read_color (stream);
	      specular = read_color (stream);
	    }
	    
	  float phong_exp = read_float (stream);
	  float transmittance = read_float (stream);
	  float ior = read_float (stream);

	  const Brdf *brdf = lambert;
	  if (phong_exp > Eps && phong_exp < 10000)
	    brdf = phong (specular * AFF_PHONG_ADJ, phong_exp);

	  if (transmittance > Eps)
	    cur_material
	      = new Glass (
		      Medium (ior,
			      AFF_MEDIUM_ABSORPTION
			      * -log (max (0.00001f,
					   min (transmittance, 1.f)))));
	  else if (specular.intensity() > Eps)
	    cur_material = new Mirror (AFF_MIRROR_IOR, specular, diffuse, brdf);
	  else
	    cur_material = new Material (diffuse, brdf);

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

	  // Ugh... make a cylinder...

	  Pos base_pos = read_pos (stream);
	  float base_radius = read_float (stream);
	  Pos apex_pos = read_pos (stream);
	  float apex_radius = read_float (stream);

	  if (base_radius > Eps && apex_radius > Eps)
	    add (make_cone (cur_material,
			    base_pos, base_radius, apex_pos, apex_radius));
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
	  cur_mesh.read_polygon (stream, cur_material, read_unsigned (stream));
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
	  cur_mesh.read_polygon (stream, cur_material, read_unsigned (stream),
				 true);
	}

      else
	throw runtime_error (string ("Unknown AFF/NFF operator: ") + cmd_buf);
    }

  cur_mesh.finish ();
}

// arch-tag: 3f073815-0f70-4721-8337-6edaced21495
