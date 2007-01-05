// filter-conv.h -- "Filter Convolver" for convolving samples through a filter
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FILTER_CONV_H__
#define __FILTER_CONV_H__

#include "snogmath.h"
#include "val-table.h"
#include "color.h"
#include "filter.h"


namespace snogray {


// Base-class for FilterConv<> (see below); holds common fields for
// parametric subclasses.
//
class FilterConvBase
{
public:

  static const float DEFAULT_NEG_CLAMP = -0.1;
  static const float MAX_FILTER_WIDTH_SCALE = 2.f;

  FilterConvBase (const ValTable &params = ValTable::NONE)
    : filter (Filter::make (params)),
      filter_radius (filter ? int (ceil (filter->max_width() - 1.0001)) : 0),
      neg_clamp (-abs (params.get_float ("neg-clamp", DEFAULT_NEG_CLAMP)))
  { }

  // Anti-aliasing filter.
  //
  const Filter *filter;

  // "Radius" of FILTER.  This is an integer defining the number of
  // adjacent pixels on all sides of a pixel which are effected by output
  // samples within it.  It is calculated by expanding the maximum
  // filter width to a pixel boundary, and subtracting one to eliminate
  // the center pixel.
  //
  int filter_radius;		// really unsigned, but g++ goes nuts with
				// warnings if we actually use that type

  // A clamp for the minimum negative value of filtered sample points.
  // See the comment in FilterConv::add_sample for more details.
  //
  float neg_clamp;

};


// A "filter convolver": Holds a filter and some filter-related parameters;
// when the `FilterConv<>::add_sample' method is called, will convolve the
// sample through the filter and apply the resulting derived samples to a
// generic destination of type Dst.
//
// Dst should support the following methods:
//
//   // Add a sample with value COLOR at integer coordinates PX, PY.
//   // WEIGHT controls how much this sample counts relative to other samples
//   // added at the same coordinates.
//   //
//   void add_sample (int px, int py, const Color &color, float weight);
//
//   // Return true if the given X or Y coordinate is valid.
//   //
//   // [These methods are callbacks used by Filterconv<ImageOutput>.]
//   //
//   bool valid_x (int px) { return px >= 0 && px < int (width); }
//   bool valid_y (int py) { return py >= min_y && py < int (height); }
//
template<class Dst>
class FilterConv : public FilterConvBase
{
public:

  FilterConv (const ValTable &params) : FilterConvBase (params) { }

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
  void add_sample (float sx, float sy, const Color &color, Dst &dst)
  {
    // The center pixel affected
    //
    int x = int (sx), y = int (sy);

    if (filter)
      {
	// Add the light from COLOR to all pixels supported by the
	// output filter.
	//
	for (int fy = -filter_radius; fy <= filter_radius; fy++)
	  {
	    int py = y + fy; // y-coordinate of pixel within the filter's radius

	    // Check to make sure the coordinates are inside the physical
	    // output boundaries.
	    //
	    if (dst.valid_y (py))
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
		  if (dst.valid_x (px))
		    {
		      // Weight of the filter at this point
		      //
		      float w = filter->val (sx - (px + 0.5), sy - (py + 0.5));

		      // The sample weighted by the filter.
		      //
		      Color samp = color * w;

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
			  // scale the color and W as convropriate to bring it
			  // within range.
			  //
			  if (min_comp < neg_clamp)
			    {
			      float clamp_scale = neg_clamp / min_comp;
			      w *= clamp_scale;
			      samp *= clamp_scale;
			    }
			}

		      dst.add_sample (px, py, samp, w);
		    }
		}
	  }
      }
    else
      // There's no filter, so just add to the nearest pixel
      //
      dst.add_sample (x, y, color, 1);
  }
};


}

#endif // __FILTER_CONV_H__


// arch-tag: aeab9dc8-70b4-48d8-8331-0bc1a3d2d7df
