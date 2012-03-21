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


-- Users typically have the snogray module as their default global
-- environment, so it needs to also export standard globals too.
--
setmetatable (snogray, { __index = _G })


----------------------------------------------------------------

-- Return a table containing every key in KEYS as a key, with value true.
--
local function set (keys)
   local s = {}
   for i,v in ipairs (keys) do
      s[v] = true
   end
   return s
end


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
-- colors
--

local colors = {}

local function is_color (val)
   return swig.type (val) == 'Color'
end
snogray.is_color = is_color

local color_keys = set{
   'r', 'red', 'g', 'green', 'b', 'blue', 'grey', 'gray',
   'i', 'intens', 'intensity', 'bright', 'brightness'
}

local function is_color_spec (obj)
   local ot = type (obj)
   if ot == 'number' or is_color (obj) or (ot == 'string' and colors[obj]) then
      return true
   elseif ot ~= 'table' then
      return false
   end
   
   for k,v in pairs (obj) do
      local kt = type (k)
      local vt = type (v)
      local inh = false
      if kt == 'number' then
	 if k == 1 and is_color_spec (v) then
	    inh = true
	 elseif k > 1 and inh then
	    return false
	 elseif k > 3 or vt ~= 'number' then
	    return false
	 end
      elseif not color_keys[k] or vt ~= 'number' then
	 return false
      end
   end

   return true
end
snogray.is_color_spec = is_color_spec

local function color (val, ...)
   if is_color (val) then
      return val
   else
      local t = type (val)

      if t == "number" then
	 return raw.Color (val, ...)
      elseif t == "string" then
	 return colors[val] or error ("unknown color name: "..val, 2)
      elseif t == "table" then
	 local r,g,b

	 if not next (val) then
	    return white      -- default to white if _nothing_ specified
	 end

	 if type (val[1]) == "number" then
	    if #val == 1 then
	       r, g, b = val[1], val[1], val[1]
	    else
	       r, g, b = val[1], val[2], val[3]
	    end
	 elseif val[1] then
	    local inherit = color (val[1])
	    r, g, b = inherit:r(), inherit:g(), inherit:b()
	 end

	 local grey = val.grey or val.gray
	 r = val.red or val.r or grey or r or 0
	 g = val.green or val.g or grey or g or 0
	 b = val.blue or val.b or grey or b or 0

	 local intens =
	    val.intensity
	    or val.intens
	    or val.i
	    or val.brightness
	    or val.bright

	 if intens then
	    local max = math.max (r,g,b)
	    if max > 0 then
	       local scale = intens / max
	       r = r * scale
	       g = g * scale
	       b = b * scale
	    end
	 end

	 local scale = val.scale or val.s
	 if scale then
	    r = r * scale
	    g = g * scale
	    b = b * scale
	 end

	 return raw.Color (r, g, b)
	 
      else
	 error ("invalid color specification: "..tostring(val), 2)
      end
   end
end
snogray.color = color

function snogray.grey (level)
   return raw.Color (level)
end
snogray.gray = snogray.grey

snogray.white = snogray.grey (1)
snogray.black = snogray.grey (0)

local function define_color (name, val)
   colors[name] = color (val)
end
snogray.define_color = define_color

define_color ("white",	snogray.white)
define_color ("black",	snogray.black)
define_color ("red",	{red=1})
define_color ("green",	{green=1})
define_color ("blue",	{blue=1})
define_color ("cyan",	{blue=1, green=1})
define_color ("magenta",{blue=1, red=1})
define_color ("yellow",	{red=1, green=1})


----------------------------------------------------------------
-- Basic texture support
--

local function is_float_tex (val)
   return swig.type (val) == 'Tex<float>'
end
snogray.is_float_tex = is_float_tex

local function is_color_tex (val)
   return swig.type (val) == 'Tex<Color>'
end
snogray.is_color_tex = is_color_tex

-- Return VAL, which should either be a color or a color texture, boxed
-- into a TexVal<Color> container.
--
local function color_tex_val (val)
   if is_float_tex (val) then
      val = raw.grey_tex (raw.FloatTexVal (val))
   elseif is_color_spec (val) then
      val = color (val)
   end
   return raw.ColorTexVal (val)
