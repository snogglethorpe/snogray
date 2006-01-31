// image-rgbe.cc -- Radiance RGBE / .hdr (aka .pic) format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// The RGBE format is a shared-exponent floating-point image format used
// by the Radiance rendering system.
//
// It is basically an ASCII header terminated by a blank line, followed
// by an ASCII size-indicator line, and then a series of binary scanlines.
//
// The ASCII size-indicator line is of the format:  -Y height +X width
// where "height" and "width" are integers.
//
// Each scanline consists of two constant bytes with a value 2, a 16-bit
// big-endian line-length, and four run-length-encoded sections,
// containing all the red bytes for the scanline, then all the blue
// bytes, all the green bytes, then all the exponent bytes.
//
// The run-length-encoding consists of a series of runs, each run
// containing a length byte and either a single data byte (if the length
// is greater than 128, in which case the data byte should be repeated
// that many times), or a series of data bytes (as many as the length
// indicates).
//


#include <cmath>

#include "excepts.h"

#include "image-rgbe.h"

using namespace Snogray;
using namespace std;


typedef unsigned char byte;

struct RgbeColor
{
  static const int exp_offs = 128;

  RgbeColor () : r (0), g (0), b (0), exp (0) { }

  RgbeColor (const Color &col)
    : r (0), g (0), b (0), exp (0)
  {
    double max_comp = col.red;

    if (col.green > max_comp)
      max_comp = col.green;
    if (col.blue > max_comp)
      max_comp = col.blue;

    if (max_comp > 1e-32)
      {
	int iexp;
	float adj = frexp (max_comp, &iexp) * 255.9999 / max_comp;

	r = byte (adj * col.red);
	g = byte (adj * col.green);
	b = byte (adj * col.blue);
	exp = iexp + exp_offs;
      }
  }

  operator Color () const
  {
    if (exp == 0)
      return 0;
    else
      {
	float scale = ldexp (1.0, int (exp) - (exp_offs + 8));
	return Color (scale * (r + 0.5), scale * (g + 0.5), scale * (b + 0.5));
      }
  }

  byte r, g, b, exp;
};



// Output

class RgbeImageSink : public ImageSink
{  
public:

  RgbeImageSink (const RgbeImageSinkParams &params);
  ~RgbeImageSink ();

  virtual void write_row (const ImageRow &row);

private:

  static const unsigned MIN_RUN_LEN = 4;

  void write_rle_component (byte RgbeColor::*component);

  unsigned width, height;

  ofstream outf;

  RgbeColor *row_buf;
};

RgbeImageSink::RgbeImageSink (const RgbeImageSinkParams &params)
  : ImageSink (params),
    width (params.width), height (params.height),
    outf (params.file_name, ios_base::out|ios_base::binary|ios_base::trunc),
    row_buf (new RgbeColor[width])
{
  outf << "#?RADIANCE\n";
  outf << "# Written by snogray\n";
  outf << "\n";
  outf << "-Y " << height << " +X " << width << "\n";
}

RgbeImageSink::~RgbeImageSink ()
{
  delete[] row_buf;
}

void
RgbeImageSink::write_rle_component (byte RgbeColor::*component)
{
  unsigned i = 0;

  while (i < width)
    {
      unsigned run_start, run_len;

      // find start of a run

      for (run_start = i; run_start < width; run_start += run_len)
	{
	  run_len = 1;

	  while (run_len < 127
		 && run_start + run_len < width
		 && (row_buf[run_start + run_len].*component
		     == row_buf[run_start].*component))
	    run_len++;

	  if (run_len >= MIN_RUN_LEN)
	    break;
	}

      if (run_start - i > 1 && run_start - i < MIN_RUN_LEN)
	{
	  unsigned j = i + 1;

	  while (row_buf[j++].*component == row_buf[i].*component)
	    if (j == run_start)
	      {
		outf.put (128 + run_start - i);
		outf.put (row_buf[i].*component);
		i = run_start;
		break;
	      }
	}

      while (i < run_start)
	{
	  unsigned len = run_start - i;

	  if (len > 128)
	    len = 128;

	  outf.put (len);
	  while (len--)
	    outf.put (row_buf[i++].*component);
	}

      if (run_len >= MIN_RUN_LEN)
	{
	  outf.put (128 + run_len);
	  outf.put (row_buf[run_start].*component);
	}
      else
	run_len = 0;

      i += run_len;
    }
}

