#include <iostream>

#include "mray.h"

using namespace std;

int main ()
{
  Mray mray;
  Color color = mray.render (0, 0);
  cout << "Rendering (u = 0, v = 0):  " << color << endl;
}

// arch-tag: 2bd751cf-5474-4782-bee1-9e58ce38ab7d
