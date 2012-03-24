-- limit-cmdline.lua -- Command-line option for rendering "limit"
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
local limit_cmdline = {}

local lpeg = require 'lpeg'
local lpeg_utils = require 'snogray.lpeg-utils'
local clp = require 'snogray.cmdlineparser'

function limit_cmdline.option_parser (params)
   local function parse_limit (arg)
      local WS = lpeg_utils.OPT_WS
      local UINT = WS * lpeg_utils.UINT
      local FLOAT = WS * lpeg_utils.FLOAT

      local function percent_to_frac (perc)
	 if perc == 100 then return 0 else return perc / 100 end
      end
      local PERCENTAGE = (UINT * lpeg.P'%') / percent_to_frac

      local COORD = PERCENTAGE + UINT + FLOAT
      local COORDS = lpeg_utils.bracketed (COORD * lpeg.S",x" * COORD, true)

      local function make_extent_limit (x, y, w, h)
	 return {x = x, y = y, w = w, h = h}
      end
      local EXTENT_LIMIT = (COORDS * lpeg.S'+:' * COORDS) / make_extent_limit

      local function make_abs_limit (x, y, lx2, ly2)
	 return {x = x, y = y, x2 = lx2, y2 = ly2}
      end
      local ABS_LIMIT = (COORDS * lpeg.P'-' * COORDS) / make_abs_limit

      local LIMIT = EXTENT_LIMIT + ABS_LIMIT

      params.limit = LIMIT:match (arg)
      if not params.limit then
	 error ('invalid limit argument "'..arg..'"', 0)
      end
   end

   return clp {
      { "-L/--limit=X,Y+W,H", nil,
	doc = "Limit output to area X,Y - X+W,Y+H" },
      { "-L/--limit=X1,Y1-X2,Y2", parse_limit,
	doc = [[Limit output to area X1,Y1 - X2,Y2\+
	        \|Limit coordinates/sizes can be percentages
	        (with "%") or fractions (with ".") of the nominal
		output image size, or integer numbers of pixels;
		the actual output image will then be the size of
		the limit]] },
   }
end

function limit_cmdline.bounds (params, width, height)
   local limit = params and params.limit
   if limit then
      local function apply_part (part, dim, zero_val)
	 if part == 0 then
	    return zero_val
	 elseif part < 1 then
	    return dim * part
	 else
	    return part
	 end
      end

      local x = apply_part (limit.x, width, 0)
      local y = apply_part (limit.y, height, 0)
      local w, h = limit.w, limit.h
      if w and h then
	 w = apply_part (w, width, width - x)
	 h = apply_part (h, height, height - x)
      else
	 w = apply_part (limit.x2, width, width) - x
	 h = apply_part (limit.y2, height, height) - y
      end

      return x, y, w, h
   else
      return 0, 0, width, height
   end
end

-- return module
--
return limit_cmdline
