-- snogray.lua -- Lua scene interface for snogray
--
--  Copyright (C) 2007-2012  Miles Bader <miles@gnu.org>
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
local snogray = {}


-- Imports
--
local table = require 'snogray.table'
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"
local color = require 'snogray.color'
local texture = require 'snogray.texture'
local material = require 'snogray.material'
local light = require 'snogray.light'


-- Users typically have the snogray module as their default global
-- environment, so it needs to also export standard globals too.
--
setmetatable (snogray, { __index = _G })


----------------------------------------------------------------
-- Vector/position/bounding-box manipulation
--

local pos, vec = raw.Pos, raw.Vec

snogray.pos = pos
snogray.vec = vec
snogray.bbox = raw.BBox
snogray.ray = raw.Ray
snogray.uv = raw.UV

-- Handy scene origin position.
--
snogray.origin = pos (0, 0, 0)

snogray.midpoint = raw.midpoint
snogray.dot = raw.dot
snogray.cross = raw.cross


----------------------------------------------------------------
-- compat color support
--

snogray.color = color.std


----------------------------------------------------------------
-- compat texture support
--

snogray.image_tex = texture.image
snogray.mono_image_tex = texture.mono_image

snogray.grey_tex = texture.grey
snogray.intens_tex = texture.intens

snogray.check_tex = texture.check
snogray.check3d_tex = texture.check3d

snogray.perturb_pos_tex = texture.perturb_pos
snogray.perturb_uv_tex = texture.perturb_uv

snogray.linterp_tex = texture.linterp
snogray.sinterp_tex = texture.sinterp

snogray.rescale_tex = texture.rescale

snogray.plane_map_tex = texture.plane_map
snogray.cylinder_map_tex = texture.cylinder_map
snogray.lat_long_map_tex = texture.lat_long_map

snogray.perlin_tex = texture.perlin
snogray.perlin_abs_tex = texture.perlin_abs

snogray.x_tex = texture.x
snogray.y_tex = texture.y
snogray.z_tex = texture.z
snogray.u_tex = texture.u
snogray.v_tex = texture.v

snogray.worley_tex = texture.worley
snogray.worley_id_tex = texture.worley_id

snogray.xform_tex = texture.xform
snogray.scale_tex = texture.scale
snogray.rotate_tex = texture.rotate
snogray.rot_tex = texture.rot

snogray.arith_tex = texture.arith
snogray.mul_tex = texture.mul
snogray.add_tex = texture.add
snogray.sub_tex = texture.sub
snogray.div_tex = texture.div
snogray.mod_tex = texture.mod
snogray.pow_tex = texture.pow
snogray.floor_tex = texture.floor
snogray.ceil_tex = texture.ceil
snogray.trunc_tex = texture.trunc
snogray.min_tex = texture.min
snogray.max_tex = texture.max
snogray.avg_tex = texture.avg
snogray.mirror_tex = texture.mirror
snogray.abs_tex = texture.abs
snogray.neg_tex = texture.neg
snogray.sin_tex = texture.sin
snogray.cos_tex = texture.cos
snogray.tan_tex = texture.tan
snogray.atan2_tex = texture.atan2

snogray.cmp_tex = texture.cmp

snogray.eq_tex = texture.eq
snogray.ne_tex = texture.ne
snogray.lt_tex = texture.lt
snogray.le_tex = texture.le
snogray.gt_tex = texture.gt
snogray.ge_tex = texture.ge

snogray.fourier_series_tex = texture.fourier_series
snogray.perlin_series_tex = texture.perlin_series
snogray.perlin_abs_series_tex = texture.perlin_abs_series


----------------------------------------------------------------
-- compat material support
--

snogray.is_ior = material.is_ior
snogray.ior = material.ior
snogray.is_ior_spec = material.is_ior_spec
snogray.lambert = material.lambert
snogray.cook_torrance = material.cook_torrance
snogray.mirror = material.mirror
snogray.glass = material.glass
snogray.thin_glass = material.thin_glass
snogray.glow = material.glow
snogray.stencil = material.stencil


----------------------------------------------------------------
-- transforms
--

