// render-cmdline.h -- Command-line options for rendering parameters
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDER_CMDLINE_H__
#define __RENDER_CMDLINE_H__


#define RENDER_OPTIONS_HELP "\
 Rendering options:\n\
  -a, --oversample=NUM       Use NUM samples for each output pixel\n\
  -n, --direct-samples=NUM   Use NUM light samples for direct lighting\n\
\n\
  -S, --surface-integ=INTEG  Use surface-integrator INTEG (default \"direct\")\n\
                               Options include:\n\
                                 \"direct\"     -- direct-lighting\n\
                                 \"path\"       -- path-tracing\n\
\n\
  -A, --background-alpha=ALPHA Use ALPHA as the opacity of the background\n\
\n\
  -R, --render-options=OPTS  Set output-image options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"min-trace\"  -- minimum trace ray length"

#if 0
"\n						\
\n\
  -w, --wire-frame[=PARAMS]  Output in \"wire-frame\" mode; PARAMS has the form\n\
                               [TINT][/COLOR][:FILL] (default: 0.7/1:0)\n\
                               TINT is how much object color affects wires\n\
                               COLOR is the base color of wires\n\
                               FILL is the intensity of the scene between wires\n"
#endif

#define RENDER_SHORT_OPTIONS "a:n:S:R:" // "w::"

#define RENDER_LONG_OPTIONS						\
  { "oversample",	required_argument, 0, 'a' },			\
  { "anti-alias",	required_argument, 0, 'a' },			\
  { "samples",		required_argument, 0, 'n' },			\
  { "surface-integ",	required_argument, 0, 'S' },			\
  { "sint",		required_argument, 0, 'S' },			\
  { "background-alpha", required_argument, 0, 'A' },			\
  { "bg-alpha",		required_argument, 0, 'A' },			\
  { "render-options",	required_argument, 0, 'R' }/*,			\
  { "wire-frame",	optional_argument, 0, 'w' }*/

#define RENDER_OPTION_CASES(clp, params)				\
  case 'a':								\
    params.set ("oversample", clp.unsigned_opt_arg ());			\
    break;								\
  case 'S':								\
    clp.store_opt_arg_with_sub_options ("surface-integ",		\
					params, ".", ",/");		\
    break;								\
  case 'R':								\
    clp.parse_opt_arg (params);						\
    break;								\
  /*case 'w':								\
    params.wire_frame = true;						\
    if (clp.opt_arg ())							\
      params.wire_frame_params.parse (clp);				\
      break;*/								\
  case 'n':								\
    params.set ("light-samples", clp.unsigned_opt_arg ());		\
    break;								\
  case 'A':								\
    params.set ("background-alpha", clp.float_opt_arg ());		\
    break;

#endif /* __RENDER_CMDLINE_H__ */

// arch-tag: 52eb3dc2-2c90-4a00-a093-216a52ca0f6d
