// load-3ds.cc -- Load 3ds scene file
//
//  Copyright (C) 2006-2008, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "config.h"

#include <string>
#include <vector>
#include <map>
#include <cstring>

#include <lib3ds/types.h>
#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/camera.h>
#include <lib3ds/node.h>
#include <lib3ds/light.h>
#include <lib3ds/matrix.h>

#include "util/excepts.h"
#include "material/mirror.h"
#include "material/thin-glass.h"
#include "surface/surface-group.h"
#include "surface/sphere.h"
#include "surface/mesh.h"
#include "camera/camera.h"
#include "material/phong.h"
#include "material/lambert.h"
#include "material/cook-torrance.h"
#include "material/material-dict.h"
#include "material/glow.h"

#include "load-3ds.h"

using namespace snogray;

// The index of refraction we use for reflective objects.
//
#define TDS_METAL_IOR		Ior (0.25, 3)

// A node name which should be ignored.
//
#define DUMMY_NODE_NAME		"$$$DUMMY"



namespace { // keep local to file

struct TdsLoader
{
  TdsLoader (SurfaceGroup *_scene,
	     const MaterialDict &_user_materials = MaterialDict ())
    : scene (_scene), single_mesh (0), file (0),
      user_materials (_user_materials)
  { }
  TdsLoader (Mesh *_dest_mesh,
	     const MaterialDict &_user_materials = MaterialDict ())
    : scene (0), single_mesh (_dest_mesh), file (0),
      user_materials (_user_materials)
  { }
  ~TdsLoader () { if (file) lib3ds_file_free (file); }

  // A single entry in a linked list of names (used for tracking the
  // 3ds named node hierarchy).
  //
  struct Name
  {
    Name (const char *_name, const Name *_next)
      : name ((_name && strcmp (_name, DUMMY_NODE_NAME) != 0) ? _name : ""),
	next (_next)
    { }

    bool valid () const { return !name.empty (); }

    std::string name;
    const Name *next;
  };

  // Load 3ds scene file FILENAME into memory.
  //
  void load (const std::string &filename);

  // Import all meshes/lights in the 3ds scene, transformed by XFORM,
  // into the snogray scene or mesh associated with this loader.
  //
  void convert (const Xform &xform = Xform::identity);

  // Import 3ds scene objects underneath NODE, transformed by XFORM,
  // into the snogray scene or mesh associated with this loader.
  // ENCLOSING_NAMES is a list of the names of parent nodes.
  //
  void convert (Lib3dsNode *node, const Xform &xform = Xform::identity,
		const Name *enclosing_names = 0);

  void set_camera (Camera &camera, Lib3dsCamera *c, const Xform &xform);

  // Return a snogray material for a material reference to a material
  // called NAME (may be zero for "default") in the geometric context
  // specified by the hierarchy of names in HIER_NAMES (innermost first).
  //
  // See the function documentation of TdsLoader::lookup_material for a
  // more detailed description of how exactly this is done.
  //
  Ref<const Material> lookup_material (const char *name, const Name *hier_names=0);

  // Return a snogray material corresponding to the 3ds material loaded
  // with the file called NAME.  Does not consider user materials.
  //
  Ref<const Material> lookup_file_material (const char *name);

  // Return a snogray material corresponding to the 3ds material M.
  //
  Ref<const Material> convert_material (Lib3dsMaterial *m);

  // 3ds to snogray conversion methods for various primitive types.
  //
  Vec vec (Lib3dsVector &v) { return Vec (v[0], v[1], v[2]); }
  Pos pos (Lib3dsPoint &p) { return Pos (p.pos[0], p.pos[1], p.pos[2]); }
  Pos pos (Lib3dsVector &p) { return Pos (p[0], p[1], p[2]); }
  Color color (Lib3dsRgba &rgba) { return Color (rgba[0], rgba[1], rgba[2]); }
  Color color (Lib3dsRgb &rgb) { return Color (rgb[0], rgb[1], rgb[2]); }

  // If non-null, scene to add stuff to.
  //
  SurfaceGroup *scene;

  // If non-null, all meshes will be added to this mesh.
  //
  Mesh *single_mesh;

