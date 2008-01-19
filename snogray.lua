-- snogray.lua -- Lua scene interface for snogray
--
--  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--


module ("snogray", package.seeall)

local raw = require "snograw"


----------------------------------------------------------------
--
-- A facility for adding hooks into the swig metatable for an object

-- Return a table attached to OBJ (a userdata object or table), which is
-- used as to satisfy __index queries on OBJ.  If such a table already
-- exists for OBJ, it is returned, otherwise a new one is added and
-- returned.  The added table is consulted before any previously
-- existing __index hook, and the previous hook called only when a
-- request is not found in the wrapper table.
--
local function index_wrappers (obj)
   local mt = getmetatable (obj)

   if not mt.__raw_index then
      mt.__raw_index = mt.__index

      -- Note that we can't just make the old __index function an
      -- __index for our new __index table, because the first argument
      -- passed to it would be the wrong thing (the metatable, not the
      -- underlying object).
      --
      mt.__index =
	 function (obj, key)
	    local mt = getmetatable (obj)
	    local rawi = mt.__raw_index
	    local wraps = mt.__index_wrappers
	    return (wraps and wraps[key]) or rawi (obj, key)
	 end
      mt.__index_wrappers = {}
   end

   return mt.__index_wrappers
end

local function has_index_wrappers (obj)
   return getmetatable(obj).__index_wrappers
end

-- Lookup KEY in the wrapped object OBJ without invoking any wrapper.
--
local function nowrap_index (obj, key)
   return getmetatable (obj).__raw_index (obj, key)
end

-- Call method METH in the wrapped object OBJ without invoking any wrapper.
--
local function nowrap_meth_call (obj, meth, ...)
   return getmetatable (obj).__raw_index (obj, meth) (obj, ...)
end


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
--
-- Vector/position/bounding-box manipulation

pos = raw.Pos
vec = raw.Vec
bbox = raw.BBox

-- Handy scene origin position.
--
origin = pos (0, 0, 0)

midpoint = raw.midpoint
dot = raw.dot
cross = raw.cross


----------------------------------------------------------------
--
-- colors

local colors = {}

function define_color (name, val)
   colors[name] = color (val)
end

function is_color (val)
   local st = swig_type (val)
   return st == "snogray::Color *" or st == "_p_snogray__Color" 
end

local color_keys = set{
   'r', 'red', 'g', 'green', 'b', 'blue', 'grey', 'gray',
   'i', 'intens', 'intensity', 'bright', 'brightness',
   's', 'scale'
}

function is_color_spec (obj)
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
	 if k == 1 and vt == 'string' then
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

function color (val, ...)
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


local material_types = set{
   "snogray::Material *",
   "_p_snogray__Material",
   "snogray::CookTorrance *",
   "_p_snogray__CookTorrance",
   "snogray::Lambert *",
   "_p_snogray__Lambert",
   "snogray::Phong *",
   "_p_snogray__Phong",
   "snogray::Mirror *",
   "_p_snogray__Mirror",
   "snogray::Glass *",
   "_p_snogray__Glass",
   "snogray::Plastic *",
   "_p_snogray__Plastic",
}

function is_material (val)
   -- ugh; isn't there some way in swig to do a sub-class test?
   return material_types[swig_type (val)]
end

-- Remember MAT to avoid garbage collection.
--
local function material (mat)
   scene:add (mat)
   return mat
end

function is_ior (val)
   return swig_type (val) == "_p_snogray__Ior"
end

function is_ior_spec (obj)
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
   if is_color_spec (params) then
      diff = color (params)
   else
      diff = color (params.diffuse or params.color or params[1] or 1)
   end
   return material (raw.Lambert (diff))
end

function cook_torrance (params)
   local diff, spec, m, i

   if is_color_spec (params) then
      diff = color (params)
      spec = white
      m = 0.1
      i = 1.5
   else
       diff = color (params.diffuse or params.diff or params.d
		     or params.color or params[1] or 1)
       spec = color (params.specular or params.spec or params.s
		     or params[2] or 1)
       m = params.m or params[3] or 1
       i = ior (params.ior or params[4] or 1.5)
   end
   
   return material (raw.CookTorrance (diff, spec, m, i))
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

   if type (params) == "number" or is_color_spec (params) then
      _reflect = params
   elseif is_ior_spec (params) then
      _ior = params
   elseif params then
      _reflect = params.reflect or params.reflectance or params[1] or _reflect
      _ior = params.ior or params[2] or _ior
      _col = params.color or params[3] or _col
   end

   return material (raw.Mirror (ior (_ior), color (_reflect), color (_col)));
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

