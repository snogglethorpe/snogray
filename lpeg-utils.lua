-- lpeg-utils.lua -- Useful functions for parsing with LPeg
--
--  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
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
NL = S"\r\n\f"		-- newline char

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


-- Signals an error with a simple message quoting the problem line.
--
function parse_err (msg)
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

   local msg = "line "..tostring(line_num)..": " .. (msg or "parse error")

   error (msg, 0)
end


-- Read and parse FILENAME by repeatedly matching PATTERN (if results
-- are desired, PATTERN should record them as a side-effect).
-- Repetition of PATTERN must cover the entire file, otherwise an error
-- is signaled.
--
function parse_file (filename, pattern)
   local stream, err = io.open (filename, "r")
   if not stream then
      error (err)
   end

   local text = stream:read'*a'

   -- Save old parse state, to allow recursive invocation.
   --
   local old_parse_state = parse_state

   parse_state = {filename = filename, text = text, err_pos = 1}

   local len = #text
   local pos = 1
   while pos <= len do
      parse_state.err_pos = pos
      pos = pattern:match (text, pos)
      if not pos then
	 parse_err ()
      end
   end

   -- Restore caller's parse state.
   --
   parse_state = old_parse_state
end
