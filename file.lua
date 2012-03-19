-- file.lua -- Miscellaneous file-related functions
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
local file = {}

function file.exists (filename)
   local stream = io.open (filename, "r")
   if stream then
      stream:close ()
      return true
   else
      return false
   end
end

-- Choose a "backup filename" (using the GNU convention of suffixes like
-- ".~1~", ".~2~" etc), and rename FILE_NAME to it.  The backup filename
-- is returned.  If this cannot be done an exception is thrown.
--
function file.rename_to_backup_file (filename, backup_limit)
   local backup_name

   local backup_num = 1
   while backup_num < backup_limit do
      backup_name = filename .. ".~" .. tostring (backup_num) .. "~"
      if not file.exists (backup_name) then
	 break
      end
      backup_num = backup_num + 1
   end
   if backup_num == backup_limit then
      error (filename .. ": Too many backup files already exist", 0)
   end

   os.rename (filename, backup_name)

   return backup_name
end

-- return the module
--
return file
