-- render-cmdline.lua -- Command-line handling for renderer params
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

local clp = require 'snogray.cmdlineparser'
local render = require 'snogray.render'

-- module
--
local render_cmdline = {}

function render_cmdline.option_parser (params)
   return clp {
      { "-n/--samples=NUM", { params, "samples", 'unsigned' },
        doc = [[Use NUM samples for each output pixel]] },
      { "-D/--direct-samples=NUM", { params, "direct_samples", 'unsigned' },
        doc = [[Use NUM samples for direct lighting]] },
      { "-S/--surface-integ=INTEG",
	function (val)
	   clp.store_with_sub_params (val, "surface_integ", params, "type")
	end,
        doc = [[Use surface-integrator INTEG (default "direct"):\+
	        \|"direct"  -- direct-lighting
	        \|"path"    -- path-tracing
	        \|"photon"  -- photon-mapping]] },
      { "-A/--background-alpha=ALPHA", { params, "background_alpha", 'float' },
        doc = [[Use ALPHA as the opacity of the background]] },
      { "-R/--render-options=OPTIONS",
	function (options) clp.parse_params (options, params) end,
        doc = [[Set output-image options; OPTS has the format
	        OPT1=VAL1[,...]; current options include:\+
		\|"min-trace"  -- minimum trace ray length]] }
   }
end

function render_cmdline.make_global_render_state (scene, render_params)
   return render.global_state (scene, render_params);
end

-- return module
--
return render_cmdline
