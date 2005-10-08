// image.cc -- Image datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstdio>
#include <cmath>

#include <libpng/png.h>

#include "image.h"

const float Image::DEFAULT_TARG_GAMMA;

unsigned char
Image::color_component_to_byte (Color::component_t com)
{
  if (com < 0)
    return 0;

  if (gamma_correction != 1.0)
    com = powf (com, gamma_correction);

  if (com >= 0.9999)
    return 255;
  else
    return (unsigned char)(256.0 * com);
}

void Image::write_png_file (const char *name)
{
  FILE *stream = fopen (name, "wb");

  if (! stream)
    {
      fprintf (stderr, "%s: Could not open output file: %m\n", name);
      exit (99);
    }

  png_structp png
    = png_create_write_struct (PNG_LIBPNG_VER_STRING, (png_voidp)0, 0, 0);
  if (! png)
    {
      fprintf (stderr, "%s: Could not create PNG struct\n", name);
      fclose (stream);
      exit (98);
    }

  png_infop png_info = png_create_info_struct (png);
  if (! png_info)
    {
      png_destroy_write_struct(&png, (png_infopp)0);
      fclose (stream);
      fprintf (stderr, "%s: Could not create PNG info struct\n", name);
      exit (97);
    }

  if (setjmp (png_jmpbuf (png)))
    {
      png_destroy_write_struct (&png, &png_info);
      fclose (stream);
      fprintf (stderr, "%s: Error writing PNG file\n", name);
      exit (96);
    }

  png_set_IHDR (png, png_info, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_gAMA (png, png_info, targ_gamma);

  png_init_io (png, stream);

  png_write_info (png, png_info);

  for (unsigned y = 0; y < height; y++)
    {
      unsigned char row[width * 3], *p = row;

      for (unsigned x = 0; x < width; x++)
	{
	  const Color &col = (*this)(x, y);
 	  *p++ = color_component_to_byte (col.red);
 	  *p++ = color_component_to_byte (col.green);
 	  *p++ = color_component_to_byte (col.blue);
	}

      png_write_row (png, row);
    }

  png_write_end (png, png_info);
  png_destroy_write_struct (&png, &png_info);
  fclose (stream);
}

void
Image::interpolate_pixels (const Image &src_image, unsigned aa_factor)
{
  float scale = 1 / (float)(aa_factor * aa_factor);

  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++)
      {
	Color interp_color;
	unsigned src_base_x = x * aa_factor;
	unsigned src_base_y = y * aa_factor;

	for (unsigned offs_y = 0; offs_y < aa_factor; offs_y++)
	  for (unsigned offs_x = 0; offs_x < aa_factor; offs_x++)
	    interp_color
	      += src_image (src_base_x + offs_x, src_base_y + offs_y);

	(*this) (x, y) = interp_color * scale;
      }
}

// arch-tag: 3e9296c6-5ac7-4c39-8b79-45ce81b5d480
