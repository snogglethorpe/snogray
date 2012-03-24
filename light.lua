-- light.lua -- Light support
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
local light = {}

-- imports
--
local raw = require "snogray.snograw"
local color = require 'snogray.color'
local image = require 'snogray.image'


function light.point (pos, intens, ...)
   return raw.PointLight (pos, color.std (intens), ...)
end

function light.sphere (pos, radius, intens)
   return raw.SphereLight (pos, radius, color.std (intens))
end

function light.triangle (corner, side1, side2, intens)
   return raw.TriparLight (corner, side1, side2, false, color.std (intens))
end

function light.far (dir, angle, intens)
   return raw.FarLight (dir:unit(), angle, color.std (intens))
end

function light.envmap (image_or_filename, ...)
   if type (image_or_filename) == 'string' then
      image_or_filename = image.new (image_or_filename)
   end
   return raw.EnvmapLight (raw.envmap (image_or_filename), ...)
end


-- return the module
--
return light
