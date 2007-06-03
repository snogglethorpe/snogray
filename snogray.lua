-- snogray.lua -- Lua scene interface for snogray
--
--  Copyright (C) 2007  Miles Bader <miles@gnu.org>
--
-- This file is subject to the terms and conditions of the GNU General
-- Public License.  See the file COPYING in the main directory of this
-- archive for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--


module ("snogray", package.seeall)

require ("snograw")

local raw = snograw


----------------------------------------------------------------
--
-- Vector/position/bounding-box manipulation

pos = raw.Pos
vec = raw.Vec
bbox = raw.BBox

-- Handy scene origin position.
--
origin = pos (0, 0, 0)

midpoint = raw.midpoint

----------------------------------------------------------------
--
-- global scene parameters, set up by automagic call to "start_load"

local raw_scene, raw_camera


----------------------------------------------------------------
--
-- colors

local colors = {}

function define_color (name, val)
   colors[name] = color (val)
end

function is_color (val)
   return swig_type (val) == "_p_snogray__Color"
end

function color (val)
   if is_color (val) then
      return val
   else
      local t = type (val)

      if t == "number" then
	 return raw.Color (val)
      elseif t == "string" then
	 return colors[val] or error ("unknown color name: "..val)
      elseif t == "table" then
	 local r,g,b

	 if not next (val) then
	    return white      -- default to white if _nothing_ specified
	 end

	 if type (val[1]) == "number" then
	    r, g, b = val[1], val[2], val[3]
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
	 error ("invalid color specification: "..val)
      end
   end
end

function grey (level)
   return raw.Color (level)
end
gray = grey

white = grey (1)
black = grey (0)

define_color ("white",	white)
define_color ("black",	black)
define_color ("red",	{red=1})
define_color ("green",	{green=1})
define_color ("blue",	{blue=1})
define_color ("cyan",	{blue=1, green=1})
define_color ("magenta",{blue=1, red=1})
define_color ("yellow",	{red=1, green=1})


----------------------------------------------------------------
--
-- materials

function is_material (val)
   return swig_type (val) == "_p_snogray__Material"
end

-- Allocate a material, and also remember it to avoid garbage collection.
--
local function material (color, brdf)
   local mat = raw.Material (color, brdf)
   scene:add (mat)
   return mat
end

function is_ior (val)
   return swig_type (val) == "_p_snogray__Ior"
end

-- Index of Refraction:  { n = REAL_IOR, k = IMAG_IOR }
--
function ior (n, k)
   if (type (n) == "number" or  is_ior (n)) and not k then
      return n
   elseif n and k then
      return raw.Ior (n, k)
   else
      local params = n
      return raw.Ior (params.n or params[1], params.k or params[2])
   end
end

