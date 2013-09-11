-- surface.lua -- Surface support
--
--  Copyright (C) 2012, 2013  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

-- module
--
local surface = {}

-- imports
--
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"
local transform = require 'snogray.transform'
local accel = require 'snogray.accel'

local pos, vec = raw.Pos, raw.Vec
local cross = raw.cross
local midpoint = raw.midpoint


--
-- Surfaces are the representation of object geometry in a scene.
--
-- Although they usually enclose some volume, they are inherently
-- two-dimensional, like a thin shell (although sometimes certain
-- materials, like material.glass, can be used to achieve volumetric
-- effects).
--
-- Surfaces have no effect on rendering until they are added to the
-- scene, using the scene:add (SURFACE) method.
--

--
-- All surfaces implement the following methods:
--
--   SURFACE:bbox () -- Return an axis-aligned bounding-box enclosing
--			SURFACE
--


----------------------------------------------------------------
-- Primitive surface types
--

--
-- In all of the following surface-creation functions, the MAT
-- parameter is a material, as returned from the snogray.material
-- module.
--

-- surface.sphere -- Return a sphere surface
--
-- args: (MAT, CENTER, RADIUS)
--   or: (MAT, CENTER, AXIS)
--   or: (MAT, CENTER, RADIUS, FRAME)
--
-- Return a sphere object with a center at position CENTER, and a
-- radius defined by either the scalar RADIUS or the magnitude of the
-- vector AXIS.
--
-- For purposes of texture-mapping, spheres have an "axis" (the
-- North-South pole), and an orientation around that axis.  If passed
-- just a simple scalar radius the sphere's axis lies along the
-- z-axis.  If passed a vector, AXIS, that defines the sphere's axis.
-- If passed a frame, FRAME, then the sphere's axis lies along FRAME's
-- z-axis, and the origin of the sphere's texture U-coordinate at
-- location (1,0,0) in FRAME.
--
surface.sphere = raw.Sphere

-- surface.sphere2 -- Return a sphere surface
--
-- args: (MAT, OBJ_TO_WORLD_XFORM)
--
-- An alternative sphere surface defined by the transform matrix
-- OBJ_TO_WORLD_XFORM, which maps a canonical 1-unit radius sphere at
-- the origin to world space.
--
-- TODO: merge this with surface.sphere.
--
surface.sphere2 = raw.Sphere2

-- surface.tripar -- Return a triangle/parallelogram surface
--
-- args: (MAT, CORNER, EDGE1, EDGE2, IS_PARALLELOGRAM)
--
-- Return a triangle or parallelogram surface.  If IS_PARALLELOGRAM is
-- false, it is a triangle (see surface.triangle), otherwise it is a
-- parallelogram (see surface.parallelogram).
--
surface.tripar = raw.Tripar

-- surface.triangle -- Return a triangle surface
--
-- args: (MAT, CORNER, EDGE1, EDGE2)
--
-- Return a triangle surface with two edges defined by vectors EDGE1
-- and EDGE2 extending from the position CORNER.
--
function surface.triangle (mat, v0, e1, e2)
   return surface.tripar (mat, v0, e1, e2, false)
end

-- surface.parallelogram -- Return a parallelogram surface
--
-- args: (MAT, CORNER, EDGE1, EDGE2)
--
-- Return a parallelogram surface with its first two edges defined by
-- vectors EDGE1 and EDGE2 extending from the position CORNER.  The
-- remaining two edges are parallel to the first two.
--
function surface.parallelogram (mat, v0, e1, e2)
   return surface.tripar (mat, v0, e1, e2, true)
end

-- surface.rectangle -- Alias for surface.parallelogram
--
surface.rectangle = surface.parallelogram

