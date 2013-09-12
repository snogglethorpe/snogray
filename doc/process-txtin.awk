#!/usr/bin/nawk
# Simple text preprocessor for documentation files
#
#  Copyright (C) 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

#
# This is a simple preprocessor for text files.  The basic purpose is
# to allow most function-level documentation to be kept in source
# files, from where it can be extracted and included in documentation.
#
# It is a little different from tools like doxygen or luadoc/ldoc, in that
# it:
#
#  + ... is far more modest, it just generates a single text stream which
#    is more or less a lightly-massaged copy of its input files.
#
#  + ... tries to generate documentation from "natural" function comments
#    without using special markup, instead relying on certain simple
#    commenting conventions.  The intention is that function comments
#    should read naturally both as comments and as documentation.
#
#  + ... is implemented as a single small awk script, and has no other
#    dependencies (other than awk, which is a standard system tool).
#
# It reads the input files specified on the command line and writes
# their contents to standard-output, with a slight amount of special
# processing:
#
#  1. A line of the form ".sec ..." is converted into a "Section
#     header" including the text "...".
#
#  2. A line of the form ".lua-module LUA_FILENAME" will cause the Lua
#     source file LUA_FILENAME to be read, and stylized comments
#     therein to be processed and echoed to standard output as
#     documentation.
#
# The processing of Lua source files in (2) is as follows:
#
#  A. "Block comments" of the form:
#
#        --
#        -- ...text...
#        -- ...more text...
#        --
#
#      Are emitted to standard-output, with the comment leaders
#      removed, and the text refilled.
#
#  B. "Sub-section" comments of the form:
#
#        ----------------
#        -- ...text...
#
#     Are emitted as "sub-section headers", with the text within
#     refilled.  The initial line of dashes can be any length greater
#     than 16.
#
#  C. "Function header" comments (with a following function definition)
#     of the form:
#
#        -- module.function -- Description
#        --
#        -- args: (ARG1, ARG2, ...)
#        --   or: ...
#        --
#        -- ...Explanatory text...
#        --
#        function module.function (...)
#          ...etc...
#
#     [The function definition line can also have the form
#     "module.function = ..."]
#
#     Are processed and emitted as follows:
#
#       + Only the comment portion is emitted
#
#       + Comment leaders are removed
#
#       + Text is refilled and wrapped in paragraphs separated by blank
#         lines (except as noted in the following items)
#
#       + Lines of the form "WORD: ..." always start a new paragraph,
#         even if not preceded by a blank line, and any wrapped text is
#         indented so that it aligns underneath the "..." following the
#         ":".  This is intended for the "args:" and "or:" lines.
#
#       + Lines of the form "...text... -- ...more text..." similarly
#         always start a new paragraph, even if not preceded by a blank
#         line, and any following wrapped text is intended to align
#         underneath "...more text...".  This feature allows simple
#         description lists to be formated, where the item to be
#         described precedes the " -- " and the description follows it.
#
#       + Similarly, lines that begin with "NUMBER. ", "(NUMBER) ",
#         "+ ", "* ", or "o ", representing enumerated or bullet
#         lists, always begin a new paragraph, and following text is
#         indented to follow the prefix.
#
#      All the actual Lua code, and any comments not fitting the above
#      patterns, are ignored.
#

#
# TODO:
#
#   + Add support for emitting html and/or latex.  Given the relative
#     simplicity of our formatting, and reliance on conventions, this
#     shouldn't actually be too hard I think, mainly just setting the
#     right margin really high to avoid wrapping, and adding some
#     post-processing when we emit various chunks of text.
#
#     Things like variables can be rendered in italic pretty simply as
#     we always write them in all-uppercase.
#
#     Some simple convention can be added to tag various variable
#     patterns as referencing a certain documentation entry, so
#     e.g. "POS1" would get wrapped in a hyperlink to an anchor at the
#     doc entry for "coord.pos", and have some appropriate mouseover
#     text added.
#
#   + Rewrite in Lua.  I like AWK, but the lack of good data-structure
#     support (and proper local variables) is painful, and this code
#     is much cruftier that it should be...
#

BEGIN {
  file = 0
  fill_width = 65
  rule_width = fill_width
  subsec_hdr_pfx = "-- "
  subsec_pfx = ""
  fundoc_pfx = "   "
  special_prefix_regexp = "^(.* -- | *([-a-zA-Z_0-9]+ *:|[0-9]+[)\\].]|[([][0-9]+[)\\]]|[+*o]) ) *"
}


