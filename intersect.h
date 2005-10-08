#include "ray.h"

class Intersect {
public:
  bool none () { !obj; }

  bool set_if_closer (dim_t new_distance, Point3 new_point, Vec3 new_normal)
  {
    if (!obj || new_distance < distance)
      {
	point = new_point;
	normal = new_normal;
      }
  }

  Ray ray;

  Obj *obj; /* If 0, no intersection */
  dim_t distance;
  Point3 point;
  Vec3 normal;
};

// arch-tag: cce437f9-75b6-42e5-bb0f-ee18693d6799