end

-- Return TEX, which should be a texture, converted into a
-- floating-point texture if it isn't one already.
--
local function float_tex (tex)
   if is_color_tex (tex) then
      tex = raw.intens_tex (raw.ColorTexVal (tex))
   end
   return tex
end

-- Return VAL, which should either be a number or a float texture, boxed
-- into a TexVal<float> container.
--
local function float_tex_val (val)
   if is_color_tex (val) then
      val = raw.intens_tex (raw.ColorTexVal (val))
   end
   return raw.FloatTexVal (val)
end

-- Return VAL boxed into either a TexVal<Color> or a TexVal<float>
-- container, whichever is appropriate.
--
local function tex_val (tex)
   if is_float_tex (tex) then
      return raw.FloatTexVal (tex)
   elseif is_color_tex (tex) then
      return raw.ColorTexVal (tex)
   elseif type (tex) == 'number' then
      return raw.FloatTexVal (tex)
   else
      return raw.ColorTexVal (color (tex))
   end
end

-- Return VAL1 and VAL2 boxed into a pair of either TexVal<Color> or
-- TexVal<float> containers, whichever are appropriate.  Both VAL1 and
-- VAL2 are examined to make the decision; a mixture of Color and float
-- values results in the floating value being automatically converted to
-- a Color value to match.
--
local function tex_vals (val1, val2)
   if is_color_tex (val1) or is_color_tex (val2)
      or (type(val1) ~= 'number' and is_color_spec (val1))
      or (type(val2) ~= 'number' and is_color_spec (val2))
   then
      return color_tex_val (val1), color_tex_val (val2)
   else
      return float_tex_val (val1), float_tex_val (val2)
   end
end


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
	 mat.bump_map = float_tex (bump)
      end

      -- opacity (alpha transparency)
      --
      local opacity = params.opacity or params.alpha
      -- a simple 1 or color(1) means "fully opaque", so can be ignored
      if opacity and opacity ~= 1 and opacity ~= color(1) then
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
      spec = white
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
   local _reflect = white
   local _col = black
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
   local _absorb = black

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
   _absorb = color (_absorb)

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
   local _color = white

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

   _color = color (_color)

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
   return raw.PointLight (pos, color (intens), ...)
end

function snogray.sphere_light (pos, radius, intens)
   return raw.SphereLight (pos, radius, color (intens))
end

function snogray.triangle_light (corner, side1, side2, intens)
   return raw.TriparLight (corner, side1, side2, false, color (intens))
end

function snogray.far_light (dir, angle, intens)
   return raw.FarLight (dir:unit(), angle, color (intens))
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
-- Miscellaneous texture sources and operators
--

-- Image textures (read from a file)
--
snogray.image_tex = raw.image_tex
snogray.mono_image_tex = raw.mono_image_tex

-- Return a "grey_tex" texture object using the floating-point texture
-- VAL as a source.  This can be used to convert a floating-point
-- texture into a color texture.
--
function snogray.grey_tex (val) return raw.grey_tex (float_tex_val (val)) end

-- Return a "intens_tex" texture object using the color texture VAL as a
-- source.  This can be used to convert a color-point texture into a
-- floating-point texture.
--
function snogray.intens_tex (val) return raw.intens_tex (color_tex_val (val)) end

-- Return a "check" texture, which evaluates to either TEX1 or TEX2 in a
-- check pattern.
--
function snogray.check_tex (tex1, tex2)
   return raw.check_tex (tex_vals (tex1, tex2))
end
function snogray.check3d_tex (tex1, tex2)
   return raw.check3d_tex (tex_vals (tex1, tex2))
end

function snogray.perturb_pos_tex (src, x, y, z)
   return raw.perturb_pos_tex (tex_val (src), float_tex_val (x),
			       float_tex_val (y), float_tex_val (z))
end
function snogray.perturb_uv_tex (src, u, v)
   return raw.perturb_uv_tex (tex_val (src),
			      float_tex_val (u), float_tex_val (v))
end

-- Return an interpolation texture, which interpolates between two
-- textures according to the value of its control parameter.
--
function snogray.linterp_tex (control, val1, val2)
   return raw.linterp_tex (float_tex_val (control), tex_vals (val1, val2))