# ----------------------------------------------------------------
# Misc string functions
#

# Return a string containing COUNT copies of CHUNK, separated by SEP
# if it is given.
#
function rep(chunk, count, sep      ,res)
{
  res = ""
  if (count > 0) {
    res = chunk
    while (count-- > 1)
      res = res sep chunk
  }
  return res
}


# ----------------------------------------------------------------
# Table-of-Contents processing
#
# Super simple processing of table of contents
#

BEGIN { table_of_contents = "" }

function add_table_of_contents_entry(tag, text)
{
  if (table_of_contents && tag !~ /[.]./)
    table_of_contents = table_of_contents "\n"
  table_of_contents = table_of_contents sprintf ("  %-6s %s\n", tag, text)
}

function emit_table_of_contents()
{
  printf ("%s", table_of_contents)   # no trailing newline
  table_of_contents = ""
}


# ----------------------------------------------------------------
# Output collection
#
# We want to delay final output so that it's possible to have forward
# references (e.g. a TOC).
#

BEGIN { num_output_chunks = 0 }

# Add TEXT to end of the list of output chunks.  It will be eventually
# emitted as output, followed by an additional newline (although TEXT may
# itself contain other newline).
#
function add_output_chunk(text)
{
  output_chunks[num_output_chunks++] = text
}

# Add a "special" output chunk with associated text SPECIAL to to end of
# the list of output chunks.  It will eventually be processed in order with
# normal output chunks, by calling "emit_special_output_chunk" with
# SPECIAL.
#
function add_special_output_chunk(special)
{
  special_output_chunks[num_output_chunks++] = special
}

# Print all accumulated output chunks to standard output in the order they
# were added, and clear the list of chunks.
#
function emit_output_chunks(    i)
{
  for (i = 0; i < num_output_chunks; i++)
    if (i in special_output_chunks)
      emit_special_output_chunk(special_output_chunks[i])
    else
      print output_chunks[i]

  num_output_chunks = 0
  delete output_chunks
}

function emit_special_output_chunk(special)
{
  if (special == ".toc")
    emit_table_of_contents()
}


# ----------------------------------------------------------------
# Simple text formatting
#
# The following "fmted_" functions take a string of a special form
# (which is usually passed in a paramter called "FMTED"), consisting
# of zero-or-more "already-formatted" lines, each newline-terminated,
# followed by final "pending" text, which is not newline-terminated.
#

# Retun TEXT with all tab characters replaced by an equivalent number
# of spaces.
#
function expand_tabs(text      ,before,after)
{
  while (match (text, /^[^\t]*\t/)) {
    before = substr (text, RSTART, RLENGTH - 1)
    after =  substr (text, RSTART + RLENGTH)
    do
      before = before " "
    while ((length (before) % 8) != 0)
    text = before after
  }
  return text
}

# Return a new fmted string with INITIAL_TEXT in the
# "pending" section.
#
function fmted_new(initial_text)
{
  return initial_text
}

# Return FMTED with any text in the "pending" section removed.
#
function fmted_no_pending(fmted)
{
  if (fmted ~ /\n/)
    sub (/\n[^\n]*$/, "\n", fmted)
  else
    fmted = ""
  return fmted
}

# Return only the "pending" text from FMTED.
#
function fmted_pending(fmted)
{
  sub (/.*\n/, "", fmted)
  return fmted
}

# Return FMTED with the line NEW added to the "already-formatted"
# section.  A newline will be added, so NEW should not contain one.
#
function fmted_add_fmted(fmted, new)
{
  return fmted_no_pending(fmted) new "\n" fmted_pending(fmted)
}

