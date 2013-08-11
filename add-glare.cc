// add-glare.cc -- Add glare effects ("bloom") to an image
//
//  Copyright (C) 2007, 2008, 2011-2013  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <fftw3.h>

#include "util/snogmath.h"
#include "util/num-cores.h"
#include "util/radical-inverse.h"

#include "add-glare.h"

using namespace snogray;


#if 0
// XXX testing

	// support = width of gaussian support (1-n/2)
	// alpha = sharpness (larger is sharper; 0.1 = pretty sharp,
	//         0.01 = blurry)
	// min = minimum value of gaussian (if non-zero, then when filtering,
	//         every point will receive at least that much contribution
	//         from every other point!)

float gauss1d (float offs, float rad, float a)
{
  return max (exp (-a*offs*offs) - exp(-a*rad*rad), 0.f);
}

float gauss (float x_offs, float y_offs, float rad, float a)
{
  return gauss1d (x_offs, rad, a) * gauss1d (y_offs, rad, a); 
}
#endif


// Bloom filter

#if 0
// bloom filter point-spread-function (PSF).  Theta is the angular
// deviation from the center, in radians.
//
static double
wacky_bloom_filter (double theta)
{
  double power = 0.2;
  return pow (10., -6 * pow (theta*1500, power));
}
#endif




