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


-- Signals an error with a simple message quoting the problem line
--
function parse_err (text, pos, msg)
   local line_num = 1
   local count_pos = 1
   local bol_pos = 1

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

   local msg_pfx = "parse error on line "..tostring(line_num)..": "
   if msg then
      msg_pfx = msg_pfx .. msg .. ": "
   end

   error (msg_pfx .. LINE_CONTENTS:match (text, bol_pos), 0)
end

-- Call the lpeg pattern PATTERN's match function with TEXT and POS, and
-- return the result if it is non-nil.  If it returns nil (meaning that
-- there was no match), signal an error using parse_err.  If GET_ERR_POS
-- is non-nil, it should a function that will return a position in the
-- text for reporting errors.
--
function match_or_err (pattern, text, pos, get_err_pos)
   local next_pos = pattern:match (text, pos)
   if not next_pos then
      if get_err_pos then
	 pos = get_err_pos () or pos
      end
      parse_err (text, pos)
   end
   return next_pos
end

-- Read and parse FILENAME by repeatedly matching PATTERN (if results
-- are desired, PATTERN should record them as a side-effect).
-- Repetition of PATTERN must cover the entire file, otherwise an error
-- is signaled.  If GET_ERR_POS is non-nil, it should a function that
-- will return a position in the text for reporting errors.
--
function parse_file (filename, pattern, get_err_pos)
   local stream, err = io.open (filename, "r")
   if not stream then
      error (err)
   end

   local text = stream:read'*a'

   local len = #text
   local pos = 1
   while pos <= len do
      pos = match_or_err (pattern, text, pos, get_err_pos)
   end
end
