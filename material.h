#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "vec.h"
#include "color.h"

class Material
{
public:
  virtual const Color render (const class Intersect &isec, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const = 0;
};

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
