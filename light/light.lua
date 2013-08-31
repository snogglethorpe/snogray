-- light.lua -- Light support
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
local light = {}

-- imports
--
local raw = require "snogray.snograw"
local color = require 'snogray.color'
local image = require 'snogray.image'


--
-- Note that in addition to using explicit light objects, an arbitrary
-- surface can be made light-emitting by using the "material.glow"
-- material.
--


-- light.point -- Return a point light (or a spotlight)
--
-- args: (POS, COLOR)
--   or: (POS, COLOR, ANGLE, DIR, FRINGE_ANGLE)
--
-- POS is the position of the light, and COLOR is its power.
--
-- If ANGLE and DIR are supplied, it is a spotlight that radiates in a
-- cone whose center points in direction DIR and subtends angle ANGLE.
-- FRINGE_ANGLE is optional (default 0), if supplied, is the angle of
-- a region inside the outer edge where the light falls off towards
-- the edge (otherwise the light inside the cone is a constant
-- intensity).
--
function light.point (pos, intens, ...)
   return raw.PointLight (pos, color.std (intens), ...)
end

-- light.sphere -- Return a sphere light
--
-- args: (POS, RADIUS, INTENS)
--
-- POS is the position of the light, RADIUS its radius, and INTENS the
-- intensity (per unit area on the surface of the sphere) of the
-- emitted light.  Note that INTENS may not be a texture.
--
function light.sphere (pos, radius, intens)
   return raw.SphereLight (pos, radius, color.std (intens))
end

-- light.triangle -- Return a triangle light
--
-- args: (CORNER, SIDE1, SIDE2, INTENS)
--
-- The light is a triangle shape defined by a corner at position
-- CORNER and two sides SIDE1 and SIDE2 which are vectors from CORNER.
-- INTENS is the intensity (per unit area on the surface of the
-- triangle) of the emitted light, and may be a color.  Note that
-- INTENS may not be a texture.
--
function light.triangle (corner, side1, side2, intens)
   return raw.TriparLight (corner, side1, side2, false, color.std (intens))
end

-- light.far -- Return a "far" (at infinity) disk light
--
-- args: (DIR, ANGLE, INTENS)
--
-- This defines a light which is a disk at "infinity".  It appears to
-- be a disk whose center is in direction DIR (a vector) from every
-- point in the scene.  ANGLE is the angle between the sides of the
-- apparent cone with that disk as its base (the "size" of the disk is
-- also the same for every point in the scene).  INTENS is the
-- intensity of the light, and may be a color.
--
function light.far (dir, angle, intens)
   return raw.FarLight (dir:unit(), angle, color.std (intens))
end

-- light.envmap -- Return an environment-map light
--
-- args: (IMAGE_OR_FILENAME)
--   or: (IMAGE_OR_FILENAME, FRAME)
--
-- This defines a light which covers the entire sphere surrounding the
-- scene with an "environment map", which is an equirectangular
-- panorama.  If IMAGE_OR_FILENAME is a string, it should be the name
-- of an image file, otherwise it should be an image defined by some
-- other means.
--
-- If FRAME is supplied, it specifies the orientation of the
-- environment map.
--
function light.envmap (image_or_filename, ...)
   if type (image_or_filename) == 'string' then
      image_or_filename = image.new (image_or_filename)
   end
   return raw.EnvmapLight (raw.envmap (image_or_filename), ...)
end


-- return the module
--
return light
