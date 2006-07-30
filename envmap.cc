// envmap.cc -- Environment maps
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <string>
#include <fstream>

#include "excepts.h"
#include "image-io.h"
#include "cubemap.h"
#include "spheremap.h"

#include "envmap.h"


using namespace Snogray;
using namespace std;


Envmap *
Snogray::load_envmap (const string &filename)
{
  if (ImageIo::recognized_filename (filename))
    //
    // Load from a single image file
    {
      Image image (filename);

      try
	{
	  return make_envmap (image);
	}
      catch (runtime_error &err)
	{
	  throw file_error (filename + ": Error loading environment-map: "
			    + err.what ());
	}
    }
  else
    throw runtime_error (filename + ": Unrecognized environment-map file type");
}

Envmap *
Snogray::make_envmap (const Image &image)
{
  unsigned size;
  unsigned w = image.width, h = image.height;

  if (((size = w / 3) * 3 == w && size * 4 == h)
      || ((size = w / 4) * 4 == w && size * 3 == h))
    //
    // 4x3 or 3x4 aspect ratio: "cross" format cubemap
    //
    return new Cubemap (image);

  else if (w == h)
    return new Spheremap<DebevecMapping> (image);
//     return new Spheremap<MirrorBallMapping> (image);

  else if (w == h * 2)
    return new Spheremap<LatLongMapping> (image);

  else
    throw bad_format ("Unrecognized environment-map image size");
}


// arch-tag: 7b474cbf-edf1-47de-a29a-24ee442a0b57