-- Make a transform.
--
local function xform (...)
   --
   -- SWIG has bugs in handling overloads in conjunction with
   -- table-to-array conversion (it correctly generates code to do the
   -- conversion, but the type-checking code for distinguishing the
   -- various overload cases checks the wrong type), so we use a
   -- separate helper function "raw_xform" if the argument is a table
   -- [of transform matrix elements].
   --
   local nargs = select ('#', ...)
   if nargs == 1 and type (select (1, ...)) == 'table' then
      return raw.raw_xform (...)
   else
      return raw.Xform (...)
   end
end
snogray.xform = xform

snogray.identity_xform = raw.Xform_identity

local function is_xform (val)
   return swig.type (val) == 'Xform'
end
snogray.is_xform = is_xform

-- Various transform constructors.  We make local versions of a few
-- commonly used ones.
--
local scale, translate = raw.Xform_scaling, raw.Xform_translation
snogray.scale = scale
snogray.translate = translate
snogray.rotate = raw.Xform_rotation
snogray.rotate_x = raw.Xform_x_rotation
snogray.rotate_y = raw.Xform_y_rotation
snogray.rotate_z = raw.Xform_z_rotation
snogray.basis_xform = raw.Xform_basis

-- ... and some abbreviations for them (a bit silly, but composed
-- transforms can get rather long...).
--
snogray.trans = snogray.translate
snogray.rot = snogray.rotate
snogray.rot_x = snogray.rotate_x
snogray.rot_y = snogray.rotate_y
snogray.rot_z = snogray.rotate_z

-- Transform which converts the z-axis to the y-axis; this is useful
-- because many scene files are set up that way.
--
snogray.xform_z_to_y = snogray.rotate_x (-math.pi / 2)
snogray.xform_y_to_z = snogray.xform_z_to_y:inverse ()

-- Transform which converts the x-axis to the y-axis.
--
snogray.xform_x_to_y = snogray.rotate_z (-math.pi / 2)
snogray.xform_y_to_x = snogray.xform_x_to_y:inverse ()


-- Transform which inverts the z-axis (as many models use a different
-- convention for the z-axis).
--
snogray.xform_flip_x = scale (-1, 1, 1)
snogray.xform_flip_y = scale (1, -1, 1)
snogray.xform_flip_z = scale (1, 1, -1)


snogray.xform_identity = scale (1)


----------------------------------------------------------------
-- Tessellation
--

snogray.tessel_sphere = raw.tessel_sphere
snogray.tessel_sinc = raw.tessel_sinc
snogray.tessel_torus = raw.tessel_torus


----------------------------------------------------------------
-- meshes
--

snogray.mesh = raw.Mesh

snogray.mesh_vertex_group = raw.mesh_vertex_group
snogray.mesh_vertex_normal_group = raw.mesh_vertex_normal_group

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin.  Only a single scale factor is used for all
-- dimensions, so that a transformed object isn't distorted, merely
-- resized/translated.
--
function snogray.normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = snogray.midpoint (bbox.max, bbox.min)
   local max_size = bbox:max_size ()

   return scale (2 / max_size) (translate (-center.x, -center.y, -center.z))
end

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin in the x and z dimensions, but with a minimum y value
-- of zero (so it has a "zero y base").  Only a single scale factor is
-- used for all dimensions, so that a transformed object isn't
-- distorted, merely resized/translated.
--
function snogray.y_base_normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = snogray.midpoint (bbox.max, bbox.min)
   local size = bbox.max - bbox.min
   local max_size = bbox:max_size ()

   return scale (2 / max_size) (translate (-center.x, size.y / 2 - center.y, -center.z))
end

-- Resize a mesh to fit in a 1x1x1 box, centered at the origin (but with
-- the bottom at y=0).  Returns MESH.
--
function snogray.normalize (mesh, xf)
   local norm = snogray.y_base_normalize_xform (mesh)
   if xf then norm = xf (norm) end
   mesh:transform (norm)
   return mesh
end


----------------------------------------------------------------
-- Misc surface types
--

snogray.frame = raw.Frame

snogray.sphere = raw.Sphere
snogray.sphere2 = raw.Sphere2

snogray.tripar = raw.Tripar

function snogray.triangle (mat, v0, e1, e2)
   return snogray.tripar (mat, v0, e1, e2, false)
end

