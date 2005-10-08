#include "sphere.h"

Space::dist_t
Sphere::intersection_distance (const Ray &ray) const
{
  Vec dir = ray.dir.unit ();
  Vec diff = ray.origin - center;
  float dirdiff = dir.dot (diff);
  float dirdir =  dir.dot (dir);
  float determ = dirdiff*dirdiff - dirdir * (diff.dot(diff) - radius*radius);

  if (determ >= 0)
    {
      float common = -dirdiff / dirdir;
      float determfactor = sqrtf (determ) / dirdir;

      if (determ == 0 && common > 0)
	return common;
      else
	{
	  float t0 = common - determfactor;
	  float t1 = common + determfactor;

	  if (t0 > 0)
	    return t0;
	  else if (t1 > 0)
	    return t1;
	}
    }

  return 0;
}

void
Sphere::closest_intersect (Intersect &isec) const
{
  const Ray &ray = isec.ray;
  Space::dist_t dist = intersection_distance (ray);

  if (dist > isec.distance || (dist > 0 && !isec.obj))
    {
      Pos isec_point = ray.extension (dist);
      isec.set (this, dist, isec_point, (isec_point - center).unit ());
    }
}

bool
Sphere::intersects (const Ray &ray) const
{
  return intersection_distance (ray) > 0;
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
