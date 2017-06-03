// image.swg -- SWIG interfaces for snogray image-handling
//
//  Copyright (C) 2011-2013, 2017  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


%{
#include "image/tuple-matrix.h"
#include "image/image.h"
#include "image/image-sampled-output.h"
#include "image/recover-image.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  // fwd decls
  class Color;


  template<class T>
  class TupleMatrix
  {
  public:

    void load (const char *filename, const ValTable &params = ValTable::NONE);
    void save (const char *filename, const ValTable &params = ValTable::NONE)
      const;

    T operator() (unsigned x, unsigned y);

    const unsigned tuple_len;
    const unsigned width, height;
  };
  %extend TupleMatrix<Color>
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"image<%d, %d>", $self->width, $self->height);
      return static_rep_buf;
    }

    void set (unsigned x, unsigned y, const Color &val)
    {
      (*$self)(x,y) = val;
    }
  }
  %extend TupleMatrix<float>
  {
    const char* __str__()
    {
      snprintf (static_rep_buf, sizeof static_rep_buf,
		"mono_image<%d, %d>", $self->width, $self->height);
      return static_rep_buf;
    }

    void set (unsigned x, unsigned y, float val)
    {
      (*$self)(x,y) = val;
    }
  }
  %template(Image) TupleMatrix<Color>;
  %template(ImageRef) Ref<TupleMatrix<Color> >;


  class ImageSampledOutput
  {
  public:

    ImageSampledOutput (const char *filename,
	   		unsigned width, unsigned height,
			const snogray::ValTable &params = snogray::ValTable::NONE);

    bool has_alpha_channel () const;

    unsigned filter_x_radius () const;
    unsigned filter_y_radius () const;

    intens_t max_intens () const;

    unsigned width, height;

    float intensity_scale;   // intensity multiplier (1 == nop)
    float intensity_power;   // power which intensity is raised to (1 == nop)
  };


  class ImageRow
  {
  public:

    ImageRow (unsigned _width = 0);
    void clear ();
    void resize (unsigned w);

    Tint &operator[];
    std::size_t size ();

    unsigned width;
  };

  // SWIG can't handle nested types, so define global versions of some
  // ImageIo types.
  //
  %{
    namespace snogray {
       enum ImageRowOrder {
         FIRST_ROW_AT_TOP = ImageIo::FIRST_ROW_AT_TOP,
         FIRST_ROW_AT_BOTTOM = ImageIo::FIRST_ROW_AT_BOTTOM
       };
    }
  %}
  enum ImageRowOrder { FIRST_ROW_AT_TOP, FIRST_ROW_AT_BOTTOM };


  class ImageScaledOutput
  {
  public:

    ImageScaledOutput (const char *filename,
		       unsigned src_width, unsigned src_height,
		       const ValTable &params = ValTable::NONE);

    void write_row (const ImageRow &row);

    bool has_alpha_channel () const;

    intens_t max_intens () const;

    unsigned width, height;

    float intensity_scale;   // intensity multiplier (1 == nop)
    float intensity_power;   // power which intensity is raised to (1 == nop)
  };


  class ImageInput
  {
  public:

    ImageInput (const char *filename, const ValTable &params = ValTable::NONE);

    bool has_alpha_channel () const;
    ImageRowOrder row_order () const;
#if 0
    ImageRowIndices row_indices () const;
#endif

    void read_row (ImageRow &row);

    unsigned width, height;
  };


  unsigned recover_image (const char *partial_image_filename,
			  const char *output_filename,
			  const ValTable &output_params,
			  ImageSampledOutput &dst);


} // namespace snogray


// The following are object-creation wrappers, which we use in place of
// swig-generated constructors for any classes that we want to manage via
// reference-counting.  This is because swig will do its own memory
// management for object which it constructs, and we want to avoid that.
//
// These wrapper functions simply create the object using new and return a
// reference to it; swig happily uses the reference instead, with almost
// exactly the same functionality as the underlying object pointer.
//
%inline %{
  namespace snogray {


    // Image
    typedef TupleMatrix<Color> Image;
    static Ref<Image> image (unsigned _width, unsigned _height)
    {
      return new Image (_width, _height);
    }
    static Ref<Image> image (const char *filename,
			     const ValTable &params = ValTable::NONE)
    {
      return new Image (filename, params);
    }


  }
%}
