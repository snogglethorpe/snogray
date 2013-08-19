# Makefile.am fragment defining extra Lua install dirs
#
#  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


# Lua files loaded directly.
#
pkgluadir = $(pkgdatadir)/lua

# Lua modules (loaded via the Lua 'require' function).
#
pkgluamoduledir = $(pkgluadir)/module

# Lua modules (loaded via the Lua 'require' function) whose module
# names have a "snogray." prefix.
#
pkgluamodulesnograydir = $(pkgluamoduledir)/snogray
