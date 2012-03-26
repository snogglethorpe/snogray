-- snogray.lua -- Top-level driver for snogray
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

local cmdline = ...

local default_width, default_height = 720, 480


----------------------------------------------------------------
-- Imports

local string = require 'snogray.string'
local commify = string.commify
local commify_with_units = string.commify_with_units
local round_and_commify = string.round_and_commify
local lpad = string.left_pad

local file = require 'snogray.file'

local clp = require 'snogray.cmdlineparser'

local load = require 'snogray.load'
local render = require 'snogray.render'
local image = require 'snogray.image'
local sys = require 'snogray.sys'
local accel = require 'snogray.accel'
local scene = require 'snogray.scene'
local camera = require 'snogray.camera'
local environ = require 'snogray.environ'

local img_out_cmdline = require 'snogray.image-sampled-output-cmdline'
local render_cmdline = require 'snogray.render-cmdline'
local scene_cmdline = require 'snogray.scene-cmdline'
local camera_cmdline = require 'snogray.camera-cmdline'
local limit_cmdline = require 'snogray.limit-cmdline'


----------------------------------------------------------------
-- Parse command-line options
--

-- option values
--
local quiet = false
local progress = true
local recover = false
local num_threads = sys.num_cores ()
local render_params = {}
local scene_params = {}
local camera_params = {}
local output_params = {}

-- All parameters together.
--
local params = { render = render_params, scene = scene_params,
		 camera = camera_params, output = output_params }


-- Command-line parser.
--
local parser = clp {
   desc = "Ray-trace an image",
   usage = "SCENE_FILE [OUTPUT_IMAGE_FILE]",
   prog_name = cmdline[0],
   package = "snogray",
   version = environ.version,

   -- sub-parsers
   --
   "Rendering options:",
   render_cmdline.option_parser (render_params),
   "Scene options:",
   scene_cmdline.option_parser (scene_params),
   "Camera options:",
   camera_cmdline.option_parser (camera_params),
   "Output image options:",
   img_out_cmdline.option_parser (output_params,
				  default_width / default_height),
   limit_cmdline.option_parser (params),

   -- Misc options; we put these last as they're rarely used.
   --
   "Misc options:",
   { "-C/--continue", function () recover = true end,
     doc = [[Continue a previously aborted render]] },
   { "-j/--threads=NUM",
     function (num) num_threads = clp.unsigned_argument (num) end,
     doc = [[Use NUM threads for rendering (default all cores)]] },
   --{ "limit",		required_argument, 0, 'L' },
   { "-q/--quiet", function () quiet = true end,
     doc = [[Do not output informational or progress messages]] },
   { "-p/--progress", function () progress = true end, hidden = true,
     doc = [[Output progress indicator despite --quiet]] },
   { "-P/--no-progress", function () progress = false end,
     doc = [[Do not output progress indicator]] },
}

local args = parser (cmdline)

if #args < 1 or #args > 2 then
   parser:usage_error ()
end

local scene_file = args[1]
local output_file = args[2] -- may be null


----------------------------------------------------------------
-- Load the scene
--

local beg_time = os.time ()
local scene_beg_ru = sys.rusage () -- begin marker for scene loading
local scene = scene.new ()	-- note, shadows variable, but oh well
local camera = camera.new ()	-- ditto

-- Stick the output filenamein OUTPUT_PARAMS, so the scene loader can
-- see it (and optionally, change it).
--
output_params.filename = output_file

load.scene (scene_file, scene, camera, params)

-- Do post-load scene setup (nothing can be added to scene after this).
--
-- We need to do this _before_ processing the camera command-line, as
-- it actually uses the rendering machinery for autofocusing.
--
scene:setup (accel.factory())

scene_cmdline.apply (scene_params, scene)
camera_cmdline.apply (camera_params, camera, scene)

-- Get the output file back, in case loading the scene changed it.
--
output_file = output_params.filename

-- The output file is optional on the command-line, but must be supplied
-- by the scene if not there.
--
if not output_file then
   error ("no output file specified", 0)
end

local scene_end_ru = sys.rusage () -- end marker for scene setup


----------------------------------------------------------------
-- Setup the output file
--

