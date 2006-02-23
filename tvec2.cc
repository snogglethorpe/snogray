#include "vec.h"

void do_tests (Vec &v1, Vec &v2, Vec &v3)
{
  v2 = v1.unit ();
  v3 = cross (v1, v2);
}

// arch-tag: 30cdf6dd-b7e2-4a6d-bcf6-482e03971c42
