// cubetex.h -- Texture wrapped around a cube
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CUBETEX_H__
#define __CUBETEX_H__

#include <string>
#include <istream>

#include "color.h"
#include "vec.h"
#include "tex2.h"
#include "image.h"

namespace Snogray {

class Cubetex
{
public:

  Cubetex (const std::string &spec);
  ~Cubetex ();

  void load (const std::string &filename);
  void load (std::istream &stream, const std::string &filename_pfx);
  void load (const Image &image);

  Color map (const Vec &dir) const;

private:

  Vec parse_axis_dir (const std::string &str);

  struct Face
  {
    Face () : tex (0) { }

    Tex2<Color> *tex;

    Vec u_dir, v_dir;
  };

  // The faces in order are:  right, left, top, bottom, front, back
  //
  Face faces[6];
};

}

#endif /* __CUBETEX_H__ */

// arch-tag: dacaf2e7-e81c-4562-b7db-9917e9d55994
