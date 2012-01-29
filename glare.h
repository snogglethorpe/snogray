// glare.h -- Add glare effects ("bloom") to an image
//
//  Copyright (C) 2007, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLARE_H__
#define __GLARE_H__

#include "glare-psf.h"
#include "image.h"


namespace snogray {


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
extern void add_glare (const GlarePsf &glare_psf, Image &image,
		       float diag_field_of_view,
		       float threshold = 1, bool glare_only = false);


}


#endif // __GLARE_H__
