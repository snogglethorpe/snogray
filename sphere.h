#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "obj.h"

class Sphere : public Obj {
public:
  virtual void closest_intersect (Intersect &isec) const;
  virtual bool intersects (const Ray &ray) const;

  Space::dist_t radius;
};

#endif /* __SPHERE_H__ */

// arch-tag: 0284bdaf-1330-46c4-875b-919152357490
