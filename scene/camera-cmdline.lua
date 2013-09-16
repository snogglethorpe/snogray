-- camera-cmdline.lua -- Command-line options for camera control
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
local camera_cmdline = {}

-- imports
--
local lpeg = require 'lpeg'
local lpeg_utils = require 'snogray.lpeg-utils'
local clp = require 'snogray.cmdlineparser'
local render = require 'snogray.render'
local coord = require 'snogray.coord'
local transform = require 'snogray.transform'

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C


----------------------------------------------------------------
-- Camera control-string interpretation
--

-- basic lexical stuff
local WS = lpeg_utils.OPT_WS
local NUM = WS * lpeg_utils.FLOAT

-- Shoot a ray into SCENE_CONTENTS in a direction corresponding to the
-- location X, Y in CAMERA's image space, and return the distance to
-- the first surface hit, or nil if no surface is hit.
--
local function probe_scene (x, y, scene_contents, camera)
   local global_render_state = render.global_state (scene_contents, {})
   local render_context = render.context (global_render_state)

   local film_pos = coord.uv (x, y)
   local probe = camera:eye_ray (film_pos)
   probe.t1 = scene_contents:bbox ():diameter ()

   if global_render_state.scene:intersect (probe, render_context) then
      return probe.dir * probe:length ()
   end

   return nil
end

-- Interpret the camera-control string CMDS, applying it to CAMERA
-- (and optionally using SCENE_CONTENTS in the case auto-focus is requested).
--
local function interpret_camera_cmds (cmds, camera, scene_contents)
   -- Return -VAL if CAMERA reverses handedness, otherwise return VAL.
   --
   local function negate_if_reverse_handed (val)
      if camera.handedness_reversed then
	 return -val
      else
	 return val
      end
   end

  -- Translate the "direction indicator" letter DIR_INDIC into a real
   -- direction vector.
   --
   local function make_dir (dir_indic)
      if dir_indic == 'r' then
	 return camera.right
      elseif dir_indic == 'l' then
	 return -camera.right
      elseif dir_indic == 'u' then
	 return camera.up
      elseif dir_indic == 'd' then
	 return -camera.up
      elseif dir_indic == 'f' then
	 return camera.forward
      elseif dir_indic == 'b' then
	 return -camera.forward
      elseif dir_indic == 'x' then
	 return coord.vec (1,0,0)
      elseif dir_indic == 'y' then
	 return coord.vec (0,1,0)
      elseif dir_indic == 'z' then
	 return coord.vec (0,0,1)
      else
	 error ("invalid direction indicator '"..dir_indic.."'", 0)
      end
   end

   -- Translate the rotation axis "indicator" letter ROT_AXIS_INDIC
   -- into a real axis vector for that rotation.
   --
   -- Note that the letters themselves indicate either a real axis (x,
   -- y, z, a), or a relative "direction of movement" (u, d, l, r),
   -- which is orthogonal to the actual axis of rotation.
   --
   local function make_rot_axis (rot_axis_indic)
      if rot_axis_indic == 'u' then	-- rotate up => around "right" axis
	 return negate_if_reverse_handed (-camera.right)
      elseif rot_axis_indic == 'd' then	-- rotate down => around "right" axis
	 return negate_if_reverse_handed (camera.right)
      elseif rot_axis_indic == 'l' then	-- rotate left => around "up" axis
	 return camera.up
      elseif rot_axis_indic == 'r' then	-- rotate right => around "up" axis
	 return -camera.up
      elseif rot_axis_indic == 'a' then	-- rotate "around camera axis"
	 return camera.forward
      elseif rot_axis_indic == 'x' then
	 return coord.vec (1, 0, 0)
      elseif rot_axis_indic == 'y' then
	 return coord.vec (0, 1, 0)
      elseif rot_axis_indic == 'z' then
	 return coord.vec (0, 0, 1)
      else
	 error ("invalid rotation-axis indicator '"..rot_axis_indic.."'", 0)
      end
   end

   local function make_angle (deg)
      return 
   end
   local function make_rot_xform (axis, angle)
      return transform.rotate (axis, angle * math.pi / 180)
   end

   -- actual command functions
   local function move_to (pos) camera:move (pos) end
   local function point_at (pos) camera:point (pos) end
   local function zoom (amount) camera:zoom (amount) end
   local function move (dir, dist) camera:move (dir * dist) end
   local function rotate (rot_xform) camera:rotate (rot_xform) end
   local function orbit (rot_xform) camera:orbit (rot_xform:inverse()) end
   local function set_foc_len (foclen)
      --
      -- preserve the camera's current f-stop, if any (otherwise,
      -- the camera retains the old aperture setting, which
      -- corresponds to a different f-stop with the new focal
      -- length).
      --
      local f_stop = camera:f_stop ();
      camera:set_focal_length (foclen);
      camera:set_f_stop (f_stop);
   end
   local function set_f_stop (f_stop) camera:set_f_stop (f_stop) end
   local function set_focus (dist) camera:set_focus (dist) end
   local function set_scene_unit (dist) camera:set_scene_unit (dist) end
   local function set_horiz () camera:set_orientation (camera.ORIENT_HORIZ) end
   local function set_vert () camera:set_orientation (camera.ORIENT_VERT) end
   local function auto_focus (x, y)
      camera:set_focus (probe_scene (x, y, scene_contents, camera))
   end
   local function unrecog_cmd (letter)
      error ("unrecognized cammera command '"..letter.."'", 0)
   end

   -- various parameter types
   local POS = lpeg_utils.bracketed (NUM * P"," * NUM * P"," * NUM, true)
   local XY  = lpeg_utils.bracketed (NUM * P"," * NUM, true)
   local AXIS = C (S"udlraxyz")	/ make_rot_axis
   local DIR = C (S"udlrfbxyz") / make_dir
   local ROT_XFORM = (AXIS * NUM) / make_rot_xform

   -- individual command syntaxes
   local MOVE_TO	= (P"g" * POS) / move_to -- "g" = "goto"
   local POINT_AT	= (P"t" * POS) / point_at -- "t" = "target"
   local ZOOM		= (P"z" * NUM) / zoom
   local MOVE		= (P"m" * DIR * NUM) / move
   local ROTATE		= (P"r" * ROT_XFORM) / rotate
   local ORBIT		= (P"o" * ROT_XFORM) / orbit
   local SET_FOC_LEN	= (P"l" * NUM) / set_foc_len
   local SET_F_STOP     = (P"f" * S":/"^-1 * NUM) / set_f_stop
   local SET_FOCUS	= (P"d" * NUM) / set_focus -- "d" = "(focus) distance"
   local AUTO_FOCUS	= (P"a" * XY) / auto_focus
   local SET_SCENE_UNIT = (P"u" * NUM) / set_scene_unit
   local SET_HORIZ	= P"h" / set_horiz
   local SET_VERT	= P"v" / set_vert
   local UNRECOG_CMD	= P(1)^1 / unrecog_cmd

   -- top-level syntax
   local COMMAND = MOVE_TO + POINT_AT + ZOOM + MOVE + ROTATE + ORBIT
      + SET_FOC_LEN + SET_F_STOP + SET_FOCUS + AUTO_FOCUS + SET_SCENE_UNIT
      + SET_HORIZ + SET_VERT + UNRECOG_CMD
   local COMMANDS = (COMMAND * WS * (S";," * WS)^0)^1

   COMMANDS:match (cmds)
