#include "vector.h"
#include <cassert>
#include <math.h>
#define EPSILON 1e-5

Vector::Vector(float x, float y, float z) : X(x), Y(y), Z(z) {}

Vector::Vector() : X(0), Y(0), Z(0) {}

float Vector::dot(const Vector& v) const
{
	return X * v.X + Y * v.Y + Z * v.Z;
}

Vector Vector::cross(const Vector& v) const
{
	return Vector(
	   Y * v.Z - Z * v.Y,
	   Z * v.X - X * v.Z,
	   X * v.Y - Y * v.X
   );
}

Vector Vector::operator+(const Vector& v) const
{
	return Vector(X + v.X, Y + v.Y, Z + v.Z);
}

Vector Vector::operator-(const Vector& v) const
{
	return Vector(X - v.X, Y - v.Y, Z - v.Z);
}


Vector Vector::operator*(float c) const
{
	return Vector(X * c, Y * c, Z * c);
}

Vector operator*(float c, const Vector& v) {
	return Vector(v.X * c, v.Y * c, v.Z * c);  // Global operator for float * Vector
}

Vector Vector::operator-() const
{
	return Vector(-X, -Y, -Z);
}

Vector& Vector::operator+=(const Vector& v)
{
	X += v.X,
	Y += v.Y,
	Z += v.Z;

	return *this;
}

float Vector::length() const
{

	return sqrt(lengthSquared());
}

Vector& Vector::normalize() {
    float l = length();
    if (l > 0) {
        X /= l;
        Y /= l;
        Z /= l;
    }
    return *this;
}

float Vector::lengthSquared() const
{
	return X * X + Y * Y + Z * Z;
}

Vector Vector::reflection(const Vector& normal) const {
	float dotProduct = this->dot(normal);
	return *this - 2 * dotProduct * normal;
}

bool Vector::triangleIntersection(const Vector& d, const Vector& a, const Vector& b, const Vector& c, float& s) const
{
	Vector ba = b - a;
	Vector ca = c - a;
	Vector n = ba.cross(ca).normalize();

	float distance = n.dot(a);
	s = (distance - n.dot(*this)) / n.dot(d);

	if (s < EPSILON) return false;

	Vector p = *this + s * d;


	float abc = (b - a).cross(c - a).length() * 0.5f;
	float abp = (b - a).cross(p - a).length() * 0.5f;
	float acp = (c - a).cross(p - a).length() * 0.5f;
	float bcp = (c - b).cross(p - b).length() * 0.5f;

	return (abc + EPSILON >= abp + acp + bcp);
}