  // Handle for lib3ds data structures.
  //
  Lib3dsFile *file;

  // A mapping from material names to snogray materials, for materials
  // loaded from the 3ds file.
  //
  MaterialDict loaded_materials;

  // A mapping from material names to materials specified by the user;
  // these override materials from the 3ds file.
  //
  const MaterialDict &user_materials;
};

} // namespace


// TdsLoader::convert_material

// Return a snogray material corresponding to the 3ds material M.
//
Ref<const Material>
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

  if (m->transparency > 0)
    //
    // The thin_glass material we use for transparency has no real
    // color; it only transmits light, or reflects due to Fresnel
    // reflection.
    //
    // We use the plastic's index of refraction to try to control
    // shininess: a shininess of 0 means an IOR of 1, so no Fresnel
    // reflection from the surface; a shininess of 1 means an IOR of
    // 2, which should provide some nice reflections.
    //
    mat = new ThinGlass (m->transparency, 1 + m->shininess);
  else
    {
      Color diffuse = color (m->diffuse);
      Color specular = color (m->specular);

      if (m->shading == LIB3DS_PHONG && m->shininess > 0)
	mat = new CookTorrance (diffuse, specular, pow (100, -m->shininess));
      else if (m->shading == LIB3DS_METAL)
	mat = new Mirror (TDS_METAL_IOR, specular,
			  new CookTorrance (diffuse, specular,
					    pow (100, -m->shininess),
					    TDS_METAL_IOR));
      else
	mat = new Lambert (diffuse);
    }

  return mat;
}


// TdsLoader::lookup_file_material

// Return a snogray material corresponding to the 3ds material loaded with
// the file called NAME.  Does not consider user materials.
//
Ref<const Material>
TdsLoader::lookup_file_material (const char *name)
{
  std::string mat_name (name);

  // If we already loaded something with this name, just use that.
  //
  if (loaded_materials.contains (mat_name))
    return loaded_materials.get (mat_name, 0);

  // Try to load a material from the file.
  //
  Lib3dsMaterial *m = lib3ds_file_material_by_name (file, name);

  if (m)
    {
      Ref<const Material> mat = convert_material (m);
      loaded_materials.add (mat_name, mat);
      return mat;
    }

  return Ref<const Material> (); // null ref
}


// TdsLoader::lookup_material

