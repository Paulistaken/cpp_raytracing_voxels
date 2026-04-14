#ifndef DEFTYPES

typedef int i32;
typedef unsigned int u32;

class Vec2{
    public:
    double x;
    double y;
    Vec2(const double& x, const double& y);
    Vec2(const Vec2&);
    Vec2();
    Vec2 operator+(const Vec2& other) const;
    void operator+=(const Vec2& other);
    Vec2 operator-(const Vec2& other) const;
    void operator-=(const Vec2& other);
    Vec2 operator*(const double& v) const;

    double dist(const Vec2& other) const;
};
class Rect2{
    public:
    double x;
    double y;
    double w;
    double h;
    Rect2();
    Rect2(const Vec2& pos, const Vec2& size);
    bool Vec2Intersect(const Vec2& v) const;
};


class Vec3{
    public:
    double x;
    double y;
    double z;
    Vec3();
    Vec3(const Vec3&);
    Vec3(const double& x, const double& y, const double& z);
    Vec3 operator+(const Vec3& other) const;
    void operator+=(const Vec3& other);
    Vec3 operator-(const Vec3& other) const;
    void operator-=(const Vec3& other);
    Vec3 operator*(const double& v) const;
    void operator*=(const double& v);

    double abs() const;
    double dist(const Vec3& other) const;
};

class Mat3{
    public:
    double data[9];
    Mat3();
    Mat3(const Mat3&);
    Mat3(const double data[9]);
    Mat3 operator*(const Mat3& other) const;
    void operator*=(const Mat3& other);
    Vec3 operator * (const Vec3& v) const;
};
Mat3 from_euler_angles(const Vec3& angle);

class Transform3{
    public:
    Vec3 pos;
    Vec3 euler_angle;
    Transform3();
    Transform3(const Vec3& pos, const Vec3& angle);
    Vec3 get_forward() const;
};

#define DEFTYPES
#endif // DEFTYPES
