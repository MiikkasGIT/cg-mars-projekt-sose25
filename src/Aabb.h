//
//  Aabb.hpp
//  CGXcode
//
//  Created by Philipp Lensing on 02.11.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef Aabb_hpp
#define Aabb_hpp

#include <stdio.h>
#include "vector.h"

class AABB
{
public:
    Vector Min;
    Vector Max;
    AABB();
    AABB(const Vector& min, const Vector& max);
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
    Vector size() const;
};



#endif /* Aabb_hpp */