function norm_glow (intens)
   local ng = raw.NormGlow (intens or 1)
   scene:add (ng)		-- protect against GC
   return ng;
end   


----------------------------------------------------------------
--
-- material dicts

function is_material_dict (val)
   local st = swig_type (val)
   return st == "_p_snogray__MaterialDict" or st == "snogray::MaterialDict *"
end

function material_dict (init)
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
--
-- transforms

-- Make a transform.
--
xform = raw.Xform

identity_xform = raw.Xform_identity

function is_xform (val)
   local st = swig_type (val)
   return st == "snogray::Xform *|snogray::TXform<float > *|snogray::TXform<snogray::dist_t > *"
      or st == "_p_snogray__TXformTdouble_t"
      or st == "_p_snogray__TXformTfloat_t"
end

-- Various transform constructors.
--
scale = raw.Xform_scaling
translate = raw.Xform_translation
rotate = raw.Xform_rotation
rotate_x = raw.Xform_x_rotation
rotate_y = raw.Xform_y_rotation
rotate_z = raw.Xform_z_rotation

-- ... and some abbreviations for them (a bit silly, but composed
-- transforms can get rather long...).
--
trans = translate
rot = rotate
rot_x = rotate_x
rot_y = rotate_y
rot_z = rotate_z

-- Transform which converts the z-axis to the y-axis; this is useful
-- because many scene files are set up that way.
--
xform_z_to_y = rotate_x (-math.pi / 2)
xform_y_to_z = xform_z_to_y:inverse ()

-- Transform which converts the x-axis to the y-axis.
--
xform_x_to_y = rotate_z (-math.pi / 2)
xform_y_to_x = xform_x_to_y:inverse ()


-- Transform which inverts the z-axis (as many models use a different
-- convention for the z-axis).
--
xform_flip_x = scale (-1, 1, 1)
xform_flip_y = scale (1, -1, 1)
xform_flip_z = scale (1, 1, -1)


----------------------------------------------------------------
--
-- GC protection
--
-- Swig's handling of garbage collection trips us up in various cases:
-- objects stored in the scene get GCed because the Lua garbage
-- collector doesn't know that there's a reference from one userdata
-- object to another.
--
-- To prevent this, we keep a table in Lua of external object
-- references, in a form that the garbage collector can follow.
--

local gc_refs = {}

-- Make the keys in GC_REFS weak so that entries don't prevent referring
-- objects from being garbage collected (if nobody else refers to them).
--
setmetatable (gc_refs, { __mode = 'k' })

