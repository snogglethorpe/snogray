-- accel.lua -- Search accelerators
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
local accel = {}

-- imports
--
local raw = require "snogray.snograw"


-- Default search accelerator.
--
accel.default = "octree"


-- Constructors for factories of each accelerator type.
--
local accel_factory_ctors = {
   octree = raw.OctreeBuilderFactory
}

-- Actual factory objects for each accelerator type.
--
local accel_factories = {}


-- Return an accelerator factory for accelerators of type TYPE; TYPE
-- defaults to accel.default.  If there is no such type of
-- accelerator, an error is signalled.
--
function accel.factory (type)
   type = type or accel.default

   local factory = accel_factories[type]

   if not factory then
      local ctor = accel_factory_ctors[type]
      if not ctor then
	 error ('Unknown search accelerator type "'..type..'"')
      end

      factory = ctor ()
      accel_factories[type] = factory
   end

   return factory
end


-- return module
--
return accel
