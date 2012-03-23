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
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"

local color = require 'snogray.color'
local is_color_spec = color.is_color_spec

local texture = require 'snogray.texture'
local color_tex_val = texture.color_tex_val
local float_tex_val = texture.float_tex_val
local is_color_tex = texture.is_color_tex
local is_float_tex = texture.is_float_tex


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
-- materials
--

local function is_material (val)
   return swig.type (val) == 'Material'
end
snogray.is_material = is_material

local function is_ior (val)
   return swig.type (val) == "Ior"
end
snogray.is_ior = is_ior

local function is_ior_spec (obj)
   local ot = type (obj)
   if is_ior (obj) or ot == 'number' then
      return true
   elseif ot == 'table' then
      for k,v in pairs (obj) do
	 if type (v) ~= 'number' then
	    return false
	 elseif k ~= 1 and k ~= 2 and k ~= 'n' and k ~= 'k' then
	    return false
	 end
      end
      return true
   else
      return false
   end
end
snogray.is_ior_spec = is_ior_spec

-- Index of Refraction:  { n = REAL_IOR, k = IMAG_IOR }
--
local function ior (n, k)
   if (type (n) == "number" or  is_ior (n)) and not k then
      return n
   elseif n and k then
      return raw.Ior (n, k)
   else
      local params = n
      return raw.Ior (params.n or params[1], params.k or params[2])
   end
end
snogray.ior = ior

-- Do common material post-processing to the material MAT, using
-- parameters from the table PARAMS, and return MAT.  If PARAMS is not a
-- table, it is ignored.
--
local function postproc_material (mat, params)
   if type (params) == 'table' then
      -- bump-mapping
      --
      local bump = params.bump_map or params.bump
      -- we ignore scalar bump maps, as they have no effect
      if bump and type (bump) ~= 'number' then
	 if texture.is_color_tex (bump) then
	    bump = texture.intens (bump)
	 elseif not texture.is_float_tex (bump) then
	    error ("Invalid bump map "..tostring(bump))
	 end
	 mat.bump_map = bump
      end

      -- opacity (alpha transparency)
      --
      local opacity = params.opacity or params.alpha
      -- a simple 1 or color(1) means "fully opaque", so can be ignored
      if opacity and opacity ~= 1 and opacity ~= color.white then
	 mat = snogray.stencil (opacity, mat)
      end
   end

   return mat
end   

