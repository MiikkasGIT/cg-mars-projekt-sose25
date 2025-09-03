#ifndef __SimpleRayTracer__color__
#define __SimpleRayTracer__color__

#include <iostream>

class Color
{
public:
    float R;
    float G;
    float B;
    
    Color();
    Color( float r, float g, float b);
    Color operator*(const Color& c) const;
    Color operator*(const float Factor) const;
    Color operator+(const Color& c) const;
    Color& operator+=(const Color& c);
};

#endif /* defined(__SimpleRayTracer__color__) */
