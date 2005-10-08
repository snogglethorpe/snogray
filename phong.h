#ifndef __PHONG_H__
#define __PHONG_H__

#include "material.h"

class Phong : public Material
{
public:
  Phong (const Color &_diffuse_color, const Color &_specular_color,
	 float _exponent)
    : diffuse_color (_diffuse_color), specular_color (_specular_color),
      exponent (_exponent)
  { }
  Phong (const Color &color, float _exponent)
    : diffuse_color (color / 4),
      specular_color ((Color::white - color.intensity()) / 2),
      exponent (_exponent)
  { }

  virtual const Color render (const class Intersect &isec, const Vec &eye_dir,
			      const Vec &light_dir, const Color &light_color)
    const;

  Color diffuse_color, specular_color;
  float exponent;
};

#endif /* __PHONG_H__ */

// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
