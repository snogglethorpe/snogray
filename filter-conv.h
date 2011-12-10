// filter-conv.h -- "Filter Convolver" for convolving samples through a filter
//
//  Copyright (C) 2005-2008, 2010-2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_FILTER_CONV_H
#define SNOGRAY_FILTER_CONV_H

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

  // These should be simple named constants, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  static float default_neg_clamp () { return -0.1; }
  static float max_filter_width_scale () { return 2.f; }

  FilterConvBase (const ValTable &params = ValTable::NONE)
    : filter (Filter::make (params)),
      filter_x_radius (filter ? int (ceil (filter->x_width - 1.0001)) : 0),
      filter_y_radius (filter ? int (ceil (filter->y_width - 1.0001)) : 0),
      neg_clamp (-abs (params.get_float ("neg-clamp", default_neg_clamp ())))
  { }
  ~FilterConvBase () { delete filter; }

  // Anti-aliasing filter.
  //
  const Filter *filter;

  // "Radius" of FILTER, for x- and y-dimensions.  This is an integer
  // defining the number of adjacent pixels on all sides of a pixel
  // which are effected by output samples within it.  It is calculated
  // by expanding the filter width in the given dimension to a pixel
  // boundary, and subtracting one to eliminate the center pixel.
  //
  // (this value is really unsigned, but g++ goes nuts with warnings
  // if we actually use that type)
  //
  int filter_x_radius, filter_y_radius;

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
//   // Add a sample with value SAMP at integer coordinates PX, PY.
//   // WEIGHT controls how much this sample counts relative to other samples
//   // added at the same coordinates.
//   //
//   void add_sample (int px, int py, const Samp &samp, float weight);
//
//   // Return true if the given X or Y coordinate is valid.
//   //
//   // [These methods are callbacks used by Filterconv<ImageOutput>.]
//   //
//   bool valid_x (int px) { return px >= 0 && px < int (width); }
//   bool valid_y (int py) { return py >= min_y && py < int (height); }
//
template<class Dst, typename Samp>
class FilterConv : public FilterConvBase
{
public:

  FilterConv (const ValTable &params) : FilterConvBase (params) { }

  // Add a sample with value SAMP at floating point position SX, SY.
  // SAMP's contribution to adjacent pixels is determined by the
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
  void add_sample (float sx, float sy, const Samp &samp, Dst &dst)
  {
    // The center pixel affected
    //
    int x = int (sx), y = int (sy);

    if (filter)
      {
	// Add the light from SAMP to all pixels supported by the
	// output filter.
	//
	for (int fy = -filter_y_radius; fy <= filter_y_radius; fy++)
	  {
	    int py = y + fy; // y-coordinate of pixel within the filter's radius

	    // Check to make sure the coordinates are inside the physical
	    // output boundaries.
	    //
	    if (dst.valid_y (py))
	      for (int fx = -filter_x_radius; fx <= filter_x_radius; fx++)
		{
		  int px = x + fx; // x-coordinate of the pixel

		  // If PX, PY is a valid coordinate in the output (it may not
		  // be, where the filter's support area goes past the edge of
		  // the output image), find the filter magnitude of position
		  // SX, SY using PX, PY as the filter's center position.
		  // This is the amount by which we scale SAMP's (which was
		  // calculated at SX, SY) contribution to the pixel PX, PY.
		  //
		  if (dst.valid_x (px))
		    {
		      // Weight of the filter at this point
		      //
		      float w = filter->val (sx - (px + 0.5), sy - (py + 0.5));

		      // The sample weighted by the filter.
		      //
		      Samp weighted_samp = samp * w;

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
			  float min_comp = weighted_samp.min_component ();

			  // If the minimum color component (which should be
			  // negative, because W is) is less that NEG_CLAMP,
			  // scale the color and W as convropriate to bring it
			  // within range.
			  //
			  if (min_comp < neg_clamp)
			    {
			      float clamp_scale = neg_clamp / min_comp;
			      w *= clamp_scale;
			      weighted_samp *= clamp_scale;
			    }
			}

		      dst.add_sample (px, py, weighted_samp, w);
		    }
		}
	  }
      }
    else
      // There's no filter, so just add to the nearest pixel
      //
      dst.add_sample (x, y, samp, 1);
  }
};


}

#endif // SNOGRAY_FILTER_CONV_H


// arch-tag: aeab9dc8-70b4-48d8-8331-0bc1a3d2d7df
