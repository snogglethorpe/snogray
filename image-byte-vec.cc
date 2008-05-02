// image-byte-vec.cc -- Common code for image formats based on vectors of bytes
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

#include "snogmath.h"
#include "excepts.h"
#include "string-funs.h"

#include "image-byte-vec.h"

using namespace snogray;


// ByteVecIo constructor

ByteVecIo::ByteVecIo (const ValTable &params)
  //
  // pixel-format related fields are set in the body below.
  //
  : target_gamma (params.get_float ("gamma", DEFAULT_TARGET_GAMMA))
{
  // We figure out pixel-format from these two user parameters.
  // "pixel-format", aka "pxfmt", is the main parameter, and
  // "alpha_channel" (aka "alpha") can be used to inpendentaly add an
  // alpha-channel (if the format specified by the "pixel-format"
  // parameter already has an alpha channel, then the "alpha-channel"
  // is silently ignored).
  //
  std::string pxfmt_name = params.get_string ("pixel-format,pxfmt", "rgb");
  bool alpha_channel = params.get_bool ("alpha-channel,alpha", false);

  std::string orig_pxfmt_name = pxfmt_name;
  size_t pxfmt_name_len = pxfmt_name.length ();

  // Stuff we figure out.
  //
  PixelFormat pxfmt;		// the pixel format
  unsigned comp_len = 1;	// length of each pixel format in bytes

  if (pxfmt_name_len > 2 && pxfmt_name.substr (pxfmt_name_len - 2) == "16")
    {
      comp_len = 2;
      pxfmt_name.erase (pxfmt_name_len - 2);
      pxfmt_name_len -= 2;
    }

  if (pxfmt_name_len > 1 && tolower (pxfmt_name[pxfmt_name_len - 1]) == 'a')
    {
      alpha_channel = true;
      pxfmt_name.erase (pxfmt_name_len - 1);
      pxfmt_name_len -= 1;
    }

  if (pxfmt_name == "g")
    pxfmt = PIXEL_FORMAT_GREY;
  else if (pxfmt_name == "rgb")
    pxfmt = PIXEL_FORMAT_RGB;
  else
    throw std::runtime_error ("unknown pixel format: " + orig_pxfmt_name);

  if (alpha_channel)
    pxfmt = pixel_format_add_alpha_channel (pxfmt);

  // Finally, set the pixel format.
  //
  set_pixel_format (pxfmt, comp_len);
}


ByteVecIo::ByteVecIo (PixelFormat pxfmt, unsigned _bytes_per_component,
		      float _target_gamma)
  //
  // pixel-format related fields are set in the body below.
  //
  : target_gamma (_target_gamma)
{
  set_pixel_format (pxfmt, _bytes_per_component);
}


// ByteVecIo::set_pixel_format

// Set the pixel format.
//
void
ByteVecIo::set_pixel_format (PixelFormat pxfmt, unsigned _bytes_per_component)
{
  pixel_format = pxfmt;

  if (pixel_format_base (pxfmt) == PIXEL_FORMAT_GREY)
    {
      num_channels = 1;
      pixel_format_name = "g";
    }      
  else
    {
      num_channels = 3;
      pixel_format_name = "rgb";
    }

  if (pixel_format_has_alpha_channel (pxfmt))
    {
      num_channels++;
      pixel_format_name += "a";
    }

  bytes_per_component = _bytes_per_component;
  if (bytes_per_component == 2)
    pixel_format_name += "16";
}


// Output

ByteVecImageSink::ByteVecImageSink (const std::string &filename,
				    unsigned width, unsigned height,
				    const ValTable &params)
  : ImageSink (filename, width, height, params), ByteVecIo (params),
    component_scale (1 << (bytes_per_component * 8)),
    max_component ((1 << (bytes_per_component * 8)) - 1),
    gamma_correction (1 / target_gamma),
    output_row (width * num_channels * bytes_per_component)
{
}

void
ByteVecImageSink::write_row (const ImageRow &row)
{
  unsigned width = row.width;
  ByteVec::iterator p = output_row.begin ();

  bool rgb = (pixel_format_base (pixel_format) == PIXEL_FORMAT_RGB);
  bool alpha_channel = (pixel_format_has_alpha_channel (pixel_format));

  for (unsigned x = 0; x < width; x++)
    {
      const Tint &tint = row[x];

      // We use a "pre-multiplied alpha" representation, whereas most
      // byte-oriented RGBA formats use a "separate alpha"
      // representation (because that avoids precision problems with the
      // small range of the components).
      //
      // Convert our representation to separate-alpha form by dividing
      // each color component by the alpha value.
      //
      // If we're not writing alpha information, then we skip the
      // division (even there's an internal alpha value not equal to
      // one), because we actually want the scaled "dimmer" colors to be
      // written to a color only image.
      //
      Tint::alpha_t alpha = 1, inv_alpha = 1;

      if (alpha_channel)
	{
	  alpha = tint.alpha;
	  inv_alpha = (alpha != 1 && alpha != 0) ? 1 / alpha : 1;
	}

      if (rgb)
	{
	  put_color_component (p, tint.r () * inv_alpha);
	  put_color_component (p, tint.g () * inv_alpha);
	  put_color_component (p, tint.b () * inv_alpha);
	}
      else
	put_color_component (p, tint.intensity () * inv_alpha);

      if (alpha_channel)
	put_alpha_component (p, alpha);
    }

  write_row (output_row);
}

float
ByteVecImageSink::max_intens () const
{
  return 1;
}


// Input

ByteVecImageSource::ByteVecImageSource (const std::string &filename,
					const ValTable &params)
  : ImageSource (filename, params), ByteVecIo (params),
    component_scale (
      1 / Color::component_t ((1 << (bytes_per_component * 8)) - 1)),
    input_row (0)
{ }

void
ByteVecImageSource::set_specs (unsigned _width, unsigned _height,
			       PixelFormat pxfmt, unsigned _bytes_per_component)
{
  width = _width;
  height = _height;

  set_pixel_format (pxfmt, _bytes_per_component);

  unsigned bit_depth = bytes_per_component * 8;
  component_scale = 1 / Color::component_t ((1 << bit_depth) - 1);

  if (width)
    input_row.resize (width * num_channels * bytes_per_component);
}

void
ByteVecImageSource::read_row (ImageRow &row)
{
  read_row (input_row);

  bool rgb = (pixel_format_base (pixel_format) == PIXEL_FORMAT_RGB);
  bool alpha_channel = (pixel_format_has_alpha_channel (pixel_format));

  unsigned width = row.width;
  ByteVec::const_iterator p = input_row.begin ();

  for (unsigned x = 0; x < width; x++)
    {
      Color::component_t r, g, b, a;

      r = get_color_component (p);

      if (rgb)
	{
	  g = get_color_component (p);
	  b = get_color_component (p);
	}
      else
	g = b = r;

      if (alpha_channel)
	{
	  a = get_alpha_component (p);

	  // We use a "pre-multiplied alpha" representation, whereas
	  // most byte-oriented RGBA formats use a "separate alpha"
	  // representation (because that avoids precision problems with
	  // the small range of the components).
	  //
	  // Convert the input representation to pre-multiplied form by
	  // multiplying each color component by the alpha value.
	  //
	  r *= a;
	  g *= a;
	  b *= a;
	}
      else
	a = 1;

      row[x] = Tint (Color (r, g, b), a);
    }
}


// arch-tag: ee0370d1-7cdb-42d4-96e3-4cf7757cc2cf
