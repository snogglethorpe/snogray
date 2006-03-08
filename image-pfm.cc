// image-pfm.cc -- PFM ("Portable Float Map") format image handling
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// PFM is a floating-point image format used by "HDR Shop".
//
// It is basically an ASCII header followed by the raw raster, where
// each pixel is 1 or 3 IEEE floating point numbers in binary format.
//
// The first line of the header is a magic number, consisting of the
// characters "PF\n" for RGB, or "Pf\n" for grey-scale.  The next line
// is the ASCII decimal height and width separated by a space.  The 3rd
// and last line is a (ASCII, floating-point) "scale factor", with the
// added wrinkle that if the scale factor is negative, the raster uses
// little-endian IEEE floats, and if it's positive, the raster uses
// big-endian IEEE floats.
//
// A description can be found at:
//
//    http://netpbm.sourceforge.net/doc/pfm.html
//
// However it is not supported in official netpbm releases.
//


#include "excepts.h"

#include "image-pfm.h"

using namespace Snogray;
using namespace std;

static bool
little_endian ()
{
  union { unsigned long l; char c[4]; } u;
  u.l = 0x12345678;
  return u.c[0] == 0x78;
}


// Output

class PfmImageSink : public ImageSink
{  
public:

  PfmImageSink (const PfmImageSinkParams &params);
  ~PfmImageSink ();

  virtual void write_row (const ImageRow &row);

private:

  unsigned width, height;

  ofstream outf;

  // For whatever stupid reason, PFM files (unlike every other image
  // format) are store with the _last_ line first.  So for simplicity,
  // we just read the whole damn thing into memory.
  //
  float *raster;

  unsigned next_y;
};

PfmImageSink::PfmImageSink (const PfmImageSinkParams &params)
  : ImageSink (params),
    width (params.width), height (params.height),
    outf (params.file_name, ios_base::out|ios_base::binary|ios_base::trunc),
    raster (new float[width * height * 3]),
    next_y (0)
{
  outf << "PF\n" << params.width << " " << params.height << "\n";
  outf << (little_endian() ? "-1" : "1") << "\n";
}

PfmImageSink::~PfmImageSink ()
{
  outf.write (reinterpret_cast<char *>(raster), width * height * 12);

  delete[] raster;
}

void
PfmImageSink::write_row (const ImageRow &row)
{
  float *p = raster + (width * (height - 1 - next_y) * 3);

  for (unsigned x = 0; x < row.width; x++)
    {
      const Color &col = row[x];

      *p++ = col.r();
      *p++ = col.g();
      *p++ = col.b();
    }

  next_y++;
}

ImageSink *
PfmImageSinkParams::make_sink () const
{
  return new PfmImageSink (*this);
}


// Input

class PfmImageSource : public ImageSource
{  
public:

  PfmImageSource (const PfmImageSourceParams &params);
  ~PfmImageSource ();

  virtual void read_size (unsigned &width, unsigned &height);
  virtual void read_row (ImageRow &row);

private:

  float maybe_byte_swap (const float &f)
  {
    if (byte_swap_floats)
      {
	union { float f; char c[4]; } u1, u2;
	u1.f = f;
	u2.c[0] = u1.c[3];
	u2.c[1] = u1.c[2];
	u2.c[2] = u1.c[1];
	u2.c[3] = u1.c[0];
	return u2.f;
      }
    else
      return f;
  }

  unsigned width, height;

  ifstream inf;

  float *raster;

  bool byte_swap_floats;

  unsigned next_y;
};

PfmImageSource::PfmImageSource (const PfmImageSourceParams &params)
  : inf (params.file_name, ios_base::binary),
    next_y (0)
{
  char magic[10];

  inf.read (magic, 3);
  if (magic[0] != 'P' || magic[1] != 'F' || magic[2] != '\n')
    throw bad_format ("not a PMF file");

  float scale;

  inf >> width >> height;
  inf.get(); // skip one character (normally newline)

  inf >> scale;
  inf.get(); // skip one character (normally newline)

  bool file_little_endian = (scale < 0);

  byte_swap_floats = (file_little_endian != little_endian ());

  raster = new float[width * height * 3];

  inf.read (reinterpret_cast<char *>(raster), width * height * 12);
}

PfmImageSource::~PfmImageSource ()
{
  delete[] raster;
}

void
PfmImageSource::read_size (unsigned &_width, unsigned &_height)
{
  _width = width;
  _height = height;
}

void
PfmImageSource::read_row (ImageRow &row)
{
  float *p = raster + (width * (height - 1 - next_y) * 3);

  for (unsigned x = 0; x < row.width; x++)
    {
      float r = maybe_byte_swap (*p++);
      float g = maybe_byte_swap (*p++);
      float b = maybe_byte_swap (*p++);
      row[x].set_rgb (r, g, b);
    }

  next_y++;
}

ImageSource *
PfmImageSourceParams::make_source () const
{
  return new PfmImageSource (*this);
}

// arch-tag: 225e07ef-d906-4866-9120-b7d360506249