void
RgbeImageSink::write_row (const ImageRow &row)
{
  // Scanline header
  //
  outf.put (2);
  outf.put (2);
  outf.put ((width >> 8) & 0xFF);
  outf.put (width & 0xFF);

  // Transform our internal color format to RGBE encoded colors.
  //
  for (unsigned i = 0; i < width; i++)
    row_buf[i] = row[i];

  // Write the various components of the RGBE-encoded colors in
  // run-length-encoded form.
  //
  write_rle_component (&RgbeColor::r);
  write_rle_component (&RgbeColor::g);
  write_rle_component (&RgbeColor::b);
  write_rle_component (&RgbeColor::exp);
}

ImageSink *
RgbeImageSinkParams::make_sink () const
{
  return new RgbeImageSink (*this);
}


// Input

class RgbeImageSource : public ImageSource
{  
public:

  RgbeImageSource (const RgbeImageSourceParams &params);
  ~RgbeImageSource ();

  virtual void read_size (unsigned &width, unsigned &height);
  virtual void read_row (ImageRow &row);

private:

  void read_rle_component (byte RgbeColor::*component);

  unsigned width, height;

  ifstream inf;

  RgbeColor *row_buf;
};

RgbeImageSource::RgbeImageSource (const RgbeImageSourceParams &params)
  : inf (params.file_name, ios_base::binary)
{
  char magic[12];

  // Check magic number
  //
  inf.read (magic, 11);
  magic[11] = '\0';
  if (strcmp (magic, "#?RADIANCE\n") != 0)
    throw bad_format ("not a Radiance RGBE file");

  // Skip lines until we find a blank line
  //
  while (!inf.eof () && inf.peek () != '\n')
    while (!inf.eof () && inf.get () != '\n')
      ;
  inf.get(); // eat blank line

  // Parse the size line; the format is:  -Y height +X width
  //
  bool ok = (inf.get() == '-' && inf.get() == 'Y');
  if (ok)
    {
      inf >> height;

      ok = ! inf.fail ();
      if (ok)
	{
	  inf >> ws;

	  ok = (inf.get() == '+' && inf.get() == 'X');
	  if (ok)
	    {
	      inf >> width;

	      ok = !inf.fail () && inf.get () == '\n';
	    }
	}
    }
  if (! ok)
    throw bad_format ("malformed dimension line");

  row_buf = new RgbeColor[width];
}

RgbeImageSource::~RgbeImageSource ()
{
  delete[] row_buf;
}

void
RgbeImageSource::read_size (unsigned &_width, unsigned &_height)
{
  _width = width;
  _height = height;
}

void
RgbeImageSource::read_rle_component (byte RgbeColor::*component)
{
  unsigned i = 0;

  while (i < width)
    {
      byte len = inf.get ();

      if (len > 128)
	{
	  byte val = inf.get ();

	  len -= 128;

	  while (len-- > 0)
	    row_buf[i++].*component = val;
	}
      else
	while (len-- > 0)
	  row_buf[i++].*component = inf.get ();
    }
}

void
RgbeImageSource::read_row (ImageRow &row)
{
  // Each line begins with two constant bytes, which check
  //
  if (inf.get() != 2 || inf.get() != 2)
    throw bad_format ("invalid line header");

  // The next two bytes are the length of the line in pixels encoded as
  // a big-endian 16-bit number.  This should match the width of the
  // image we got from the image header.
  //
  unsigned ll_hi = inf.get ();
  unsigned ll_lo = inf.get ();
  if ((ll_hi << 8) + ll_lo != width)
    throw bad_format ("line/image width mismatch");

  // Read in the various components of the rgbe-encoded colors.
  //
  read_rle_component (&RgbeColor::r);
  read_rle_component (&RgbeColor::g);
  read_rle_component (&RgbeColor::b);
  read_rle_component (&RgbeColor::exp);

  // Transform them to our internal color format.
  //
  for (unsigned i = 0; i < width; i++)
    row[i] = row_buf[i];
}

ImageSource *
RgbeImageSourceParams::make_source () const
{
  return new RgbeImageSource (*this);
}

// arch-tag: 07aa953d-c887-434a-ad33-405ab1976006