# Return FMTED with the "pending" text filled, wrapped, and appended
# to the already-formatted section, along with a new empty pending
# section.
#
# Wrapping is done to preserve a line-length of WIDTH, and any wrapped
# lines are prefixed with the string LMARGIN.
#
# Wrapping follows the rules mentioned in the file comment above, in
# that the wrapping is done so that wrapped text aligns with text
# following a "special prefix" as defined by SPECIAL_PREFIX_REGEXP
# and described in the file header comment.
#
function fmted_flush_para(fmted, width, lmargin    ,pending,first,pfx,pfx_len,wrap_point)
{
  if (lmargin)
    width -= length (lmargin)
  else
    lmargin = ""
  if (width < 1)
    width = 1

  if (fmted) {
    pending = fmted_pending(fmted)
    fmted = fmted_no_pending(fmted)

    # If there's a special prefix-defining syntax, use that in
    # preference to just left margin whitespace.
    #
    if (match (pending, special_prefix_regexp))
      pfx_len = (RSTART - 1) + RLENGTH
    else
      pfx_len = 0

    if (pfx_len == 0 || pfx_len >= width) {
      if (match (pending, /^ */))
	pfx_len = (RSTART - 1) + RLENGTH
      else
	pfx_len = 0
    }


    if (pfx_len < width)
      pfx = rep(" ", pfx_len)
    else {
      pfx_len = 0
      pfx = ""
    }

    first = 1
    while (length (pending) > width) {
      wrap_point = width - (first ? 0 : pfx_len)
      while (wrap_point > 1 && substr (pending, wrap_point, 1) != " ")
	wrap_point--
      if (wrap_point > 1) {
	while (wrap_point > 1 && substr (pending, wrap_point, 1) == " ")
	  wrap_point--
	if (substr (pending, wrap_point, 1) != " ")
	  wrap_point++
      } else {
	wrap_point = width
	while (wrap_point < length (pending) && substr (pending, wrap_point, 1) != " ")
	  wrap_point++
      }
      fmted = fmted_add_fmted(fmted, lmargin (first ? "" : pfx) substr (pending, 1, wrap_point - 1))
      while (wrap_point < length (pending) && substr (pending, wrap_point, 1) == " ")
	wrap_point++
      pending = substr (pending, wrap_point)
      first = 0
    }

    if (length (pending) > 0)
      fmted = fmted_add_fmted(fmted, lmargin (first ? "" : pfx) pending)
  }

  return fmted
}

# Make sure all text in FMTED is formated by calling fmted_flush_para
# with WIDTH and LMARGIN, and then return the already-formatted
# portion with some cleanups (particularly, line-final whitespace is
# removed).
#
function fmted_finish(fmted, width, lmargin)
{
  fmted = fmted_flush_para(fmted, width, lmargin)
  if (fmted) {
    gsub (/ *\n/, "\n", fmted)
    sub (/\n$/, "", fmted)
  } else
    fmted = ""
  return fmted
}

# Return FMTED with the text in NEW appended to the "pending" section,
# possibly moving existing text in the pending section to the
# already-formatted section if NEW starts a new line/paragraph.  WIDTH
# and LMARGIN are used for any wrapping/filling that needs to be done.
#
# Per the rules mentioned in the file comment, text is filled in
# blank-line separated paragraphs, except that if NEW matches
# SPECIAL_PREFIX_REGEXP, it always starts a new paragraph.
#
function fmted_add(fmted, new, width, lmargin)
{
  new = expand_tabs(new)

  if (! fmted)
    return new

  if (new ~ /^ *$/ || new ~ special_prefix_regexp)
    fmted = fmted_flush_para(fmted, width, lmargin)
  else if (fmted !~ /\n$/)
    sub (/^ */, " ", new)

  return fmted new
}

# Return FMTED with a blank line appended.  WIDTH and LMARGIN are used
# for any wrapping/filling that needs to be done.
#
function fmted_add_blank_line(fmted, width, lmargin)
{
  if (! fmted)
    return 0

  return fmted_flush_para(fmted, width, lmargin) lmargin "\n"
}


# ----------------------------------------------------------------
# pending chunks


function emit_extra_vert_ws()
{
  if (! first) {
    add_output_chunk("")
    add_output_chunk("")
  }
  first = 0
}


# ---- subsec comment chunks

function discard_pending_subsec_header()
{
  pending_subsec_header = 0
  pending_subsec_tag = 0
  pending_subsec_desc = 0
}

function emit_pending_subsec_header()
{
  if (pending_subsec_header) {
    subsec_num++

    emit_extra_vert_ws()
    add_output_chunk(rep("-", rule_width))
    add_output_chunk(pending_subsec_header)
    add_output_chunk("--")

    add_table_of_contents_entry(pending_subsec_tag, pending_subsec_desc)

    discard_pending_subsec_header()
  }
}

function add_pending_subsec_header(tag, desc, comment)
{
  if (pending_subsec_header)  {
    # If we still have a pending subsec comment, that means no
    # function entries were emitted in that subsection, so just
    # discard the whole thing.
    discard_pending_subsec_header()
    discard_pending_block_comments()
  } else {
    # Flush out any contents at end of previous subsection.
    emit_pending_fun_comment()
    emit_pending_block_comments()
  }

  pending_subsec_header = comment
  pending_subsec_tag = tag
  pending_subsec_desc = desc
}


