#ifndef __LAMBERT_H__
#define __LAMBERT_H__

#include "material.h"

class Lambert : public Material
{
public:
  Lambert (const Color &_color) : color (_color) { }

  virtual const Color render (const class Intersect &isec, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const;

  Color color;
};

#endif /* __LAMBERT_H__ */

// arch-tag: ca8981f3-5471-4e8a-ac8b-2e3e54c83b64
