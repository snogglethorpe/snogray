-- scene-cmdline.lua -- Command-line options for scene manipulation
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
local scene_cmdline = {}

-- imports
--
local clp = require 'snogray.cmdlineparser'
local coord = require 'snogray.coord'
local light = require 'snogray.light'
local transform = require 'snogray.transform'
local color = require 'snogray.color'


-- Command-line parser
--
function scene_cmdline.option_parser (scene_params)
   return clp {
      { "-b/--background=BG", { scene_params, "background" },
	doc = [[Use BG as a background and light-source;
	        BG may be a color or the name of an
	        environment-map image file]] },
      { "-B/--background-orientation=[AXIS][HAND][ROTATION]",
	{ scene_params, "background_orientation" },
	doc = [[Rotate the background and/or change its axis:\+
	        \|AXIS \>is the vertical axis, with values
	        "x", "y", or "z" (default "y")
		\|HAND is the "handedness", with values
		"r" or "l" (default "r")
		\|ROTATION is an amount to rotate the background
		around the vertical axis, in degrees]] },
      { "-I/--scene-options=OPTS", set_scene_options,
	doc = [[Set scene options; OPTS has the format
	        OPT1=VAL1[,...]; current options include:\+
		\|"format"    -- scene file type
		\|"background"-- scene background]] },
   }
end

function scene_cmdline.apply (scene_params, scene)
   local bg = scene_params.background
   if bg then
      local pi = math.pi
      local smatch = string.match

      -- The syntax is "TYPE:SPEC", where TYPE is "grey", "rgb", or
      -- "envmap".  "TYPE:" may be omitted in which case the default
      -- is "envmap" (in an automated situation, an explicit "envmap:"
      -- prefix may be used to avoid ambiguity in the case of files
      -- containing colons).
      --
      local type, spec = smatch (bg, "([^:]*):(.*)")
      if not type then
	 -- default is environment-map
	 type, spec = "envmap", bg
      end

      -- Each of the various types of backgrounds creates an
      -- appropriate light object.
      --
      local bg_light

      if type == 'grey' or type == 'gray' then
	 -- constant grey background

	 spec = tonumber (spec)
	 if spec then
	    -- Light using a constant intensity
	    bg_light = light.far (coord.vec (0,1,0), pi*2, spec)
	 end
      elseif type == 'rgb' then
	 -- constant rgb color background

	 local r, g, b = smatch(spec, "([0-9.]+),%s*([0-9.]+),%s*([0-9.]+)")
	 r, g, b = tonumber (r), tonumber (g), tonumber (b)
	 if r and g and b then
	    bg_light = light.far (coord.vec (0,1,0), pi*2, color.rgb (r, g, b))
	 end
      elseif type == 'envmap' then
	 -- environment-map background

	 local envmap = smatch (bg, "envmap:(.*)") or bg

	 -- A frame which defines a transformation from
	 -- environment-map-coordinates to world coordinates.
	 --
	 local envmap_mapping_frame = coord.frame ()

	 -- Environment-map orientation parameters.
	 --
	 -- We use left-handed coordinates by default, and most
	 -- scenes have a vertical Y axis; most environment maps are
	 -- right-handed, and the environment-mapping code assumes a
	 -- vertical Z-axis.  The default rotation is chosen to
	 -- preserve compatibility with old scenes.
	 --
	 local axis, handedness = 'y', 'r'
	 local rotation = 90

	 -- If the user specified some non-default options for the
	 -- environment-map orientation, parse them.
	 --
	 local bg_orient = scene_params.background_orientation
	 if bg_orient then
	    bg_orient = string.lower (bg_orient)

	    local new_axis, new_handedness, rotation_delta
	       = smatch (bg_orient, "([xyz]?)([rl]?)(-?[0-9.]*)")

	    if rotation_delta ~= '' then
	       rotation_delta = tonumber (rotation_delta)
	    end

	    if not new_axis or (rotation_delta ~= '' and not rotation_delta)
	    then
	       error ('Invalid background orientation "'..bg_orient..'"', 0)
	    end

	    if new_axis ~= '' then axis = new_axis end
	    if new_handedness ~= '' then handedness = new_handedness end

	    if rotation_delta ~= '' then
	       -- Note that we _subtract_ ROTATION_DELTA from the
	       -- the default rotation (which is non-zero), as it
	       -- seems more natural for a positive user angle
	       -- argument to cause a counter-clockwise rotation
	       -- (when viewed from the axis top, it would be a
	       -- positive angle).
	       --
	       rotation = rotation - rotation_delta
	    end
	 end

	 -- Background transfomration matrix
	 --
	 local xform = transform.new ()
	 
	 -- If the environment-map "handedness" isn't our default
	 -- left-handedness, flip the transform, which will reverse
	 -- handedness.
	 --
	 if handedness == 'r' then
	    xform:scale (-1, 1, 1)
	 end

	 -- Do any desired about the vertical axis.
	 --
	 if rotation ~= 0 then
	    xform:rotate_z (rotation * pi / 180)
	 end

	 -- If the desired vertical axis isn't the default z-axis,
	 -- rotate the desired axis into z's place.
	 --
	 if axis == 'x' then
	    xform:rotate_y (-pi / 2)
	 elseif axis == 'y' then
	    xform:rotate_x (-pi / 2)
	 end

	 -- Apply the final transformation to the frame we'll use.
	 --
	 envmap_mapping_frame:transform (xform)

	 -- Create the environment map.
	 --
	 bg_light = light.envmap (envmap, envmap_mapping_frame)
      end

      if not bg_light then
	 error ('Invalid background "'..bg..'"', 0)
      end

      scene:add (bg_light)
   end
end


-- return the module
--
return scene_cmdline
