// gaussian-filter.h -- Gaussian filter
//
//  Copyright (C) 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_GAUSSIAN_FILTER_H
#define SNOGRAY_GAUSSIAN_FILTER_H

#include "snogmath.h"


namespace snogray {


// A gaussian filter that operates on the square-root of its input
// values.  This class is useful for cases where the user would
// otherwise need to take a square-root.
//
template<typename InpT, typename ValT = InpT>
class GaussianSqrtFilter
{
public:

  // RADIUS_SQUARED is RADIUS², where RADIUS is the width the
  // filter on either side of the origin, beyond which its value
  // is zero.
  //
  // ALPHA is used to calculate α, where α is 1 / σ², and σ is
  // the standard deviation of the underlying gaussian function:
  // α = ALPHA / RADIUS².
  //
  GaussianSqrtFilter (InpT _radius_squared, InpT _alpha)
    : radius_squared (_radius_squared),
      alpha (_alpha / _radius_squared),
      gauss_edge_value (gauss_sqrt (_radius_squared)),
      filter_scale (1 / (1 - gauss_edge_value))
  { }

  // Return the value of the filter at an offset of
  // sqrt (OFFS_SQUARED) from the center.
  //
  ValT operator() (InpT offs_squared) const
  {
    if (offs_squared >= radius_squared)
      return 0;
    else
      return (gauss_sqrt (offs_squared) - gauss_edge_value) * filter_scale;
  }


private:

  // Return the value of our gaussian function at a distance of
  // sqrt (OFFS_SQUARED) from the center.  This is the "raw"
  // gaussian, before we've adjusted to compensate for the edge
  // value.
  //
  ValT gauss_sqrt (InpT offs_squared) const
  {
    return ValT (exp (-alpha * offs_squared));
  }

  // The square of the "radius" of the filter.  At offsets greater
  // than the radius, it will be exactly zero.
  //
  InpT radius_squared;

  // The value α for the gaussian filter, which is equal to 1 /
  // σ², where σ is the standard deviation.  This is calculated
  // so that it approaches zero at the radius (the gaussian
  // function itself is never zero, so we add an adjustment to make
  // it so).
  //
  InpT alpha;

  // Value of the gaussian filter at the radius.  We subtract this
  // from the calculated filter value to ensure that it exactly hits
  // zero at the radius.
  //
  ValT gauss_edge_value;

  // A scale factor to compensate for the fact that we subtract
  // FILTER_EDGE_VALUE from the filter.  A gaussian filter has a value
  // of 1 at the center, so this is 1 / (1 - GAUSS_EDGE_VALUE).
  //
  ValT filter_scale;
};


// A gaussian filter.
//
template<typename InpT, typename ValT = InpT>
class GaussianFilter
{
public:

  // RADIUS is the width the filter on either side of the origin,
  // beyond which its value is zero.  ALPHA is used to calculate α,
  // where α is 1 / σ², and σ is the standard deviation of the
  // underlying gaussian function:  α = ALPHA / RADIUS².
  //
  GaussianFilter (InpT radius, InpT alpha)
    : gauss_sqrt_filter (radius * radius, alpha)
  { }

  // Return the value of the filter at an offset of OFFS from the
  // center.
  //
  ValT operator() (InpT offs) const
  {
    return gauss_sqrt_filter (offs * offs);
  }

private:

  GaussianSqrtFilter<InpT, ValT> gauss_sqrt_filter;
};


}

#endif //  SNOGRAY_GAUSSIAN_FILTER_H