end
function snogray.sinterp_tex (control, val1, val2)
   return raw.sinterp_tex (float_tex_val (control), tex_vals (val1, val2))
end

-- Return a texture which rescales VAL from the range [IN_MIN, IN_MAX]
-- to the range [OUT_MIN, OUT_MAX].  The default output range is [0,1].
--
function snogray.rescale_tex (val, in_min, in_max, out_min, out_max)
   val = tex_val (val)
   out_min = out_min or 0
   out_max = out_max or 1
   if swig.type (val) == 'TexVal<Color>' then
      in_min = color (in_min)
      in_max = color (in_max)
      out_min = color (out_min)
      out_max = color (out_max)
   end
   return raw.rescale_tex (val, in_min, in_max, out_min, out_max)
end

snogray.plane_map_tex = raw.plane_map_tex
snogray.cylinder_map_tex = raw.cylinder_map_tex
snogray.lat_long_map_tex = raw.lat_long_map_tex

-- A cache of "singleton" texture sources, whose instances have no
-- state, and really only one shared instance is needed.
--
local singleton_tex_cache = {}
local function singleton_tex_fun (name, create)
   return function ()
	     local inst = singleton_tex_cache[name]
	     if not inst then
		inst = create ()
		singleton_tex_cache[name] = inst
	     end
	     return inst
	  end
end

snogray.perlin_tex = singleton_tex_fun ('perlin', raw.perlin_tex)
snogray.perlin_abs_tex
   = singleton_tex_fun ('perlin_abs',
			function() return snogray.abs_tex (snogray.perlin_tex ()) end)

snogray.x_tex = singleton_tex_fun ('x', function () return raw.coord_tex(0) end)
snogray.y_tex = singleton_tex_fun ('y', function () return raw.coord_tex(1) end)
snogray.z_tex = singleton_tex_fun ('z', function () return raw.coord_tex(2) end)
snogray.u_tex = singleton_tex_fun ('u', function () return raw.coord_tex(3) end)
snogray.v_tex = singleton_tex_fun ('v', function () return raw.coord_tex(4) end)

snogray.worley_tex = raw.worley_tex

local worley_id_kinds = { SCALE = 0, MOD = 1, scale = 0, mod = 1 }

function snogray.worley_id_tex (kind, min, max)
   if kind then
      if worley_id_kinds[kind] then
	 kind = worley_id_kinds[kind]
      end
   else
      kind = worley_id_kinds['scale']
   end

   if min then
      if not max then
	 max = min
	 min = 0
      end
   else
      min = 0
      max = 1
   end

   return raw.worley_id_tex (kind, min, max)
end


----------------------------------------------------------------
-- Texture transformations
--

-- Return a texture which transforms TEX by the transform XFORM.
--
-- Actually it's the texture coordinates which are transformed (before
-- giving them to TEX), so for instance, to make TEX get "smaller", you
-- would use a value of XFORM which scales by an amount greater than 1.
--
function snogray.xform_tex (xform, tex)
   return raw.xform_tex (xform, tex_val (tex))
end

-- Convenience functions for various sorts of texture transformations.
--
function snogray.scale_tex (amount, tex) return snogray.xform_tex (snogray.scale (amount), tex) end
function snogray.rotate_tex (amount, tex) return snogray.xform_tex (snogray.rotate (amount), tex) end
snogray.rot_tex = snogray.rotate_tex


----------------------------------------------------------------
-- Texture arithmetic
--

-- Encoding for arith_tex operations.
--
local arith_tex_ops = {
   ADD = 0, SUB = 1, MUL = 2, DIV = 3, MOD = 4, POW = 5,
   FLOOR = 6, CEIL = 7, TRUNC = 8, -- floor/ceil/trunc (X / Y) * Y
   MIN = 9, MAX = 10, AVG = 11,
   MIRROR = 12,			   -- abs (X - Y)
   SIN = 13, COS = 14, TAN = 15,   -- sin/cos/tan (X * 2 * PI / Y)
   ATAN2 = 16
}

