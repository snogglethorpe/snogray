#include <cmath>

typedef float coord_t;
typedef float dim_t;

class Tuple3 {
public:
  Tuple3 (dim_t _x, dim_t _y, dim_t _z) { x = _x; y = _y; z = _z; }
  Tuple3 () { x = y = z = 0; }
  coord_t x, y, z;
};  

class Vec3 : public Tuple3 {
public:
  Vec3 (dim_t _x, dim_t _y, dim_t _z) { x = _x; y = _y; z = _z; }
  Vec3 () { x = y = z = 0; }

  Vec3 (const Vec3 &vec) { x = vec.x; y = vec.y; z = vec.z; }

  Vec3 operator* (const float scale) const
  {
    return Vec3 (x * scale, y * scale, z * scale);
  }
  Vec3 operator+ (const Vec3 &v2) const
  {
    return Vec3 (x + v2.x, y + v2.y, z + v2.z);
  }
  Vec3 operator- (const Vec3 &v2) const
  {
    return Vec3 (x - v2.x, y - v2.y, z - v2.z);
  }

  dim_t dot (const Vec3 &v2) const
  {
    return x * v2.x + y * v2.y + z * v2.z;
  }
  dim_t length_squared () const
  {
    return x * x + y * y + z * z;
  }
  dim_t length () const
  {
    return sqrtf (x * x + y * y + z * z);
  }

  Vec3 cross (const Vec3 &vec2) const
  {
    return Vec3 (y * vec2.z - z * vec2.y,
		 z * vec2.x - x * vec2.z,
		 x * vec2.y - y * vec2.x);
  }

  Vec3 normal () const
  {
    dim_t len = length ();
    if (len == 0)
      return Vec3 (0, 0, 0);
    else
      return Vec3 (x / len, y / len, z / len);
  }
};

class Point3 : public Tuple3 {
  Point3 (dim_t _x, dim_t _y, dim_t _z) { x = _x; y = _y; z = _z; }
  Point3 () { x = y = z = 0; }

  Point3 (const Point3 &point) { x = point.x; y = point.y; z = point.z; }

  Point3 operator+ (const Vec3 &v) const
  {
    return Point3(x + v.x, y + v.y, z + v.z);
  }
  Vec3 operator- (const Point3 &p2) const
  {
    return Vec3(x - p2.x, y - p2.y, z - p2.z);
  }
};

// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
