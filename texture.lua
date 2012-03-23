-- texture.lua -- Texture support
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
local texture = {}

-- imports
--
local swig = require 'snogray.swig'
local color = require 'snogray.color'
local raw = require 'snogray.snograw'


----------------------------------------------------------------
------------------------------ HACK ----------------------------
-- Until we have a separate snogray.xform module, duplicate these
-- functions from the snogray.snogray module here (we can't require
-- the snogray.snogray module without a require loop, because it
-- requires this module).
local function is_xform (val)
   return swig.type (val) == 'Xform'
end
local scale = raw.Xform_scaling
---------------------------- END HACK --------------------------
----------------------------------------------------------------


----------------------------------------------------------------
-- Texture predicates
--

local function is_float_tex (val)
   return swig.type (val) == 'Tex<float>'
end
texture.is_float_tex = is_float_tex

local function is_color_tex (val)
   return swig.type (val) == 'Tex<Color>'
end
texture.is_color_tex = is_color_tex

local function is_tex (val)
   local type = swig.type (val)
   return type == 'Tex<Color>' or type == 'Tex<float>'
end
texture.is_tex = is_tex


----------------------------------------------------------------
-- Texture values ("tex-vals")
--
-- These are the basic input type to many snogray primitives.  They
-- can refer to either a real texture, or a constant value.
--

-- Return VAL, which should either be a color or a texture, boxed into
-- a TexVal<Color> container.
--
local function color_tex_val (val)
   if is_float_tex (val) then
      val = raw.grey_tex (raw.FloatTexVal (val))
   elseif color.is_color_spec (val) then
      val = color.std (val)
   elseif not is_color_tex (val) then
      error ("Invalid color texture "..tostring(val), 0)
   end
   return raw.ColorTexVal (val)
end
texture.color_tex_val = color_tex_val

-- Return VAL, which should either be a float or a texture, boxed into
-- a TexVal<float> container.
--
local function float_tex_val (val)
   if is_color_tex (val) then
      val = raw.intens_tex (raw.ColorTexVal (val))
   elseif type (val) ~= 'number' and not is_float_tex (val) then
      error ("Invalid float texture "..tostring(val), 0)
   end
   return raw.FloatTexVal (val)
end
texture.float_tex_val = float_tex_val

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
      return raw.ColorTexVal (color.std (tex))
   end
end
texture.tex_val = tex_val

-- Return VAL1 and VAL2 boxed into a pair of either TexVal<Color> or
-- TexVal<float> containers, whichever are appropriate.  Both VAL1 and
-- VAL2 are examined to make the decision; a mixture of Color and
-- float values results in the floating value being automatically
-- converted to a Color value to match.
--
local function tex_vals (val1, val2)
   if is_color_tex (val1) or is_color_tex (val2)
      or (type(val1) ~= 'number' and color.is_color_spec (val1))
      or (type(val2) ~= 'number' and color.is_color_spec (val2))
   then
      return color_tex_val (val1), color_tex_val (val2)
   else
      return float_tex_val (val1), float_tex_val (val2)
   end
end
texture.tex_vals = tex_vals


----------------------------------------------------------------
-- Miscellaneous texture sources and operators
--

-- Image textures (read from a file)
--
texture.image = raw.image_tex
texture.mono_image = raw.mono_image_tex

-- Return a "grey_tex" texture object using the floating-point texture
-- VAL as a source.  This can be used to convert a floating-point
-- texture into a color texture.
--
function texture.grey (val) return raw.grey_tex (float_tex_val (val)) end

-- Return a "intens_tex" texture object using the color texture VAL as
-- a source.  This can be used to convert a color-point texture into a
-- floating-point texture.
--
function texture.intens (val) return raw.intens_tex (color_tex_val (val)) end

-- Return a "check" texture, which evaluates to either TEX1 or TEX2 in
-- a check pattern.
--
function texture.check (tex1, tex2)
   return raw.check_tex (tex_vals (tex1, tex2))
end
function texture.check3d (tex1, tex2)
   return raw.check3d_tex (tex_vals (tex1, tex2))
end

function texture.perturb_pos (src, x, y, z)
   return raw.perturb_pos_tex (tex_val (src), float_tex_val (x),
			       float_tex_val (y), float_tex_val (z))
end
function texture.perturb_uv (src, u, v)
   return raw.perturb_uv_tex (tex_val (src),
			      float_tex_val (u), float_tex_val (v))
end

-- Return an interpolation texture, which interpolates between two
-- textures according to the value of its control parameter.
--
function texture.linterp (control, val1, val2)
   return raw.linterp_tex (float_tex_val (control), tex_vals (val1, val2))
end
function texture.sinterp (control, val1, val2)
   return raw.sinterp_tex (float_tex_val (control), tex_vals (val1, val2))
end

-- Return a texture which rescales VAL from the range [IN_MIN, IN_MAX]
-- to the range [OUT_MIN, OUT_MAX].  The default output range is [0,1].
--
function texture.rescale (val, in_min, in_max, out_min, out_max)
   val = tex_val (val)
   out_min = out_min or 0
   out_max = out_max or 1
   if swig.type (val) == 'TexVal<Color>' then
      in_min = color.std (in_min)
      in_max = color.std (in_max)
      out_min = color.std (out_min)
      out_max = color.std (out_max)
   end
   return raw.rescale_tex (val, in_min, in_max, out_min, out_max)
end

texture.plane_map = raw.plane_map_tex
texture.cylinder_map = raw.cylinder_map_tex
texture.lat_long_map = raw.lat_long_map_tex

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

