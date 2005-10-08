// image-aa.cc -- Image anti-aliasing
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

#include "image.h"

using namespace Snogray;


// Filter types

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
//float r = (float)(size + 1) / 2;
  float x = offs;
  return (M_SQRT2 * (1 / (2 * sqrt (M_PI))) * powf (M_E, -x*x / 2));
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

  for (unsigned x = 0, src_base_x = 0;
       x < aa_width;
       x++, src_base_x += aa_factor)
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
		    {
		      Color col = (*src_row)[src_x];
		      float filt_val = kernel_row[offs_x];

		      if (aa_max_intens == 0)
			aa_color += col * filt_val;
		      else
			// Make sure to do anti-aliasing using the clamped
			// value; otherwise the information added by
			// anti-aliasing is lost in subsequenet clamping
			aa_color += col.clamp (aa_max_intens) * filt_val;
		    }
		}
	    }

	  kernel_row += aa_kernel_size;
	}

      (*aa_row)[x] = aa_color;
    }
}

// arch-tag: 5a5c9b8b-7c13-416c-821b-1f72aa4eccb8
