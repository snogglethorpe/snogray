#include "vec.h"

void do_tests (Vec &v1, Vec &v2, Vec &v3)
{
  v2 = v1.normal();
  v3 = v1.cross (v2);
}

// arch-tag: 30cdf6dd-b7e2-4a6d-bcf6-482e03971c42
