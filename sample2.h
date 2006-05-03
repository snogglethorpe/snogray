// 2d sample
//
template<typename T>
class TSample2
{
public:

  TSample2 (T _u, T _v) : u (_u), v (_v) { }

  T u, v;
};

typedef TSample2<float> Sample2;

// arch-tag: 20a48904-95a2-4412-915a-eda8a374c005