end


----------------------------------------------------------------
-- command-line parsing
--

function camera_cmdline.option_parser (camera_params)
   local function add_cmds (cmds)
      if camera_params.commands then
	 camera_params.commands = camera_params.commands..","..cmds
      else
	 camera_params.commands = cmds
      end
   end

   return clp {
      { "-c/--camera=COMMANDS", add_cmds,
	doc = [[Move/point the camera according to COMMANDS:\+
		\|g X,Y,Z     \>goto absolute location X, Y, Z
		\|t X,Y,Z     point at target X, Y, Z
		\|m[rludfb] D move distance D in the given dir
			      (right, left, up, down, fwd, back)
		\|m[xyz] D    move distance D on the given axis
		\|r[rlud] A   rotate A deg in the given dir
		\|ra A        rotate A deg around center axis
		\|r[xyz] A    rotate A degrees around [xyz]-axis
		\|o[xyz] A    orbit A degrees around [xyz]-axis
		\|z SCALE     zoom by a factor of SCALE
		\|l FOC_LEN   set lens focal-length to FOC_LEN
		\|f F_STOP    set lens aperture to F_STOP
		\|d DIST      set focus distance to DIST
		\|a X,Y       auto-focus at point X,Y on image
		\|u SIZE      set scene unit to SIZE, in mm
		\|h           set camera orientation to horizontal
		\|v           set camera orientation to vertical]] },

      -- some more verbose options for settings
      --
      { "--camera-focal-length=FOCLEN", function (arg) add_cmds ("l"..arg) end,
	doc = "set lens focal-length to FOC_LEN" },
      { "--camera-aperture=F_STOP", function (arg) add_cmds ("f"..arg) end,
	doc = "set lens aperture to F_STOP" },
      { "--camera-focus=DISTANCE", function (arg) add_cmds ("d"..arg) end,
	doc = "set focus distance to DISTANCE" },
      { "--camera-auto-focus=X,Y", function (arg) add_cmds ("a"..arg) end,
	doc = "auto-focus at point X,Y in image" },
      { "--camera-zoom=SCALE", function (arg) add_cmds ("z"..arg) end,
	doc = "zoom by a factor of SCALE" }
   }
end

function camera_cmdline.apply (camera_params, camera, scene_contents)
   local cmds = camera_params.commands
   if cmds then
      interpret_camera_cmds (cmds, camera, scene_contents)
   end
end


-- return the module
--
return camera_cmdline