-- If we're in recovery mode, move an existing output file out of the
-- way; otherwise an existing output file is an error (to prevent
-- accidental overwriting).
--
local recover_backup = nil
if file.exists (output_file) then
   if recover then
      recover_backup = file.rename_to_backup_file (output_file, 99)
      if not quiet then
	 print ("* recover: "..output_file..": Backup in "..recover_backup)
      end
   else
      error (output_file..": Output file already exists\n"
	     .."To continue a previously aborted render, use the `--continue' option", 0)
   end
end


-- The nominal image size.
--
local width = output_params.width or default_width
local height = output_params.height or default_height

camera:set_aspect_ratio (width / height)

-- The "limit", which is the portion the nominal image which we will
-- actually render.
--
local limit_x, limit_y, limit_width, limit_height
   = limit_cmdline.bounds (params, width, height)

-- Make the output image reflect the limit rather than the "nominal"
-- image size.
--
output_params.sample_base_x = limit_x
output_params.sample_base_y = limit_y
output_params.width = limit_width
output_params.height = limit_height

-- Create the output.
--
local image_out = img_out_cmdline.make_output (output_file, output_params)

if output_params.alpha_channel and not output.has_alpha_channel () then
   error (output_file..": alpha-channel not supported", 0)
end

-- If recovering, do the actual recovery.
--
if recover_backup then
   local num_rows_recovered
      = image.recover (recover_backup, output_file, output_params, image_out)

   if not quiet then
      print ("* recover: "..output_file..": Recovered "
	     ..tostring(num_rows_recovered).." rows")
   end

   if num_rows_recovered == limit_height then
      print (output_file..": Entire image was recovered, not rendering")
      return
   end

   -- Remove the recovered rows from what we will render.
   --
   limit_y = limit_y + num_rows_recovered
   limit_height = limit_height - num_rows_recovered
end


----------------------------------------------------------------
-- Pre-render info output
--

if not quiet then
   print ("* scene: "
	  ..commify_with_units (scene:num_surfaces(),
				" top-level surface",
				" top-level surfaces")
          ..", "
	  ..commify_with_units (scene:num_lights (),
				" light", " lights"))
   print ("* camera: at "..tostring (camera.pos)
	  ..", pointing at "
          ..tostring (camera.pos + camera.forward * camera.target_dist)
	  .." (up = "..tostring(camera.up)
	  ..", right = "..tostring(camera.right)..")")
   local cam_desc = "focal-length "..round_and_commify(camera:focal_length())
   if camera.aperture ~= 0 then
      cam_desc = cam_desc..", f-stop f:"..round_and_commify (camera:f_stop())
                 ..", focus distance "..round_and_commify (camera.focus)
   end
   print ("* camera: "..cam_desc)

   if num_threads ~= 1 then
      print ("* using "..tostring(num_threads).." threads")
   end
end


----------------------------------------------------------------
-- Pre-render setup (this can be time-consuming)
--

local setup_beg_ru = sys.rusage ()
local grstate = render_cmdline.make_global_render_state (scene, render_params)
local setup_end_ru = sys.rusage ()


----------------------------------------------------------------
-- Rendering
--

-- The pattern of pixels we will render; we add a small margin around
-- the output image to keep the edges clean.
--
local x_margin = image_out:filter_x_radius ()
local y_margin = image_out:filter_y_radius ()
local render_pattern
   = render.pattern (limit_x - x_margin, limit_y - y_margin,
		     limit_width + x_margin * 2, limit_height + y_margin * 2)

local render_stats = render.stats ()
local render_mgr = render.manager (grstate, camera, width, height)

local tty_prog = sys.tty_progress ("rendering...")

local render_beg_ru = sys.rusage () -- begin marker for rendering

render_mgr:render (num_threads, render_pattern, image_out,
		   tty_prog, render_stats)

local render_end_ru = sys.rusage () -- end marker for rendering
local end_time = os.time ()


----------------------------------------------------------------
-- Post-render reporting
--

