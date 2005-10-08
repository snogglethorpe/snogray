#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "color.h"

class Light {
public:
  Light (const Pos &_pos, const Color &col)
    : pos (_pos), color (col)
  { }
  Light (const Pos &_pos, const Color &col, float intens)
    : pos (_pos), color (col * intens)
  { }
  Light (const Pos &_pos, float intens)
    : pos (_pos), color (Color::white * intens)
  { }
  Light (const Pos &_pos)
    : pos (_pos), color (Color::white)
  { }

  Pos pos;
  Color color;
};

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
