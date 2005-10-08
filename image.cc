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

#include <cmath>

#include <libpng/png.h>

#include "image.h"

using namespace Snogray;

ImageSink::~ImageSink () { }

ImageOutput::ImageOutput (const class ImageSinkParams &params,
			  unsigned _aa_factor, unsigned aa_overlap,
			  float (*aa_filter)(int, unsigned))
  : sink (params.make_sink ()),
    aa_factor (_aa_factor),
    aa_kernel_size (_aa_factor + aa_overlap),
    recent_rows (new ImageRow*[_aa_factor + aa_overlap]),
    aa_row (0), aa_kernel (0),
    next_row_offs (0), num_accumulated_rows (0)
{
  if (aa_kernel_size > 1)
    {
      aa_row = new ImageRow (params.width);

      aa_kernel
	= make_aa_kernel (aa_filter, aa_kernel_size);
    }

  for (unsigned offs = 0; offs < aa_kernel_size; offs++)
    recent_rows[offs] = new ImageRow (params.width * aa_factor);
}

ImageOutput::~ImageOutput ()
{
  write_accumulated_rows ();

  delete sink;

  for (int offs = 0; offs < aa_factor; offs++)
    delete recent_rows[offs];
  delete[] recent_rows;

  if (aa_row)
    delete aa_row;
  if (aa_kernel)
    delete[] aa_kernel;
}

void
ImageOutput::write_accumulated_rows ()
{
  if (num_accumulated_rows)
    {
      if (aa_kernel_size > 1)
	// Anti-alias recent rows and write the result to our output file
	{
	  fill_aa_row ();
	  sink->write_row (*aa_row);
	}
      else
	// For non-anti-aliased output, just write what we have
	sink->write_row (*recent_rows[0]);

      num_accumulated_rows = 0;
    }
}

ImageRow &
ImageOutput::next_row ()
{
  if (num_accumulated_rows >= aa_factor)
    // Once we accumulate enough output rows, write to our output sink
    write_accumulated_rows ();

  num_accumulated_rows++;

  if (next_row_offs >= aa_kernel_size)
    next_row_offs = 0;

  return *recent_rows[next_row_offs++];
}


// Anti-aliasing

// "Box" filter weights each pixel in the source evenly
float
ImageOutput::aa_box_filter (int offs, unsigned size)
{
  return 1 / (float)size;
}

// Triangle filter weights pixels in the source linearly according to
// the (inverse of) their distance from the center pixel
float
ImageOutput::aa_triang_filter (int offs, unsigned size)
{
  float r = (float)(size + 1) / 2;

  if (offs < 0)
    offs = -offs;
  
  return (1 - (offs / r)) / r;
}

// Gaussian filter weights pixels in the source using a gaussian distribution
float
ImageOutput::aa_gauss_filter (int offs, unsigned size)
{
  float r = (float)(size + 1) / 2;
  float x = offs;
  return (M_SQRT2 * (1 / (2 * sqrt (M_PI))) * pow (M_E, -x*x / 2));
}

float *
ImageOutput::make_aa_kernel (float (*aa_filter)(int offs, unsigned size),
			     unsigned kernel_size)
{
  float *kernel = new float[kernel_size * kernel_size];
  unsigned center_offs = kernel_size / 2;

  // Calculate kernel elements.  This assumes that the filter is "separable".
  for (unsigned y = 0; y < kernel_size; y++)
    {
      int offs_y = y - center_offs;
      float y_filt_fact = (*aa_filter) (offs_y, kernel_size);

      for (unsigned x = 0; x < kernel_size; x++)
	{
	  int offs_x = x - center_offs;
	  float x_filt_fact = (*aa_filter) (offs_x, kernel_size);

	  kernel[y * kernel_size + x] = x_filt_fact * y_filt_fact;
	}
    }

  // Because we are using discrete samples, it's possible for small
  // kernel sizes to sum to some number noticeably less than one, making
  // the output image too dim; calculate a compensenation factor.
  float kernel_sum = 0;
  for (unsigned y = 0; y < kernel_size; y++)
    for (unsigned x = 0; x < kernel_size; x++)
      kernel_sum += kernel[y * kernel_size + x];

  // If it's significant, adjust the kernel by that factor
  if (kernel_sum < 0.99 || kernel_sum > 1)
    {
      float compensation_factor = 1 / kernel_sum;

      for (unsigned y = 0; y < kernel_size; y++)
	for (unsigned x = 0; x < kernel_size; x++)
	  kernel[y * kernel_size + x] *= compensation_factor;
    }

  return kernel;
}

void
ImageOutput::fill_aa_row ()
{
  unsigned aa_overlap = aa_kernel_size - aa_factor;
  unsigned aa_width = aa_row->width;
  unsigned src_width = aa_width * aa_factor;

  for (unsigned x = 0, src_base_x = 0; x < aa_width; x++, src_base_x += aa_factor)
    {
      Color aa_color;
      const float *kernel_row = aa_kernel;

      for (unsigned offs_y = 0; offs_y < aa_kernel_size; offs_y++)
	{
	  unsigned src_row_num = next_row_offs + offs_y;

	  if (src_row_num >= aa_kernel_size)
	    src_row_num -= aa_kernel_size;

	  ImageRow *src_row = recent_rows[src_row_num];

	  for (unsigned offs_x = 0; offs_x < aa_kernel_size; offs_x++)
	    {
	      unsigned src_x = src_base_x + offs_x;

	      if (src_x > aa_overlap)
		{
		  src_x -= aa_overlap;

		  if (src_x < src_width)
		    aa_color += (*src_row)[src_x] * kernel_row[offs_x];
		}
	    }

	  kernel_row += aa_kernel_size;
	}

      (*aa_row)[x] = aa_color;
    }
}

// arch-tag: 3e9296c6-5ac7-4c39-8b79-45ce81b5d480
