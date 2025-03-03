#ifndef VECTOR2_HPP
#define VECTOR2_HPP
class Vector2
{
public:
    float x, y;
    Vector2();
    Vector2(float x, float y);
    bool operator==(Vector2 v);
    Vector2 operator+(Vector2 v);
    Vector2 operator-(Vector2 v);
    Vector2 operator*(float f);
    float operator*(Vector2 v);
    Vector2 operator/(float f);
    Vector2 operator+=(Vector2 v);

    float Magnitude();
    Vector2 Normalize();
    float Distance(Vector2 v);
    static float Distance(Vector2 v1, Vector2 v2);
    float Dot(Vector2 v);
    static float Dot(Vector2 v1, Vector2 v2);
    float Cross(Vector2 v);
    static float Cross(Vector2 v1, Vector2 v2);

    static float Angle(Vector2 v1, Vector2 v2);

    static float SignedAngle(Vector2 v1, Vector2 v2);
};

Vector2 operator*(float f, Vector2 v);

#endif