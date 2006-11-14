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
lmap_size_divisor (const Ref<Image> &img)
{
  unsigned w = img->width, h = img->height;
  unsigned div = 1;

  while (w / div > 1024 || h / div> 1024)
    div++;

  return div;
}

template<>
Ref<Image>
Spheremap<LatLongMapping>::light_map () const
{
  Ref<Image> emap = tex.matrix;

  unsigned lmap_block_size = lmap_size_divisor (emap);

  Ref<Image> lmap = new Image (emap->width / lmap_block_size,
			       emap->height / lmap_block_size);

  unsigned w = lmap->width, h = lmap->height;

  float avg_scale = 1.f / (lmap_block_size * lmap_block_size);

  for (MatrixTex2<Color>::const_iterator p = tex.begin ();
       p != tex.end (); ++p)
    {
      unsigned x = p.x () / lmap_block_size;
      unsigned y = h - p.y () / lmap_block_size - 1;

      /// XXX some textures have a size just _slightly_ more than a power
      /// of two, which screws up our simple assumptions; in such a case,
      /// just ignore a thin edge of the input texture.
      //
      if (x >= w || y >= h)
	continue;

      lmap->put (x, y, lmap->get (x, y) + p.val () * avg_scale);
    }

  return lmap;
}


// arch-tag: e4ffff07-7a1d-42a2-9195-be4e49bee1f4
