// load-3ds.cc -- Load 3ds scene file
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <string>
#include <map>

#include <lib3ds/types.h>
#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/camera.h>
#include <lib3ds/node.h>
#include <lib3ds/light.h>
#include <lib3ds/matrix.h>

#include "excepts.h"
#include "scene.h"
#include "mirror.h"
#include "plastic.h"
#include "sphere.h"
#include "mesh.h"
#include "scene.h"
#include "camera.h"
#include "phong.h"
#include "cook-torrance.h"
#include "sphere-light.h"
#include "glow.h"

#include "load-3ds.h"

using namespace snogray;
using namespace std;

// The index of refraction we use for reflective objects.
//
#define TDS_METAL_IOR			Ior (0.25, 3)



struct TdsLoader
{
  TdsLoader (Scene *_scene,
	     const MaterialMap &_user_materials = MaterialMap ())
    : scene (_scene), single_mesh (0), file (0),
      user_materials (_user_materials)
  { }
  TdsLoader (Mesh *_dest_mesh,
	     const MaterialMap &_user_materials = MaterialMap ())
    : scene (0), single_mesh (_dest_mesh), file (0),
      user_materials (_user_materials)
  { }
  ~TdsLoader () { if (file) lib3ds_file_free (file); }

  // Load 3ds scene file FILENAME into memory.
  //
  void load (const string &filename);

  // Import all meshes/lights in the 3ds scene, transformed by XFORM,
  // into the snogray scene or mesh associated with this loader.
  //
  void convert (const Xform &xform = Xform::identity);

  // Import 3ds scene objects underneath NODE, transformed by XFORM,
  // into the snogray scene or mesh associated with this loader.
  //
  void convert (Lib3dsNode *node, const Xform &xform = Xform::identity);

  void set_camera (Camera &camera, Lib3dsCamera *c, const Xform &xform);

  // Return a snogray material corresponding to the 3ds material called NAME.
  // If this loader has a scene associated with it, the material will be
  // added to it.
  //
  const Material *lookup_material (const char *name);

  // Return a snogray material corresponding to the 3ds material M.
  // If this loader has a scene associated with it, the material will be
  // added to it.
  //
  const Material *convert_material (Lib3dsMaterial *m);

  // 3ds to snogray conversion methods for various primitive types.
  //
  Vec vec (Lib3dsVector &v) { return Vec (v[0], v[1], v[2]); }
  Pos pos (Lib3dsPoint &p) { return Pos (p.pos[0], p.pos[1], p.pos[2]); }
  Pos pos (Lib3dsVector &p) { return Pos (p[0], p[1], p[2]); }
  Color color (Lib3dsRgba &rgba) { return Color (rgba[0], rgba[1], rgba[2]); }
  Color color (Lib3dsRgb &rgb) { return Color (rgb[0], rgb[1], rgb[2]); }

  // If non-null, scene to add stuff to.
  //
  Scene *scene;

  // If non-null, all meshes will be added to this mesh.
  //
  Mesh *single_mesh;

  // Handle for lib3ds data structures.
  //
  Lib3dsFile *file;

  // A mapping from material names to snogray materials, for materials
  // loaded from the 3ds file.
  //
  MaterialMap loaded_materials;

  // A mapping from material names to materials specified by the user;
  // these override materials from the 3ds file.
  //
  const MaterialMap &user_materials;
};



// Return a snogray material corresponding to the 3ds material M.
// If this loader has a scene associated with it, the material will be
// added to it.
//
const Material *
TdsLoader::convert_material (Lib3dsMaterial *m)
{
  const Material *mat;

//   cout << "Material: " << m->name << endl;
//   cout << "   shading:      " << m->shading << endl;
//   cout << "   transparency: " << m->transparency << endl;
//   cout << "   specular:     " << color (m->specular) << endl;
//   cout << "   diffuse:      " << color (m->diffuse) << endl;
//   cout << "   shininess:    " << m->shininess << endl;
//   cout << "   shin_str:     " << m->shin_strength << endl;

  Color diffuse = color (m->diffuse);

  if (m->transparency > 0)
    //
    // The plastic material we use for transparency has no real color or
    // BRDF; it only transmits light, or reflects due to Fresnel
    // reflection.
    //
    // We use the plastic's index of refraction to try to control
    // shininess: a shininess of 0 means an IOR of 1, so no Fresnel
    // reflection from the surface; a shininess of 1 means an IOR of 2,
    // which should provide some nice reflections.
    //
    mat = new Plastic (m->transparency, 1 + m->shininess);
  else
    {
      const Brdf *brdf = lambert;
      Color specular = color (m->specular);

      if (m->shading == LIB3DS_PHONG && m->shininess > 0)
	brdf = cook_torrance (specular, pow (100, -m->shininess));
      else if (m->shading == LIB3DS_METAL)
	brdf = cook_torrance (specular, pow (100, -m->shininess),
			      TDS_METAL_IOR);

      if (m->shading == LIB3DS_METAL)
	mat = new Mirror (TDS_METAL_IOR, specular, diffuse, brdf);
      else
	mat = new Material (diffuse, brdf);
    }

  if (scene)
    scene->add (mat);

  return mat;
}

