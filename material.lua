-- material.lua -- Material support
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
local material = {}

-- imports
--
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"
local color = require 'snogray.color'

local texture = require 'snogray.texture'
local color_tex_val = texture.color_tex_val
local float_tex_val = texture.float_tex_val


----------------------------------------------------------------
-- Material predicates
--

-- Return true if OBJ is a snogray material.
--
local function is_material (val)
   return swig.type (val) == 'Material'
end
material.is_material = is_material


----------------------------------------------------------------
-- Support functions
--

-- Return true if VAL refers to a color, color texture, or something
-- that looks like it can be converted to a color or color texture.
--
local function is_color_val (val)
   return color.is_color_spec (val) or texture.is_color_tex (val)
end

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
	 mat = material.stencil (opacity, mat)
      end
   end

   return mat
end   


----------------------------------------------------------------
-- Indices of refraction
--

-- Return true if OBJ is an "index of refraction".
--
local function is_ior (val)
   return swig.type (val) == "Ior"
end
material.is_ior = is_ior

-- Return true if OBJ is something the material.ior function would
-- recognize.
--
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
material.is_ior_spec = is_ior_spec

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
material.ior = ior


----------------------------------------------------------------
-- Individual materials
--

-- Return a lambertian material with diffuse color DIFF (default 0.5),
-- and generic material properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "diff" / "diffuse" / 1  --  diffuse reflection color
--
-- + any generic material properties
--
function material.lambert (diff, params)
   if not is_color_val (diff) then
      -- handle single table argument case (other args should be nil)
      params = diff
      diff = params.diffuse or params.diff or params.color or diff[1] or 1
   end

   -- defaults
   diff = diff or 0.5

   -- convert to primitive type
   diff = color_tex_val (diff)

   return postproc_material (raw.lambert (diff), params)
end

-- Return a Cook-Torrance material with diffuse color DIFF, specular
-- color SPEC (default white), microfact slope M (default 0.1; larger
-- means duller), index of refraction _IOR (default 1.5), and generic
-- material properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "diff" / "diffuse" / 1   --  diffuse reflection color
--   "spec" / "specular" / 2  --  specular reflection color
--   "m"                      --  microfact slope
--   "ior"                    --  index of reflection
--
-- + any generic material properties
--
function material.cook_torrance (diff, spec, m, _ior, params)
   if not is_color_val (diff) then
      -- handle single table argument case (other args should be nil)
      params = diff
      diff = params.diffuse or params.diff or params.d
	 or params.color or params[1]
      spec = params.specular or params.spec or params.s
	 or params[2]
      m = params.m or params[3]
      _ior = params.ior or params[4]
   end

   -- defaults
   spec = spec or color.white
   m = m or 0.1
   _ior = _ior or 1.5

   -- convert to primitive types
   diff = color_tex_val (diff)
   spec = color_tex_val (spec)
   m = float_tex_val (m)
   _ior = ior (_ior)

   return postproc_material (raw.cook_torrance (diff, spec, m, _ior), params)
end

local default_mirror_ior = ior (0.25, 3)

-- Return a mirror material with reflectance REFLECT (default white,
-- meaning perfect reflectance), index of refraction _IOR (default
-- 1.5), underlying material UNDER (default black), and generic
-- material properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "reflect" / "reflectance" / 1  --  reflectance
--   "ior" / 3                      --  index of reflection
--   "underlying" / "under"/ 4      --  underlhying material
--
-- + any generic material properties
--
-- If the first parameter is an index of refraction, it is used as the
-- mirror's index of refraction, and other parameters are defaulted.
--
function material.mirror (reflect, _ior, under, params)
   if not is_color_val (reflect) then
      -- assume single argument (other args should be nil)
      if is_ior_spec (params) then
	 -- IOR only case
	 _ior = reflect
      else
	 -- single table argument case
	 params = reflect
	 reflect = params.reflect or params.reflectance or params.refl or params[1]
	 _ior = params.ior or params[2]
	 under = params.underlying or params.under or params[3]
      end
   end

   -- defaults
   reflect = reflect or color.white
   _ior = _ior or default_mirror_ior
   under = under or color.black

   -- convert to primitive types
   _ior = ior (_ior)
   reflect = color_tex_val (reflect)
   if not is_material (under) then
      under = color_tex_val (under)
   end

   return postproc_material (raw.mirror (_ior, reflect, under), params)
end

-- Return a glass material with index of refraction _IOR (default
-- 1.5), absorption ABSORB (default black, meaning no absorption), and
-- generic material properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "ior" / 1                      --  index of reflection
--   "absorb" / "absorption" / 2    --  absorption
--
-- + any generic material properties
--
function material.glass (_ior, absorb, params)
   if not is_ior_spec (_ior) then
      -- handle single table argument case (other args should be nil)
      params = _ior
      _ior = params.ior or params[1]
      absorb = params.absorption or params.absorb or params[2]
   end

   -- defaults
   _ior = _ior or 1.5
   absorb = absorb or color.black

   -- convert to primitive types
   _ior = ior (_ior)
   absorb = color.std (absorb)

   return postproc_material (raw.glass (raw.Medium (_ior, absorb)), params)
end

-- Return a thin-glass material with index of refraction _IOR (default
-- 1.5), color COL (default white, meaning transparent), and generic
-- material properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "ior" / 1              --  index of reflection
--   "color" / "col" / 2    --  color
--
-- + any generic material properties
--
function material.thin_glass (_ior, col, params)
   if not is_ior_spec (_ior) then
      -- handle single table argument case (other args should be nil)
      params = _ior
      _ior = params.ior or params[1]
      col = params.color or params.col or params[2]
   end

   -- defaults
   _ior = _ior or 1.5
   col = col or color.white

   -- convert to primitive types
   _ior = ior (_ior)
   col = color.std (col)

   return postproc_material (raw.thin_glass (col, _ior), params)
end

-- Return a "glow" (light-emitting) material with color COL,
-- underlying material UNDER (default nothing), and generic material
-- properties PARAMS.
--
-- A single table may also be passed, in which case it can contain the
-- following entries:
--
--   "color" / "col" / 1           --  color
--   "underlying" / "under" / 2    --  underlying material
--
-- + any generic material properties
--
function material.glow (col, under, params)
   if not is_color_val (col) then
      -- assume single argument (other args should be nil)
      params = col
      col = params.color or params.col or params[1]
      under = params.underlying or params.under or params[2]
   end

   -- convert to primitive types
   col = color_tex_val (col)

   if under then
      return raw.glow (col, under)
   else
      return raw.glow (col)
   end
end   

function material.norm_glow (intens)
   return raw.norm_glow (intens or 1)
end   

-- Return a stencil material, which makes an underlying material
-- transparent or translucent, with degree of opacity OPACITY, and
-- underlying material UNDER
--
-- [Note that unlike most material constructors, this has no "table
-- form".]
--
function material.stencil (opacity, under)
   opacity = color_tex_val (opacity)
   return raw.stencil (opacity, under)
end


----------------------------------------------------------------
-- material dicts
--

function material.is_material_dict (val)
   return swig.type (val) == 'MaterialDict'
end

function material.material_dict (init)
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


-- return the module
--
return material
