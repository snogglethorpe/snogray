#ifndef __INTERSECT_H__
#define __INTERSECT_H__

#include "ray.h"
#include "color.h"

class Intersect {
public:
  Intersect (const Ray &_ray)
  {
    ray = _ray;
    obj = 0;
  }

  bool hit () { !!obj; }

  bool set_if_closer (Vec::dist_t new_distance, Pos new_point, Vec new_normal)
  {
    if (!obj || new_distance < distance)
      {
	point = new_point;
	normal = new_normal;
      }
  }

  Color render () const;

  Ray ray;

  class Obj *obj; /* If 0, no intersection */
  Vec::dist_t distance;
  Pos point;
  Vec normal;
};

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