// Return a snogray material corresponding to the 3ds material called NAME.
// If this loader has a scene associated with it, the material will be
// added to it.
//
const Material *
TdsLoader::lookup_material (const char *name)
{
  string sname (name);

  // Process named materials.
  //
  if (! sname.empty ())
    {
      // If the user specified something, always use that first.
      //
      if (user_materials.contains (sname))
	return user_materials.get (sname);

      // If we already loaded something with this name, just use that.
      //
      if (loaded_materials.contains (sname))
	return loaded_materials.get (sname);

      // Try to load a material from the file.
      //
      Lib3dsMaterial *m = lib3ds_file_material_by_name (file, name);

      if (m)
	{
	  const Material *mat = convert_material (m);
	  loaded_materials.add (sname, mat);
	  return mat;
	}

      // If we can't find a named material, fall through and treat it as
      // unnamed.
    }

  // Unnamed materials

  const Material *mat = user_materials.get_default ();

  if (!mat && single_mesh)
    mat = single_mesh->material;

  if (! mat)
    throw bad_format (string ("Unknown material in 3ds scene file: ") + sname);

  return mat;
}



void
TdsLoader::set_camera (Camera &camera, Lib3dsCamera *c, const Xform &xform)
{
  Vec up (0, 0, 1);

  up *= Xform::z_rotation (c->roll * M_PI / 180);

  Xform dir_xform = xform.inverse().transpose();

//   cout << "Camera:" << endl;
//   cout << "   orig pos:   " << pos (c->position) << endl;
//   cout << "   orig targ:  " << pos (c->target) << endl;
//   cout << "   orig up:    " << up << endl;
//   cout << "   xform pos:  " << pos (c->position) * xform << endl;
//   cout << "   xform targ: " << pos (c->target) * xform << endl;
//   cout << "   xform up:   " << up * dir_xform << endl;

  camera.set_vert_fov (c->fov * M_PI / 180);
  camera.move (pos (c->position) * xform);
  camera.point (pos (c->target) * xform, up * dir_xform);
}



// Import 3ds scene objects underneath NODE, transformed by XFORM,
// into the snogray scene or mesh associated with this loader.
//
void
TdsLoader::convert (Lib3dsNode *node, const Xform &xform)
{
  for (Lib3dsNode *child = node->childs; child; child = child->next)
    convert (child, xform);

  if (node->type == LIB3DS_OBJECT_NODE && strcmp (node->name,"$$$DUMMY") != 0)
    {
      Lib3dsMesh *m = lib3ds_file_mesh_by_name (file, node->name);

      if (m
#ifdef HAVE_LIB3DS_OBJ_FLAGS
	  && !(m->obj_flags & LIB3DS_OBJF_HIDDEN)
#endif
	  )
	{
	  Lib3dsMatrix N, M, X;
	  Lib3dsObjectData *d = &node->data.object;
	  lib3ds_matrix_copy (N, node->matrix);
	  lib3ds_matrix_translate_xyz (N, -d->pivot[0], -d->pivot[1], -d->pivot[2]);
	  lib3ds_matrix_copy (M, m->matrix);
	  lib3ds_matrix_inv (M);
	  lib3ds_matrix_mul (X, N, M);

	  Xform vert_xform = Xform (X) * xform;

	  Mesh *mesh = single_mesh;

	  if (! mesh)
	    mesh = new Mesh ();

	  // Add vertices without normals
	  //
	  Mesh::vert_index_t base_vertex = mesh->num_vertices ();
	  for (unsigned v = 0; v < m->points; v++)
	    mesh->add_vertex (pos (m->pointL[v]) * vert_xform);

	  // See if the mesh has _any_ smoothed faces.
	  //
	  bool smoothing = false;
	  for (unsigned t = 0; t < m->faces; t++)
	    if (m->faceL[t].smoothing)
	      {
		smoothing = true;
		break;
	      }

	  // If smoothing, calculate the number of faces adjacent to each
	  // vertex.
	  //
	  unsigned char *vert_face_counts = 0;
	  if (smoothing)
	    {
	      vert_face_counts = new unsigned char[m->points];

	      for (unsigned v = 0; v < m->points; v++)
		vert_face_counts[v] = 0;
	      for (unsigned t = 0; t < m->faces; t++)
		for (unsigned i = 0; i < 3; i++)
		  vert_face_counts[m->faceL[t].points[i]]++;
	    }

	  // Add all faces to the mesh.
	  //
	  for (unsigned t = 0; t < m->faces; t++)
	    {
	      Lib3dsFace *f = &m->faceL[t];

	      // Vertex indices of this face.
	      //
	      unsigned vind[3] = {
		base_vertex + f->points[0],
		base_vertex + f->points[1],
		base_vertex + f->points[2]
	      };

	      // If the triangle is non-smoothed but the mesh has some
	      // smoothed faces, then the triangle must have its own
	      // vertices, so make sure it does.
	      //
	      if (smoothing && !f->smoothing)
		for (unsigned i = 0; i < 3; i++)
		  if (vert_face_counts[vind[i] - base_vertex] > 1)
		    //
		    // This vertex is shared, but we need our own, so
		    // copy it, and use the copy instead.
		    {
		      // Remember that we're not sharing the old vertex.
		      //
		      vert_face_counts[vind[i] - base_vertex]--;

		      // Make the new vertex
		      //
		      vind[i] = mesh->add_vertex (mesh->vertex (vind[i]));
		    }

	      mesh->add_triangle (vind[0], vind[1], vind[2],
				  lookup_material (f->material));
	    }

	  // If smoothing, compute vertex normals.  This turns on smoothign
	  // for the whole mesh, but we made sure that only faces which
	  // should be smoothed share vertices.
	  //
	  if (smoothing)
	    {
	      mesh->compute_vertex_normals ();
	      delete vert_face_counts;
	    }

	  if (scene && !single_mesh)
	    scene->add (mesh);
	}
    }
}



