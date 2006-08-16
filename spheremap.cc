// spheremap.cc -- Texture wrapped around a sphere
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#include "spheremap.h"


using namespace Snogray;


static unsigned
rmap_size_divisor (const Ref<Image> &img)
{
  unsigned w = img->width, h = img->height;
  unsigned div = 1;

  while (w / div > 1024 || h / div> 1024)
    div++;

  return div;
}

template<>
Ref<Image>
Spheremap<LatLongMapping>::radiance_map () const
{
  Ref<Image> emap = tex.matrix;

  unsigned rmap_block_size = rmap_size_divisor (emap);

  Ref<Image> rmap = new Image (emap->width / rmap_block_size,
			       emap->height / rmap_block_size);

  // We scale the rmap pixels by the sphere-area factor, and an area
  // factor, to make radiance correct.  Basically if every pixel in EMAP
  // has a value of 1, we want the sum of all pixels in RMAP to be 4 * PI
  // (the surface area of a sphere).
  //
  // TOTAL_PIXEL_SPHERE_AREA is the sum of all the per-pixel scale factors;
  // we can multiply each pixel by 4 * PI and divide by
  // TOTAL_PIXEL_SPHERE_AREA to get the final correct radiance value.
  //
  float total_pixel_sphere_area = 0;

  unsigned w = rmap->width, h = rmap->height;

  // Initialize each pixel in the light map as the sum of a block of pixels
  // in EMAP, scaled by factor to correct for the differing sphere area of
  // each pixel.
  //
  for (MatrixTex2<Color>::const_iterator p = tex.begin ();
       p != tex.end (); ++p)
    {
      float pixel_sphere_area = mapping.sphere_area (p.uv ());
      total_pixel_sphere_area += pixel_sphere_area;

      unsigned x = p.x () / rmap_block_size;
      unsigned y = h - p.y () / rmap_block_size - 1;

      /// XXX some textures have a size just _slightly_ more than a power
      /// of two, which screws up our simple assumptions; in such a case,
      /// just ignore a thin edge of the input texture.
      //
      if (x >= w || y >= h)
	continue;

      rmap->put (x, y, rmap->get (x, y) + p.val () * pixel_sphere_area);
    }

  // Now scale each pixel by 1 / TOTAL_PIXEL_SPHERE_AREA to make the total
  // sum correct.
  //
  float scale = (4 * M_PIf) / total_pixel_sphere_area;

  for (unsigned y = 0; y < rmap->height; y++)
    for (unsigned x = 0; x < rmap->width; x++)
      rmap->put (x, y, rmap->get (x, y) * scale);

  return rmap;
}


// arch-tag: e4ffff07-7a1d-42a2-9195-be4e49bee1f4
