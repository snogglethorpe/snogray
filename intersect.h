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

  void set (const class Obj *_obj, Vec::dist_t _distance, Pos _point, Vec _normal)
  {
    point = _point;
    normal = _normal;
  }

  Color render () const;

  Ray ray;

  const class Obj *obj; /* If 0, no intersection */
  Vec::dist_t distance;
  Pos point;
  Vec normal;
};

#endif /* __INTERSECT_H__ */

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