# ---- block comment chunks

function discard_pending_block_comments()
{
  pending_block_comments = 0
}

function emit_pending_block_comments()
{
  if (pending_block_comments) {
    emit_pending_subsec_header()

    emit_extra_vert_ws()

    add_output_chunk(pending_block_comments)

    discard_pending_block_comments()
  }
}

function add_pending_block_comment(comment)
{
  emit_pending_fun_comment()

  if (pending_block_comments)
    pending_block_comments = pending_block_comments "\n\n\n" comment
  else
    pending_block_comments = comment
}


# ---- function comment chunks

function discard_pending_fun_comment()
{
  pending_fun_comment_name = 0
  pending_fun_comment_hdr = 0
  pending_fun_comment_body = 0
}

function emit_pending_fun_comment()
{
  if (pending_fun_comment_name) {
    emit_pending_subsec_header()
    emit_pending_block_comments()

    emit_extra_vert_ws()

    add_output_chunk(pending_fun_comment_hdr)
    if (pending_fun_comment_body) {
      add_output_chunk("")
      add_output_chunk(pending_fun_comment_body)
    }

    discard_pending_fun_comment()
  }
}

function add_pending_fun_comment(fun_name, comment_hdr, comment_body)
{
  # Flush out anything left pending before this.
  emit_pending_subsec_header()
  emit_pending_block_comments()
  emit_pending_fun_comment()

  pending_fun_comment_name = fun_name
  pending_fun_comment_hdr = comment_hdr
  pending_fun_comment_body = comment_body
}

function add_pending_fun_alias_comment(alias_name, fun_name)
{
  if (pending_fun_comment_name == fun_name)
    pending_fun_comment_hdr = pending_fun_comment_hdr "\n" alias_name " (alias)"
}


# ----------------------------------------------------------------
# include_lua_module

