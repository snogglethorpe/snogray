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


-- imports
--
local raw = require ('snogray.snograw')


-- Return true if a file called FILENAME exists and is readable.
--
function file.exists (filename)
   local stream = io.open (filename, "r")
   if stream then
      stream:close ()
      return true
   else
      return false
   end
end


-- Return a string containing the entire contents of the file
-- FILENAME.  If an error occurs, return nil and an error description.
--
-- This is basically equivalent to io.open(filename,"r"):read"*a"
-- (plus closing the file) but much more efficient and less likely to
-- thrash the system to death when reading huge files.
--
function file.read (filename)
   local contents = raw.read_file (filename)
   if not contents then
      local stream, err = io.open (filename, "r")
      if not stream then
	 return nil, err
      end
      contents = stream:read ("*a")
      stream:close ()
   end
   return contents
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
