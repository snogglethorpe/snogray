#include "material.h"

class Lambert : public Material
{
public:
  Lambert (const Color &_color) { color = _color; }

  virtual Color render (const class Intersect &isec,
			const Color &light_color, const Vec &light_dir)
    const;

  Color color;
};

// arch-tag: ca8981f3-5471-4e8a-ac8b-2e3e54c83b64
