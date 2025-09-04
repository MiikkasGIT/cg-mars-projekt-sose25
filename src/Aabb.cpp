#include "Aabb.h"
#include <cmath>

AABB::AABB() : Min(0,0,0), Max(0,0,0) {}
AABB::AABB(const Vector& min, const Vector& max) : Min(min), Max(max) {}
AABB::AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
: Min(minX, minY, minZ), Max(maxX, maxY, maxZ) {}

Vector AABB::getCenter() const {
    return (Min + Max) * 0.5f;
}

Vector AABB::getCenterBottom() const {
    return Vector((Min.X + Max.X) * 0.5f, Min.Y, (Min.Z + Max.Z) * 0.5f);
}

Vector AABB::size() const {
    return Max - Min;
}

// robuste, mutierende Transform: 8 Ecken -> neue Min/Max
void AABB::transform(const Matrix& M)
{
    Vector corners[8] = {
        Vector(Min.X, Min.Y, Min.Z),
        Vector(Max.X, Min.Y, Min.Z),
        Vector(Min.X, Max.Y, Min.Z),
        Vector(Max.X, Max.Y, Min.Z),
        Vector(Min.X, Min.Y, Max.Z),
        Vector(Max.X, Min.Y, Max.Z),
        Vector(Min.X, Max.Y, Max.Z),
        Vector(Max.X, Max.Y, Max.Z)
    };

    Vector p0 = M * corners[0];
    Vector newMin = p0;
    Vector newMax = p0;

    for(int i=1;i<8;++i){
        Vector p = M * corners[i];
        if(p.X < newMin.X) newMin.X = p.X;
        if(p.Y < newMin.Y) newMin.Y = p.Y;
        if(p.Z < newMin.Z) newMin.Z = p.Z;
        if(p.X > newMax.X) newMax.X = p.X;
        if(p.Y > newMax.Y) newMax.Y = p.Y;
        if(p.Z > newMax.Z) newMax.Z = p.Z;
    }

    Min = newMin;
    Max = newMax;
}

void AABB::verifyIntegrity()
{
    if (Min.X > Max.X) { float t = Min.X; Min.X = Max.X; Max.X = t; }
    if (Min.Y > Max.Y) { float t = Min.Y; Min.Y = Max.Y; Max.Y = t; }
    if (Min.Z > Max.Z) { float t = Min.Z; Min.Z = Max.Z; Max.Z = t; }
}

Vector AABB::getSeparationVector(AABB* o) {
    if (!o) return Vector(0,0,0);

    float moveX1 = Max.X - o->Min.X;
    float moveX2 = o->Max.X - Min.X;
    float moveX  = (moveX1 < moveX2) ?  moveX1 : -moveX2;

    float moveZ1 = Max.Z - o->Min.Z;
    float moveZ2 = o->Max.Z - Min.Z;
    float moveZ  = (moveZ1 < moveZ2) ?  moveZ1 : -moveZ2;

    // kleinste Penetrationsachse (XZ)
    if (std::fabs(moveX) < std::fabs(moveZ))
        return Vector(moveX, 0, 0);
    else
        return Vector(0, 0, moveZ);
}

void AABB::moveTo(const Vector& newCenter)
{
    Vector half = size() * 0.5f;
    Min = newCenter - half;
    Max = newCenter + half;
    verifyIntegrity();
}