# Read the Lua source-file FILE, and emit its contents following the
# rules described in the file-header comment.  If the special strings
# @srcdir@, @top_srcdir@, @builddir@, and @top_builddir@ occur in
# FILE, they are replaced by the values of those variables.
#
function include_lua_module(file)
{
  curfun = 0
  in_header = 0
  in_subsec = 0
  subsec_num = 1
  subsec_comment = 0

  sub (/^@srcdir@/, srcdir, file)
  sub (/^@top_srcdir@/, top_srcdir, file)
  sub (/^@builddir@/, builddir, file)
  sub (/^@top_builddir@/, top_builddir, file)

  module = file
  sub (/^.*\//, "", module)
  sub (/[.]lua$/, "", module)

  module_entry_regexp = "^" module "[.][_a-zA-Z][_a-zA-Z0-9]*$"

  blanks = 0
  first = 1
  while ((getline line < file) > 0) {
    if (line ~ /^[ \t]/)
      continue  # skip everything not at top-level

    nf = split (line, f)

    if (!in_header && line ~ /^--/) {
      # drop unmatched function header-comment
      curfun = 0
    }

    if (curfun) {
      if (f[1] == "--") {
	if (nf == 1) {
	  if (in_fundoc_hdr) {
	    fundoc_hdr = fmted_flush_para(fundoc_hdr, fill_width, subsec_pfx)
	    fundoc = fmted_new("")
	    in_fundoc_hdr = 0
	  } else
	    blanks++
	} else {
	  sub (/^-- ?/, "", line)

	  if (in_fundoc_hdr) {
	    fundoc_hdr = fmted_add(fundoc_hdr, line, fill_width, subsec_pfx fundoc_pfx)
	  } else {
	    if (blanks)
	      fundoc = fmted_add_blank_line(fundoc, fill_width, subsec_pfx fundoc_pfx)
	    fundoc = fmted_add(fundoc, line, fill_width, subsec_pfx fundoc_pfx)
	    blanks = 0
	  }
	}
      } else if (nf > 0) {
	if (f[1] ~ module_entry_regexp && f[2] == "="	\
	    && f[3] ~ module_entry_regexp		\
	    && (nf == 3 || f[4] ~ /^--/))
	  # alias
	  add_pending_fun_alias_comment(f[1], f[3])
	else if ((f[1] == "function" && f[2] == curfun)	\
		 || (f[1] == curfun && f[2] == "=")	\
		 || (nf == 1 && f[1] == curfun)) # hack; "=" is on next line?
	{
	  add_pending_fun_comment(curfun,
				  fmted_finish(fundoc_hdr, fill_width, subsec_pfx),
				  fmted_finish(fundoc, fill_width, subsec_pfx fundoc_pfx))

	  fundoc = 0
	  fundoc_hdr = 0
	  curfun = 0
	}

	in_header = 0
	in_fundoc_hdr = 0
      } else
	fundoc = fmted_add_blank_line(fundoc, fill_width, subsec_pfx fundoc_pfx)
    } else if (in_subsec) {
      if (nf == 0) {
	subsec_desc = fmted_finish(subsec_desc, fill_width)
	subsec_comment = fmted_finish(subsec_comment, fill_width, subsec_hdr_pfx)
	add_pending_subsec_header(subsec_tag, subsec_desc, subsec_comment)
	in_subsec = 0
      } else {
	sub (/^---* */, "", line)
	subsec_desc = fmted_add(subsec_desc, line, fill_width)
	subsec_comment = fmted_add(subsec_comment, line, fill_width, subsec_hdr_pfx)
      }
    } else if (in_block_comment) {
      if (nf == 0) {
	add_pending_block_comment(fmted_finish(block_comment, fill_width))
	in_block_comment = 0
      } else if (f[1] != "--") {
	in_block_comment = 0
	block_comment = 0
      } else if (nf == 1) {
	blanks++
      } else {
	sub (/^-- ?/, "", line)
	if (blanks)
	  block_comment = fmted_add_blank_line(block_comment, fill_width)
	block_comment = fmted_add(block_comment, line, fill_width)
	blanks = 0
      }
    } else {
      if (f[1] == "--" && f[2] ~ module_entry_regexp) {
	curfun = f[2]
	sub (/^-- ?/, "", line)
	fundoc_hdr = fmted_new(line)
	blanks = 0
	in_header = 1
	in_fundoc_hdr = 1
      } else if (f[1] ~ module_entry_regexp && f[2] == "=" \
		 && f[3] ~ module_entry_regexp		  \
		 && (nf == 3 || f[4] ~ /^--/))
	# alias
	add_pending_fun_alias_comment(f[1], f[3])
      else if (line ~ /^---------------/) {
	in_subsec = 1
	subsec_tag = sec_num "." subsec_num
	subsec_desc = ""
	subsec_comment = subsec_tag "  "
      } else if (nf == 1 && f[1] == "--") {
	in_block_comment = 1
	block_comment = 0
	blanks = 0
      }
    }
  }
  close (file)

  emit_pending_block_comments()
  emit_pending_fun_comment()
  discard_pending_subsec_header()
}

# Print a section header.
#
function print_sec_hdr(string)
{
  sec_num++
  subsec_num = 0

  add_output_chunk(rep("#", rule_width))
  add_output_chunk("#")
  add_output_chunk("# " sec_num ". " string)
  add_output_chunk("#")
  add_output_chunk(rep("#", rule_width))

  add_table_of_contents_entry(sec_num ".", string)
}

# Print a document title.
#
function print_doc_title(title)
{
  title = expand_tabs(title)
  sub (/^ */, "", title)
  sub (/ *$/, "", title)
  
  width = rule_width - 4
  if (width < length (title))
    width = length (title)

  line = rep("#", width)
  empty = rep(" ", width)
  title_pad = rep(" ", (width - length (title)) / 2)

  add_output_chunk("##" line "##")
  add_output_chunk("##" line "##")
  add_output_chunk("##" empty "##")
  add_output_chunk("##" title_pad title title_pad  "##")
  add_output_chunk("##" empty "##")
  add_output_chunk("##" line "##")
  add_output_chunk("##" line "##")
}

# Process a Lua source module.
#
$1 == ".lua-module" { include_lua_module($2); next }

# Emit a section header.
#
$1 == ".sec" { sub (/^ *[.]sec */, ""); print_sec_hdr($0); next }

# Emit a table-of-comments
#
$1 == ".toc" { add_special_output_chunk($0); next }

# Document title
#
$1 == ".doc-title" { sub (/^ *[.][^ ]* */, ""); print_doc_title($0); next }

# Most text in the input is just echoed to the output.
#
{ add_output_chunk($0) }

END { emit_output_chunks() }
