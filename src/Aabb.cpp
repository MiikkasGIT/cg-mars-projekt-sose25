//
//  Aabb.cpp
//  CGXcode
//
//  Created by Philipp Lensing on 02.11.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "Aabb.h"

AABB::AABB()
{
    
}
AABB::AABB(const Vector& min, const Vector& max) : Min(min), Max(max)
{
    
}
AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) : Min(minX, minY, minZ), Max(maxX, maxY, maxZ)
{
    
}

Vector AABB::size() const
{
    return Max-Min;
}

