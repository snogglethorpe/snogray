// image-cmdline.h -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_CMDLINE_H__
#define __IMAGE_CMDLINE_H__

#include "image.h"
#include "cmdlineparser.h"

namespace Snogray {

// This class can be used when parsing image parameters
struct ImageCmdlineSinkParams : ImageSinkParams
{
  ImageCmdlineSinkParams (CmdLineParser &_clp) : clp (_clp) { }

  // This is called when something wrong is detect with some parameter
  virtual void error (const char *msg) const;

  void parse_aa_filter_opt_arg ()
  {
    const char *filt_name = clp.opt_arg ();
    if (strcmp (filt_name, "box") == 0)
      aa_filter = ImageOutput::aa_box_filter;
    else if (strcmp (filt_name, "triang") == 0)
      aa_filter = ImageOutput::aa_triang_filter;
    else if (strcmp (filt_name, "gauss") == 0)
      aa_filter = ImageOutput::aa_gauss_filter;
    else
      clp.opt_err ("requires an anti-aliasing filter name"
		   " (box, triang, gauss)");
  }

  // Returns a name for the specified aa_filter
  const char *aa_filter_name()
  {
    float (*filt) (int offs, unsigned size)
      = aa_filter ? aa_filter : ImageOutput::DEFAULT_AA_FILTER;
    if (filt == ImageOutput::aa_box_filter)
      return "box";
    else if (filt == ImageOutput::aa_triang_filter)
      return "triang";
    else if (filt == ImageOutput::aa_gauss_filter)
      return "gauss";
    else
      return "???";
  }

  // We keep track of this so that we may format error messages nicely
  CmdLineParser &clp;
};

// This class can be used when parsing image parameters
struct ImageCmdlineSourceParams : ImageSourceParams
{
  ImageCmdlineSourceParams (CmdLineParser &_clp) : clp (_clp) { }

  // This is called when something wrong is detect with some parameter
  virtual void error (const char *msg) const;

  // We keep track of this so that we may format error messages nicely
  CmdLineParser &clp;
};

}

#endif /* __IMAGE_CMDLINE_H__ */

// arch-tag: d728801d-ce3a-414e-89a1-60b259197526
