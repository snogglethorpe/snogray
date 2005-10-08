#include <iostream>

#include "vec.h"

using namespace std;

extern void do_tests (Vec3 &v1, Vec3 &v2, Vec3 &v3);

int main ()
{
  Vec3 v1 (1, 2, 3), v2, v3;

  do_tests (v1, v2, v3);

  cout << "V1: " << v1.x << ", " << v1.y << ", " << v1.z << endl;
  cout << "V2: " << v2.x << ", " << v2.y << ", " << v2.z << endl;
  cout << "V3: " << v3.x << ", " << v3.y << ", " << v3.z << endl;
}

// arch-tag: ae3f89ba-a69d-44f2-a8e6-08edfef90b56
