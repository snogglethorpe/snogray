#include "vec.h"

void do_tests (Vec3 &v1, Vec3 &v2, Vec3 &v3)
{
  v2 = v1.normalize();
  v3 = v1.cross (v2);
}

// arch-tag: 30cdf6dd-b7e2-4a6d-bcf6-482e03971c42