-- surface.parallelepiped -- Return a parallelepiped surface
--
-- args: (MAT, CORNER, UP_EDGE, RIGHT_EDGE, FWD_EDGE)
--
-- Return a parallelepiped surface (the three-dimensional analogue of
-- a parallelogram) with its first three edges defined by vectors
-- UP_EDGE, RIGHT_EDGE, and FWD_EDGE extending from the position
-- CORNER.  The remaining three edges are parallel to the first three.
--
function surface.parallelepiped (mat, corner, up, right, fwd)
   return surface.group {
      surface.parallelogram (mat, corner, right, up);
      surface.parallelogram (mat, corner, up, fwd);
      surface.parallelogram (mat, corner, fwd, right);

      surface.parallelogram (mat, corner + up, right, fwd);
      surface.parallelogram (mat, corner + right, fwd, up);
      surface.parallelogram (mat, corner + fwd, up, right);
   }
end

-- surface.cube -- Alias for surface.parallelepiped
--
surface.cube = surface.parallelepiped

-- surface.box -- Alias for surface.parallelepiped
--
surface.box = surface.parallelepiped

-- Given the "axis" of a disk (a vector orthogonal to its surface) and
-- its radius, return two radius vectors appropriate for an ellipse
-- surface.
--
local function disk_radii (axis, radius)
   local rad1 = axis:perpendicular():unit() * radius
   local rad2 = cross (rad1:unit(), axis:unit()) * radius
   return rad1, rad2
end

-- surface.ellipse -- Return an elliptical / disk surface.
--
-- args: (MAT, OBJ_TO_WORLD_XFORM)
--   or: (MAT, BASE, RADIUS1, RADIUS2)
--   or: (MAT, BASE, AXIS, RADIUS)
--
-- If the OBJ_TO_WORLD_XFORM form is used, OBJ_TO_WORLD_XFORM is a
-- transform transforming a canonical ellipse centered at the origin
-- and extending one unit in the x- and y-directions, into the desired
-- ellipse in world space.
--
-- Otherwise BASE is the position of the center of the ellipse.
-- RADIUS1 and RADIUS2 are vectors pointing from BASE along the minor
-- and major axes.  Alternatively, AXIS is a vector perpendicular to
-- the ellipse's surface, from BASE (only its direction is used) and
-- RADIUS is the single radius of a disk.
--
function surface.ellipse (mat, ...)
   -- Detect the AXIS+RADIUS case, and handle appropriately.
   --
   local num_rest_args = select ('#', ...)
   if num_rest_args == 3 then
      local base = select (1, ...)
      local axis = select (2, ...)
      local radius = select (3, ...)

      if type (radius) == 'number' then
	 return raw.Ellipse (mat, base, disk_radii (axis, radius))
      end
      -- ... otherwise just fall through
   end

   -- Directly call the underlying constructor.
   --
   return raw.Ellipse (mat, ...)
end

-- surface.disk -- Alias for surface.ellipse
--
surface.disk = surface.ellipse

-- surface.cylinder -- Return a cylindrical surface (with no ends)
--
-- args: (MAT, OBJ_TO_WORLD_XFORM)
--   or: (MAT, BASE, AXIS, RADIUS)
--
-- If the OBJ_TO_WORLD_XFORM form is used, OBJ_TO_WORLD_XFORM is a
-- transform transforming a canonical cylinder centered at the origin,
-- with an ais extending along the z-axis, and an extent of one unit
-- in each direction.
--
-- Otherwise BASE is the position of the center of the bottom of the
-- cylinder, AXIS is a vector extending from BASE to the center of the
-- cylinder's top, and RADIUS is the cylinder's radius perpendicular
-- to AXIS.
--
surface.cylinder = raw.Cylinder

