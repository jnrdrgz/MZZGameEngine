#pragma once
#include <cmath>

struct Vector2
{
public:
    Vector2() :x{ 0 }, y{ 0 }
    {

    }

    Vector2(float x, float y) :x{ x }, y{ y }
    {

    }

    Vector2 operator+(Vector2 other) const {
        return Vector2(other.x + x, other.y + y);
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;

        return *this;
    }

    Vector2 operator-(Vector2 other) const {
        return Vector2(other.x - x, other.y - y);
    }

    Vector2 operator-() const {
        return Vector2(-x, -y);
    }

    Vector2 operator*(float n) const {
        return Vector2(x * n, y * n);
    }

    Vector2 operator/(float n) const {
        return Vector2(x * (1.0f / n), y * (1.0f / n));
    }

    Vector2& operator*=(float n) {
        x *= n;
        y *= n;

        return *this;
    }

    int mag() {
        return sqrt(x * x + y * y);
    }

    void normalize() {
        float m = mag();
        if (m != 0) {
            x /= m;
            y /= m;
        }
    }

    float x, y;

};
