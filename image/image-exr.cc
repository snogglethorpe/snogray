// image-exr.cc -- EXR format image handling
//
//  Copyright (C) 2005-2008, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "image-exr.h"


using namespace snogray;



// ----------------------------------------------------------------
// ExrImageSink: EXR image output


ExrImageSink::ExrImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const ValTable &params)
  : ImageSink (filename, width, height, params),
    outf (filename.c_str(), width, height,
	  (params.get_bool ("alpha_channel,alpha")
	   ? Imf::WRITE_RGBA
	   : Imf::WRITE_RGB)),
    row_buf (width), cur_y (0)
{
  if (params.contains ("gamma"))
    open_err ("OpenEXR format does not use gamma correction");
}


void
ExrImageSink::write_row (const ImageRow &row)
{
  for (unsigned x = 0; x < row.width; x++)
    {
      const Tint &tint = row[x];
      const Color &col = tint.alpha_scaled_color ();

      // Note that EXR files use pre-multiplied alpha like we do.
      //
      Imf::Rgba rgba (col.r(), col.g(), col.b(), tint.alpha);

      row_buf[x] = rgba;
    }

  outf.setFrameBuffer (&row_buf[0] - cur_y * row.width, 1, row.width);
  outf.writePixels ();

  cur_y++;
}



// ----------------------------------------------------------------
// ExrImageSource: EXR image input


ExrImageSource::ExrImageSource (const std::string &filename,
				const ValTable &params)
  : ImageSource (filename, params), inf (filename.c_str()), cur_y (0)
{
  const Imf::Header &hdr = inf.header ();
  const Imath::Box2i &data_window = hdr.dataWindow ();

  width = data_window.max.x - data_window.min.x + 1;
  height = data_window.max.y - data_window.min.y + 1;

  row_buf.resize (width);
}


void
ExrImageSource::read_row (ImageRow &row)
{
  inf.setFrameBuffer (&row_buf[0] - cur_y * row.width, 1, row.width);
  inf.readPixels (cur_y);

  for (unsigned x = 0; x < row.width; x++)
    {
      const Imf::Rgba &rgba = row_buf[x];

      // Note that EXR files use pre-multiplied alpha like we do.
      //
      Tint tint;
      tint.set_scaled_rgba (rgba.r, rgba.g, rgba.b, rgba.a);

      row[x] = tint;
    }

  cur_y++;
}


// arch-tag: a6c557de-fa39-4773-8357-dee599502a47