-- surface.solid_cylinder - Return a cylindrical surface (with ends)
--
-- args: (MAT, OBJ_TO_WORLD_XFORM, END_MAT1, END_MAT2)
--   or: (MAT, BASE, AXIS, RADIUS, END_MAT1, END_MAT2)
--
-- The arguments are the same as surface.cylinder, with the addition
-- of optional arguments END_MAT1 and END_MAT2.  END_MAT1 and END_MAT2
-- can be used to specify separate materials for the cylinder ends;
-- END_MAT1 defaults to MAT if not supplied, and END_MAT2 defaults to
-- END_MAT1.
--
function surface.solid_cylinder (mat, arg1, ...)

   -- There are two argument conventions for cylinders, which we handle
   -- separately.
   --
   if transform.is_transform (arg1) then -- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
      local xform = arg1
      local emat1, emat2 = select (1, ...), select (2, ...)

      local base = xform (pos (0, 0, -1))
      local axis = xform (vec (0, 0, 2))
      local r1 = xform (vec (1, 0, 0))
      local r2 = xform (vec (0, 1, 0))

      return surface.group {
	 surface.cylinder (mat, xform);
	 surface.ellipse (emat1 or mat, base, r1, r2);
	 surface.ellipse (emat2 or emat1 or mat, base + axis, r1, r2);
      }
   else	      -- args: MAT, BASE, AXIS, RADIUS [, END_MAT1 [, END_MAT2]]
      local base = arg1
      local axis = select (1, ...)
      local radius = select (2, ...)
      local emat1, emat2 = select (3, ...), select (4, ...)

      local au = axis:unit()
      local r1u = au:perpendicular()
      local r1 = r1u * radius
      local r2 = cross (r1u, au) * radius

      return surface.group {
	 surface.cylinder (mat, base, axis, radius);
	 surface.ellipse (emat1 or mat, base, r1, r2);
	 surface.ellipse (emat2 or emat1 or mat, base + axis, r1, r2);
      }
   end
end



----------------------------------------------------------------
-- Meshes
--

surface.mesh = raw.Mesh

surface.mesh_vertex_group = raw.mesh_vertex_group
surface.mesh_vertex_normal_group = raw.mesh_vertex_normal_group

-- Resize a mesh to fit in a 1x1x1 box, centered at the origin (but with
-- the bottom at y=0).  Returns MESH.
--
function surface.normalize_mesh (mesh, xf)
   local norm = surface.y_base_normalize_xform (mesh)
   if xf then norm = xf (norm) end
   mesh:transform (norm)
   return mesh
end

--
-- Not yet documented:
--
--  + surface.mesh
--  + surface.mesh_vertex_group
--  + surface.mesh_vertex_normal_group
--  + surface.normalize_mesh
--



----------------------------------------------------------------
-- Surface containers
--

-- surface.group -- Return a surface-group, a surface defined by the
--	union of other surfaces
--
-- args: ()
--   or: (SURFACES)
--
-- Return a grouped surface.  If the table SURFACES is provided, it
-- should be a list of surfaces to add to the group, otherwise the
-- group is initially empty.
--
-- Surface-groups support the following methods:
--
--   GROUP:add (SURFACE)	-- Add SURFACE to GROUP
--   GROUP:add (SURFACES)	-- Add surfaces in the table SURFACES to GROUP
--
function surface.group (surfs)
   local group = raw.SurfaceGroup ()

   -- Initialize wrapper functions if necessary
   --
   if not swig.has_index_wrappers (group) then
      local wrap = swig.index_wrappers (group)

      -- Augment raw add method to (1) record the link between a group
      -- and the surfaces in it so GC can see it, and (2) support adding
      -- a table of surfaces all at once.
      --
      function wrap:add (surf)
	 if (type (surf) == "table") then
	    for k,v in pairs (surf) do
	       self:add (v)
	    end
	 else
	    if swig.need_obj_gc_protect then
	       swig.gc_ref (self, surf)
	    end
	    swig.nowrap_meth_call (self, "add", surf)
	 end
      end
   end

   if surfs then
      group:add (surfs)
   end

   return group
end

-- surface.model -- Return a surface model
--
-- args: (SURFACE, ACCEL_TYPE)
--
-- Return a "surface model" containing SURFACE, which may be a single
-- surface or a table containing a list of surfaces.  The resulting
-- model can then be instantiated multiple times using
-- surface.instance.
--
-- ACCEL_TYPE is optional, and is the type of search accelerator to
-- use with this model; if not provided, it will default to something
-- reasonable.
--
-- Note that a surface model is _not_ a real surface, and cannot be
-- added to a scene directly.  Rather, it must be instantiated using
-- the surface.instance function (which returns a real surface).
--
function surface.model (surf, accel_type)

   -- If SURF is actually a table, make a surface-group to hold its
   -- members, and wrap that instead.
   --
   if type (surf) == "table" then
      if #surf == 1 then
	 surf = surf[1]
      else
	 surf = surface.group (surf)
      end
   end

   local mod = raw.model (surf, accel.factory (accel_type))

   if swig.need_obj_gc_protect then
      -- Record the GC link between MOD and SURF.
      --
      swig.gc_ref (mod, surf)
   end

   return mod
