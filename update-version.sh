#!/bin/sh

default_version="$1"
version_source_file="$2"

new_source="$version_source_file.new"

(
  echo '#include "version.h"'
  ( git describe 2>/dev/null || echo "$default_version" )	\
  | sed 's@v*\(.*\)@const char *snogray::snogray_version = "\1";@'
) > "$new_source"

if cmp -s "$new_source" "$version_source_file"; then
  rm "$new_source"
else
  mv "$new_source" "$version_source_file"
fi
