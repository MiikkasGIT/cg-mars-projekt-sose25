#ifndef Aabb_hpp
#define Aabb_hpp

#include "vector.h"
#include "matrix.h"

class AABB
{
public:
    // Konstruktion
    AABB();
    AABB(const Vector& min, const Vector& max);
    AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

    // Geometrie
    Vector Min;
    Vector Max;

    // Utilities
    Vector size() const;
    Vector getCenter() const;
    Vector getCenterBottom() const;
    void   transform(const Matrix& matrix);   // in-place (8 Ecken)
    void   moveTo(const Vector& newCenter);
    void   verifyIntegrity();

    bool  isCollision = false;   // (Schreibfehler von isColission korrigiert)
    int   type = 0;              // 0=Standard, 3=Drone
    Vector pullWithConnected;    // hier: Separation-Impuls für die Drone

    // Min. Trennvektor (aktuell X/Z – wie bisher)
    Vector getSeparationVector(AABB* other);
};

#endif /* Aabb_hpp */