end

-- surface.instance -- Return a surface instantiating a surface model
--
-- args: (MODEL, MODEL_TO_WORLD_XFORM)
--
-- Return a surface which replicates MODEL as transformed by
-- MODEL_TO_WORLD_XFORM.  The memory overhead of a surface instance is
-- very small, so creating multiple instances from a single model is a
-- good way to create duplicates of a complex surface without
-- excessive memory use.
--
local surf_instance
if swig.need_obj_gc_protect then
   -- If we need to protect against scene-object gcing, wrap the
   -- instance constructor to record the GC link between an instance
   -- and its model.
   --
   function surf_instance (model, xform)
      local inst = raw.Instance (model, xform)
      swig.gc_ref (inst, model)
      return inst
   end
else
   surf_instance = raw.Instance	-- just use raw constructor
end
surface.instance = surf_instance



----------------------------------------------------------------
-- Tessellated surfaces
--

--
-- Tessellated surfaces are created by algorithmically building a mesh.
--
-- The tessellation functions have the following common parameters:
--
--   MAT       -- A material, as returned from the snogray.material module
--   MAX_ERR   -- The size of the largest tolerable error.  The
--                tessellation will be refined until the tessellated
--                surface is within MAX_ERR of the real surface at all
--                points.
--
-- All tesselation functions return a mesh surface, as from
-- surface.mesh, and can be further manipulated if desired.
--

-- surface.tessel_sphere -- Return a tessellated sphere surface
--
-- args: (MAT, OBJ_TO_WORLD_XFORM, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, RADIUS, MAX_ERR)
--
surface.tessel_sphere = raw.tessel_sphere

-- surface.tessel_torus -- Return a tessellated torus surface
--
-- args: (MAT, OBJ_TO_WORLD_XFORM, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, RADIUS, MAX_ERR)
--
surface.tessel_torus = raw.tessel_torus

-- surface.tessel_sinc -- Return a tessellated sinc surface
--
-- args: (MAT, OBJ_TO_WORLD_XFORM, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, MAX_ERR)
--   or: (MAT, ORIGIN, AXIS, RADIUS, MAX_ERR)
--
surface.tessel_sinc = raw.tessel_sinc

--
-- Not sufficiently documented:
--
--  + surface.tessel_sphere
--  + surface.tessel_sinc
--  + surface.tessel_torus
--



----------------------------------------------------------------
-- "Normalization" transform calculations
--

-- surface.normalize_xform -- Return a surface normalization transform
--
-- args: (SURFACE)
--
-- Return a transform matrix which will warp SURFACE to be in a 2x2x2
-- box centered at the origin.  Only a single scale factor is used for
-- all dimensions, so that the transformed surface isn't distorted,
-- merely resized/translated.
--
function surface.normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = midpoint (bbox.max, bbox.min)
   local max_size = bbox:max_size ()

   return transform.scale (2 / max_size) (transform.translate (-center.x, -center.y, -center.z))
end

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin in the x and z dimensions, but with a minimum y value
-- of zero (so it has a "zero y base").  Only a single scale factor is
-- used for all dimensions, so that a transformed object isn't
-- distorted, merely resized/translated.
--
function surface.y_base_normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = midpoint (bbox.max, bbox.min)
   local size = bbox.max - bbox.min
   local max_size = bbox:max_size ()

   return transform.scale (2 / max_size) (transform.translate (-center.x, size.y / 2 - center.y, -center.z))
end


--
-- Not yet documented:
--
--  + surface.y_base_normalize_xform
--


-- return the module
--
return surface
