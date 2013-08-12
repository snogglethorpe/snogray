// recover-image.h -- Recovery of partial output images
//
//  Copyright (C) 2005-2007, 2011-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RECOVER_H
#define SNOGRAY_RECOVER_H


#include <string>

#include "util/val-table.h"
#include "image/image-sampled-output.h"


namespace snogray {


// The number of rows at the end of a recovered image file that we throw
// away, to guard against garbage.
//
#define RECOVER_DISCARD_ROWS 4

// Initialize the output by opening the image file
// PARTIAL_IMAGE_FILENAME, using OUTPUT_FILENAME and OUTPUT_PARAMS to
// determine its format (because the PARTIAL_IMAGE_FILENAME may not be
// a normal image filename), and copying as many image rows as
// possible to DST; if a read-error is encountered, a small number of
// the last rows read are discarded, to guard against garbaged final
// lines.  The number of rows recovered is returned.
//
extern unsigned recover_image (const std::string &partial_image_filename,
			       const std::string &output_filename,
			       const ValTable &output_params,
			       ImageSampledOutput &dst);


}

#endif // SNOGRAY_RECOVER_H


// arch-tag: 84ae5ac2-df53-4f17-9a42-08d9d0fb51db
