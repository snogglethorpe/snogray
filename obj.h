#ifndef __OBJ_H__
#define __OBJ_H__

class Obj;

#include "vec.h"
#include "color.h"
#include "material.h"
#include "intersect.h"

class Obj {
public:
  virtual void closest_intersect (Intersect &isec) const;
  virtual bool intersects (const Ray &ray) const;

  Material *material;
};

#endif /* __OBJ_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
