Color
{
public:
  Color (dim_t _red, dim_t _green, dim_t _blue)
    { red = _red; green = _green; blue = _blue; 
  Color ()
    { red = green = blue = 0; }

  coord_t red, green, blue;
};  

// arch-tag: 389b3ebb-55a4-4d70-afbe-91bdb72d28ed
