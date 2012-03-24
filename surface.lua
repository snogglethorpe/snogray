-- surface.lua -- Surface support
--
--  Copyright (C) 2012  Miles Bader <miles@gnu.org>
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

local pos, vec = raw.Pos, raw.Vec
local cross = raw.cross
local midpoint = raw.midpoint


----------------------------------------------------------------
-- "Normalization" transform calculations
--

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin.  Only a single scale factor is used for all
-- dimensions, so that a transformed object isn't distorted, merely
-- resized/translated.
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


----------------------------------------------------------------
-- Tessellation
--

surface.tessel_sphere = raw.tessel_sphere
surface.tessel_sinc = raw.tessel_sinc
surface.tessel_torus = raw.tessel_torus


----------------------------------------------------------------
-- meshes
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


----------------------------------------------------------------
-- Misc surface types
--

surface.sphere = raw.Sphere
surface.sphere2 = raw.Sphere2

surface.tripar = raw.Tripar

function surface.triangle (mat, v0, e1, e2)
   return surface.tripar (mat, v0, e1, e2, false)
end

function surface.parallelogram (mat, v0, e1, e2)
   return surface.tripar (mat, v0, e1, e2, true)
end

-- Alias for common usage
--
surface.rectangle = surface.parallelogram

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

-- Alias for common usage
--
surface.cube = surface.parallelepiped

-- Return an elliptical surface.
--
-- args: MAT, XFORM
--   or: MAT, BASE, AXIS, RADIUS
--
surface.ellipse = raw.Ellipse

-- Return a cylindrical surface (with no ends).
--
-- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
--   or: MAT, BASE, AXIS, RADIUS [, END_MAT1 [, END_MAT2]]
--
surface.cylinder = raw.Cylinder

-- solid_cylinder is just like cylinder, but has endcaps as well.
--
-- Optionally, specific materials can be specified for the ends by at
-- the end of the argument list.
--
-- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
--   or: MAT, BASE, AXIS, RADIUS [, END_MAT1 [, END_MAT2]]
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

-- space-builder-factory used for building model spaces
--
local model_space_builder_factory = nil

-- Wrap the model constructor to record the GC link between a
-- model and the surface in it.
--
function surface.model (surf)

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

   if not model_space_builder_factory then
      local accel = require 'snogray.accel'
      model_space_builder_factory = accel.factory ()
   end

   local mod = raw.model (surf, model_space_builder_factory)

   if swig.need_obj_gc_protect then
      -- Record the GC link between MOD and SURF.
      --
      swig.gc_ref (mod, surf)
   end

   return mod
end

-- If we need to protect against scene-object gcing, wrap the instance
-- constructor to record the GC link between an instance and its
-- model.
--
if swig.need_obj_gc_protect then
   function surface.instance (model, xform)
      local inst = raw.Instance (model, xform)
      swig.gc_ref (inst, model)
      return inst
   end
else
   surface.instance = raw.Instance	-- just use raw constructor
end


-- Wrap the surface_group constructor to add some method wrappers to it,
-- and support adding a table of surfaces as well.
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


-- return the module
--
return surface