// Add glare from the point-spread-function GLARE_PSF to IMAGE.
// DIAG_FIELD_OF_VIEW is the field-of-view, in radians, of the
// diagonal of IMAGE.  THRESHOLD is the maximum image intensity that
// can be represented by the target image format or system; glare will
// only be added for image values above that intensity level, on the
// assumption that any "glare" from lower intensities will be occur
// naturally during viewing.  If GLARE_ONLY is true, then IMAGE will
// be _replaced_ by the glare effect; if it is false, then the glare
// effect is added to IMAGE.
//
void
snogray::add_glare (const GlarePsf &glare_psf, Image &image,
		    float diag_field_of_view, float threshold, bool glare_only)
{
  // Size of base image.
  //
  unsigned w = image.width;
  unsigned h = image.height;

  float image_diagonal = sqrt (w*w + h*h);

  // Conversion from an offset in pixels to an offset in radians,
  // where the image diagonal corresponds to DIAG_FIELD_OF_VIEW
  // radians.
  //
  // Note that this isn't accurate at large angles (we should really
  // use atan instead), but for the particular function we're using,
  // it doesn't matter so much.
  //
  float pixel_offset_to_angle = diag_field_of_view / image_diagonal;

  // Because the FFT operator wraps around, we need to add a margin to
  // one vertical and one horizontal edge of the original image, which
  // is big enough to absorb any wrap-around bleeding.  The margin is
  // initially black so doesn't contribute anything to the result.
  //
  unsigned w_margin = w, h_margin = h;
  unsigned tot_w = w + w_margin; // total width, including margin
  unsigned tot_h = h + h_margin; // total height, including margin

  // The size of the arrays we allocate, which includes the margins.
  //
  unsigned size = tot_w * tot_h;

#if USE_FFTW3_THREADS
  // Initialize multi-threading; we try to use all cores.
  //
  fftwf_init_threads ();
  fftwf_plan_with_nthreads (num_cores (1));
#endif

  // Allocate memory for passing data to/from the FFT routines.
  //
  fftwf_complex *filter
    = static_cast<fftwf_complex*> (fftwf_malloc (sizeof(fftwf_complex) * size));
  fftwf_complex *data
    = static_cast<fftwf_complex*> (fftwf_malloc (sizeof(fftwf_complex) * size));

  // Make plan for executing filter FFT.
  //
  fftwf_plan filter_fft_plan
    = fftwf_plan_dft_2d (tot_h, tot_w, filter, filter,
			 FFTW_FORWARD, FFTW_ESTIMATE);

  // The sum of all filter values, for later scaling.
  //
  float filter_sum = 0;

  // Calculate in the top half of the real part of the filter matrix
  // (the bottom half is just a mirror copy, and the imaginary part
  // is all zero; both will be filled in below).
  //
  for (unsigned y = 0; y < (tot_h / 2) + 1; y++)
    {
      // Calculate the first half (of the real part) of this row.
      // The second half is just a mirror copy, and will be filled in
      // below.
      //
      for (unsigned x = 0; x < (tot_w / 2) + 1; x++)
	{
	  float pixel_sum = 0;

	  // We can take advantage of the x-y symmetry of our PSF by
	  // only actually calculating the portion above the image
	  // diagonal, and mirroring it about the digonal (swapping x
	  // and y) to fill in portion below the diagonal.
	  //
	  // However in the case where the image is taller than it is
	  // wide, the portion that's below the upper-left square part
	  // of the image has no corresponding area to mirror from, so
	  // we have to calculate it explicitly.
	  //
	  if (x >= y || y >= (tot_w + 1) / 2)
	    {
	      // We're above the diagonal, or below the square portion
	      // of the image, so we actually have to calculate the
	      // filter value.

	      // Angle, in radians, of the center of this pixel from
	      // the center of the filter.
	      //
	      float pix_angle
		= sqrt (float (x * x + y * y)) * pixel_offset_to_angle;

	      // Number of samples to take for this pixel.
	      //
	      // Because of the nature of the bloom filter, which has
	      // an _extremely_ sharp peak at the origin, we take lots
	      // of samples near the origin, and many fewer for
	      // distant pixels.
	      //
	      unsigned num_samples;
	      if (pix_angle < 0.0175f) // 1deg
		num_samples = 10000;
	      else if (pix_angle < 0.0524f) // 3deg
		num_samples = 1000;
	      else
		num_samples = 1;

	      float inv_num_samples = 1 / float (num_samples);

	      // Sample the bloom filter function over this pixel.
	      //
	      for (unsigned samp = 0; samp < num_samples; samp++)
		{
		  // x/y offsets of this sample within the pixel.
		  //
		  float samp_x_offs = radical_inverse (samp + 1, 2);
		  float samp_y_offs = radical_inverse (samp + 1, 3);

		  // x/y offsets of this sample from the filter centre.
		  //
		  float x_offs = x + samp_x_offs - 0.5f;
		  float y_offs = y + samp_y_offs - 0.5f;

		  // Angular deviation from the image center in radians.
		  //
		  float theta = (sqrt (x_offs*x_offs + y_offs*y_offs)
				 * pixel_offset_to_angle);

		  float val = glare_psf (theta);

		  pixel_sum += val;
		}

	      pixel_sum *= inv_num_samples;
	    }
	  else
	    {
	      // We're below the diagonal (and in the initial square
	      // portion of the image, if the image is taller than it
	      // is wide).  Take advantage of x-y symmetry by copying
	      // the previously calculated value with x and y swapped.

	      pixel_sum = filter[y + x * tot_w][0];
	    }

	  filter[x + y * tot_w][0] = pixel_sum;

	  filter_sum += pixel_sum;
	}

      // As the matrix should be symmetrical, fill in the right half
      // of the row as a mirror copy of the left half.
      //
      for (unsigned x = (tot_w / 2) + 1; x < tot_w; x++)
	{
	  float val = filter[(tot_w - x) + y * tot_w][0];
	  filter[x + y * tot_w][0] = val;
	  filter_sum += val;
	}
    }

  // Fill in the bottom half (of the real part) of the filter matrix
  // as a mirror copy of the top half.
  //
  for (unsigned y = (tot_h / 2) + 1; y < tot_h; y++)
    for (unsigned x = 0; x < tot_w; x++)
      {
	float val = filter[x + (tot_h - y) * tot_w][0];
	filter[x + y * tot_w][0] = val;
	filter_sum += val;
      }

  // Fill in the imaginary portion of the filter matrix, which is
  // initially all zero.
  //
  for (unsigned y = 0; y < tot_h; y++)
    for (unsigned x = 0; x < tot_w; x++)
      filter[x + y * tot_w][1] = 0;

  // Normalize the filter.
  //
  float filter_scale = 1 / filter_sum;
  for (unsigned k = 0; k < size; k++)
    filter[k][0] *= filter_scale;

  // Do the filter FFT.  We calculate the FFT in-place in the same
  // array.
  //
  fftwf_execute (filter_fft_plan);
  
  // Make plan for executing forward FFTs and reverse-FFTs on DATA.
  //
  fftwf_plan data_fwd_fft_plan
    = fftwf_plan_dft_2d (tot_h, tot_w, data, data, FFTW_FORWARD, FFTW_MEASURE);
  fftwf_plan data_rev_fft_plan
    = fftwf_plan_dft_2d (tot_h, tot_w, data, data, FFTW_BACKWARD, FFTW_MEASURE);

  // Loop over the color components, convolving each plane with the filter.
  //
  for (unsigned cc = 0; cc < Color::NUM_COMPONENTS; cc++)
    {
      // Copy the data for this color plane into the DATA array, where we
      // will calculate the FFT in-place.
      //
      for (unsigned y = 0; y < h; y++)
	for (unsigned x = 0; x < w; x++)
	  {
	    float val = image.tuple (x, y)[cc];

	    // XXX clamp input to avoid inf or nan values from mucking up
	    // the result...
	    //
	    if (std::isinf (val))
	      val = 100;
	    else if (std::isnan (val))
	      val = 0;

	    // Because we're only calculate "additional" glare, that
	    // wouldn't naturally come from viewing the original
	    // image, subtract a version of the image clamped to the
	    // eventual output maximum-intensity from what we're using
	    // to compute glare, leaving only the "excess" values.
	    //
	    val = max (0.f, val - threshold);

	    data[x + y * tot_w][0] = val;
	    data[x + y * tot_w][1] = 0;
	  }

      // Clear right margin of image data.
      //
      for(unsigned y = 0; y < tot_h; y++)
	for(unsigned xo = 0; xo < w_margin; xo++)
	  data[w + xo + y * tot_w][1] = data[w + xo + y * tot_w][0] = 0;

      // Clear bottom margin of image data.
      //
      for(unsigned yo = 0; yo < h_margin; yo++)
	for(unsigned x = 0; x < w; x++)
	  data[x + (h + yo) * tot_w][1] = data[x + (h + yo) * tot_w][0] = 0;

      // Calculate the FFT of DATA in-place.
      //
      fftwf_execute (data_fwd_fft_plan);

      // Multiply DATA by FILTER.  Multiplying in frequency space is
      // equivalent to convolution in the normal space.
      //
      for (unsigned offs = 0; offs < size; offs++)
	{
	  float dre = data[offs][0],   dim = data[offs][1];
	  float fre = filter[offs][0], fim = filter[offs][1];

	  data[offs][0] = dre * fre - dim * fim; // real
	  data[offs][1] = dre * fim + dim * fre; // imag
 	}

      // Calculate the reverse-FFT of DATA in-place.
      //
      fftwf_execute (data_rev_fft_plan);

      // Put DATA back into the image.
      //
      float data_scale = 1 / float (size);
      for (unsigned y = 0; y < h; y++)
	for (unsigned x = 0; x < w; x++)
	  {
	    float val = data[x + y * tot_w][0] * data_scale;

	    // Our glare PSF includes the source image, but to improve
	    // the result, we normally we only calculate glare on
	    // parts of the image in excess of THRESHOLD.  So unless
	    // we're in "glare only" mode, add anything we _didn't_
	    // use as input to the glare calculation to the glare
	    // result, to get the final result.
	    //
	    if (! glare_only)
	      val += min (image.tuple (x, y)[cc], threshold);

	    image.tuple (x, y)[cc] = val;
	  }
    }

  // Clean up

  fftwf_destroy_plan (data_fwd_fft_plan);
  fftwf_destroy_plan (data_rev_fft_plan);
  fftwf_destroy_plan (filter_fft_plan);

  fftwf_free (filter);
  fftwf_free (data);

#if USE_FFTW3_THREADS
  fftwf_cleanup_threads ();
#endif
}
