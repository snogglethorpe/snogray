-- sys.lua -- Miscellaneous system functions
--
--  Copyright (C) 2012  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the sys COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

-- module
--
local sys = {}

local raw = require 'snogray.snograw'


-- For TTY the progress reporter, we just assume std::cout
--
function sys.tty_progress (msg)
   return raw.TtyProgress (raw.cout, msg)
end

sys.rusage = raw.Rusage

sys.num_cores = raw.num_cores


-- return the module
--
return sys