// Import all meshes/lights in the 3ds scene, transformed by XFORM,
// into the snogray scene or mesh associated with this loader.
//
void 
TdsLoader::convert (const Xform &xform)
{
  for (Lib3dsNode *node = file->nodes; node; node = node->next)
    convert (node, xform);

  if (scene)
    {
      dist_t radius = 50;
      dist_t area_scale = 1 / (4 * M_PI * radius * radius);

      for (Lib3dsLight *l = file->lights; l; l = l->next)
#ifdef HAVE_LIB3DS_OBJ_FLAGS
	if (! (l->obj_flags & LIB3DS_OBJF_HIDDEN))
#endif
	{
	  const Pos loc = pos (l->position) * xform;
	  const Color intens = color (l->color) * l->multiplier * area_scale;

	  scene->add (new SphereLight (loc, radius, intens));
	  scene->add (new Sphere (scene->add (new Glow (intens)), loc, radius));
	}
    }
}

// Load 3ds scene file FILENAME into memory.
//
void
TdsLoader::load (const string &filename)
{
  if (file)
    lib3ds_file_free (file);

  file = lib3ds_file_load (filename.c_str ());

  if (! file)
    throw file_error ("Cannot load 3ds scene file");

  // Sets the time for animated files.  XXX add user "time" parameter?
  //
  lib3ds_file_eval(file, 0);
}



// Load a 3ds scene file into SCENE and CAMERA; loads all parts of the
// scene, including lights and the first camera position.
//
void
snogray::load_3ds_file (const string &filename, Scene &scene, Camera &camera)
{
  TdsLoader l (&scene);

  l.load (filename);

  // Transform vertical Z axis into our preferred vertical Y axis
  //
  Xform xform;
  xform.rotate_x (-M_PI_2);
  xform.scale (1, 1, -1);

  if (l.file->cameras)
    {
      Lib3dsCamera *c = l.file->cameras;

#ifdef HAVE_LIB3DS_OBJ_FLAGS
      while (c && (c->obj_flags & LIB3DS_OBJF_HIDDEN))
	c = c->next;
#endif

      l.set_camera (camera, c, xform);
    }

  l.convert (xform);
}

// Load meshes (and any materials they use) from a 3ds scene file into
// MESH.  Geometry is first transformed by XFORM, and materials filtered
// through MAT_MAP.
//
void
snogray::load_3ds_file (const string &filename, Mesh &mesh,
			const MaterialMap &mat_map, const Xform &xform)
{
  TdsLoader l (&mesh, mat_map);

  l.load (filename);

  // Transform vertical Z axis into our preferred vertical Y axis
  //
  Xform file_xform;
  file_xform.rotate_x (-M_PI_2);
  file_xform.scale (1, 1, -1);
  file_xform *= xform;

  l.convert (file_xform);
}

// arch-tag: 4deb9ac0-be20-4853-b232-e6ebb54c5888