-- Add a reference from FROM to TO for the garbage-collector to follow.
-- This is for adding references that the normal gc mechanism cannot
-- deduce by itself, e.g. in userdata objects.
--
function gc_ref (from, to)
   local refs = gc_refs[from]
   if refs then
      refs[#refs + 1] = to
   else
      gc_refs[from] = { to }
   end
end


----------------------------------------------------------------
--
-- scene object
--
-- We don't use the raw scene object directly because we need to
-- gc-protect objects handed to the scene.

local function init_scene (raw_scene)
   scene = raw_scene		-- this is exported

   if not has_index_wrappers (scene) then
      local wrap = index_wrappers (scene)

      function wrap:add (thing)
	 gc_ref (self, thing)
	 return nowrap_meth_call (self, "add", thing)
      end
   end
end


----------------------------------------------------------------
--
-- meshes

mesh = raw.Mesh

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin.  Only a single scale factor is used for all
-- dimensions, so that a transformed object isn't distorted, merely
-- resized/translated.
--
function normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = midpoint (bbox.max, bbox.min)
   local max_size = bbox:max_size ()

   return translate (-center.x, -center.y, -center.z) * scale (2 / max_size)
end

-- Return a transform which will warp SURF to be in a 2x2x2 box centered
-- at the origin in the x and z dimensions, but with a minimum y value
-- of zero (so it has a "zero y base").  Only a single scale factor is
-- used for all dimensions, so that a transformed object isn't
-- distorted, merely resized/translated.
--
function y_base_normalize_xform (surf)
   local bbox = surf:bbox ()
   local center = midpoint (bbox.max, bbox.min)
   local size = bbox.max - bbox.min
   local max_size = bbox:max_size ()

   return translate (-center.x, size.y / 2 - center.y, -center.z)
      * scale (2 / max_size)
end

-- Resize a mesh to fit in a 1x1x1 box, centered at the origin (but with
-- the bottom at y=0).  Returns MESH.
--
function normalize (mesh, xf)
   local norm = y_base_normalize_xform (mesh)
   if xf then norm = norm * xf end
   mesh:transform (norm)
   return mesh
end


----------------------------------------------------------------
--
-- Misc surface types

sphere = raw.Sphere
sphere2 = raw.Sphere2

tripar = raw.Tripar

function triangle (mat, v0, e1, e2)
   return tripar (mat, v0, e1, e2, false)
end

function rectangle (mat, v0, e1, e2)
   return tripar (mat, v0, e1, e2, true)
end

ellipse = raw.Ellipse

cylinder = raw.Cylinder

-- Wrap the subspace constructor to record the GC link between a
-- subspace and the surface in it.
--
function subspace (surf)

   -- If SURF is actually a table, make a surface-group to hold its
   -- members, and wrap that instead.
   --
   if type (surf) == "table" then
      if #surf == 1 then
	 surf = surf[1]
      else
	 surf = surface_group (surf)
      end
   end

   local ss = raw.Subspace (surf)

   -- Record the GC link between SS and SURF.
   --
   gc_ref (ss, surf)

   return ss
end

-- Wrap the instance constructor to record the GC link between an
-- instance and its subspace.
--
function instance (subspace, xform)
   local inst = raw.Instance (subspace, xform)
   gc_ref (inst, subspace)
   return inst
end

-- Wrap the surface_group constructor to add some method wrappers to it,
-- and support adding a table of surfaces as well.
--
function surface_group (surfs)
   local group = raw.SurfaceGroup ()

   -- Initialize wrapper functions if necessary
   --
   if not has_index_wrappers (group) then
      local wrap = index_wrappers (group)

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
	    gc_ref (self, surf)
	    nowrap_meth_call (self, "add", surf)
	 end
      end
   end

   if surfs then
      group:add (surfs)
   end

   return group
end


----------------------------------------------------------------
--
-- Lights

function point_light (pos, intens)
   return raw.PointLight (pos, color (intens))
end

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


function filename_dir (filename)
   return string.match (filename, "^(.*)/[^/]*$")
end
function filename_ext (filename)
   return string.match (filename, "[.]([^./]*)$")
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

function eval_include (loaded, fenv, loaded_filename, err_msg)
   if loaded then
      local old_cur_filename = cur_filename
      cur_filename = loaded_filename

      setfenv (loaded, fenv)
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
   local callers_env = getfenv (2)
   eval_include (loaded, callers_env, loaded_filename, err_msg)
   return loaded_filename
end


-- Map of filenames to the environment in which they were loaded.
--
local use_envs = {}

-- A metatable for inheriting from the snogray environment
--
local inherit_snogray_metatable = { __index = snogray }

-- A table containing entries for symbol names we _don't_ want inherited
-- because of a call to use().
--
local dont_inherit_syms = {}
dont_inherit_syms["_used_files"] = true
dont_inherit_syms["_used_symbols"] = true

-- Load FILENAME in a dedicated environment, and arrange for its
-- functions and constants to be inherited by the caller's environment.
-- FILENAME is only loaded if it hasn't already been loaded (if it has,
-- then the previously loaded file is re-used).
--
-- "Inheritance" is done by copying, so that it only works properly for
-- functions and constants.
--
-- The variable "_used_files" in the caller's environment is used to
-- record which use() environments have previously been added to its
-- inheritance set.  It maps loaded filenames to the environment in
-- which they were loaded.  The variable "_used_symbols" in the caller's
-- environment is where symbols to be inherited are copied; the caller's
-- environment then inherits from this.
--
function use (filename)
   local loaded, loaded_filename, err_msg = load_include (filename)

   local callers_env = getfenv (2)
   local used_files = callers_env._used_files

   if not used_files then
      used_files = {}
      callers_env._used_files = used_files
   end

   if not used_files[loaded_filename] then
      local use_env = use_envs[loaded_filename]

      -- If this file hasn't been loaded already, load it into a new
      -- environment.
      --
      if not use_env then
	 use_env = {}
	 use_envs[loaded_filename] = use_env

	 -- Make sure the loaded file inherits the snogray interface.
	 --
	 setmetatable (use_env, inherit_snogray_metatable)

	 eval_include (loaded, use_env, loaded_filename, err_msg)
      end

      -- Arrange for the symbols defined in USE_ENV to be inherited
      -- by the caller.  Because we need "multiple inheritance" (more
      -- than one call to use()), instead of directly using Lua
      -- inheritance, we instead copy the symbols defined by each
      -- use'd file into a table, and then use Lua inheritance to
      -- inherit from it.
      --
      -- [Note that we can't copy the symbols directly into the caller's
      -- environment because we don't want them to be seen by anybody
      -- that calls use() on our caller.]
      --
      local used_syms = callers_env._used_symbols
      if not used_syms then
	 used_syms = {}
	 callers_env._used_symbols = used_syms

	 -- Replace the caller's metatable with our own metatable
	 -- that inherits from USED_SYMS; thus USED_SYMS needs to
	 -- inherit in turn from the global snogray environment.
	 --
	 setmetatable (used_syms, inherit_snogray_metatable)
	 setmetatable (callers_env, {__index = used_syms}) 
      end

      -- Copy all symbols defined by the loaded file (in USE_ENV) into
      -- the caller's "inheritance set" (USED_SYMS), where they can be
      -- seen by the caller.  We avoid copying any symbols listed in
      -- dont_inherit_syms.
      --
      for k, v in pairs (use_env) do
	 if not dont_inherit_syms[k] then
	    used_syms[k] = v
	 end
      end

      -- Remember that the caller has loaded this file.
      --
      used_files[loaded_filename] = use_env
   end

   return loaded_filename
