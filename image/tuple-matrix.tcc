// tuple-matrix.tcc -- Generic matrix storage type
//
//  Copyright (C) 2005-2008, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TUPLE_MATRIX_TCC__
#define __TUPLE_MATRIX_TCC__

#include <iostream>
#include <algorithm>

#include "util/globals.h"
#include "util/excepts.h"

#include "image-input.h"
#include "image-scaled-output.h"

#include "tuple-matrix.h"


namespace snogray {


template<typename DT>
TupleMatrixData<DT>::TupleMatrixData (unsigned _tuple_len,
				      const std::string &filename,
				      const ValTable &params)
  : tuple_len (_tuple_len), width (0), height (0)
{
  load (filename, params);
}

template<typename DT>
TupleMatrixData<DT>::TupleMatrixData (unsigned _tuple_len,
				      ImageInput &src,
				      const ValTable &params)
  : tuple_len (_tuple_len), width (0), height (0)
{
  load (src, params);
}



// ----------------------------------------------------------------
// Color / tuple translation


// Return a color from the tuple at location X, Y; if the tuple length
// is not the same as a Color, only Color::TUPLE_LEN members are returned,
// and any missing components set to zero.
//
template<typename DT>
Color
TupleMatrixData<DT>::pixel (unsigned x, unsigned y) const
{
  Color col;

  const DT *t = tuple (x, y);

  unsigned copy_limit = Color::NUM_COMPONENTS;
  if (tuple_len < Color::NUM_COMPONENTS)
    copy_limit = tuple_len;

  unsigned i;
  for (i = 0; i < copy_limit; i++)
    col[i] = t[i];
  for (; i < Color::NUM_COMPONENTS; i++)
    col[i] = 0;

  return col;
}

// Set the tuple at location X, Y from the color COL; if the tuple length
// is not the same as a Color, only the first TUPLE_LEN members are copied,
// and any missing components set to zero.
//
template<typename DT>
void
TupleMatrixData<DT>::set_pixel (unsigned x, unsigned y, const Color &col)
{
  DT *t = tuple (x, y);

  unsigned copy_limit = Color::NUM_COMPONENTS;
  if (tuple_len < Color::NUM_COMPONENTS)
    copy_limit = tuple_len;

  unsigned i;
  for (i = 0; i < copy_limit; i++)
    t[i] = col[i];
  for (; i < tuple_len; i++)
    t[i] = 0;
}



// ----------------------------------------------------------------
// Input loading


template<typename DT>
void
TupleMatrixData<DT>::load (const std::string &filename, const ValTable &params)
{
  ImageInput src (filename, params);

  // Loading a very large image can be slow (largely due to thrashing -- a
  // 6K x 3K image requires 216 MB of memory unpacked!), so tell the user
  // what we're doing.
  //
  bool emit_size_note = (!quiet && src.width * src.height > 1024 * 1024);
  if (emit_size_note)
    {
      std::string bn = filename;
      std::string::size_type last_slash = bn.find_last_of ("/");

      if (last_slash != std::string::npos)
	bn.erase (0, last_slash + 1);

      std::cout << "* loading large image: " << bn
		<< " (" << src.width << " x " << src.height << ", "
		<< (src.width * src.height * tuple_len * sizeof (DT)
		    / (1024 * 1024))
		<< " MB" << ")...";
      std::cout.flush ();
    }

  load (src, params);

  if (emit_size_note)
    {
      std::cout << "done" << std::endl;
      std::cout.flush ();
    }
}

template<typename DT>
void
TupleMatrixData<DT>::load (ImageInput &src, const ValTable &params)
{
  unsigned border = params.get_uint ("border", 0);
  bool reverse_rows = params.get_bool ("reverse_rows", false);

  const_cast<unsigned &> (width) = src.width + border * 2;
  const_cast<unsigned &> (height) = src.height + border * 2;

  data.resize (tuple_len * width * height);

  ImageRow row (src.width);

  ImageIo::RowIndices row_indices = src.row_indices ();
  if (reverse_rows)
    std::swap (row_indices.first, row_indices.last);

  for (ImageIo::RowIndices::iterator i = row_indices.begin ();
       i != row_indices.end (); ++i)
    {
      int y = *i;

      src.read_row (row);

      for (unsigned x = 0; x < src.width; x++)
	set_pixel (x + border, y + border, row[x].color);

      for (unsigned b = 0; b < border; b++)
	{
	  set_pixel (b, y + border, 0);
	  set_pixel (width - b, y + border, 0);
	}
    }
}

template<typename DT>
void
TupleMatrixData<DT>::save (const std::string &filename, const ValTable &params)
  const
{
  ImageScaledOutput out (filename, width, height, params);
  save (out, params);
}

template<typename DT>
void
TupleMatrixData<DT>::save (ImageScaledOutput &out, const ValTable &) const
{
  ImageRow row (width);
  for (unsigned y = 0; y < height; y++)
    {
      for (unsigned x = 0; x < width; x++)
	row[x] = pixel (x, y);
      out.write_row (row);
    }
}



// ----------------------------------------------------------------
// TupleMatrixData sub-image constructor


// Constructor for extracting a sub-image of BASE
//
template<typename DT>
TupleMatrixData<DT>::TupleMatrixData (unsigned _tuple_len,
				      const TupleMatrixData<DT> &base,
				      unsigned offs_x, unsigned offs_y,
				      unsigned w, unsigned h)
  : tuple_len (_tuple_len),
    width (w ? w : base.width - offs_x),
    height (h ? h : base.height - offs_y)
{
  if (offs_x + w > base.width || offs_y + h > base.height)
    throw std::runtime_error ("sub-image out of bounds");

  data.resize (tuple_len * width * height);

  unsigned copy_components = min (tuple_len, base.tuple_len);

  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++)
      {
	DT *t = tuple (x, y);
	const DT *bt = base.tuple (x, y);

	unsigned i;
	for (i = 0; i < copy_components; i++)
	  *t++ = *bt++;
	for (; i < tuple_len; i++)
	  *t++ = 0;
      }
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "tuple-matrix.cc".
// 
#if HAVE_EXTERN_TEMPLATE
EXTERN_TEMPLATE_EXTENSION extern template class TupleMatrixData<default_tuple_element_type>;
EXTERN_TEMPLATE_EXTENSION extern template class TupleMatrix<Color>;
EXTERN_TEMPLATE_EXTENSION extern template class TupleMatrix<float>;
#endif


}

#endif // __TUPLE_MATRIX_TCC__