-- Lambertian material:  { diffuse|color = 
--
function lambert (params)
   local diff
   if type (params) == "table" and (params.diffuse or params.color) then
      diff = color (params.diffuse or params.color or params[1] or 1)
   else
      diff = color (params)
   end
   return material (diff)
end

function cook_torrance (params)
   local diff, spec, m, i

   if type (params) == "table"
      and (params.diffuse or params.color
	   or params.specular or params.spec or params.m or params.ior)
   then
       diff = color (params.diffuse or params.color or params[1] or 1)
       spec = color (params.specular or params.spec or params[2] or 1)
       m = params.m or params[3] or 1
       i = ior (params.ior or params[4] or 1.5)
   else
      diff = color (params)
      spec = white
      m = 1
      i = 1.5
   end
   
   return material (diff, raw.cook_torrance (spec, m, i))
end

local default_mirror_ior = ior (0.25, 3)

-- Return a mirror material.
-- PARAMS can be:
--   REFLECTANCE
--   {ior=IOR, reflect=REFLECTANCE, color=COLOR}
--   {REFLECTANCE, ior=IOR, color=COLOR}
-- etc
--
function mirror (params)
   local _ior = default_mirror_ior
   local _reflect = white
   local _col = black

   if type (params) == "number" then
      _reflect = params
   elseif type (params) == "table"
      and (params.ior or params.reflect or params.reflectance or params.color)
   then
      _reflect = params.reflect or params.reflectance or params[1] or _reflect
      _ior = params.ior or params[2] or _ior
      _col = params.color or params[3] or _col
   elseif type (params) == "table" and (params.n or params.k) then
      _ior = params
   else
      _reflect = params
   end

   local m = raw.Mirror (ior (_ior), color (_reflect), color (_col));
   scene:add (m)		-- protect against GC

   return m
end

-- Return a glass material.
-- PARAMS can be:
--   IOR
--   {ior=IOR, absorb=ABSORPTION}
--   {ABSORPTION, ior=IOR}
-- etc
--
function glass (params)
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
   else
      _absorb = params
   end

   local gl = raw.Glass (raw.Medium (ior (_ior), color (_absorb)));
   scene:add (gl)		-- protect against GC

   return gl;
end

function glow (col)
   local gl = raw.Glow (color (col))
   scene:add (gl)		-- protect against GC
   return gl;
end   


----------------------------------------------------------------
--
-- material maps

function is_material_map (val)
   return swig_type (val) == "_p_snogray__MaterialMap"
end

function material_map (init)
   local mmap = raw.MaterialMap ()

   if init then
      if type (init) == "table" then
	 for name, mat in pairs (init) do
	    if type (name) == "boolean" then
	       mmap:set_default (mat)
	    else
	       mmap[name] = mat
	    end
	 end
      elseif is_material (init) then
	 mmap:set_default (init)
      end
   end

   return mmap
end


----------------------------------------------------------------
--
-- transforms

-- Make a transform.
--
xform = raw.Xform

identity_xform = raw.Xform_identity

function is_xform (val)
   local t = swig_type (val)
   return t == "_p_snogray__TXformTdouble_t"
      or t == "_p_snogray__TXformTfloat_t"
end

-- Various simple transformations
--
translation = raw.Xform_translation
scaling = raw.Xform_scaling
x_rotation = raw.Xform_x_rotation
y_rotation = raw.Xform_y_rotation
z_rotation = raw.Xform_z_rotation
rotation = raw.Xform_rotation

-- Transform which converts the z-axis to the y-axis; this is useful
-- because many scene files are set up that way.
--
xform_z_to_y = xform ()
xform_z_to_y:rotate_x (-math.pi / 2);
xform_z_to_y:scale (-1, 1, 1);


----------------------------------------------------------------
--
-- scene object
--
-- We don't use the raw scene object directly because swig's bogus
-- handling of garbage collection trips us up (object stored in the
-- scene get gced).

scene = {}

function scene:add (thing)
   self[#self + 1] = thing
   raw.Scene_add (raw_scene, thing)
end


----------------------------------------------------------------
--
-- meshes

mesh = raw.Mesh

-- Resize a mesh to fit in a 1x1x1 box, centered at the origin (but with
-- the bottom at y=0).
--
function normalize (mesh, xf)
   local bbox = mesh:bbox ()
   local center = midpoint (bbox.max, bbox.min)
   local size = bbox.max - bbox.min

   local norm = xform ()
   norm:translate (-center.x, size.y / 2 - center.y, -center.z)
   norm:scale (2 / bbox:max_size ())
   if xf then
      norm:transform (xf)
   end

   mesh:transform (norm)
end


----------------------------------------------------------------
--
-- Misc surface types

sphere = raw.Sphere

tripar = raw.Tripar


----------------------------------------------------------------
--
-- Lights

function sphere_light (pos, radius, intens)
   return raw.SphereLight (pos, radius, color (intens))
end

function rect_light (corner, side1, side2, intens)
   return raw.RectLight (corner, side1, side2, color (intens))
end


----------------------------------------------------------------
--
-- File handling

include_path = { "." }


local function filename_dir (filename)
   return string.match (filename, "^(.*)/[^/]*$")
end
local function filename_ext (filename)
   return string.match (filename, "[.][^./]*$")
end

function load_include (filename)
   local loaded, loaded_filename, err_msg

   if not filename_ext (filename) then
      filename = filename .. ".lua"
   end

   if string.sub (filename, 1, 1) == "/" then
      loaded_filename = filename
      loaded, err_msg = loadfile (filename)
   else
      -- First try the same directory as cur_filename.
      --
      local cur_dir = filename_dir (cur_filename)
      if cur_dir then
	 loaded_filename = cur_dir .. "/" .. filename
	 loaded, err_msg = loadfile (loaded_filename)
      end

      -- If we didn't find anything, try searching along include_path.
      --
      if not loaded then
	 local path_pos = 1
	 while not loaded and path_pos <= #include_path do
	    loaded_filename = include_path[path_pos] .. "/" .. filename
	    loaded, err_msg = loadfile (loaded_filename)
	    path_pos = path_pos + 1
	 end
      end
   end

   return loaded, loaded_filename, err_msg
end

function eval_include (loaded, loaded_filename, err_msg)
   if loaded then
      local old_cur_filename = cur_filename
      cur_filename = loaded_filename

      setfenv (loaded, getfenv ())
      loaded ()

      cur_filename = old_cur_filename
   else
      error (err_msg)
   end
end

-- Load FILENAME evaluated the current environment.  FILENAME is
-- searched for using the path in the "include_path" variable; while it
-- is being evaluating, the directory FILENAME was actually loaded from
-- is prepended to "include_path", so that any recursive includes may
-- come from the same directory.
--
function include (filename)
   local loaded, loaded_filename, err_msg = load_include (filename)
   eval_include (loaded, loaded_filename, err_msg)
   return loaded_filename
end

local used = {}

-- use is like include, but only includes the file if that hasn't
-- already been done.
--
function use (filename)
   local loaded, loaded_filename, err_msg = load_include (filename)
   if not used[loaded_filename] then
      eval_include (loaded, loaded_filename, err_msg)
      used[loaded_filename] = true
   end
   return loaded_filename
end


----------------------------------------------------------------
--
-- initialization

function start_load (filename, rscene, rcamera)
   print ("* loading scene: " .. filename)

   cur_filename = filename

   raw_scene = rscene
   raw_camera = rcamera

   -- Let users use the raw camera directly.
   --
   camera = raw_camera
end


-- arch-tag: e5dc4da4-c3f0-45e7-a4a1-a20cb4db6d6b
