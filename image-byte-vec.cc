// image-byte-vec.cc -- Common code for image formats based on vectors of bytes
//
//  Copyright (C) 2005-2008, 2010-2012  Miles Bader <miles@gnu.org>
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
  : target_gamma (params.get_float ("gamma", default_target_gamma()))
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
    component_scale (Color::component_t ((1 << (bytes_per_component * 8)) - 1)),
    max_component ((1 << (bytes_per_component * 8)) - 1),
    gamma_correction (1 / target_gamma),
    output_row (width * num_channels * bytes_per_component),
    dither (params.get_bool ("dither", true))
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
      Color color;
      if (alpha_channel)
	color = tint.unscaled_color ();
      else
	color = tint.alpha_scaled_color ();

      if (rgb)
	{
	  put_color_component (p, color.r ());
	  put_color_component (p, color.g ());
	  put_color_component (p, color.b ());
	}
      else
	put_color_component (p, color.intensity ());

      if (alpha_channel)
	put_alpha_component (p, tint.alpha);
    }

  write_row (output_row);
}

  // Floating-point to integer and range conversion for color
  // components.
  //
unsigned
ByteVecImageSink::color_component_to_int (Color::component_t com)
{
  com = max (com, 0.f);

  // Gamma-correct.
  //
  if (gamma_correction != 0)
    com = pow (com, gamma_correction);

  // Scale to the final range.
  //
  com *= component_scale;

  // Add dither, which should help avoid banding of very gradual
  // gradients when the image source has greater precision than
  // the image output format.
  //
  // If the source has the same or less precision, this should
  // basically have no effect, but to avoid problems due to
  // accumulated error, we slightly reduce the magnitude of the noise
  // (it's nominally in the range of 0-1) to ensure this.
  //
  // If we're not dithering, then bias the result so that we can
  // exactly reproduce an input from a source with the same precision,
  // even given some accumulated error.
  //
  com += dither ? (dither_noise () * 0.999f) : 0.5f;

  // Clamp to the final range.
  //
  com = min (com, max_component);

  // ... and finally, convert to an integer.
  //
  return unsigned (com);
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
			       PixelFormat pxfmt, unsigned _bytes_per_component,
			       unsigned bits_per_component)
{
  width = _width;
  height = _height;

  set_pixel_format (pxfmt, _bytes_per_component);

  // BITS_PER_COMPONENT== 0 means "use default" (8 * BYTES_PER_COMPONENT).
  //
  if (bits_per_component == 0)
    bits_per_component = 8 * bytes_per_component;

  component_scale = 1 / Color::component_t ((1 << bits_per_component) - 1);

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

      a = alpha_channel ? get_alpha_component (p) : 1;

      row[x] = Tint (Color (r, g, b), a);
    }
}


// arch-tag: ee0370d1-7cdb-42d4-96e3-4cf7757cc2cf
