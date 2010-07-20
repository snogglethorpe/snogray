// render-pattern.h -- Generator for pixel coordinates to be rendered
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

#ifndef __RENDER_PATTERN_H__
#define __RENDER_PATTERN_H__

#include "uv.h"
#include "tint.h"


namespace snogray {


// A generator object, which yields pixel coordinates to be rendered.
//
// Currently this is hardwired to just scan from the upper-left in
// scanline order.
//
class RenderPattern
{
public:

  // An iterator object for doing the actual iterating.
  //
  class iterator
  {
  public:

    iterator (int _x, int _y, const RenderPattern &_pat)
      : x (_x), y (_y), pat (_pat)
    { }

    iterator (const iterator &it)
      : x (it.x), y (it.y), pat (it.pat)
    { }

    bool operator== (const iterator &it) const
    {
      return x == it.x && y == it.y;
    }
    bool operator!= (const iterator &it) const
    {
      return ! operator== (it);
    }

    UV operator* () const { return UV (x, y); }

    iterator &operator++ ()
    {
      if (++x == pat.x_end)
	{
	  x = pat.x_beg;
	  ++y;
	}
      return *this;
    }
    iterator operator++ (int)
    {
      iterator result = *this;
      if (++x == pat.x_end)
	{
	  x = pat.x_beg;
	  ++y;
	}
      return result;
    }

    int min_y () const { return y; }

    unsigned position () const
    {
      unsigned per_row = (pat.x_end - pat.x_beg);
      return per_row * (y - pat.y_beg) + (x - pat.x_beg);
    }

  private:

    int x, y;

    const RenderPattern &pat;
  };

  RenderPattern (int _left_x, int _top_y, int _width, int _height)
    : x_beg (_left_x), y_beg (_top_y),
      x_end (_left_x + _width), y_end (_top_y + _height)
  { }

  iterator begin () const { return iterator (x_beg, y_beg, *this); }
  iterator end () const { return iterator (x_beg, y_end, *this); }

  // Return the minimum y-value will ever be returned from the iterator
  // PAT_IT in the future.
  //
  int min_y (const iterator &pat_it) const { return pat_it.min_y (); }

  // Returns an unsigned value indicating the linear "position" of
  // PAT_IT; the position of an iterator is is incremented by one each
  // time the iterator is incremented.  The maximum and minimum
  // positions for this pattern can be calculated by calling
  // RenderPattern::position using the iterators returned by
  // RenderPattern::end() and RenderPattern::begin().
  //
  unsigned position (const iterator &pat_it) const
  {
    return pat_it.position ();
  }

private:

  int x_beg, y_beg, x_end, y_end;
};


}

#endif // __RENDER_PATTERN_H__
