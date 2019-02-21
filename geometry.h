#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include <cmath>

template <class T> struct Vec3
{
    union {
        struct
        {
            T x, y, z;
        };
        T raw[3];
    };

  public:
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

    inline Vec3 operator+(const Vec3 &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    inline Vec3 operator-(const Vec3 &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    inline Vec3 operator*(const Vec3 &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    inline Vec3 operator*(float f) const { return Vec3<T>(x * f, y * f, z * f); }
    inline Vec3 operator/(const Vec3 &v) const { return Vec3<T>(x / v.x, y / v.y, z / v.z); }
    inline Vec3 operator^(const Vec3 &v) const { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }

    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3<T> &normalize(T l = 1)
    {
        *this = (*this) * (l / norm());
        return *this;
    }

    Vec3<T> cross(const Vec3<T> &v2) {
        return Vec3<T>(y*v2.z-z*v2.y, z*v2.x-x*v2.z, x*v2.y-y*v2.x);
    }
};


template <class T> struct Vec2
{
    union {
        struct
        {
            T x, y;
        };
        T raw[2];
    };

  public:
    Vec2() : x(0), y(0) {}
    Vec2(T _x, T _y) : x(_x), y(_y) {}

    inline Vec2 operator+(const Vec2 &v) const { return Vec2<T>(x + v.x, y + v.y); }
    inline Vec2 operator-(const Vec2 &v) const { return Vec2<T>(x - v.x, y - v.y); }
    inline Vec2 operator*(float f) const { return Vec2<T>(x * f, y * f); }

    float norm() const { return std::sqrt(x * x + y * y); }
    Vec2<T> &normalize(T l = 1)
    {
        *this = (*this) * (l / norm());
        return *this;
    }

    template <class > friend std::ostream& operator<<(std::ostream& stream, const Vec3<T>& vector);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <class T> std::ostream& operator<<(std::ostream& stream, const Vec3<T>& vector) {
    stream << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")\n";
    return stream;
}

template <class T> std::ostream& operator<<(std::ostream& stream, const Vec2<T>& vector) {
    stream << "(" << vector.x << ", " << vector.y << ")\n";
    return stream;
}


#endif