-- Lambertian material:  { diffuse|color = 
--
function snogray.lambert (params)
   local diff
   if is_color_spec (params) or is_color_tex (params) then
      diff = params
   else
      diff = params.diffuse or params.color or params[1] or 1
   end
   diff = color_tex_val (diff)
   return postproc_material (raw.lambert (diff), params)
end

function snogray.cook_torrance (params)
   local diff, spec, m, i

   if is_color_spec (params) or is_color_tex (params) then
      diff = params
      spec = color.white
      m = 0.1
      i = 1.5
   else
       diff = params.diffuse or params.diff or params.d
	 or params.color or params[1] or 1
       spec = params.specular or params.spec or params.s
	 or params[2] or 1
       m = params.m or params[3] or 1
       i = ior (params.ior or params[4] or 1.5)
   end

   diff = color_tex_val (diff)
   spec = color_tex_val (spec)
   m = float_tex_val (m)

   return postproc_material (raw.cook_torrance (diff, spec, m, i), params)
end

local default_mirror_ior = ior (0.25, 3)

-- Return a mirror material.
-- PARAMS can be:
--   REFLECTANCE
--   {ior=IOR, reflect=REFLECTANCE, color=COLOR}
--   {REFLECTANCE, ior=IOR, color=COLOR}
-- etc
--
function snogray.mirror (params)
   local _ior = default_mirror_ior
   local _reflect = color.white
   local _col = color.black
   local _under

   if is_color_spec (params) or is_color_tex (params) then
      _reflect = params
   elseif is_ior_spec (params) then
      _ior = params
   elseif params then
      _reflect = params.reflect or params.reflectance or params.refl or params[1] or _reflect
      _ior = params.ior or params[2] or _ior
      _col = params.color or params[3] or _col
      _under = params.underlying or params.under or params[4]
   end

   if not _under then
      _under = color_tex_val (_col)
   end

   _ior = ior (_ior)
   _reflect = color_tex_val (_reflect)

   return postproc_material (raw.mirror (_ior, _reflect, _under), params)
end

-- Return a glass material.
-- PARAMS can be:
--   IOR
--   {ior=IOR, absorb=ABSORPTION}
--   {ABSORPTION, ior=IOR}
-- etc
--
function snogray.glass (params)
   local _ior = 1.5
   local _absorb = color.black

   if type (params) == "number" then
      _ior = params
   elseif type (params) == "table"
      and (params.ior or params.absorb or params.absorption)
   then
      _ior = params.ior or params[1] or _ior
      _absorb = params.absorb or params.absorption or params[2] or _absorb
   elseif type (params) == "table" and (params.n or params.k) then
      _ior = params
   elseif params then
      _absorb = params
   end

   _ior = ior (_ior)
   _absorb = color.std (_absorb)

   return postproc_material (raw.glass (raw.Medium (_ior, _absorb)), params)
end

-- Return a thin_glass material.
-- PARAMS can be:
--   IOR
--   {ior=IOR, color=COLOR}
--   {COLOR, ior=IOR}
-- etc
--
function snogray.thin_glass (params)
   local _ior = 1.5
   local _color = color.white

   if type (params) == "number" then
      _ior = params
   elseif type (params) == "table"
      and (params.ior or params.color or params.col)
   then
      _ior = params.ior or params[1] or _ior
      _color = params.color or params.col or params[2] or _color
   elseif params then
      _color = params
   end

   _color = color.std (_color)

   return postproc_material (raw.thin_glass (_color, _ior), params)
end

function snogray.glow (col, underlying)
   col = color_tex_val (col)
   if underlying then
      return raw.glow (col, underlying)
   else
      return raw.glow (col)
   end
end   

function snogray.norm_glow (intens)
   return raw.norm_glow (intens or 1)
end   

function snogray.stencil (opacity, underlying)
   opacity = color_tex_val (opacity)
   return raw.stencil (opacity, underlying)
end

----------------------------------------------------------------
-- material dicts
--

function snogray.is_material_dict (val)
   return swig.type (val) == 'MaterialDict'
end

function snogray.material_dict (init)
   local mdict = raw.MaterialDict ()

   if init then
      if type (init) == "table" then
	 for name, mat in pairs (init) do
	    if type (name) == "boolean" then
	       mdict:set_default (mat)
	    else
	       mdict[name] = mat
	    end
	 end
      elseif is_material (init) then
	 mdict:set_default (init)
      end
   end

   return mdict
end


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
-- Lights
--

function snogray.point_light (pos, intens, ...)
   return raw.PointLight (pos, color.std (intens), ...)
end

function snogray.sphere_light (pos, radius, intens)
   return raw.SphereLight (pos, radius, color.std (intens))
end

function snogray.triangle_light (corner, side1, side2, intens)
   return raw.TriparLight (corner, side1, side2, false, color.std (intens))
end

function snogray.far_light (dir, angle, intens)
   return raw.FarLight (dir:unit(), angle, color.std (intens))
end

function snogray.envmap_light (image_or_filename, ...)
   if type (image_or_filename) == 'string' then
      image_or_filename = snogray.image (image_or_filename)
   end
   return raw.EnvmapLight (raw.envmap (image_or_filename), ...)
end

----------------------------------------------------------------
-- Images
--

snogray.image = raw.image

snogray.envmap = raw.envmap


----------------------------------------------------------------
-- array functions

-- Use binary search to locate the entries in TABLE which are closest
-- to VAL.  SUB_KEY is a key for each table entry; if non-nil it is
-- used to retrieve the key from each entry for comparison (otherwise
-- the entries themselves are used).
--
-- Two indices are returned:  LO and HI; HI always equals LO+1 unless
-- there are less than two entries in TABLE (if there are no entries,
-- then LO and HI will be 1 and 0; if there is one entry then LO and
-- HI will both be 1).  If VAL exactly matches an entry, then
-- TABLE[LO] will be that entry.  If VAL lies between two entries,
-- then LO and HI will be the indices of entries on either side of it.
-- Otherwise, LO and HI will be the indices of the two entries closest
-- to it (the two lowest entries in the table, or the two highest).
--
function snogray.binary_search (table, val, sub_key)
   local lo, hi = 1, #table

   while lo < hi - 1 do
      local mid = math.floor ((lo + hi) / 2)

      local entry = table[mid]
      if sub_key then
	 entry = entry[sub_key]
      end

      if entry > val then
	 hi = mid
      else
	 lo = mid
      end
   end

   return lo, hi
end

-- Lookup VAL in TABLE, using linear interpolation to compute entries
-- not explicitly present.
--
-- TABLE should be a sequential array of entries like {VAL1, VAL2, ...},
-- and be sorted by the values of VAL1.
--
function snogray.linear_interp_lookup (table, val)

   -- If there's only one entry in the table, just return it.
   --
   if #table == 1 then
      local entry = table[1]
      if entry[1] == val then
	 return entry
      else
	 -- Copy ENTRY so we can use the correct value for ENTRY[1]
	 local new_entry = {val}
	 for i = 2, #entry do
	    new_entry[i] = entry[i]
	 end
	 return new_entry
      end
   end

   -- Use binary search to locate the nearest entries to VAL.
   --
   local lo, hi = snogray.binary_search (table, val, 1)
   lo = table[lo]
   hi = table[hi]

   if lo[1] == val then
      -- found exact match
      return lo
   else
      -- use interpolation

      local interp = {}

      local val_diff = val - lo[1]

      local inv_denom = 1 / (hi[1] - lo[1])

      interp[1] = val
      
      for i = 2, #lo do
	 local slope = (hi[i] - lo[i]) * inv_denom
	 interp[i] = lo[i] + val_diff * slope
      end

      return interp
   end
end


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
