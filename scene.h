#include "obj.h"

class Scene {
public:
  Intersect *closest_intersect (const Ray &ray);
  
private:
  struct ObjRef {
    Obj *obj;
    ObjRef *next;
  };
  ObjRef *obj_list;
};

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
