#include "vox_render.hpp"
#include "otree.hpp"
#include "otree_ray.hpp"
#include <algorithm>
#include <iostream>
#include <raylib.h>
#include <vector>

namespace Vox_Rend {

    Color color_darken(Color col, u8 amt){
        Color cc = {0,0,0,col.a};
        cc.r = col.r - std::min(col.r,amt);
        cc.g = col.g - std::min(col.g,amt);
        cc.b = col.b - std::min(col.b,amt);
        return cc;
    }
    Color color_add_noise(Color col, u8 noise){
        u8 clr = col.r 
            + GetRandomValue(-std::min(col.r,noise), std::min((u8)(255-col.r),noise));
        u8 clg = col.g 
            + GetRandomValue(-std::min(col.g,noise), std::min((u8)(255-col.g),noise));
        u8 clb = col.b 
            + GetRandomValue(-std::min(col.b,noise), std::min((u8)(255-col.b),noise));
        return Color{clr,clg,clb,col.a};
    }
    Color mix_colors_a(Color ca, Color cb){
        Color cc = {0,0,0,std::max(ca.a,cb.a)};
        cc.r = (u8)std::max((u32)ca.r*(u32)ca.a/(u32)255 + (u32)cb.r*(u32)cb.a/(u32)255,(u32)255);
        cc.g = (u8)std::max((u32)ca.g*(u32)ca.a/(u32)255 + (u32)cb.g*(u32)cb.a/(u32)255,(u32)255);
        cc.b = (u8)std::max((u32)ca.b*(u32)ca.a/(u32)255 + (u32)cb.b*(u32)cb.a/(u32)255,(u32)255);
        return cc;
    }

    PixelData::PixelData(){
        this->deph=100000;
        this->col=BLACK;
    }
    PixelData::PixelData(Color col, f64 deph){
        this->deph = deph;
        this->col = col;
    }
    Screen::Screen(u32 sw, u32 sh, u32 vrez){
        this->update_rez(sw, sh, vrez);
    }
    fn Screen::update_rez(u32 sw, u32 sh, u32 vrez) -> void {
        u32 vrez_v = (vrez*sh)/sw;
        this->screen_width = sw;
        this->screen_heigth = sh;
        this->vir_rez_w = vrez;
        this->vir_rez_h = vrez_v;
        this->pixels = std::vector(vrez_v,std::vector(vrez,PixelData()));
    }
    fn Screen::reset_scr(Color bcol) -> void{
        for (auto& prow : this->pixels){
            for(auto & px : prow){
                px.col = bcol;
                px.deph = 100000;
            }
        }
    }
    fn Screen::set_pixel(u32 x, u32 y, Color col, f64 deph) -> void{
        if (x >= this->vir_rez_w || y >= this->vir_rez_h) return;
        auto& pixel = this->pixels[y][x];
        if (deph > pixel.deph) return;
        pixel = col.a==255 ? PixelData(col,deph) : PixelData(mix_colors_a(pixel.col,col),pixel.deph);
    }
    fn Screen::render_scr() const -> void {
        for (int y = 0; y < this->vir_rez_h; y++){
            for(int x = 0; x < this->vir_rez_w; x++){
                auto& pixel = this->pixels[y][x];
                const Color col = pixel.deph < 99999 ? color_add_noise(pixel.col,5) : pixel.col;
                u32 scx = this->screen_width / this->vir_rez_w;
                u32 scy = this->screen_heigth / this->vir_rez_h; 
                DrawRectangle((x)*scx,(y)*scy,scx,scy,col);
            }
        }
    }

    fn Screen::get_pixel(u32 x, u32 y) const -> std::optional<PixelData>{
        if (x >= this->vir_rez_w || y >= this->vir_rez_h) return {};
        return this->pixels[y][x];
    }

    fn Screen::render_otree(const OCTTree::OctTree& otree, const DT3::Transform3& cam) -> void{
        f64 anglestep_v = 60.0 / this->vir_rez_h;
        f64 anglestep_h = 60.0 / this->vir_rez_w;

        OCTTree::OCTRay::OCTRayOptions opts;
        opts.max_detail(0);

        DTMat::Mat3 rot = DTMat::from_euler_angles(cam.euler_angle);

        for (int angv = 0; angv <= this->vir_rez_h; angv++){
            for(int angh = 0; angh <= this->vir_rez_w; angh++){
                
                f64 anglh = angh * anglestep_h - 30.0;
                f64 anglv = angv * anglestep_v - 30.0;

                f64 cam_h = anglh / 180.0 * PI;
                f64 cam_v = anglv / 180.0 * PI;

                DTMat::Mat3 rotcam = DTMat::from_euler_angles(DT3::Vec3(cam_v,cam_h,0));
                DTMat::Vec3 cam_dir = rotcam * DT3::Vec3(0,0,1);
                DTMat::Vec3 dir = rot * cam_dir;

                OCTTree::OCTRay::OCTRay ray = OCTTree::OCTRay::OCTRay(cam.pos,dir);
                auto vl = ray.send_ray(otree, opts);
                if (vl.has_value()){
                    auto [vpos, vcol] = vl.value();
                    f64 cam_dist = vpos.dist(cam.pos);
                    f64 dst = 1.0/std::max(cam_dist/10.0,1.0);
                    vcol = color_darken(vcol,255- dst * 255);
                    this->set_pixel(angh, angv, vcol, cam_dist);
                }
            }
        }
    }
}