// Return a snogray material for a material reference to a material
// called NAME (may be zero for "default") in the geometric context
// specified by the hierarchy of names in HIER_NAMES (innermost first).
//
// The resulting material can come from either user specified names or
// names loaded with the 3ds file.  The search order is (where GN0
// ... GNn are the node names, from innermost to outermost, and MAT_NAME
// is the name used in the material reference):
//
//   step1:
//     user_materials[GN0 + "." + MAT_NAME]
//     user_materials[GN1 + "." + MAT_NAME]
//       ...
//     user_materials[GNn + "." + MAT_NAME]
//   step2:
//     user_materials[MAT_NAME]
//   step3:
//     loaded_materials[MAT_NAME]
//   step4:
//     user_materials[GN0]
//     user_materials[GN1]
//       ...
//     user_materials[GNn]
//   step5:
//     default_user_material
//
// A user name->material mapping ("user_materials" above) may be a
// "negative" entry where the material mapped to is NULL; finding such a
// NULL material mapping in step1 or step2 causes the search to stop and
// skip directly to step4 without considering materials loaded with the
// file ("loaded_materials" in step3).  This behavior is intended to
// make it easy to override useless "boring default" material references
// that exist in many 3ds files.
//
// If any user mapping is found, even NULL, step5 (the final default) is
// skipped.
//
// Even if a non-material is found in steps 1-3, step4 is still
// performed, but only negative (NULL) mappings are considered; if one
// is found, then that overrides the material found.  This behavior is
// intended to make it easy to suppress rendering of an entire object
// even if it's composed of many materials (some of which may be shared
// with other objects).
//
// The final material returned may be NULL, in which case no surface is
// rendered.
//
Ref<const Material>
TdsLoader::lookup_material (const char *name, const Name *hier_names)
{
  std::string mat_name (name);
  Ref<const Material> mat;
  bool found_user_mapping = false;

  // If this is a named material reference, first lookup materials by
  // name.
  //
  if (! mat_name.empty ())
    {
      // Step 1:  Look for a user material mapping with a combined
      // geometry + material name.
      //
      for (const Name *hn = hier_names;
	   hn && !found_user_mapping; hn = hn->next)
	if (hn->valid ())
	  {
	    std::string geom_mat_name (hn->name);
	    geom_mat_name += ':';
	    geom_mat_name += mat_name;

	    if (user_materials.contains (geom_mat_name))
	      {
		mat = user_materials.get (geom_mat_name, 0);
		found_user_mapping = true;
	      }
	  }

      // Step 2:  Look for a user material mapping using only a material
      // name.
      //
      if (!found_user_mapping && user_materials.contains (mat_name))
	{
	  mat = user_materials.get (mat_name, 0);
	  found_user_mapping = true;
	}

      // Step 3:  Look for a named material definition loaded from the
      // file.
      //
      if (! found_user_mapping)
	mat = lookup_file_material (name);
    }

  // Now consider unnamed materials if necessary

  // Step 4:  Look for a user material mapping using only the object name.
  //
  for (const Name *hn = hier_names; hn; hn = hn->next)
    if (hn->valid () && user_materials.contains (hn->name))
      {
	Ref<const Material> obj_mat = user_materials.get (hn->name, 0);
	
	// If we already found some material in steps 1-3, OBJ_MAT
	// overrides it only if it's NULL (providing the ability to
	// suppress earlier mappings).
	//
	if (!mat || !obj_mat)
	  {
	    mat = obj_mat;
	    found_user_mapping = true;
	    break;
	  }
      }

  // Step 5:  As a last-ditch effort, try a default material.
  //
  if (!found_user_mapping && !mat)
    mat = user_materials.get_default ();
#if 0
  if (!found_user_mapping && !mat && single_mesh)
    mat = &*single_mesh->material;
#endif

  return mat;
}



void
TdsLoader::set_camera (Camera &camera, Lib3dsCamera *c, const Xform &xform)
{
  Vec up (0, 0, 1);

  up.transform (Xform::z_rotation (float (c->roll) * (PIf / 180)));

  Xform dir_xform = xform.inverse().transpose();

//   cout << "Camera:" << endl;
//   cout << "   orig pos:   " << pos (c->position) << endl;
//   cout << "   orig targ:  " << pos (c->target) << endl;
//   cout << "   orig up:    " << up << endl;
//   cout << "   xform pos:  " << xform (pos (c->position)) << endl;
//   cout << "   xform targ: " << xform (pos (c->target))  << endl;
//   cout << "   xform up:   " << dir_xform (up)  << endl;

  camera.set_vert_fov (float (c->fov) * (PIf / 180));
  camera.move (xform (pos (c->position)));
  camera.point (xform (pos (c->target)), dir_xform (up));
}



namespace { // keep local to file

// A structure to keep track of per-vertex information while loading in
// a 3ds mesh.
//
struct VertInfo
{
  VertInfo (int _smoothing = 0, bool _used = false)
    : used (_used), smoothing (_smoothing), next_split_vertex (0)
  { }

  // True if this vertex has already been used for at least one
  // triangle.  Note that we can't use (smoothing != 0) to keep track
  // of this information, because a face may have its smoothing flags
  // set to zero (which would mean never share vertices).
  //
  bool used;

  // The vertex index in our mesh.  Only valid if USED is true.
  //
  Mesh::vert_index_t index;

  // Smoothing flags for this vertex.
  //
  int smoothing;

  // Either the vertex-info index of the next vertex (with different
  // smoothing flags) which was split from this one, or zero.
  //
  int next_split_vertex;
};

} // namespace


