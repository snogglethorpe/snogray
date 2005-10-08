#include "sphere.h"

#include "intersect.h"

Space::dist_t
Sphere::intersection_distance (const Ray &ray) const
{
  Vec dir = ray.dir;		// must be a unit vector
  Vec diff = ray.origin - center;
  float dir_diff = dir.dot (diff);
  float dir_dir =  dir.dot (dir);
  float determ = dir_diff*dir_diff - dir_dir * (diff.dot(diff) - radius*radius);

  if (determ >= 0)
    {
      float common = -dir_diff / dir_dir;

      if (determ == 0 && common > 0)
	return common;
      else
	{
	  float determ_factor = sqrtf (determ) / dir_dir;
	  float t0 = common - determ_factor;
	  float t1 = common + determ_factor;

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
  isec.set_obj_if_closer (this, intersection_distance (isec.ray));
}

void
Sphere::finish_intersect (Intersect &isec) const
{
  Space::dist_t dist = isec.distance;
  isec.point = isec.ray.extension (dist);
  isec.normal = (isec.point - center).unit ();
}

bool
Sphere::intersects (const Ray &ray, Space::dist_t max_dist_squared) const
{
  Space::dist_t dist = intersection_distance (ray);
  return (dist * dist) < max_dist_squared;
}

// arch-tag: dc88fe85-ed78-4f90-bbe2-7e670fde73a6
