#include <cmath>
#include "Vector2.hpp"

#pragma region Vector2
// Vector2 class implementation
Vector2::Vector2() : x(0), y(0) {}

Vector2::Vector2(float x, float y) : x(x), y(y) {}

bool Vector2::operator==(Vector2 v) {
    return x == v.x && y == v.y;
}

Vector2 Vector2::operator+(Vector2 v) {
    return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator-(Vector2 v) {
    return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator*(float f) {
    return Vector2(x * f, y * f);
}

Vector2 operator*(float f, Vector2 v) {
    return Vector2(v.x * f, v.y * f);
}

Vector2 Vector2::operator/(float f) {
    return Vector2(x / f, y / f);
}

Vector2 Vector2::operator+=(Vector2 v) {
    x += v.x;
    y += v.y;
    return *this;
}

float Vector2::Magnitude() {
    if (x == 0 && y == 0) {
        return 0;
    }
    return std::sqrt(x * x + y * y);
}

Vector2 Vector2::Normalize() {
    float magnitude = Magnitude();
    if (magnitude == 0) {
        return Vector2(0, 0);
    }
    return Vector2(x / magnitude, y / magnitude);
}

float Vector2::Distance(Vector2 v) {
    return std::sqrt((v.x - x) * (v.x - x) + (v.y - y) * (v.y - y));
}

float Vector2::Distance(Vector2 v1, Vector2 v2) {
    return std::sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

float Vector2::Dot(Vector2 v) {
    return x * v.x + y * v.y;
}

float Vector2::Dot(Vector2 v1, Vector2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

float Vector2::Cross(Vector2 v) {
    return x * v.y - y * v.x;
}

float Vector2::Cross(Vector2 v1, Vector2 v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

float Vector2::Angle(Vector2 v1, Vector2 v2) {
    float angle = std::acos(v1.Dot(v2) / (v1.Magnitude() * v2.Magnitude()));
    return angle * (180.0f / M_PI); // Convert radians to degrees
}

float Vector2::SignedAngle(Vector2 v1, Vector2 v2) {
    float angle = std::atan2(v2.y, v2.x) - std::atan2(v1.y, v1.x);
    return angle * (180.0f / M_PI); // Convert radians to degrees
}

#pragma endregion