end


----------------------------------------------------------------
--
-- Autoloading

-- Add a stub for file-extension EXT to LOADER_TABLE that will load
-- LOADER_FILE and call the function named LOADER_NAME (which
-- LOADER_FILE must define); the stub will also install that function
-- into LOADER_TABLE so that it can be called directly for subsequent
-- files of the same type.
--
local function add_autoload_stub (loader_table, ext, loader_file, loader_name)
   loader_table[ext]
      = function (...)
	   print ("* autoloading: "..loader_file)

	   local contents, err = loadfile (loader_file)

	   if contents then
	      local environ = {}
	      setmetatable (environ, inherit_snogray_metatable)
	      setfenv (contents, environ)

	      contents ()	-- Finish loading

	      local loader = environ[loader_name]
	      if loader then
		 loader_table[ext] = loader
		 return loader (...)
	      else
		 error ("loading "..loader_file.." didn't define "..loader_name)
	      end
	   else
	      error (err, 0)
	   end
	end
end


----------------------------------------------------------------
--
-- Scene loading

-- Table of scene loaders for various file extensions.
--
local scene_loaders = {}


-- Load a scene from FILENAME into RSCENE and RCAMERA (the "raw" scene
-- and camera objects).
--
-- Return true for a successful load, false if FILENAME is not
-- recognized as loadable, or an error string if an error occured during
-- loading.
--
function load_scene (filename, fmt, ...)
   local loader = scene_loaders[fmt]
   if loader then
      return loader (filename, ...)
   else
      return false
   end
end


----------------------------------------------------------------
--
-- Mesh loading

-- Table of mesh loaders for various file extensions.
--
local mesh_loaders = {}


-- Load a mesh from FILENAME into MESH.
--
-- Return true for a successful load, false if FILENAME is not
-- recognized as loadable, or an error string if an error occured during
-- loading.
--
function load_mesh (filename, fmt, ...)
   local loader = mesh_loaders[fmt]
   if loader then
      return loader (filename, ...)
   else
      return false
   end
end


add_autoload_stub (mesh_loaders, "obj", "load-obj.lua", "load_obj")
add_autoload_stub (mesh_loaders, "ug", "load-ug.lua", "load_ug")


----------------------------------------------------------------
--
-- Lua scene description loader

-- Load Lua scene description from FILENAME into RSCENE and RCAMERA.
-- Return true if the scene was loaded successfully, or an error string.
--
function scene_loaders.lua (filename, rscene, rcamera)

   -- Load the user's file!  This just constructs a function from the
   -- loaded file, but doesn't actually evaluate it.
   --
   local contents, err = loadfile (filename)

   if not contents then
      error (err, 0)		-- propagate the loading error
   end

   -- Make a new environment to evaluate the file contents in; it will
   -- inherit from "snogray" for convenience.  There are no global
   -- pointers to this table so it and its contents will be garbage
   -- collected after loading.
   --
   local environ = {}
   setmetatable (environ, inherit_snogray_metatable)
   setfenv (contents, environ)

   -- Remember filename being loaded, so we can find other files in
   -- the same location.
   --
   cur_filename = filename

   -- Set up the scene object for the user code to use.
   --
   init_scene (rscene)

   -- Let users use the raw camera directly.
   --
   camera = rcamera

   -- Finally, evaluate the loaded file!
   --
   contents ()

   return true
end

-- Other types of Lua file.
--
scene_loaders.luac = scene_loaders.lua
scene_loaders.luo = scene_loaders.lua


-- arch-tag: e5dc4da4-c3f0-45e7-a4a1-a20cb4db6d6b
