# Automake Makefile template for Snogray doc directory
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

include $(top_srcdir)/config.mk
include $(top_srcdir)/install-dirs.mk

dist_pkgdoc_DATA = tools.txt usage.txt issues.txt

EXTRA_DIST = building.txt process-txtin.awk

%.txt: %.txt.in
	$(AWK) -f $(srcdir)/process-txtin.awk top_srcdir='$(top_srcdir)' "$<" > "$@"

