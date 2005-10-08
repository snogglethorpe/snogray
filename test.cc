#include <iostream>

#include "snogray.h"
#include "sphere.h"

using namespace std;

int main ()
{
  SnogRay snogray;
  Lambert lamb;

  snogray.scene.add (new Sphere (&lamb, Pos (0, 2, 7), 5));

  Color color = snogray.render (0, 0);

  cout << "Rendering (u = 0, v = 0):  " << color << endl;
}

// arch-tag: 2bd751cf-5474-4782-bee1-9e58ce38ab7d
