#include "color.h"

const Color Color::black (0,0,0);
const Color Color::white (1,1,1);
const Color Color::funny (0,10,0);

std::ostream&
operator<< (std::ostream &os, const Color &col)
{
  os << "rgb<" << col.red << ", " << col.green << ", " << col.blue << ">";
  return os;
}

// arch-tag: 11e71f8e-3323-473e-95ce-e3e07e6197d8
