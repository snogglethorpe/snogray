// render-packet.h -- Container for pixels to be rendered and the results
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDER_PACKET_H__
#define __RENDER_PACKET_H__

#include "tint.h"


namespace snogray {


// A list of pixels to be rendered (in the output image space), and the
// results of rendering them.
//
class RenderPacket
{
public:

  // The result of rendering a single sample inside a pixel.
  //
  struct Result
  {
    Result (const UV &_coords, const Tint &_val)
      : coords (_coords), val (_val)
    { }

    UV coords;		// includes offset within the pixel
    Tint val;		// Result of rendering
  };

  // Coordinates of pixels to be rendered.
  //
  std::vector<UV> pixels;

  // Render results.  Multiple samples are rendered within each pixel,
  // so there are usually many more output results than input pixels.
  //
  std::vector<Result> results;
};


}

#endif // __RENDER_PACKET_H__
