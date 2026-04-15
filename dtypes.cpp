#include "dtypes.hpp"
#include <cmath>
#include <ostream>

namespace DT2{
    Vec2::Vec2(const Vec2& other){this->x=other.x;this->y=other.y;}
    Vec2::Vec2(const f64& x, const f64& y) {this->x=x;this->y=y;}
    Vec2::Vec2(){this->x=0;this->y=0;}
    Vec2 Vec2::operator+(const Vec2& other) const { return Vec2(this->x+other.x,this->y+other.y); }
    void Vec2::operator+=(const Vec2& other) {this->x += other.x;this->y += other.y;}
    Vec2 Vec2::operator-(const Vec2& other) const { return Vec2(this->x-other.x,this->y-other.y); }
    void Vec2::operator-=(const Vec2& other) {this->x -= other.x;this->y -= other.y;}
    Vec2 Vec2::operator*(const f64& v) const { return Vec2(this->x*v,this->y*v); }

    f64 Vec2::dist(const Vec2& other) const{
        return std::sqrt(std::pow(this->x-other.x,2)+std::pow(this->y-other.y,2));
    }
    Rect2::Rect2() {this->x=0;this->y=0;this->w=0;this->h=0;}
    Rect2::Rect2(const Vec2& pos, const Vec2& size) {this->x=pos.x;this->y=pos.y;this->w=size.x,this->h=size.y;}
    bool Rect2::Vec2Intersect(const Vec2& v) const {
        return v.x >= this->x && v.y >= this->y && v.x < this->x + this->w && v.y < this->y+this->h;
    }
}

namespace DT3{
    Vec3::Vec3(const Vec3& other){this->x=other.x;this->y=other.y;this->z=other.z;}
    Vec3::Vec3(const f64& x, const f64& y,const f64& z) {this->x=x;this->y=y;this->z=z;}
    Vec3::Vec3(){this->x=0;this->y=0;this->z=0;}
    Vec3 Vec3::operator+(const Vec3& other) const { return Vec3(this->x+other.x,this->y+other.y,this->z+other.z); }
    void Vec3::operator+=(const Vec3& other) {this->x += other.x;this->y += other.y;this->z += other.z;}
    Vec3 Vec3::operator-(const Vec3& other) const { return Vec3(this->x-other.x,this->y-other.y,this->z-other.z); }
    void Vec3::operator-=(const Vec3& other) {this->x -= other.x;this->y -= other.y;this->z-=other.z;}
    Vec3 Vec3::operator*(const f64& v) const { return Vec3(this->x*v,this->y*v,this->z*v); }
    void Vec3::operator*=(const f64& v) { this->x*=v;this->y*=v;this->z*=v; }

    std::ostream& Vec3::operator<<(std::ostream& os) const{
        return os << "x:" << this->x << ", y:" << this->y <<", z:" << this->z;
    }

    f64 Vec3::dist(const Vec3& other) const{
        return std::sqrt(std::pow(this->x-other.x,2)+std::pow(this->y-other.y,2)+std::pow(this->z-other.z,2));
    }
    f64 Vec3::abs() const{
        return std::sqrt(std::pow(this->x,2)+std::pow(this->y,2)+std::pow(this->z,2));
    }

    Transform3::Transform3(){this->pos=Vec3();this->euler_angle=Vec3();}
    Transform3::Transform3(const Vec3& pos, const Vec3& angle){this->pos=pos;this->euler_angle=angle;}
    Vec3 Transform3::get_forward() const{
        return DTMat::from_euler_angles(this->euler_angle)*Vec3(0,0,1);
    }
}

namespace DTMat{
    typedef DT3::Vec3 Vec3;

    Mat3::Mat3(){
        #pragma omp simd
        for(auto& dt : this->data){dt=0;}
    }
    Mat3::Mat3(const Mat3& data){
        #pragma omp simd
        for(int i=0;i<9;i++){this->data[i]=data.data[i];}
    }
    Mat3::Mat3(const f64 data[9]){
        #pragma omp simd
        for(int i=0;i<9;i++){this->data[i]=data[i];}
    }
    Mat3 Mat3::operator*(const Mat3& other) const {
        Mat3 nmat = Mat3();
        for(int i1=0;i1<3;i1++){
            for(int i2=0;i2<3;i2++){
                #pragma omp simd
                for(int i3=0;i3<3;i3++){
                    nmat.data[i1+(i2*3)]+=this->data[i3+(i1*3)]*other.data[(i3*3)+i2];
                }
            }
        }
        return nmat;
    }

    Vec3 Mat3::operator*(const Vec3& v) const{
        f64 nvdt[3] = {0,0,0};
        for(int i1=0;i1<3;i1++){
            for(int i3=0;i3<3;i3++){
                f64 vdt[3] = {v.x,v.y,v.z};
                nvdt[i1]+=this->data[i3+(i1*3)]*vdt[i3];
            }
        }
        return Vec3(nvdt[0],nvdt[1],nvdt[2]);
    }

    void Mat3::operator*=(const Mat3& other){
        Mat3 nw = *this * other;
        *this=nw;
    }

    Mat3 from_euler_angles(const Vec3& angle){
        f64 rot_v[9] = {1,0,0,0,std::cos(angle.x),std::sin(angle.x),0,-std::sin(angle.x),std::cos(angle.x)}; 
        f64 rot_h[9] = {std::cos(angle.y),0,-std::sin(angle.y),0,1,0,std::sin(angle.y),0,std::cos(angle.y)}; 
        Mat3 rot = Mat3(rot_v)*Mat3(rot_h);
        return rot;
    }

}

