#include <ostream>
#ifndef DEFTYPES

typedef int i32;
typedef unsigned int u32;

typedef float f32;
typedef double f64;

namespace DT2{
    class Vec2{
        public:
        f64 x;
        f64 y;
        Vec2(const f64& x, const f64& y);
        Vec2(const Vec2&);
        Vec2();
        Vec2 operator+(const Vec2& other) const;
        void operator+=(const Vec2& other);
        Vec2 operator-(const Vec2& other) const;
        void operator-=(const Vec2& other);
        Vec2 operator*(const f64& v) const;
        f64 dist(const Vec2& other) const;
    };
    class Rect2{
        public:
        f64 x;
        f64 y;
        f64 w;
        f64 h;
        Rect2();
        Rect2(const Vec2& pos, const Vec2& size);
        bool Vec2Intersect(const Vec2& v) const;
    };
}

namespace DT3{
    class Vec3{
        public:
        f64 x;
        f64 y;
        f64 z;
        Vec3();
        Vec3(const Vec3&);
        Vec3(const f64& x, const f64& y, const f64& z);
        Vec3 operator+(const Vec3& other) const;
        void operator+=(const Vec3& other);
        Vec3 operator-(const Vec3& other) const;
        void operator-=(const Vec3& other);
        Vec3 operator*(const f64& v) const;
        void operator*=(const f64& v);
        
        std::ostream& operator<<(std::ostream& os) const;

        f64 abs() const;
        f64 dist(const Vec3& other) const;
    };

    class Transform3{
        public:
        Vec3 pos;
        Vec3 euler_angle;
        Transform3();
        Transform3(const Vec3& pos, const Vec3& angle);
        Vec3 get_forward() const;
    };
}

namespace DTMat{
    class Mat3{
        public:
        f64 data[9];
        Mat3();
        Mat3(const Mat3&);
        Mat3(const f64 data[9]);
        Mat3 operator*(const Mat3& other) const;
        void operator*=(const Mat3& other);
        DT3::Vec3 operator * (const DT3::Vec3& v) const;
    };
    Mat3 from_euler_angles(const DT3::Vec3& angle);

}

#define DEFTYPES
#endif // DEFTYPES
