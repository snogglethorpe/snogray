#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "color.h"

class Material
{
public:
  virtual Color render (const class Intersect &isec,
			const Color &light_color, const Vec &light_dir)
    const = 0;
};

#endif /* __MATERIAL_H__ */

// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
