-- cmdlineparser.lua -- Parsing of GNU-style command-line options
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
local cmdlineparser = {}

-- make the module callable, as a shortcut for "cmdlineparser.new"
--
setmetatable (cmdlineparser,
	      { __call = function (m, ...) return m.new (...) end })

-- method table for parsers
--
local cmdlineparser_meths = {}

-- shared metatable for parsers
--
local parser_metatable = {
   __index = cmdlineparser_meths,
   is_cmdline_parser = true
}


----------------------------------------------------------------
-- Parameters


-- How many columns we use for help output
--
local help_max_columns = 79

-- The column (starting from 1) in which we put option descriptions
--
local help_option_doc_column = 30


----------------------------------------------------------------
-- Imports
--

local string = require 'snogray.string'
local sep_concat, right_pad = string.sep_concat, string.right_pad

local table = require 'snogray.table'

local string_fill = require 'snogray.string-fill'

-- Local names for some commonly used functions
--
local str_match, str_gmatch, str_find, str_sub, str_gmatch, str_gsub
   = string.match, string.gmatch, string.find, string.sub,
     string.gmatch, string.gsub
local array_delete = table.remove


----------------------------------------------------------------
-- Functions for parsing various types of arguments (mainly used for
-- option arguments, although there's no explicit connectin).
--

function cmdlineparser.unsigned_argument (string)
   local val = str_match (string, "^%d+$") and tonumber (string)
   if not val then
      error ("invalid integer argument \""..string.."\"", 0)
   end
   return val
end

function cmdlineparser.float_argument (string)
   local val = tonumber (string)
   if not val then
      error ("invalid numeric argument \""..string.."\"", 0)
   end
   return val
end

local standard_type_parsers = {
   unsigned = cmdlineparser.unsigned_argument,
   float = cmdlineparser.float_argument
}


----------------------------------------------------------------
-- Module functions
--

-- return a new parser
--
function cmdlineparser.new (options)
   local parser = table.shallow_copy (options)
   parser.option_matcher_cache = {}
   parser.arg_type_parsers
      = setmetatable ({}, {__index = standard_type_parsers})
   return setmetatable (parser, parser_metatable)
end

function cmdlineparser.error (msg)
   return error (msg, 0)
end


----------------------------------------------------------------
-- Parser type-parsing method

function cmdlineparser_meths:parse_arg (string, type, default)
   if not string then
      if not default then
	 self:error ("no default argument")
      end
      return default
   end

   if type then
      local parsers = self.arg_type_parsers
      local parser = (parsers and parsers[type]) or standard_type_parsers[type]
      if not parser then
	 self:error ("no parser for argument type \""..type.."\"")
      end
      return parser (string)
   else
      return string
   end
end


----------------------------------------------------------------
-- Parser helper functions
--

-- Return true if this is an "option entry" in a parser.
--
local function is_option (parser_entry)
   return type (parser_entry) == 'table' and type (parser_entry[1]) == 'string'
end

-- Return true if THING is a parser.
--
local function is_parser (thing)
   local mt = getmetatable (thing)
   return mt and mt.is_cmdline_parser
end


----------------------------------------------------------------
-- Option help
--

local option_doc_pfx = string.rep (" ", help_option_doc_column - 1)

local function one_option_help (option)
   if option.no_help then
      return nil
   end

   local before_arg = str_match (option[1], "^[^=]*")
   local arg = str_match (option[1], "^[^=]*=(.*)$")

   local short_opts, long_opts = nil, nil
   for opt in str_gmatch (before_arg, "[^/]+") do
      if str_match (opt, "^%-%-") then
	 long_opts = sep_concat (long_opts, ", ", opt)
      else
	 short_opts = sep_concat (short_opts, ", ", opt)
      end
   end

   if arg then
      if long_opts then
	 long_opts = long_opts.."="..arg
      else
	 short_opts = short_opts.." "..arg
      end
   end

   local opts
   if short_opts then
      opts = sep_concat (short_opts, ", ", long_opts)
   elseif long_opts then
      opts = "    "..long_opts
   end
   opts = "  "..opts

   local doc = option.doc
   if doc then
      doc = str_gsub (string_fill (doc,
				   help_max_columns
				      - (help_option_doc_column - 1),
				   { initial_left_margin = "",
				     left_margin = "  " }),
		      "\n", "\n"..option_doc_pfx)
      if #opts < help_option_doc_column - 1 then
	 opts = string.right_pad (opts, help_option_doc_column - 1)..doc
      else
	 opts = opts.."\n"..option_doc_pfx..doc
      end
   end

   return opts
end

local function options_help (parser)
   local help = nil
   local add_sep = false
   for k = 1, #parser do
      local entry = parser[k]

      local entry_help
      if is_parser (entry) then
	 entry_help = options_help (entry)
	 add_sep = true
      elseif is_option (entry) then
	 entry_help = one_option_help (entry)
      elseif type (entry) == 'string' then
	 entry_help = " "..entry.."\n"
      end

      if entry_help then
	 help = sep_concat (help, "\n", entry_help)

	 if add_sep and help then
	    help = help.."\n"
	    add_sep = false
	 end
      end
   end
   return help
end


----------------------------------------------------------------
-- Standard help/usage/etc messages.
--

local function nice_prog_name (prog_name)
   return str_match (prog_name, ".*/([^/]+)$") or prog_name or "???"
end

-- Return a usage message.
--
function cmdlineparser_meths:usage_msg ()
   local prog_name = nice_prog_name (self.prog_name)
   local usage = "Usage: "..prog_name.." [OPTION...]"
   if self.usage then
      usage = usage.." "..self.usage
   end
   return usage
end

-- Print a usage message to stderr and exit with an error status.
--
function cmdlineparser_meths:usage_error ()
   io.stderr:write (self:usage_msg ().."\n")
   os.exit (1)
end

-- Return a version message for this parser.
--
function cmdlineparser_meths:version_msg ()
   local msg = nice_prog_name (self.prog_name)
   if self.package and self.package ~= msg then
      msg = msg.." ("..self.package..")"
   end
   return sep_concat (msg, " ", self.version)
end

-- Return a help message for this parser.
--
function cmdlineparser_meths:help_msg ()
   local desc = (self.desc
		 and string_fill (self.desc, help_max_columns,
				  { initial_left_margin = "",
				    left_margin = "  " }))
   local doc = self.doc and string_fill (self.doc, help_max_columns)
   
   return sep_concat (self:usage_msg (),
		      "\n", desc,
		      "\n\n", options_help (self),
		      "\n\n", doc)
end


----------------------------------------------------------------
-- Help option parser
--

-- Return an option parser which is like BASE_PARSER, but also
-- includes standard options (--help, --version, etc).
--
function cmdlineparser.add_standard_options (base_parser)
   -- Print STR and exit.
   local function pr_and_exit (str) print (str) os.exit (0) end

   -- There's a little trickiness here: we assign the parser we're
   -- creating to a local variable, which the help function closes
   -- over that variable, meaning it can see the entire parser of
   -- which it it is a part (so the newly added options are included
   -- in the help output).
   --
   local outer_parser
   outer_parser = cmdlineparser {
      -- copy various info fields from BASE_PARSER
      prog_name = base_parser.prog_name,
      usage = base_parser.usage,
      desc = base_parser.desc,
      doc = base_parser.doc,
      version = base_parser.version,
      package = base_parser.package,

      -- handle wrapped options
      base_parser,

      -- standard options
      { "--help", function () pr_and_exit (outer_parser:help_msg ()) end },
      { "--usage", function () pr_and_exit (outer_parser:usage_msg ()) end },
      { "--version", function () pr_and_exit (outer_parser:version_msg ()) end }
   }
   return outer_parser
end


----------------------------------------------------------------
-- Main parsing option-parsing method
--

-- Signal an error MSG, in PARSER trying to parse CMDLINE.
--
local function parse_error (parser, cmdline, msg)
   error (msg, 0)
end

-- Make a "matcher" for the parser option entry OPTION.  A matcher is
-- just somewhat more processed verion of the option entry.
--
local function make_matcher (option)
   local before_arg = str_match (option[1], "^[^=]*")
   local arg = str_match (option[1], "^[^=]*=(.*)$")

   local opts = {}
   for opt in str_gmatch (before_arg, "[^/]+") do
      opts[#opts+1] = opt
   end

   return { choices = opts, arg = arg, action = option[2], option = option }
end

-- Split the string ARG into "option" and "option argument" parts, and
-- return them in that order; if there is no option argument part, nil
-- is returned for that.
--
-- A third return value is non-nil only if ARG is a short option, and
-- is ARG rewritten with the option removed (leaving any remaining
-- short-options in ARG).
--
local function split_option_arg (arg)
   if str_match (arg, "^%-%-") then
      -- long option
      return str_match (arg, "^[^=]*"), str_match (arg, "^[^=]*=(.*)")
   else
      -- short option
      local opt, opt_arg, rewrite_opt
      opt = str_match (arg, "^%-.")
      opt_arg = str_match (arg, "^%-.(..*)")
      return opt, opt_arg, opt_arg and ("-"..opt_arg)
   end
end

local function try_match (parser, matcher, cmdline)
   if not matcher.action then
      return false
   end

   local opt = cmdline[1]
   local opt_arg, rewrite_non_arg_opt

   opt, opt_arg, rewrite_non_arg_opt = split_option_arg (opt)

   local choices = matcher.choices
   for i = 1, #choices do
      if opt == choices[i] then
	 -- yay, MATCHER matched!

	 if matcher.arg then
	    -- This option needs an argument; if there wasn't one as part
	    -- of the option, try the next entry in CMDLINE
	    if not opt_arg then
	       if #cmdline < 2 then
		  parse_error (parser, cmdline,
			       "option requires an argument -- '"..opt.."'")
	       end
	       opt_arg = cmdline[2]
	       array_delete (cmdline, 2)
	    end
	    rewrite_non_arg_opt = nil
	 else
	    opt_arg = nil
	 end

	 if rewrite_non_arg_opt then
	    cmdline[1] = rewrite_non_arg_opt
	 else
	    array_delete (cmdline, 1)
	 end

	 local action = matcher.action
	 local function do_action ()
	    if type (action) == 'table' then
	       local receiver = action[1]
	       if type (receiver) == 'table' then
		  receiver[action[2]]
		     = parser:parse_arg (opt_arg, action[3], action[4])
	       elseif type (receiver) == 'function' then
		  receiver (parser:parse_arg (opt_arg, action[2], action[3]))
	       else
		  error ("invalid option action", 0)
	       end
	    elseif matcher.option.pass_option_name then
	       action (opt, opt_arg)
	    else
	       action (opt_arg)
	    end
	 end

	 local ok, err = pcall (do_action)
	 if not ok then
	    parse_error (parser, cmdline, opt..": "..err)
	 end

	 return true
      end
   end

   return false
end

local function try_option (parser, option, cmdline)
   local matcher_cache = parser.option_matcher_cache
   local matcher = matcher_cache[option]
   if not matcher then
      matcher = make_matcher (option)
      matcher_cache[option] = matcher
   end
   return try_match (parser, matcher, cmdline)
end

-- Parse a single option at the beginning of CMDLINE, and return true
-- if some option matched, or false if none did.  If an option
-- matched, it is removed from CMDLINE.
--
local function parse_one_opt (parser, cmdline)
   for k = 1, #parser do
      local entry = parser[k]

      local matched = false
      if is_parser (entry) then
	 -- See if the sub-parser ENTRY succeeds.
	 matched = parse_one_opt (entry, cmdline)
      elseif is_option (entry) then
	 -- See if the option description ENTRY matches.
	 matched = try_option (parser, entry, cmdline)
      end

      -- If ENTRY matched, then we can stop looking
      if matched then
	 return matched
      end
   end

   return false
end

-- Parse options in the table CMDLINE, and return a table of non-option
-- arguments.
--
function cmdlineparser_meths:parse_options (cmdline)
   cmdline = table.shallow_copy (cmdline)

   -- Wrap ourselves with a parser to handle standard command-line options.
   --
   self = cmdlineparser.add_standard_options (self)

   -- Accumulated non-options args (which we allow to be intermixed
   -- with options).
   --
   local non_option_args = {}

   while #cmdline > 0 do
      if cmdline[1] == "--" then
	 -- "--" separates options from non-options; add all remaining
	 -- arguments (regardless of syntax) to NON_OPTION_ARGS, and
	 -- stop parsing.
	 for i = 2, #cmdline do
	    non_option_args[#non_option_args + 1] = cmdline[i]
	 end
	 break			-- stop parsing
      end

      if str_match (cmdline[1], "^%-") then
	 local ok = parse_one_opt (self, cmdline)
	 if not ok then
	    local opt = split_option_arg (cmdline[1])
	    parse_error (self, cmdline, "unrecognized option '"..opt.."'")
	 end
      else
	 non_option_args[#non_option_args + 1] = cmdline[1]
	 array_delete (cmdline, 1)
      end
   end

   return non_option_args
end


-- make parsers callable, acting like a call to "parse:parse_options"
--
parser_metatable.__call = cmdlineparser_meths.parse_options


----------------------------------------------------------------
-- parse_params
--

-- First, if PARAMS_SEPARATOR_PAT is not null, split STRING into parts
-- separated by that pattern.  Then parse each part as a one of the
-- following parameter definition types:
-- + "NAME=VALUE" or "NAME:VALUE" -- a string parameter called NAME
-- with the given value.
-- + "!NAME" or "no-NAME" -- A false boolean parameter called NAME.
-- + "NAME" -- a true boolean parameter called NAME.
--
function cmdlineparser.parse_params (string, table, params_separator_pat)
   function parse_one_param (string)
      local name, val = str_match (string, "^([^=:]+)%s*[=:]%s*(.*)$")
      if name then
	 table[str_gsub (name, "-", "_")] = val
      else
	 string = str_gsub (string, "-", "_")
	 if str_match ("^!", string) then
	    table[str_sub (string, 2)] = false
	 elseif str_match ("^no_", string) then
	    table[str_sub (string, 4)] = false
	 else
	    table[string] = true
	 end
      end
   end
   if params_separator_pat then
      local param_search_pos = 1
      repeat
	 local sep_beg, sep_end
	    = str_find (string, params_separator_pat, param_search_pos)
	 if sep_beg then
	    parse_one_param (str_sub (string, param_search_pos, sep_beg - 1))
	    param_search_pos = sep_end + 1
	 end
      until not sep_beg
      parse_one_param (str_sub (string, param_search_pos))
   else
      parse_one_param (string)
   end
end


----------------------------------------------------------------
-- store_with_sub_params
--

-- First, split STRING into a "main value" and "sub-parameters", at
-- the pattern SUB_PARAMS_SEPARATOR_PAT.  Then, store the main value into
-- a subtable of TABLE called NAME with a key of MAIN_SUBKEY.  The
-- optional values will be further split apart using
-- SUB_PARAM_SEPARATOR_PAT, as if with parse_opt_arg, and each
-- OPT_NAME=OPT_VAL pair will be stored into the subtable as well,
-- with OPT_NAME as the key.
--
-- The default value for SUB_PARAMS_SEPARATOR_PAT is "%s*[/,]%s*".
-- The default value for SUB_PARAM_SEPARATOR_PAT is
-- SUB_PARAMS_SEPARATOR_PAT.
--
-- For example: if STRING is "oink/bar=zoo,zing=3", NAME is "plugh",
-- MAIN_SUBKEY is "type", then the main value will be "oink", and the
-- following entries will be stored into a sub-table of TABLE called
-- "plugh":
--
--   TABLE.plugh.type = "oink"
--   TABLE.plugh.bar  = "zoo"
--   TABLE.plugh.zing = 3
--
function cmdlineparser.store_with_sub_params (string, name, table, main_subkey,
					      sub_params_separator_pat,
					      sub_param_separator_pat)
   sub_params_separator_pat = sub_params_separator_pat or "%s*[/,]%s*"
   sub_param_separator_pat
      = sub_param_separator_pat or sub_params_separator_pat

   local subtable = table[name]
   if not subtable then
      subtable = {}
      table[name] = subtable
   end

   local sub_params_sep_beg, sub_params_sep_end
      = str_find (string, sub_params_separator_pat)
   if sub_params_sep_end then
      subtable[main_subkey] = str_sub (string, 1, sub_params_sep_beg - 1)
      cmdlineparser.parse_params (str_sub (string, sub_params_sep_end + 1),
				  subtable, sub_param_separator_pat)
   else
      subtable[main_subkey] = string
   end
end


----------------------------------------------------------------

-- return the module
--
return cmdlineparser
