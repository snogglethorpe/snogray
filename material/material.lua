-- material.lua -- Material support
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

-- material.is_material -- Material predicate
--
-- args: (OBJ)
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
-- Parameters handled:
--
--   bump_map / bump
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

-- material.is_ior -- IOR (index-of-refraction) predicate
--
-- args: (OBJ)
--
-- Return true if OBJ is an "index of refraction" object.
--
local function is_ior (val)
   return swig.type (val) == "Ior"
end
material.is_ior = is_ior

-- material.is_ior_spec -- IOR "spec" predicate
--
-- args: (OBJ)
--
-- Return true if OBJ is something that can be turned into an IOR:
-- either an IOR object (as the material.ior function would return),
-- or a valid argument for the material.ior function.
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

-- material.ior -- Return an IOR ("index of refraction")
--
-- args: (N, K)
--   or: { n = REAL_IOR, k = IMAG_IOR }
--
-- Return an index-of-refraction object, where N and K are its real
-- and imaginary components.
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

-- material.lambert -- Return a lambertian material
--
-- args: (DIFF, [PARAMS])
--   or: { diffuse = DIFF, ...PARAMS... }
--
-- DIFF (default 0.5) is the color of diffuse reflection; it may be a
-- color, a grey-level, or a texture of either sort.  PARAMS is a
-- table of generic material parameters.
--
-- The "diffuse" keyword argument may also be spelled "diff" or "color".
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

-- material.cook_torrance -- Return a Cook-Torrance material
--
-- A Cook-Torrance material is a "physically-based" surface model.
-- It can range from dull to glossy, both plastic-like and metallic
-- materials (the latter by using an imaginary IOR).
--
-- args: (DIFF, SPEC, M, IOR, PARAMS)
--   or: { diffuse = DIFF, specular = SPEC, m = M, ior = IOR, ...PARAMS... }
-- 
-- DIFF is the color of diffuse reflection, SPEC (default white) is
-- the color of specular reflection, M (default 0.1) is the "microfacet
-- slope" (larger means duller / less shiny), IOR (default 1.5) is the
-- index of refraction.  DIFF and SPEC may be colors, grey-levels, or
-- textures of either sort.  M may be a number or a floating-point
-- texture.  PARAMS is a table of generic material parameters.
--
-- All parameters except DIFF are optional.
--
-- The "diffuse" keyword argument may also be spelled "diff" or "d".
-- The "specular" keyword argument may also be spelled "spec" or "s".
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

-- material.mirror -- Return a mirror material
--
-- args: (REFLECT, IOR, UNDER, PARAMS)
--   or: (IOR)
--   or: { reflect = REFLECT, ior = IOR, underlying = UNDER, ...PARAMS... }
--
-- REFLECT (default white, meaning perfect reflectance) is the
-- reflectance, IOR (default 1.5) is the index of refraction, UNDER
-- (default black) is an underlying material.  REFLECT may be a color,
-- a grey-levels, or a textures of either sort.  UNDER should be
-- either nil (for "nothing") or a material.  PARAMS is a table of
-- generic material parameters.
--
-- All parameters are optional.
--
-- The "reflect" keyword argument may also be spelled "reflectance".
-- The "underlying" keyword argument may also be spelled "under".
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

-- material.glass -- Return a glass (transparent, volumetric) material
--
-- args: (IOR, ABSORB, PARAMS)
--   or: { ior = IOR, absorb = ABSORB, ...PARAMS... }
--
-- IOR (default 1.5) is the index of refraction, ABSORB (default no
-- absorption) the color/amount of light absorbed over a given
-- distance within the glass.  PARAMS is a table of generic material
-- parameters.
--
-- All parameters are optional.
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

-- material.thin_glass -- Return a "thin-glass" (transparent,
--   non-volumetric) material
--
-- args: (IOR, COLOR, PARAMS)
--   or: { ior = IOR, color = COLOR, ...PARAMS... }
--
-- IOR (default 1.5) is the index of refraction, COLOR (default
-- transparent) is the color cast given to light passing through.
-- PARAMS is a table of generic material parameters.
--
-- All parameters are optional.
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

-- material.glow -- Return a light-emitting material
--
-- args: (COLOR, UNDER, PARAMS)
--   or: { color = COLOR, underlying = UNDER, ...PARAMS... }
--
-- COLOR is the intensity of the emitted light; it may be a color, a
-- grey-level, or a texture of either sort.  UNDER (default nothing)
-- is a material that controls reflection, or nil, meaning no
-- reflection.  PARAMS is a table of generic material parameters.
--
-- All parameters except COLOR are optional.
--
-- The "color" keyword argument may also be spelled "col".
-- The "underlying" keyword argument may also be spelled "under".
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

-- material.norm_glow -- Return a light-emitting material for debugging
--
-- This special material emits light whose color corresponds to the
-- normal of the underlying surface.
--
-- args: (INTENS)
--
-- INTENS is the intensity of the emitted light.
--
function material.norm_glow (intens)
   return raw.norm_glow (intens or 1)
end   

-- material.stencil -- Return "stencil" material
--
-- A stencil material can make an underlying material transparent or
-- translucent, depending on the intensity (which may be a texture).
--
-- args: (OPACITY, UNDER)
--
-- OPACITY is the degree to which the underlying material shows
-- through, UNDER is the underlying material.  OPACITY may be a number
-- or a floating-point number; a value of 1 means UNDER is fully
-- represented, and 0 is the same as no material at all.
--
function material.stencil (opacity, under)
   opacity = color_tex_val (opacity)
   return raw.stencil (opacity, under)
end


----------------------------------------------------------------
-- material dicts (obsolete?)
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