-- Return a texture which performs operation OP on input textures ARG1
-- and ARG2.  Both color and floating-point textures are handled (a
-- mixture of both results in the floating-point texture being converted
-- to color before applying the operation).
--
function snogray.arith_tex (op, arg1, arg2)
   op = arith_tex_ops[op]
   return raw.arith_tex (op, tex_vals (arg1, arg2))
end

-- Alias for the arith_tex MUL operation.  This function treats the
-- second operand specially because it is used to overload the "*"
-- operator for textures, which we want to work for texture-xform
-- operations too.
--
function snogray.mul_tex (tex1, tex2_or_xform)
   if is_xform (tex2_or_xform) then
      return snogray.xform_tex (tex2_or_xform, tex1)
   else
      return snogray.arith_tex ('MUL', tex1, tex2_or_xform)
   end
end

-- Convenient aliases for the various other arith_tex operations.
--
function snogray.add_tex (...) return snogray.arith_tex ('ADD', ...) end
function snogray.sub_tex (...) return snogray.arith_tex ('SUB', ...) end
function snogray.div_tex (...) return snogray.arith_tex ('DIV', ...) end
function snogray.mod_tex (...) return snogray.arith_tex ('MOD', ...) end
function snogray.pow_tex (...) return snogray.arith_tex ('POW', ...) end
function snogray.floor_tex (x, y) return snogray.arith_tex ('FLOOR', x, y or 1) end
function snogray.ceil_tex (x, y) return snogray.arith_tex ('CEIL', x, y or 1) end
function snogray.trunc_tex (x, y) return snogray.arith_tex ('TRUNC', x, y or 1) end
function snogray.min_tex (...) return snogray.arith_tex ('MIN', ...) end
function snogray.max_tex (...) return snogray.arith_tex ('MAX', ...) end
function snogray.avg_tex (...) return snogray.arith_tex ('AVG', ...) end
function snogray.mirror_tex (...) return snogray.arith_tex ('MIRROR', ...) end
function snogray.abs_tex (tex) return snogray.arith_tex ('MIRROR', tex, 0) end
function snogray.neg_tex (tex) return snogray.arith_tex ('SUB', 0, tex) end
function snogray.sin_tex (x, y) return snogray.arith_tex ('SIN', x, y or 2*math.pi) end
function snogray.cos_tex (x, y) return snogray.arith_tex ('COS', x, y or 2*math.pi) end
function snogray.tan_tex (x, y) return snogray.arith_tex ('TAN', x, y or 2*math.pi) end
function snogray.atan2_tex (...) return snogray.arith_tex ('ATAN2', ...) end

-- Install operator overloads for the texture metatable MT.
--
local function setup_tex_metatable (mt)
   mt.__add = snogray.add_tex
   mt.__sub = snogray.sub_tex
   mt.__mul = snogray.mul_tex
   mt.__div = snogray.div_tex
   mt.__unm = snogray.neg_tex
   mt.__pow = snogray.pow_tex
   mt.__mod = snogray.mod_tex
end

-- There's a metatable for each underlying texture datatype, currently
-- Color and float.  We install the same overload functions for both.
--
setup_tex_metatable (getmetatable (snogray.intens_tex (0))) -- float
setup_tex_metatable (getmetatable (snogray.grey_tex (0))) -- Color


----------------------------------------------------------------
-- Texture comparison
--

-- Encoding for cmp_tex operations.
--
local cmp_tex_ops = { EQ = 0, NE = 1, LT = 2, LE = 3, GT = 4, GE = 5 }

-- Return a texture which compares CVAL1 to CVAL2 using the comparison
-- operator OP, and returns the value of RVAL1 if the result is true,
-- and RVAL2 otherwise.  CVAL1 and CVAL2 are interpreted as
-- floating-point textures.  RVAL1 and RVAL2 may be either color or
-- floating-point textures (a mixture of both results in the
-- floating-point texture being converted to color before applying the
-- operation).
--
function snogray.cmp_tex (op, cval1, cval2, rval1, rval2)
   op = cmp_tex_ops[op]
   return raw.cmp_tex (op, float_tex_val (cval1), float_tex_val (cval2),
   		       tex_vals (rval1, rval2))
end

