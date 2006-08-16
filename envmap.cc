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
#include "string-funs.h"

#include "envmap.h"


using namespace Snogray;
using namespace std;


// Return an appropriate subclass of Envmap, initialized from SPEC
// (usually a filename to load).  FMT is the type of environment-map.
//
// If FMT is "", any colon-separated prefix will be removed from SPEC,
// and used as the format name (and ther remainder of SPEC used as the
// actual filename); if FMT is "auto", SPEC will be left untouched, and
// an attempt will be made to guess the format based on the image size.
//
Envmap *
Snogray::load_envmap (const string &spec, const string &_fmt)
{
  string filename = spec;
  string fmt = _fmt;

  if (fmt.empty ())
    fmt = strip_prefix (filename, ":");

  if (ImageIo::recognized_filename (filename))
    //
    // Load from a single image file
    {
      Ref<Image> image = new Image (filename);

      try
	{
	  return make_envmap (image, fmt);
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



// Return an appropriate subclass of Envmap, initialized from IMAGE.
// FMT is the type of environment-map (specifically, the type of mapping
// from direction to image coordinates).  If FMT is "" or "auto", an
// attempt will be made to guess the format based on the image size.
//
Envmap *
Snogray::make_envmap (const Ref<Image> &image, const string &_fmt)
{
  string fmt = _fmt;

  if (fmt.empty() || fmt == "auto")
    //
    // Try to guess the proper fmt
    {
      unsigned size;
      unsigned w = image->width, h = image->height;

      if (((size = w / 3) * 3 == w && size * 4 == h)
	  || ((size = w / 4) * 4 == w && size * 3 == h))
	fmt = "cube";     // 4x3 or 3x4 aspect ratio: "cross" cubemap
      else if (w == h)
	fmt = "debevec";	// Debevec angular mapping
      else if (w == h * 2)
	fmt = "latlong";	// Latitude-longitude ("panorama") mapping
      else
	throw bad_format ("Unrecognized environment-map image size");
    }

  fmt = downcase (fmt);
  fmt = strip (fmt, " \t-_");

  // Remove "map" at the end.
  //
  if (ends_in (fmt, "map"))
    fmt.erase (fmt.length() - 3);

  if (fmt == "d" || fmt == "debevec" || fmt == "angular")
    return new Spheremap<DebevecMapping> (image);

  else if (fmt == "m" || fmt == "mball" || fmt == "mirror"
	   || fmt == "mirrorball")
     return new Spheremap<MirrorBallMapping> (image);

  else if (fmt == "l" || fmt == "ll" || fmt == "latlong"
	   || fmt == "panorama" || fmt == "latitudelongitude")
    return new Spheremap<LatLongMapping> (image);

#if 0
  else if (fmt == "mercator" || fmt == "cylinder" || fmt == "cylindrical")
    return new Spheremap<MercatorMapping> (image);
#endif

  else if (fmt == "c" || fmt == "cube")
    return new Cubemap (image);

  else
    throw bad_format ("Unrecognized environment-map format \"" + fmt + "\"");
}


// arch-tag: 7b474cbf-edf1-47de-a29a-24ee442a0b57
