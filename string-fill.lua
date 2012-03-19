-- string-fill.lua -- String filling
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

--
-- This module only contains a single function, so we return that from require
--


local str_match = string.match


local function match_word (str, pos)
   -- first try a sequence of hypens, which are considered a word when
   -- proceeded by whitespace
   --
   local word, sep_pos = str_match (str, "^(%-+[^- \t\n\\]*)()", pos)

   -- Then try a normal word, which stops at any hypen, whitespace, or
   -- backslash.
   --
   if not word then
      word, sep_pos = str_match (str, "^([^- \t\n\\]+)()", pos)
   end

   if not word then
      word, sep_pos = "", pos
   end

   return word, sep_pos
end


-- Fill STR to fit in COLS columns, splitting lines at whitespace and
-- hyphens (whitespace is removed at line splits, whereas hyphens are
-- retained).
--
-- The following additional special syntax is supported:
--
--  \-  A "soft" hypen, which is removed _except_ when it is used as a
--      split-point, in which case it is replaced by a normal hypen
--      instead.
--
--  \|  A line-break.  All whitespace _before_ a line-break is
--      removed, but all whitespace _following_ it is preserved
--      (allowing indentation).
--
--  \>  Set the "wrap margin" -- the left margin used when filling the
--      current paragraph -- to the current column.
--
--  \+[NUM]  Increase the current left margin by NUM (default 2) spaces
--
--  \-[NUM]  Decrease the current left margin by NUM (default 2) spaces
--
local function string_fill (str, cols, params)
   params = params or {}
   local left_margin = params.left_margin or ""
   local cur_left_margin = params.initial_left_margin or left_margin
   local wrap_margin = params.wrap_margin or ""
   local filled = ""
   local pos = 1
   local no_split, pre_split, post_split = nil, "", ""
   local pending_ws = cur_left_margin
   local cur_col = #pending_ws + 1
   local bol = true

   local function clear_pending_ws ()
      if pending_ws then
	 cur_col = cur_col - #pending_ws
	 pending_ws = nil
      end
   end
   local function output_pending_ws ()
      if pending_ws then
	 filled = filled..pending_ws
	 pending_ws = nil
      end
   end
   local function add_pending_ws (ws)
      if pending_ws then
	 pending_ws = pending_ws..ws
      else
	 pending_ws = ws
      end
      cur_col = cur_col + #ws
   end

   local function newline (wrapping)
      if pre_split ~= "" then
	 output_pending_ws ()
	 filled = filled..pre_split
	 pre_split = ""
      else
	 clear_pending_ws ()
      end

      filled = filled.."\n"
      cur_col = 1

      add_pending_ws (left_margin)
      if wrapping then
	 add_pending_ws (wrap_margin)
      end
      add_pending_ws (post_split)

      cur_left_margin = left_margin
      bol = true
   end

   local function add_word (word)
      if word ~= "" then
	 output_pending_ws ()
	 cur_col = cur_col + #word
	 filled = filled..word
	 bol = false
      end
   end

   while pos <= #str do
      local word, sep_pos = match_word (str, pos)
      pos = sep_pos

      -- Do wrapping or spacing
      --
      if not bol then
	 if not no_split then
	    -- explicit wrapping
	    newline (false)
	 elseif cur_col - 1 + #no_split + #word > cols then
	    -- wrapping due to filling
	    newline (true)
	 else
	    -- not wrapping
	    add_pending_ws (no_split)
	 end
      end
      no_split, pre_split, post_split = nil, "", ""

      add_word (word)

      -- Figure out how to deal with the separator following WORD
      --
      if pos <= #str then
	 local arg, next
	 arg, next = str_match (str, "^([ \t]+)()", sep_pos)
	 if arg then
	    no_split = arg
	 else
	    arg, next = str_match (str, "^([-]+)()", sep_pos)
	    if arg then
	       no_split = arg
	       pre_split = arg   -- hypens are always retained
	    elseif str_match (str, "^\\%-", sep_pos) then
	       no_split = ""
	       pre_split = "-"
	       next = sep_pos + 2
	    else
	       arg, next = str_match (str, "^\\|([ \t]*)()", sep_pos)
	       if arg then
		  if bol then
		     clear_pending_ws ()
		     add_pending_ws (cur_left_margin)
		  else
		     newline (false)
		  end

		  add_pending_ws (arg)
	       else
		  post_split = ""
		  next = str_match (str, "^\\>()", sep_pos)
		  if next then
		     wrap_margin = string.rep (" ", cur_col - 1 - #left_margin)
		     no_split = ""
		  else
		     local amount
		     arg, amount, next
			= str_match (str, "^\\([-+])([0-9]*)()", sep_pos)
		     if arg then
			if amount == '' then amount = 2 end
			amount = amount + (#cur_left_margin - #left_margin)
			if arg == '+' then
			   left_margin
			      = left_margin..string.rep (" ", amount)
			elseif #left_margin > amount then
			   left_margin
			      = string.sub (left_margin,
					    1, #left_margin - amount)
			else
			   left_margin = ""
			end
			no_split = ""
		     else
			next = str_match (str, "^\n[ \t]*\n+()", sep_pos)
			if next then
			   newline (false)
			   newline (false)
			else
			   next = str_match (str, "^\n[ \t]*()", sep_pos)
			   if next then
			      no_split = " "
			   else
			      error ('unknown string-filling escape "'
				     ..string.sub(str,sep_pos,sep_pos + 2)
				  ..'"')
			   end
			end
		     end
		  end
	       end
	    end
	 end

	 pos = next
      end
   end

   return filled
end


-- return the function
--
return string_fill
