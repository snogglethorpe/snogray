-- image-sampled-output-cmdline.lua -- Command-line parsing for image output
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
local img_output_cmdline = {}

local cmdlineparser = require 'snogray.cmdlineparser'
local image = require 'snogray.image'

function img_output_cmdline.option_parser (params, default_aspect_ratio)
   local function set_size (arg)
      local size = string.match (arg, "^%d+$")
      if size and default_aspect_ratio then
	 size = tonumber (size)
	 if default_aspect_ratio > 1 then
	    params.width = size
	    params.height = size / default_aspect_ratio
	 else
	    params.height = size
	    params.width = size * default_aspect_ratio
	 end
      else
	 local w, h = string.match (arg, "^(%d+)%s*[,x]%s*(%d+)$")
	 w = tonumber (w)
	 h = tonumber (h)
	 if not w or not h then
	    cmdlineparser.error ("invalid size option \""..arg.."\"")
	 end
	 params.width = w
	 params.height = h
      end
   end

   local function set_exposure (arg)
      local err = false

      -- First look for an exposure; it can either an explicit
      -- multiplicative factor, prefixed by "*" or "/", or an
      -- adjustment in "stops", prefixed by "+" or "-" (+N is
      -- equivalent to *(2^N)).  A number with no prefix is treated as
      -- if it were preceded by "*".
      --
      local exposure_op, exposure_val, tail
	 = string.match (arg, "^([-+*/]?)([0-9.]+)()")

      if exposure_val then
	 exposure_val = tonumber (exposure_val)
	 if exposure_val then
	    if exposure_op == '+' then
	       exposure_val = 2 ^ exposure_val
	    elseif exposure_op == '-' then
	       exposure_val = 2 ^ -exposure_val
	    elseif exposure_op == '/' then
	       exposure_val = 1 / exposure_val
	    end
	    params.exposure = exposure_val
	 else
	    err = true
	 end
      else
	 tail = 1
      end

      -- Now look for a contrast adjustment, which should be prefixed by "^".
      --
      local contrast_val, ntail = string.match (arg, "%^([0-9.]+)()", tail)
      if contrast_val then
	 contrast_val = tonumber (contrast_val)
	 if contrast_val then
	    params.contrast = contrast_val
	 else
	    err = true
	 end
	 tail = ntail
      end

      if err or tail ~= #arg + 1 then
	 cmdlineparser.error ("invalid exposure option \""
			      ..arg.."\" (expected (+|-|*|/)NUM[^NUM])")
      end
   end

   return cmdlineparser {
      { "-s/--size=WIDTHxHEIGHT", set_size,
        doc = [[Set output image size to WIDTH by HEIGHT pixels]] },
      { "-s/--size=SIZE", nil,
        doc = [[Set largest image dimension to SIZE, preserving
	        aspect ratio]] },
      { "-F/--filter=FILTER[/PARAM=VAL...]", set_filter,
        doc = [[Filter to apply to the output image, and optional parameters;
	      FILTER may be one of "mitchell", "gauss", or "box"
	      (default "mitchell") ]] },
      { "-e/--exposure=EXPOSURE", set_exposure,
        doc = [[Increase/decrease output brightness/contrast
		EXPOSURE can have one of the forms:\+
		\|+STOPS  -- \>make output 2^STOPS times brighter
		\|-STOPS  -- make output 2^STOPS times dimmer
		\|*SCALE  -- make output SCALE times brighter
		\|/SCALE  -- make output SCALE times dimmer
		\|^POWER  -- raise output to the POWER power]] },
      { "--dither", { params, "dither", true} , no_help = true },
      { "--no-dither", { params, "dither", false},
        doc = [[Do not add dithering noise to LDR output formats
                (dithering is used by default for low-dynamic-range
	        output formats, where it helps prevent banding of
                very shallow gradients) ]] },
      { "-O/--output-options", set_output_opts,
        doc = [[Set output-image options; OPTS has the format
		OPT1=VAL1; current options include:\+
		\|"format"  -- \>output file type
		\|"gamma"   -- target gamma correction
		\|"quality" -- image compression quality (0-100)
		\|"filter"  -- output filter
		\|"exposure"-- output exposure ]] }
   }
end

function img_output_cmdline.make_output (file, params)
   return image.sampled_output (file, params.width, params.height, params)
end

return img_output_cmdline
