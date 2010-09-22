-- lpeg-utils.lua -- Useful functions for parsing with LPeg
--
--  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

module ("lpeg-utils", package.seeall)

local lpeg = require "lpeg"
local snogray = require "snogray" -- for read_file

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

-- number components
local PM = S"+-"^-1
local DIGS = R"09"^1
local OPT_DIGS = R"09"^0
local FMANT =  PM * ((P"." * DIGS) + (DIGS * (P"." * OPT_DIGS)^-1))
local FEXP = P"e" * PM * DIGS

-- whole numbers
INT = (PM * DIGS) / tonumber
FLOAT = (FMANT * FEXP^-1) / tonumber

-- whitespace
OPT_WS = S" \t\r\n\f"^0	-- optional whitespace
REQ_WS = S" \t\r\n\f"^1 -- required whitespace
OPT_HORIZ_WS = S" \t"^0	-- optional non-newline whitespace
REQ_HORIZ_WS = S" \t"^1 -- required non-newline whitespace
NL = S"\r"^-1 * S"\n"	-- newline char

-- line
LINE = (1 - NL)^0
LINE_NL = LINE * NL
LINE_CONTENTS = C (LINE)

-- whitespace followed by numbers (useful shortcuts)
WS_FLOAT = OPT_WS * FLOAT
WS_INT = OPT_WS * INT


-- Global state during parsing.
--
local parse_state


-- Set the error position used if a parse error occurs.
--
function set_err_pos (pos)
   parse_state.err_pos = pos
end


local function update_err_pos (text, pos)
   parse_state.err_pos = pos
   return pos
end

-- This special lpeg pattern updates the current error position
--
ERR_POS = P(update_err_pos)


local function cur_line ()
   local line_num = 1
   local count_pos = 1
   local bol_pos = 1

   local pos = parse_state.err_pos
   local text = parse_state.text

   while count_pos < pos do
      line_num = line_num + 1
      bol_pos = count_pos
      count_pos = LINE_NL:match (text, count_pos)
   end
   if count_pos > pos then
      line_num = line_num - 1
   else
      bol_pos = count_pos
   end

   return line_num, bol_pos
end


-- Return a nice error string quoting MSG and the error location,
-- assuming parse_state is setup with appropriate location info.
--
local function make_err_string (msg)
   local filename = parse_state.filename
   local line_num = cur_line ()
   return filename..":"..tostring(line_num)..": " .. (msg or "parse error")
end

-- Signal an error with a message including MSG and the approximate
-- error location.  Should be called from within a parser invocation
-- using parse_file.
--
function parse_err (msg)
   -- We know that this function should be called from within a call
   -- to parse_file, so wrap the error message in a table to make it
   -- clear where it comes from.  parse_file will in turn catch our
   -- error and correctly deal with the contents.
   --
   error ({parse_err = make_err_string (msg)}, 0)
end

-- Print a warning message including MSG and the approximate file
-- location.  Should be called from within a parser invocation using
-- parse_file.
--
function parse_warn (msg)
   print (make_err_string (msg))
end

-- Read and parse FILENAME by repeatedly matching PATTERN; nothing is
-- returned (if results are desired, PATTERN should record them as a
-- side-effect).  Repetition of PATTERN must cover the entire file,
-- otherwise an error is signaled.
--
function parse_file (filename, pattern)
   local text, err = snogray.read_file (filename)
   if not text then
      error (err, 0)
   end

   -- Save old parse state, to allow recursive invocation.
   --
   local old_parse_state = parse_state

   parse_state = {filename = filename, text = text, err_pos = 1}

   local function parse ()
      local len = #text
      local pos, old_pos = 1, nil
      while pos <= len do
	 parse_state.err_pos = pos
	 local new_pos = pattern:match (text, pos)
	 if not new_pos or new_pos == pos then
	    parse_err ()
	 end
	 pos = new_pos
      end
   end
   local ok, err = pcall (parse)

   -- if there was an error, rethrow it
   if not ok then
      if type (err) == 'table' and err.parse_err then
	 -- the error came from parse_err, so it already has location
	 -- info etc attached

	 if not old_parse_state then
	    -- The error came from parse_err, and we're not in a
	    -- recursive parse, so turn the table into a string that's
	    -- nicer for our callers.

	    err = err.parse_err
	 end
      else
	 -- the error _didn't_ come from parse_err; attach location
	 -- info to it
	 --
	 err = make_err_string (err)

	 -- if we're in a recursive parse, wrap ERR to make it clear
	 -- it already has location info attached
	 --
	 if old_parse_state then
	    err = {parse_err = err}
	 end
      end

      -- Restore caller's parse state before we call error
      --
      parse_state = old_parse_state

      error (err, 0)		-- propagate the error
   end

   -- Restore caller's parse state.
   --
   parse_state = old_parse_state
end
