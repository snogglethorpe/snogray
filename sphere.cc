#include "sphere.h"

void
Sphere::closest_intersect (Intersect &isec) const
{
  const Ray &ray = isec.ray;
  Vec dir = ray.dir.normal ();
  Vec diff = ray.origin - pos;
  float dirdiff = dir.dot (diff);
  float dirdir =  dir.dot (dir);
  float determ
    = dirdiff*dirdiff - dirdir * (diff.dot(diff) - radius*radius);

  if (determ >= 0)
    {
      float common = -dirdiff / dirdir;
      float determfactor = sqrtf (determ) / dirdir;

      if (determ == 0 && common > 0)
	{
	  start = 0;
	  end = common;
	  /* ... */
	}
      else
	{
	  float t0 = common - determfactor;
	  float t1 = common + determfactor;

	  if (t0 <= 0 && t1 <= 0)
	    {
	    }
	}
    }
}

bool
Sphere::intersects (const Ray &ray) const
{
  return false;
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