function snogray.parallelogram (mat, v0, e1, e2)
   return snogray.tripar (mat, v0, e1, e2, true)
end

-- Alias for common usage
--
snogray.rectangle = snogray.parallelogram

function snogray.parallelepiped (mat, corner, up, right, fwd)
   return snogray.surface_group {
      snogray.parallelogram (mat, corner, right, up);
      snogray.parallelogram (mat, corner, up, fwd);
      snogray.parallelogram (mat, corner, fwd, right);

      snogray.parallelogram (mat, corner + up, right, fwd);
      snogray.parallelogram (mat, corner + right, fwd, up);
      snogray.parallelogram (mat, corner + fwd, up, right);
   }
end

-- Alias for common usage
--
snogray.cube = snogray.parallelepiped

-- Return an elliptical surface.
--
-- args: MAT, XFORM
--   or: MAT, BASE, AXIS, RADIUS
--
snogray.ellipse = raw.Ellipse

-- Return a cylindrical surface (with no ends).
--
-- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
--   or: MAT, BASE, AXIS, RADIUS [, END_MAT1 [, END_MAT2]]
--
snogray.cylinder = raw.Cylinder

-- solid_cylinder is just like cylinder, but has endcaps as well.
--
-- Optionally, specific materials can be specified for the ends by at
-- the end of the argument list.
--
-- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
--   or: MAT, BASE, AXIS, RADIUS [, END_MAT1 [, END_MAT2]]
--
function snogray.solid_cylinder (mat, arg1, ...)

   -- There are two argument conventions for cylinders, which we handle
   -- separately.
   --
   if is_xform (arg1) then -- args: MAT, XFORM [, END_MAT1 [, END_MAT2]]
      local xform = arg1
      local emat1, emat2 = select (1, ...), select (2, ...)

      local base = xform (pos (0, 0, -1))
      local axis = xform (vec (0, 0, 2))
      local r1 = xform (vec (1, 0, 0))
      local r2 = xform (vec (0, 1, 0))

      return snogray.surface_group {
	 snogray.cylinder (mat, xform);
	 snogray.ellipse (emat1 or mat, base, r1, r2);
	 snogray.ellipse (emat2 or emat1 or mat, base + axis, r1, r2);
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

      return snogray.surface_group {
	 snogray.cylinder (mat, base, axis, radius);
	 snogray.ellipse (emat1 or mat, base, r1, r2);
	 snogray.ellipse (emat2 or emat1 or mat, base + axis, r1, r2);
      }
   end
end

-- space-builder-factory used for building model spaces
--
local model_space_builder_factory = nil

-- Wrap the model constructor to record the GC link between a
-- model and the surface in it.
--
function snogray.model (surf)

   -- If SURF is actually a table, make a surface-group to hold its
   -- members, and wrap that instead.
   --
   if type (surf) == "table" then
      if #surf == 1 then
	 surf = surf[1]
      else
	 surf = snogray.surface_group (surf)
      end
   end

   if not model_space_builder_factory then
      model_space_builder_factory = raw.OctreeBuilderFactory ()
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
   function snogray.instance (model, xform)
      local inst = raw.Instance (model, xform)
      swig.gc_ref (inst, model)
      return inst
   end
else
   snogray.instance = raw.Instance	-- just use raw constructor
end


-- Wrap the surface_group constructor to add some method wrappers to it,
-- and support adding a table of surfaces as well.
--
function snogray.surface_group (surfs)
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


----------------------------------------------------------------
-- compat light functions
--

snogray.point_light = light.point
snogray.sphere_light = light.sphere
snogray.triangle_light = light.triangle
snogray.far_light = light.far
snogray.envmap_light = light.envmap


----------------------------------------------------------------
-- Images
--

snogray.image = raw.image

snogray.envmap = raw.envmap


----------------------------------------------------------------
-- compat array functions

snogray.binary_search = table.binary_search
snogray.linear_interp_lookup = table.linear_interp_lookup


----------------------------------------------------------------
-- High-level rendering functions (mainly used by the top-level driver)
--

snogray.scene = raw.Scene
snogray.camera = raw.Camera

snogray.octree_builder_factory = raw.OctreeBuilderFactory


----------------------------------------------------------------

-- return module
--
return snogray
