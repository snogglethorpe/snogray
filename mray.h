#ifndef __MRAY_H__
#define __MRAY_H__

#include "color.h"
#include "camera.h"
#include "scene.h"

class Mray {
public:
  const Color render (float u, float v) const;

  Scene scene;
  Camera camera;
};

#endif /* __MRAY_H__ */

// arch-tag: c2710780-77b7-4a68-9f8d-b372ec6f2c6f