if not quiet then
   -- Return 100 * (NUM / DEN) as an int; if DEN == 0, return 0.
   --
   local function percent (num, den)
      if den == 0 then return 0 else return math.floor (100 * num / den) end
   end

   -- Return NUM / DEN as a float; if DEN == 0, return 0;
   --
   local function fraction (num, den)
      if den == 0 then return 0 else return num / den end
   end

   local function elapsed_time_string (sec)
      sec = math.floor (sec * 10 + 0.5) / 10

      local min = math.floor (sec / 60)
      sec = sec - min * 60
      local hours = math.floor (min / 60)
      min = min - hours * 60
      local days = math.floor (hours / 24)
      hours = hours - days * 24

      local rval = nil
      if days ~= 0 then
	 rval = string.sep_concat (rval, ", ", days).." days"
      end
      if hours ~= 0 then
	 rval = string.sep_concat (rval, ", ", hours).." hours"
      end
      if min ~= 0 then
	 rval = string.sep_concat (rval, ", ", min).." min"
      end
      if sec ~= 0 then
	 rval = string.sep_concat (rval, ", ", sec).." sec"
      end

      return rval
   end

   -- Print post-rendering scene statistics in RENDER_STATS.
   --
   local function print_render_stats (rstats)
      local function print_search_stats (sstats)
	 local node_tests = sstats.space_node_intersect_calls
	 local surf_tests = sstats.surface_intersects_tests
	 local neg_cache_hits = sstats.neg_cache_hits
	 local neg_cache_colls = sstats.neg_cache_collisions
	 local tot_tries = surf_tests + neg_cache_hits
	 local pos_tries = sstats.surface_intersects_hits

	 print("     tree node tests: "..lpad (commify (node_tests), 16))
	 print("     surface tests:   "..lpad (commify (tot_tries), 16)
	       .." (success = "..lpad(percent(pos_tries, tot_tries), 2).."%"
	       ..", cached = "..lpad(percent(neg_cache_hits, tot_tries), 2).."%"
	       .."; coll = "..lpad(percent(neg_cache_colls, tot_tries), 2).."%)")
      end

      local sic = rstats.scene_intersect_calls
      local sst = rstats.scene_shadow_tests

      print ""
      print "Rendering stats:"
      print "  intersect:"
      print("     rays:            "..lpad (commify (sic), 16))

      print_search_stats (rstats.intersect)

      if sst ~= 0 then
	 print "  shadow:"
	 print("     rays:            "..lpad (commify (sst), 16))

	 print_search_stats (rstats.shadow)
      end

      local ic = rstats.illum_calls
      if ic ~= 0 then
	 print "  illum:"
	 print("     illum calls:     "..lpad (commify (ic), 16))
	 if sst ~= 0 then
	    print("     average shadow rays:   "
		  ..lpad (round_and_commify (fraction (sst, ic), 3), 10))
	 end
      end
   end

   print_render_stats (render_stats)

   --
   -- Print times; a field width of 14 is enough for over a year of
   -- time...
   --

   print "Time:"

   -- Note because scene-loading often involves significant disk I/O,
   -- we add system time as well (this usually isn't a factor for
   -- other time periods we measure).
   --
   local scene_def_time
      = ((scene_end_ru:utime() - scene_beg_ru:utime())
         + (scene_end_ru:stime() - scene_beg_ru:stime()))
   if scene_def_time > 1 then
      print("  scene def cpu:       "..elapsed_time_string (scene_def_time))
   end

   local setup_time = setup_end_ru:utime() - setup_beg_ru:utime()
   if setup_time > 1 then
      print("  setup cpu:           "..elapsed_time_string (setup_time))
   end

   local render_time = render_end_ru:utime() - render_beg_ru:utime()
   print("  rendering cpu:       "..(elapsed_time_string (render_time) or "0"))

   local real_time = os.difftime (end_time, beg_time)
   print("  total elapsed:       "..(elapsed_time_string (real_time) or "0"))

   local sic = render_stats.scene_intersect_calls
   local sst = render_stats.scene_shadow_tests
   local num_eye_rays = limit_width * limit_height

   local rps, eps = 0, 0
   if render_time ~= 0 then
      rps = (sic + sst) / render_time
      erps = (num_eye_rays) / render_time
   end

   print("  rays per second:     "..round_and_commify (rps, 1))
   print("  eye-rays per second: "..round_and_commify (erps, 1))
end


--
-- ~fini~
--