// Import 3ds scene objects underneath NODE, transformed by XFORM,
// into the snogray scene or mesh associated with this loader.
// ENCLOSING_NAMES is a list of the names of parent nodes.
//
void
TdsLoader::convert (Lib3dsNode *node, const Xform &xform,
		    const Name *enclosing_names)
{
#ifdef HAVE_LIB3DS_NODE_HIDDEN_FLAG
  if (node->flags1 & LIB3DS_HIDDEN)
    return;
#endif

  const Name hier_names (node->name, enclosing_names);

  for (Lib3dsNode *child = node->childs; child; child = child->next)
    convert (child, xform, &hier_names);

  if (node->type == LIB3DS_OBJECT_NODE
      && strcmp (node->name, DUMMY_NODE_NAME) != 0)
    {
      Lib3dsMesh *m = lib3ds_file_mesh_by_name (file, node->name);

      if (m
#ifdef HAVE_LIB3DS_OBJ_FLAGS
	  && !(m->obj_flags & LIB3DS_OBJF_HIDDEN)
#elif HAVE_LIB3DS_OBJECT_FLAGS
	  && !(m->object_flags & LIB3DS_OBJECT_HIDDEN)
#endif
	  )
	{
	  Lib3dsObjectData *d = &node->data.object;
	  Lib3dsMatrix M, X;
#if HAVE_LIB3DS_MATRIX_MULT
	  lib3ds_matrix_copy (X, node->matrix);
	  lib3ds_matrix_translate_xyz (X, -d->pivot[0], -d->pivot[1], -d->pivot[2]);
	  lib3ds_matrix_copy (M, m->matrix);
	  lib3ds_matrix_inv (M);
	  lib3ds_matrix_mult (X, M);
#else
	  Lib3dsMatrix N;
	  lib3ds_matrix_copy (N, node->matrix);
	  lib3ds_matrix_translate_xyz (N, -d->pivot[0], -d->pivot[1], -d->pivot[2]);
	  lib3ds_matrix_copy (M, m->matrix);
	  lib3ds_matrix_inv (M);
	  lib3ds_matrix_mul (X, N, M);
#endif

	  Xform vert_xform = xform (Xform (X));

	  // Get the actual mesh.
	  //
	  Mesh *mesh = single_mesh;

	  // Keep track of smoothing flags applied to each vertex; we
	  // must split vertices in case two faces with different
	  // smoothing flags initially share a vertex.  This algorithm
	  // doesn't support overlapping sets of smoothing flags, but
	  // those seem to be rare anyway.
	  //
	  std::vector<VertInfo> vert_info (m->points);

	  // Simple cache to avoid the most repetitive material lookups.
	  //
	  Ref<const Material> cached_mat;
	  char cached_mat_name[64]; // 64 is from the 3DS file standard
	  cached_mat_name[0] = '\0';

	  // Add all faces to the mesh.
	  //
	  for (unsigned t = 0; t < m->faces; t++)
	    {
	      Lib3dsFace *f = &m->faceL[t];

	      // Find a material to use.  Faces _without_ materials are
	      // ignored (not added to the mesh); in general 3ds files
	      // define all their materials, so this should only occur
	      // if the user has overridden some of the materials.
	      //
	      if (strcmp (f->material, cached_mat_name) != 0)
		{
		  strcpy (cached_mat_name, f->material);
		  cached_mat = lookup_material (f->material, &hier_names);
		}
	      if (! cached_mat)
		continue;	// No material, don't use this face

	      // Create the mesh if necessary.  This is the only time we
	      // can actually set the material.
	      //
	      if (! mesh)
		mesh = new Mesh (cached_mat);
#if 0
	      else if (&*cached_mat != &*mesh->material)
		throw file_error ("Mesh cannot contain more than one material");
#endif

	      // Indices into vert_info for the vertices in this face.
	      //
	      unsigned vind[3] = { f->points[0], f->points[1], f->points[2] };

	      // For each triangle vertex, see if the currently active
	      // smoothing flags for that vertex are compatible with the
	      // face's smoothing flags.  If not, we need to either
	      // change it to be a previously split-off vertex with
	      // compatible smoothing flags, or split-off a new vertex
	      // with the face's smoothing flags.
	      //
	      for (unsigned i = 0; i < 3; i++)
		{
		  unsigned vi = vind[i];

		  while (! (vert_info[vi].smoothing & f->smoothing))
		    if (vert_info[vi].next_split_vertex)
		      {
			// Try the next previously split-off vertex.

			vi = vert_info[vi].next_split_vertex;
		      }
		    else
		      {
			// No more previously split-off vertices, so we
			// must stop the loop.

			// If the vertex at VI has already been used, we
			// must add a new vertex to the end of VERT_INFO.
			//
			if (vert_info[vi].used)
			  {
			    // Index of the new entry.
			    //
			    unsigned new_vi = vert_info.size ();

			    // Make the new entry.
			    //
			    vert_info.push_back (VertInfo ());

			    // Make the previous entry point to it.
			    //
			    vert_info[vi].next_split_vertex = new_vi;

			    vi = new_vi;
			  }

			// This vertex gets our smoothing bits.
			//
			vert_info[vi].smoothing = f->smoothing;

			break;
		      }

		  // If this vertex has never been used before, add it
		  // to the final mesh.
		  //
		  if (! vert_info[vi].used)
		    {
		      vert_info[vi].index
			= mesh->add_vertex (
				  vert_xform (pos (m->pointL[f->points[i]])));
		      vert_info[vi].used = true;
		    }

		  // Update VIND
		  //
		  vind[i] = vi;
		}

	      // Add the triangle!
	      //
	      Mesh::vert_index_t v0 = vert_info[vind[0]].index;
	      Mesh::vert_index_t v1 = vert_info[vind[1]].index;
	      Mesh::vert_index_t v2 = vert_info[vind[2]].index;

	      mesh->add_triangle (v0, v1, v2);
	    }

	  // Compute vertex normals.  This turns on smoothing for the
	  // whole mesh, but we made sure that only faces which should
	  // be smoothed share vertices.
	  //
	  mesh->compute_vertex_normals ();

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
      dist_t sc_rad = radius / 10000;
      dist_t area_scale = 1 / (dist_t (4 * PI) * sc_rad * sc_rad);

      for (Lib3dsLight *l = file->lights; l; l = l->next)
#ifdef HAVE_LIB3DS_OBJ_FLAGS
	if (! (l->obj_flags & LIB3DS_OBJF_HIDDEN))
#elif HAVE_LIB3DS_OBJECT_FLAGS
	if (! (l->object_flags & LIB3DS_OBJECT_HIDDEN))
#endif
	{
	  const Pos loc = xform (pos (l->position));
	  const Color intens = color (l->color) * l->multiplier * area_scale;

	  scene->add (new Sphere (new Glow (intens), loc, radius));
	}
    }
}

// Load 3ds scene file FILENAME into memory.
//
void
TdsLoader::load (const std::string &filename)
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
snogray::load_3ds_file (const std::string &filename,
			SurfaceGroup &scene, Camera &camera,
			const ValTable &)
{
  TdsLoader l (&scene);

  l.load (filename);

  // Transform vertical Z axis into our preferred vertical Y axis
  //
  Xform xform;
  xform.rotate_x (-PI/2);
  xform.scale (1, 1, -1);

  if (l.file->cameras)
    {
      Lib3dsCamera *c = l.file->cameras;

#ifdef HAVE_LIB3DS_OBJ_FLAGS
      while (c && (c->obj_flags & LIB3DS_OBJF_HIDDEN))
	c = c->next;
#elif HAVE_LIB3DS_OBJECT_FLAGS
      while (c && (c->object_flags & LIB3DS_OBJECT_HIDDEN))
	c = c->next;
#endif

      l.set_camera (camera, c, xform);
    }

  l.convert (xform);
}

// Load meshes (and any materials they use) from a 3ds scene file inot MESH.
//
void
snogray::load_3ds_file (const std::string &filename,
			Mesh &mesh, const ValTable &)
{
  TdsLoader l (&mesh);

  l.load (filename);

  // Transform vertical Z axis into our preferred vertical Y axis
  //
  Xform file_xform;
  file_xform.rotate_x (-PI/2);
  file_xform.scale (1, 1, -1);

  l.convert (file_xform);
}

// arch-tag: 4deb9ac0-be20-4853-b232-e6ebb54c5888
