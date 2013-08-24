-- table.lua -- Miscellaneous table functions
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


local std_table = table -- standard Lua table module


-- module
--
local table = {}


-- Inherit from the standard Lua table module, so clients can just
-- call this module 'table'
--
setmetatable (table, {__index = std_table})


-- Return a copy of OBJ, with only the top-level table structure
-- copied.  A non-table is returned as-is.
--
function table.shallow_copy (obj)
   if type (obj) == 'table' then
      local copy = {}
      for k, v in pairs (obj) do
	 copy[k] = v
      end
      obj = copy
   end
   return obj
end


-- Return a copy of OBJ, with all table structure copied.  Any
-- non-tables are just used as-is.  Only values are copied, not keys.
--
local function deep_copy (obj)
   if type (obj) == 'table' then
      local copy = {}
      for k, v in pairs (obj) do
	 copy[k] = deep_copy (v)
      end
      obj = copy
   end
   return obj
end
table.deep_copy = deep_copy


-- Return a table containing every value in KEYS as a key, with value
-- true.
--
function table.set (keys)
   local s = {}
   for i,v in ipairs (keys) do
      s[v] = true
   end
   return s
end


-- Use binary search to locate the entries in ARRAY which are closest
-- to VAL.  SUB_KEY is a key for each array entry; if non-nil it is
-- used to retrieve the key from each entry for comparison (otherwise
-- the entries themselves are used).
--
-- Two indices are returned:  LO and HI; HI always equals LO+1 unless
-- there are less than two entries in ARRAY (if there are no entries,
-- then LO and HI will be 1 and 0; if there is one entry then LO and
-- HI will both be 1).  If VAL exactly matches an entry, then
-- ARRAY[LO] will be that entry.  If VAL lies between two entries,
-- then LO and HI will be the indices of entries on either side of it.
-- Otherwise, LO and HI will be the indices of the two entries closest
-- to it (the two lowest entries in the array, or the two highest).
--
function table.binary_search (array, val, sub_key)
   local lo, hi = 1, #array

   while lo < hi - 1 do
      local mid = math.floor ((lo + hi) / 2)

      local entry = array[mid]
      if sub_key then
	 entry = entry[sub_key]
      end

      if entry > val then
	 hi = mid
      else
	 lo = mid
      end
   end

   return lo, hi
end

-- Lookup VAL in ARRAY, using linear interpolation to compute entries
-- not explicitly present.
--
-- ARRAY should be a sequential array of entries like {VAL1, VAL2, ...},
-- and be sorted by the values of VAL1.
--
function table.linear_interp_lookup (array, val)

   -- If there's only one entry in the array, just return it.
   --
   if #array == 1 then
      local entry = array[1]
      if entry[1] == val then
	 return entry
      else
	 -- Copy ENTRY so we can use the correct value for ENTRY[1]
	 local new_entry = {val}
	 for i = 2, #entry do
	    new_entry[i] = entry[i]
	 end
	 return new_entry
      end
   end

   -- Use binary search to locate the nearest entries to VAL.
   --
   local lo, hi = table.binary_search (array, val, 1)
   lo = array[lo]
   hi = array[hi]

   if lo[1] == val then
      -- found exact match
      return lo
   else
      -- use interpolation

      local interp = {}

      local val_diff = val - lo[1]

      local inv_denom = 1 / (hi[1] - lo[1])

      interp[1] = val

      for i = 2, #lo do
	 local slope = (hi[i] - lo[i]) * inv_denom
	 interp[i] = lo[i] + val_diff * slope
      end

      return interp
   end
end


-- return the module
--
return table
