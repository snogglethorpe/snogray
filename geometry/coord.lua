-- coord.lua -- Coordinate/geometry support
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
local coord = {}

-- imports
--
local raw = require "snogray.snograw"


-- coord.pos -- Return a position in space
--
-- args: (X, Y, Z)
--
-- Positions support the following methods and operators:
--
--   POS + VEC     -- Return position at offset VEC from POS
--   POS - VEC     -- Return position at offset -VEC from POS
--   POS1 - POS2   -- Return the vector between POS1 and POS2
--   POS * SCALE   -- Return POS with all components scaled by SCALE
--
--   POS:transformed (XFORM)  -- Return POS transformed by matrix XFORM
--   POS:transform (XFORM)    -- Transform POS in place by matrix XFORM
--
coord.pos = raw.Pos

-- coord.vec -- Return a vector (a direction + magnitude in space)
--
-- args: (X_DELTA, Y_DELTA, Z_DELTA)
--
-- Vectors support the following methods and operators:
--
--   VEC1 + VEC2   -- Return vector sum of VEC1 and VEC2
--   VEC1 - VEC2   -- Return vector difference of VEC1 and VEC2
--   -VEC          -- Return vector in opposite direction from VEC
--   VEC * SCALE   -- Return VEC with magnitude scaled by SCALE
--   VEC / SCALE   -- Return VEC with magnitude scaled by 1 / SCALE
--
--   VEC:null ()              -- Return true if all components of VEC are zero
--   VEC:length ()            -- Return the magnitude of VEC
--   VEC:length_squared ()    -- Return the squared magnitude of VEC
--   VEC:unit ()              -- Return a vector with the same direction as
--                               VEC, and magnitude 1
--
--   VEC:transformed (XFORM)  -- Return VEC transformed by matrix XFORM
--   VEC:transform (XFORM)    -- Transform VEC in place by matrix XFORM
--
coord.vec = raw.Vec

-- coord.bbox -- Return an axis-aligned bounding-box
--
-- args: (MIN_POS, MAX_POS)
--
-- MIN_POS and MAX_POS are the positions of two opposite corners of
-- the bounding box.  Note that every component of MIN_POS must be
-- less than or equal than every component of MAX_POS; this is not
-- checked!
--
-- Bounding boxes support the following methods and operators:
--
--   BB1 + BB2     -- Return a bounding-box including both BB1 and BB2
--   BB  + POS     -- Return a bounding-box including both BB and position POS
--
--   BB1:add (BB2)        -- Expand BB1 to include BB2
--   BB:add (POS)         -- Expand BB1 to include position POS
--
--   BB:extent ()         -- Return a vector between opposite corners of BB
--   BB:max_size ()       -- Return BB's maximum axis-aligned size
--   BB:min_size ()       -- Return BB's minimum axis-aligned dimension
--   BB:avg_size ()       -- Return the mean of BB's axis-aligned dimensions
--   BB:median_size ()    -- Return the median of BB's axis-aligned dimensions
--
--   BB:transformed (XFORM)  -- Return BB transformed by matrix XFORM
--   BB:transform (XFORM)    -- Transform BB in place by matrix XFORM
--
coord.bbox = raw.BBox
coord.ray = raw.Ray

-- coord.uv -- Return a UV-coordinate
--
-- args: (U, V)
--
coord.uv = raw.UV

-- coord.frame -- Return a frame of reference
--
-- args: (ORIGIN, X_AXIS, Y_AXIS, Z_AXIS)
--   or: (X_AXIS, Y_AXIS, Z_AXIS)
--   or: (ORIGIN)
--   or: (ORIGIN, Z_AXIS)
--   or: (Z_AXIS)
--   or: ()
--   or: (FRAME_TO_WORLD_TRANSFORM)
--
-- ORIGIN is the origin position in world coordinates where the origin
-- of the new frame is located.  X_AXIS, Y_AXIS, Z_AXIS are its basis
-- vectors.  ORIGIN defaults to (0,0,0).  If no basis-vectors are
-- given, they default to the standard basis vectors (1,0,0), (0,1,0),
-- and (0,0,1).  If only Z_AXIS is given, X_AXIS and Y_AXIS default to
-- arbitrary vectors that are orthogonal to it.
--
-- If only FRAME_TO_WORLD_TRANSFORM is given, it should be a
-- transformation matrix, in which case an equivalent frame reference
-- is returned.
--
-- Frames support the following methods and operators:
--
--   FRAME:to (VEC)       -- Return VEC transformed into a vector in FRAME
--   FRAME:to (POS)       -- Return POS transformed into a vector in FRAME
--   FRAME:from (VEC)     -- Return VEC in FRAME, into an external vector
--   FRAME:from (POS)     -- Return POS in FRAME, into an external position
--
--   FRAME:transformed (XFORM)  -- Return FRAME transformed by matrix XFORM
--   FRAME:transform (XFORM)    -- Transform FRAME in place by matrix XFORM
--
-- Note that when vectors are transformed into/out-of a frame, the
-- frame's origin is ignored, whereas transforming a position takes
-- the frame's origin into account.
--
coord.frame = raw.Frame

-- coord.origin -- Position of scene origin
--
-- A predefined position at (0,0,0.
--
coord.origin = coord.pos (0, 0, 0)

-- coord.midpoint -- Return the midpoint between two positions
--
-- args: (POS1, POS2)
--
-- Return a position halfway between positions POS1 and POS2.
--
coord.midpoint = raw.midpoint

-- coord.dot -- Vector dot product
--
-- args: (VEC1, VEC2)
--
-- Return the dot product of vectors VEC1 and VEC2, which is equal to
-- the length(VEC1) * length (VEC2) * cos (THETA), where THETA is the
-- angle between VEC1 and VEC2.
--
coord.dot = raw.dot

-- coord.cross -- Vector cross product
--
-- args: (VEC1, VEC2)
--
-- Return the cross product of vectors VEC1 and VEC2.
--
coord.cross = raw.cross


-- return the module
--
return coord
