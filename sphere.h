#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "obj.h"

class Sphere : public Obj
{
public:
  Sphere (const Material *mat, const Pos &_center, Space::dist_t _radius)
    : Obj (mat)
  {
    center = _center;
    radius = _radius;
  }

  virtual bool intersects (const Ray &ray, Space::dist_t max_dist_squared)
    const;

  virtual void closest_intersect (Intersect &isec) const;
  virtual void finish_intersect (Intersect &isec) const;

private:
  Space::dist_t intersection_distance (const Ray &ray) const;

  Pos center;
  Space::dist_t radius;
};

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
