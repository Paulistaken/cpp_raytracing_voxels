#include <ostream>
#include <optional>
#ifndef DEFTYPES

#define fn auto

typedef int i32;
typedef unsigned int u32;

typedef float f32;
typedef double f64;

namespace DT3{
    class Vec3{
        public:
        f64 x;
        f64 y;
        f64 z;
        Vec3();
        Vec3(const Vec3&);
        Vec3(const f64& x, const f64& y, const f64& z);
        fn operator+(const Vec3& other) const -> Vec3;
        fn operator+=(const Vec3& other) -> void;
        fn operator-(const Vec3& other) const -> Vec3;
        fn operator-=(const Vec3& other) -> void;
        fn operator*(f64 v) const -> Vec3;
        fn operator*=(f64 v) -> void;
        
        std::ostream& operator<<(std::ostream& os) const;

        fn abs() const -> f64;
        fn dist(const Vec3& other) const -> f64;
    };

    class Transform3{
        public:
        Vec3 pos;
        Vec3 euler_angle;
        Transform3();
        Transform3(const Vec3& pos, const Vec3& angle);
        fn get_forward() const -> Vec3;
    };
}

namespace DTMat{
    typedef DT3::Vec3 Vec3;
    class Mat3{
        public:
        f64 data[9];
        Mat3();
        Mat3(const Mat3&);
        Mat3(const f64 data[9]);
        fn operator*(const Mat3& other) const -> Mat3;
        fn operator*=(const Mat3& other) -> void;
        fn operator*(f64 v) const -> Mat3;
        fn operator/(f64 v) const -> Mat3;
        fn operator*=(f64 v) -> void;
        fn operator/=(f64 v) -> void;
        fn operator * (const DT3::Vec3& v) const -> Vec3;
        fn inverse() const -> std::optional<Mat3>;
        fn trans() const -> Mat3;
        fn adj() const -> Mat3;
        fn det() const -> f64;
    };
    fn from_euler_angles(const DT3::Vec3& angle) -> Mat3;

}


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

#define DEFTYPES
#endif // DEFTYPES
