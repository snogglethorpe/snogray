#ifndef __SPACE_H__
#define __SPACE_H__

namespace Space {
  typedef float coord_t;
  typedef coord_t dist_t;

  class Tuple3 {
  public:
    Tuple3 (coord_t _x = 0, coord_t _y = 0, coord_t _z = 0)
      : x (_x), y (_y), z (_z)
    { }

    coord_t x, y, z;
  };  
}

#endif /* __SPACE_H__ */

// arch-tag: 545c5b4f-ae0d-41a1-a743-e285876c5580
