#ifndef __SNOGRAY_H__
#define __SNOGRAY_H__

#include "color.h"
#include "camera.h"
#include "scene.h"

class SnogRay {
public:
  const Color render (float u, float v) const;

  Scene scene;
  Camera camera;
};

#endif /* __SNOGRAY_H__ */

// arch-tag: 030c7e91-757b-49f2-98c3-f70dcd83bddf
