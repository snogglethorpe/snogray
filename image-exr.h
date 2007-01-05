// image-exr.h -- EXR format image handling
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_EXR_H__
#define __IMAGE_EXR_H__

#include <ImfRgbaFile.h>

#include "image-io.h"


namespace snogray {

class ExrImageSink : public ImageSink
{  
public:

  ExrImageSink (const std::string &filename, unsigned width, unsigned height,
		const ValTable &params = ValTable::NONE)
    : ImageSink (filename, width, height, params),
      outf (filename.c_str(), width, height),
      row_buf (width), cur_y (0)
  {
    if (params.contains ("gamma"))
      open_err ("OpenEXR format does not use gamma correction");
  }

  virtual void write_row (const ImageRow &row);

private:

  Imf::RgbaOutputFile outf;

  std::vector<Imf::Rgba> row_buf;

  unsigned cur_y;
};

class ExrImageSource : public ImageSource
{  
public:

  ExrImageSource (const std::string &filename,
		  const ValTable &params = ValTable::NONE)
    : ImageSource (filename, params), outf (filename.c_str()), cur_y (0)
  {
    const Imf::Header &hdr = outf.header ();
    const Imath::Box2i &data_window = hdr.dataWindow ();

    width = data_window.max.x - data_window.min.x + 1;
    height = data_window.max.y - data_window.min.y + 1;

    row_buf.resize (width);
  }

  virtual void read_row (ImageRow &row);

private:

  Imf::RgbaInputFile outf;

  std::vector<Imf::Rgba> row_buf;

  unsigned cur_y;
};

}

#endif /* __IMAGE_EXR_H__ */


// arch-tag: aefc9982-81ff-4087-a25a-85c97c09475d