texture.perlin = singleton_tex_fun ('perlin', raw.perlin_tex)
texture.perlin_abs
   = singleton_tex_fun ('perlin_abs',
			function() return texture.abs (texture.perlin ()) end)

texture.x = singleton_tex_fun ('x', function () return raw.coord_tex(0) end)
texture.y = singleton_tex_fun ('y', function () return raw.coord_tex(1) end)
texture.z = singleton_tex_fun ('z', function () return raw.coord_tex(2) end)
texture.u = singleton_tex_fun ('u', function () return raw.coord_tex(3) end)
texture.v = singleton_tex_fun ('v', function () return raw.coord_tex(4) end)

texture.worley = raw.worley_tex

local worley_id_kinds = { SCALE = 0, MOD = 1, scale = 0, mod = 1 }

function texture.worley_id (kind, min, max)
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
-- giving them to TEX), so for instance, to make TEX get "smaller",
-- you would use a value of XFORM which scales by an amount greater
-- than 1.
--
function texture.xform (xform, tex)
   return raw.xform_tex (xform, tex_val (tex))
end

-- Convenience functions for various sorts of texture transformations.
--
function texture.scale (amount, tex)
   return texture.xform (texture.scale (amount), tex)
end
function texture.rotate (amount, tex)
   return texture.xform (texture.rotate (amount), tex)
end
texture.rot_tex = texture.rotate


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
-- mixture of both results in the floating-point texture being
-- converted to color before applying the operation).
--
function texture.arith (op, arg1, arg2)
   op = arith_tex_ops[op]
   return raw.arith_tex (op, tex_vals (arg1, arg2))
end

-- Alias for the arith_tex MUL operation.  This function treats the
-- second operand specially because it is used to overload the "*"
-- operator for textures, which we want to work for texture-xform
-- operations too.
--
function texture.mul (tex1, tex2_or_xform)
   if is_xform (tex2_or_xform) then
      return texture.xform (tex2_or_xform, tex1)
   else
      return texture.arith ('MUL', tex1, tex2_or_xform)
   end
end

-- Convenient aliases for the various other arith_tex operations.
--
function texture.add (...) return texture.arith ('ADD', ...) end
function texture.sub (...) return texture.arith ('SUB', ...) end
function texture.div (...) return texture.arith ('DIV', ...) end
function texture.mod (...) return texture.arith ('MOD', ...) end
function texture.pow (...) return texture.arith ('POW', ...) end
function texture.floor (x, y) return texture.arith ('FLOOR', x, y or 1) end
function texture.ceil (x, y) return texture.arith ('CEIL', x, y or 1) end
function texture.trunc (x, y) return texture.arith ('TRUNC', x, y or 1) end
function texture.min (...) return texture.arith ('MIN', ...) end
function texture.max (...) return texture.arith ('MAX', ...) end
function texture.avg (...) return texture.arith ('AVG', ...) end
function texture.mirror (...) return texture.arith ('MIRROR', ...) end
function texture.abs (tex) return texture.arith ('MIRROR', tex, 0) end
function texture.neg (tex) return texture.arith ('SUB', 0, tex) end
function texture.sin (x, y) return texture.arith ('SIN', x, y or 2*math.pi) end
function texture.cos (x, y) return texture.arith ('COS', x, y or 2*math.pi) end
function texture.tan (x, y) return texture.arith ('TAN', x, y or 2*math.pi) end
function texture.atan2 (...) return texture.arith ('ATAN2', ...) end

-- Install operator overloads for the texture metatable MT.
--
local function setup_tex_metatable (mt)
   mt.__add = texture.add
   mt.__sub = texture.sub
   mt.__mul = texture.mul
   mt.__div = texture.div
   mt.__unm = texture.neg
   mt.__pow = texture.pow
   mt.__mod = texture.mod
end

-- There's a metatable for each underlying texture datatype, currently
-- Color and float.  We install the same overload functions for both.
--
setup_tex_metatable (getmetatable (texture.intens (0))) -- float
setup_tex_metatable (getmetatable (texture.grey (0))) -- Color


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
function texture.cmp (op, cval1, cval2, rval1, rval2)
   op = cmp_tex_ops[op]
   return raw.cmp_tex (op, float_tex_val (cval1), float_tex_val (cval2),
   		       tex_vals (rval1, rval2))
end

-- Convenient aliases for the various cmp_tex operations.
--
function texture.eq (...) return texture.cmp_tex ('EQ', ...) end
function texture.ne (...) return texture.cmp_tex ('NE', ...) end
function texture.lt (...) return texture.cmp_tex ('LT', ...) end
function texture.le (...) return texture.cmp_tex ('LE', ...) end
function texture.gt (...) return texture.cmp_tex ('GT', ...) end
function texture.ge (...) return texture.cmp_tex ('GE', ...) end


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
-- portions of the sum, Sum[...] (F_i * OMEGA^(i-1)), so that it
-- should have roughly the same magnitude as SOURCE_TEX.  Any
-- non-constant F_i values are ignored for the purposes of
-- auto-scaling.
--
function texture.fourier_series (source_tex, params)
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

-- Call fourier_series using perlin noise as the input texture.
-- See description of fourier_series for an explanation of PARAMS.
--
function texture.perlin_series (params)
   return texture.fourier_series (texture.perlin(), params)
end

-- Call fourier_series using the absolute value of perlin noise as
-- the input texture.  See description of fourier_series for an
-- explanation of PARAMS.
--
function texture.perlin_abs_series (params)
   return texture.fourier_series (texture.perlin_abs(), params)
end


-- return the module
--
return texture