-- Convenient aliases for the various cmp_tex operations.
--
function snogray.eq_tex (...) return snogray.cmp_tex ('EQ', ...) end
function snogray.ne_tex (...) return snogray.cmp_tex ('NE', ...) end
function snogray.lt_tex (...) return snogray.cmp_tex ('LT', ...) end
function snogray.le_tex (...) return snogray.cmp_tex ('LE', ...) end
function snogray.gt_tex (...) return snogray.cmp_tex ('GT', ...) end
function snogray.ge_tex (...) return snogray.cmp_tex ('GE', ...) end


----------------------------------------------------------------
-- Perlin fourier-series textures
--

-- Return a fourier-series summation of SOURCE_TEX, according to PARAMS:
--
--   Sum[i = 1 to OCTAVES] of (SOURCE_TEX * F_i * Scale(2^(i-1)) * OMEGA^(i-1))
--
-- where OCTAVES is the maximum term index to use, F_i are perm-term user
-- multiplicative factors (defaulting to 1), "... * Scale(2^(i-1))"
-- means to scale the input coordinates to SOURCE_TEX by 2^(i-1), and
-- "... * OMEGA^(i-1)" means to multiply the resulting value of the term
-- by OMEGA^(i-1).
--
-- If PARAMS is a number, then all F_i are 1, OMEGA = 0.5, and OCTAVES =
-- PARAMS.
--
-- If PARAMS is a table, then F_i is the value at index i in PARAMS
-- (F_1 is PARAMS[], F_2 is PARAMS[2], etc), OMEGA = PARAMS.omega or
-- 0.5 if there is no "omega" in PARAMS, and OCTAVES = PARAMS.octaves,
-- or the number of array members in PRAMS if there is no "octaves"
-- entry in PARAMS.
--
-- Note that the F_i values do not need to be constants, they can also
-- be textures.
--
-- The result is automatically scaled by the inverse of the constant
-- portions of the sum, Sum[...] (F_i * OMEGA^(i-1)), so that it should
-- have roughly the same magnitude as SOURCE_TEX.  Any non-constant F_i
-- values are ignored for the purposes of auto-scaling.
--
function snogray.fourier_series_tex (source_tex, params)
   local sum = nil
   local const_factor_sum = 0

   local function add_term (octave, factor)
      if factor and factor ~= 0 then
	 local scale_factor = 1.99^octave
	 local pow_factor = omega ^ octave
	 local term_factor = factor * pow_factor

	 local term = scale (scale_factor) (source_tex)
	 if type (term_factor) ~= 'number' or term_factor ~= 1 then
	    term = term * term_factor
	 end

	 local const_factor = pow_factor
	 if type (factor) == 'number' then
	    const_factor = const_factor * factor
	 end
	 const_factor_sum = const_factor_sum + const_factor

	 if sum then
	    sum = sum + term
	 else
	    sum = term
	 end
      end
   end

   local octaves
   local cur_octave = 0
   if type (params) == 'table' then
      omega = params.omega or 0.5

      -- Add terms corresponding to entries in PARAMS.
      --
      for i, factor in ipairs (params) do
	 add_term (i-1, factor)
      end

      cur_octave = #params
      octaves = params.octaves or octaves
   else
      omega = 0.5
      octaves = params
   end

   -- Add any remaining terms, up to OCTAVES.
   --
   if octaves then
      while cur_octave <= octaves do
	 add_term (cur_octave, 1)
	 cur_octave = cur_octave + 1
      end
   end

   if const_factor_sum ~= 1
      and (type (params) ~= 'table'
	or params.auto_scale ~= false)
   then
      sum = sum * (1 / const_factor_sum)
   end

   return sum
end

-- Call fourier_series_tex using perlin noise as the input texture.
-- See description of fourier_series_tex for an explanation of PARAMS.
--
function snogray.perlin_series_tex (params)
   return snogray.fourier_series_tex (snogray.perlin_tex(), params)
end

-- Call fourier_series_tex using the absolute value of perlin noise as
-- the input texture.  See description of fourier_series_tex for an
-- explanation of PARAMS.
--
function snogray.perlin_abs_series_tex (params)
   return snogray.fourier_series_tex (snogray.perlin_abs_tex(), params)
end


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
