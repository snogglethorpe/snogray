#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if 0
#define calc_r(size)	((float)(size + 1) / 2)
#define filt(a, n) 	((1 - (a / r)) / r)
#endif

#if 1
#define calc_r(size)	((float)(size + 1) / 2)
#define filt(a,r)	(M_SQRT2 * (1 / (2 * sqrt (M_PI))) * pow (M_E, -(a)*(a) / 2))
#endif

int main (int argc, char **argv)
{
  int ox, oy;
  float sum = 0;
  int size = atoi (argv[1]);
  float r = calc_r (size);
  for (ox = -(size / 2); ox <= (size / 2); ox++)
    for (oy = -(size / 2); oy <= (size / 2); oy++)
      {
	float ax = (ox < 0) ? -ox : ox;
	float ay = (oy < 0) ? -oy : oy;
	float fx = filt (ax, n);
	float fy = filt (ay, n);
	printf ("[%2d, %2d] fx = %5.3f, fy = %5.3f, fx*fy = %5.3f\n", ox, oy, fx, fy, fx*fy);
	sum += fx * fy;
      }
  printf ("sum = %g\n", sum);
  return 0;
}

/* arch-tag: 2347a6fa-9ba4-4c57-9177-877436135f18
   (do not change this comment) */
