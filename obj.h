#ifndef __OBJ_H__
#define __OBJ_H__

#include "vec.h"
#include "color.h"
#include "ray.h"
#include "material.h"

class Obj {
public:
  Obj (const Material *mat) : material (mat) { }

  virtual bool intersects (const Ray &ray) const;

  virtual void closest_intersect (class Intersect &isec) const;
  virtual void finish_intersect (class Intersect &isec) const;

  const Material *material;
};

#endif /* __OBJ_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
