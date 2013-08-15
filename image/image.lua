-- image.lua -- Lua image handling for snogray
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
local image = {}

-- imports
--
local raw = require "snogray.snograw"
local load = require "snogray.load"


function image.new (arg1, ...)
   return raw.image (load.filename_in_cur_load_directory (arg1), ...)
end

image.sampled_output = raw.ImageSampledOutput
image.scaled_output = raw.ImageScaledOutput
image.input = raw.ImageInput
image.recover = raw.recover_image
image.row = raw.ImageRow


-- return module
--
return image
