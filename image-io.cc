// image-io.cc -- Low-level image input and output
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstring>

#include "image-io.h"

using namespace snogray;


// Handy functions to throw an error.  The file-name is prepended.

void
ImageIo::err (const std::string &msg, bool use_errno)
{
  std::string buf (filename);
  buf += ": ";
  buf += msg;
  if (use_errno)
    {
      buf += ": ";
      buf += strerror (errno);
    }
  throw std::runtime_error (buf);
}

void
ImageIo::err (const char *msg, bool use_errno)
{
  err (std::string (msg), use_errno);
}

void
ImageIo::open_err (const char *dir, const std::string &msg, bool use_errno)
{
  std::string buf ("Error opening ");
  buf += dir;
  buf += " file";
  if (! msg.empty ())
    {
      buf += ": ";
      buf += msg;
    }
  err (buf, use_errno);
}

void
ImageSink::open_err (const std::string &msg, bool use_errno)
{
  ImageIo::open_err ("output", msg, use_errno);
}

void
ImageSink::open_err (const char *msg, bool use_errno)
{
  open_err (std::string (msg), use_errno);
}

void
ImageSource::open_err (const std::string &msg, bool use_errno)
{
  ImageIo::open_err ("input", msg, use_errno);
}

void
ImageSource::open_err (const char *msg, bool use_errno)
{
  open_err (std::string (msg), use_errno);
}

void
ImageSink::flush ()
{
  // do nothing
}

float
ImageSink::max_intens () const
{
  return 0;			// no (meaningful) maximum, i.e. floating-point
}

ImageSource::~ImageSource () { }
ImageSink::~ImageSink () { }


// arch-tag: 3e9296c6-5ac7-4c39-8b79-45ce81b5d480
