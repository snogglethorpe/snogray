-- transform.lua -- Transform support
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
local transform = {}

-- imports
--
local swig = require 'snogray.swig'
local raw = require "snogray.snograw"


-- Return true if VAL is a transform.
--
function transform.is_transform (val)
   return swig.type (val) == 'Xform'
end

transform.new = raw.Xform

-- Return a transform given the elements of a 4x4 transform matrix in
-- row-major order.
--
function transform.matrix (...)
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

transform.identity = raw.Xform_identity

transform.scale = raw.Xform_scaling
transform.translate = raw.Xform_translation
transform.rotate = raw.Xform_rotation
transform.rotate_x = raw.Xform_x_rotation
transform.rotate_y = raw.Xform_y_rotation
transform.rotate_z = raw.Xform_z_rotation
transform.basis = raw.Xform_basis

-- Some abbreviations (a bit silly, but composed transforms can get
-- rather long...).
--
transform.trans = transform.translate
transform.rot = transform.rotate
transform.rot_x = transform.rotate_x
transform.rot_y = transform.rotate_y
transform.rot_z = transform.rotate_z

transform.z_to_y = transform.rotate_x (-math.pi / 2)
transform.y_to_z = transform.z_to_y:inverse ()
transform.x_to_y = transform.rotate_z (-math.pi / 2)
transform.y_to_x = transform.x_to_y:inverse ()

transform.flip_x = transform.scale (-1, 1, 1)
transform.flip_y = transform.scale (1, -1, 1)
transform.flip_z = transform.scale (1, 1, -1)


--
-- Not yet documented:
--
--  + transform.matrix
--  + transform.identity
--  + transform.is_transform
--  + transform.scale
--  + transform.translate
--  + transform.rotate
--  + transform.rotate_x
--  + transform.rotate_y
--  + transform.rotate_z
--  + transform.basis
--
--  + all_in_one.translate
--  + all_in_one.rotate
--  + all_in_one.rotate_x
--  + all_in_one.rotate_y
--  + all_in_one.rotate_z
--
--  + transform.z_to_y
--  + transform.y_to_z
--  + transform.x_to_y
--  + transform.y_to_x
--
--  + transform.flip_x
--  + transform.flip_y
--  + transform.flip_z
--



-- return the module
--
return transform
