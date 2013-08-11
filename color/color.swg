# color.swg -- SWIG interfaces for color handling
#
#  Copyright (C) 2011, 2012, 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#


%{
#include "color/color.h"
#include "color/color-io.h"
%}


// SWIG-exported interfaces.
//
// Note that these are often simplifications of the real interfaces;
// see "snograw.swg" for a more detailed explanation.
//
namespace snogray {


  class Color
  {
  public:

    typedef float component_t;

    Color ();
    Color (component_t r, component_t g, component_t b);
    Color (double grey);

    Color operator- () const;

    intens_t intensity ();

    Color clamp (intens_t max_intens) const;
    Color clamp (intens_t min_intens, intens_t max_intens) const;

    %rename(__pow) pow (float exp) const;
    Color pow (float exp) const;

    component_t min_component () const;
    component_t max_component () const;

    component_t r () const, g () const, b () const;

    void set_rgb (component_t r, component_t g, component_t b);
  };
  %extend Color
  {
    Color operator+ (const Color &col) { return *$self + col; }
    Color operator- (const Color &col) { return *$self - col; }
    Color operator* (const Color &col) { return *$self * col; }
    Color operator/ (const Color &col) { return *$self / col; }

    Color operator+ (component_t v) { return *$self + v; }
    Color operator- (component_t v) { return *$self - snogray::Color (v); }
    Color operator* (component_t v) { return *$self * v; }
    Color operator/ (component_t v) { return *$self / v; }

    bool operator== (const Color &col) { return *$self == col; }
    bool operator< (const Color &col) { return *$self < col; }
    bool operator<= (const Color &col) { return *$self <= col; }
    
    const char* __str__() { return static_rep (*$self); }
  }


}
