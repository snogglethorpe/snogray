// image-filter.cc -- Filters for image output
//
//  Copyright (C) 2006, 2007, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/excepts.h"
#include "util/val-table.h"
#include "image-mitchell-filt.h"
#include "image-gauss-filt.h"
#include "image-triangle-filt.h"
#include "image-box-filt.h"

#include "image-filter.h"


using namespace snogray;


// Return a new a filter depending on the parameters in PARAMS.
//
ImageFilter *
ImageFilter::make (const ValTable &params)
{
  std::string filter_type = params.get_string ("type", "mitchell");

  // Create the filter.
  //
  if (filter_type == "none")
    return 0;
  else if (filter_type == "mitchell")
    return new ImageMitchellFilt (params);
  else if (filter_type == "gauss")
    return new ImageGaussFilt (params);
  else if (filter_type == "triangle")
    return new ImageTriangleFilt (params);
  else if (filter_type == "box")
    return new ImageBoxFilt (params);
  else
    throw std::runtime_error (filter_type + ": unknown output filter type");
}


// arch-tag: b777ab5a-d4d2-44af-a23b-e8012cab289c
