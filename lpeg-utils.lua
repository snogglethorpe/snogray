-- Useful functions for parsing with LPeg

module ("lpeg-utils", package.seeall)

local lpeg = require "lpeg"

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

-- number components
local p_pm = S"+-"^-1
local p_digs = R"09"^1
local p_opt_digs = R"09"^0
local p_fmant = p_pm * ((P"." * p_digs) + (p_digs * (P"." * p_opt_digs)^-1))
local p_fexp = P"e" * p_pm * p_digs

-- whole numbers
p_int = (p_pm * p_digs) / tonumber
p_float = (p_fmant * p_fexp^-1) / tonumber

-- whitespace
p_opt_ws = S" \t\r\n\f"^0	-- optional whitespace
p_req_ws = S" \t\r\n\f"^1 -- required whitespace
p_opt_horiz_ws = S" \t"^0	-- optional non-newline whitespace
p_req_horiz_ws = S" \t"^1 -- required non-newline whitespace
p_nl = S"\r\n\f"		-- newline char

-- line
p_line = (1 - p_nl)^0
p_line_nl = p_line * p_nl
p_line_contents = C (p_line)

-- whitespace followed by numbers (useful shortcuts)
p_ws_float = p_opt_ws * p_float
p_ws_int = p_opt_ws * p_int


-- Signals an error with a simple message quoting the problem line
--
function parse_err (text, pos)
   local line_num = 1
   local count_pos = 1
   while count_pos < pos do
      line_num = line_num + 1
      count_pos = p_line_nl:match (text, count_pos)
   end
   error ("parse error on line "..tostring(line_num)..": "
	  ..p_line_contents:match (text, pos), 0)
end

-- Call the lpeg pattern PATTERN's match function with TEXT and POS, and
-- return the result if it is non-nil.  If it returns nil (meaning that
-- there was no match), signal an error using parse_err.
--
function match_or_err (pattern, text, pos)
   local next_pos = pattern:match (text, pos)
   if not next_pos then
      parse_err (text, pos)
   end
   return next_pos
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

   local len = #text
   local pos = 1
   while pos <= len do
      pos = match_or_err (pattern, text, pos)
   end
end
