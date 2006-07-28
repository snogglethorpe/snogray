// image-output.cc -- High-level image output
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <string>

#include "snogmath.h"
#include "excepts.h"
#include "filter.h"
#include "mitchell.h"
#include "gauss.h"
#include "box.h"

#include "image-output.h"


using namespace Snogray;



ImageOutput::ImageOutput (const std::string &filename,
			  unsigned _width, unsigned _height,
			  const Params &params)
  : width (_width), height (_height),
    filter (make_filter (params)),
    filter_radius (filter ? int (ceil (filter->max_width() - 1.0001f)) : 0),
    min_y (0),
    exposure (params.get_float ("exposure", 0)),
    neg_clamp (-abs (params.get_float ("neg-clamp", DEFAULT_NEG_CLAMP))),
    sink (ImageSink::open (filename, _width, _height, params)),
    num_buffered_rows (filter_radius * 2 + 1), num_user_buffered_rows (0),
    rows (num_buffered_rows), buf_y (0),
    intensity_scale (exposure == 0 ? 1.f : pow (2.f, exposure)),
    max_intens (sink->max_intens ())
{
  for (unsigned i = 0; i < rows.size (); i++)
    rows[i].resize (width);
}

// Write the the lowest currently buffered row to the output sink, and
// recycle its storage for use by another row.  BUF_Y is incremented
// to reflect the new lowest buffered row.
//
void
ImageOutput::flush_min_row ()
{
  SampleRow &r = rows[buf_y % num_buffered_rows];

  for (unsigned x = 0; x < width; x++)
    {
      float weight = r.weights[x];
      if (weight > 0)
	r.pixels[x] /= weight;
    }

  sink->write_row (r.pixels);

  r.clear ();

  buf_y++;
}

void
ImageOutput::set_min_y (int new_min_y)
{
  while (buf_y < new_min_y)
    flush_min_row ();
  min_y = new_min_y;
}

ImageOutput::~ImageOutput ()
{
  // Write as-yet unwritten rows
  //
  set_min_y (height);
  flush ();
}


// Low-level row handling

// Make sure at least NUM rows are buffered in memory before being
// written.  NUM is a minimum -- more rows may be buffered if necessary
// to support the output filter, or for other internal reasons.
//
void
ImageOutput::set_num_buffered_rows (unsigned num)
{
  // Growing
  //
  while (num > num_user_buffered_rows)
    {
      rows.push_back (SampleRow (width));
      num_user_buffered_rows++;
      num_buffered_rows++;
    }

  // We ignore requests to shrink the number of buffered rows
}

// Returns a row at absolute position Y.  Rows cannot be addressed
// completely randomly, as only NUM_BUFFERED_ROWS rows are buffered in
// memory; if a row which has already been output is specified, an error
// is signaled.
//
ImageOutput::SampleRow &
ImageOutput::_row (int y)
{
  if (y < buf_y)
    throw std::runtime_error ("Previously output image row addressed");

  while (int (buf_y + num_buffered_rows) <= y)
    flush_min_row ();

  return rows[y % num_buffered_rows];
}


// High-level sample-based interface

// Add a sample with value COLOR at floating point position SX, SY.
// COLOR's contribution to adjacent pixels is determined by the
// anti-aliasing filter in effect; if there is none, then it is
// basically just added to the nearest pixel.
#if 1
// The center of a pixel is located at an (0.5, 0.5) offset from its
// integer coordinates.
#else
//
// SX and SY implicitly have 0.5 added to them (so passing in values
// of SX = 2.0, SY = 3.0 actually refers to the _center_ of the pixel
// at integer coordinates (2, 3), located at (2.5, 3.5)), which allows
// convenient inter-operation between code using integer and floating
// point coordinates.
#endif
//
void
ImageOutput::add_sample (float sx, float sy, const Color &color)
{
  // The center pixel affected
  //
  int x = int (sx), y = int (sy);

  Color col = color;

  if (exposure != 0)
    col *= intensity_scale;
  if (max_intens != 0)
    col = col.clamp (max_intens);

  if (filter)
    {
      // Add the light from COLOR to all pixels supported by the
      // output filter.
      //
      for (int fy = -filter_radius; fy <= filter_radius; fy++)
	{
	  int py = y + fy; // y-coordinate of a pixel within the filter's radius

	  // Check to make sure the coordinates are inside the physical
	  // output boundaries.
	  //
	  if (py >= min_y && py < int (height))
	    {
	      SampleRow &r = row (py); // One row of output image at PY

	      for (int fx = -filter_radius; fx <= filter_radius; fx++)
		{
		  int px = x + fx; // x-coordinate of the pixel

		  // If PX, PY is a valid coordinate in the output (it may not
		  // be, where the filter's support area goes past the edge of
		  // the output image), find the filter magnitude of position
		  // SX, SY using PX, PY as the filter's center position.
		  // This is the amount by which we scale COLOR's (which was
		  // calculated at SX, SY) contribution to the pixel PX, PY.
		  //
		  if (px >= 0 && px < int (width))
		    {
		      // Weight of the filter at this point
		      //
		      float w = filter->val (sx - (px + 0.5), sy - (py + 0.5));

		      // The sample weighted by the filter.
		      //
		      Color samp = col * w;

		      // Give negative filter values some special handling
		      //
		      if (w < 0)
			{
			  // Some filters have regions of negative value to
			  // provide edge-enhancement.  However this can cause
			  // annoying black fringes on very bright lights.  To
			  // avoid this, we clamp negative values; for
			  // ordinary colors this will have no effect, but it
			  // should reduce the dark fringing (it may cause
			  // some "blooming" of very bright regions, but
			  // that's a much more pleasing effect).
			  //
			  float min_comp = samp.min_component ();

			  // If the minimum color component (which should be
			  // negative, because W is) is less that NEG_CLAMP,
			  // scale the color and W as appropriate to bring it
			  // within range.
			  //
			  if (min_comp < neg_clamp)
			    {
			      float clamp_scale = neg_clamp / min_comp;
			      w *= clamp_scale;
			      samp *= clamp_scale;
			    }
			}

		      r.pixels[px] += samp;
		      r.weights[px] += w;
		    }
		}
	    }
	}
    }
  else
    // There's no filter, so just add to the nearest pixel
    //
    {
      SampleRow &r = row (y);

      r.pixels[x] += col;
      r.weights[x] += 1;
    }
}



// Return an appropriate filter.
//
Filter *
ImageOutput::make_filter (const Params &params)
{
  std::string filter_type = params.get_string ("filter");

  if (filter_type.empty ())
    return new Mitchell ();
  else
    {
      Params filter_params;

      unsigned type_end = filter_type.find_first_of ("/");
      if (type_end < filter_type.length ())
	{
	  unsigned params_start
	    = filter_type.find_first_not_of ("/ \t", type_end + 1);

	  filter_params.parse (filter_type.substr (params_start));

	  filter_type = filter_type.substr (0, type_end);
	}

      if (filter_type == "none")
	return 0;
      else if (filter_type == "mitchell")
	return new Mitchell (filter_params);
      else if (filter_type == "gauss")
	return new Gauss (filter_params);
      else if (filter_type == "box")
	return new Box (filter_params);
      else
	throw std::runtime_error
	  (filter_type + ": unknown output filter type");
    }
}


// arch-tag: b4e1bbd7-c070-4ac9-9075-b9abcaefc30a
