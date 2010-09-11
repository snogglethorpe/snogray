#!/bin/sh

default_version="$1"
version_header="$2"

new_header="$version_header.new"

(
  echo '#ifndef __VERSION_H__'
  echo '#define __VERSION_H__'
  ( git describe 2>/dev/null || echo "$default_version" )	\
  | sed 's@v*\(.*\)@static const char *snogray_version = "\1";@'
  echo '#endif'
) > "$new_header"

if cmp -s "$new_header" "$version_header"; then
  rm "$new_header"
else
  mv "$new_header" "$version_header"
fi
