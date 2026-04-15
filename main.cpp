#include <algorithm>
#include <cmath>
#include <iostream>
#include <raylib.h>

#include "dtypes.hpp"
#include "otree.hpp"
#include "otree_ray.hpp"
#include <optional>
#include <utility>
#include <vector>

const u32 SCREEN_WIDTH = 600;
const u32 SCREEN_HEIGTH = 600;
const u32 VIR_REZ = 60;

const f64 MOVESPEED = 0.5;
const f64 ROTSPEED = 0.1;

using namespace::OCTTree;

typedef DT3::Vec3 Vec3;
typedef DT3::Transform3 Transform3;
typedef DTMat::Mat3 Mat3;

void create_otree(OctTree& otree);
void generate_sphere(OctTree& octree, const Vec3& center, f64 size, i32 voxelsize,const std::vector<Color>& fill);
void render_camera_view(const OctTree& otree, const Transform3& cam);
void insert_blocks(OctTree& otree, OctTree& otree_preview, const Transform3& cam, Color* build_colors, u32 select_build_color, f64 build_size, i32 build_vx);

int main(){

    Color build_colors[5] = {WHITE,RED,GREEN,BLUE,PURPLE};
    u32 select_build_color = 0;
    f64 build_size = 0.5;
    i32 build_vx_size = -3;

    //128x128x128
    OctTree otree(7);
    OctTree otree_preview(7);

    create_otree(otree);

    InitWindow(600, 600, "ray");
    SetTargetFPS(60);

    Transform3 cam = Transform3({35,25,60},{0.,PI,0.});
    Vec3 mov_velocity = {0,0,0};
    
    bool show_map = false;
    bool show_preview = true;

    while(!WindowShouldClose()){
        ClearBackground(Color{20,125,200,255});

        auto mouse_pos = GetMousePosition();

        DisableCursor();
        EnableCursor();
        HideCursor();

        BeginDrawing();

        render_camera_view(otree,cam);
        if (show_preview) render_camera_view(otree_preview,cam);
        if (show_preview) insert_blocks(otree, otree_preview, cam, build_colors, select_build_color, build_size, build_vx_size);

        Vec3 move_axis = Vec3(IsKeyDown(KEY_D)-IsKeyDown(KEY_A),-IsKeyDown(KEY_LEFT_SHIFT)+IsKeyDown(KEY_SPACE),IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*MOVESPEED* (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        Vec3 move_dir = DTMat::from_euler_angles(Vec3(0,cam.euler_angle.y,0))*move_axis;

        mov_velocity = mov_velocity * 0.8;

        if (move_dir.dist(Vec3(0,0,0))>=0.00001){
            mov_velocity = move_dir;
        }
        cam.pos += mov_velocity;

        if (show_map) {
            auto avl = OCTTree::otree_sendray(otree, cam.pos, cam.get_forward());
            for(i32 x = 0; x < 60; x++){
                for(i32 y = 0; y < 60; y++){
                    if (!OCTTree::otree_is_pos_filled(otree, Vec3(x,cam.pos.y,y))) continue;
                    DrawRectangle(x*10, y*10, 10, 10, BLUE);
                }
            }
            DrawCircle(cam.pos.x * 10, cam.pos.z * 10, 5, RED);
            if (avl.has_value()){
                auto [vl, vlc] = avl.value();
                DrawLine(cam.pos.x * 10, cam.pos.z * 10, vl.x * 10, vl.z * 10, RED);
            }else{
                DrawLine(cam.pos.x*10, cam.pos.z*10, cam.pos.x*10+cam.get_forward().x*600, cam.pos.z*10+cam.get_forward().z*600, GREEN);
            }
        }

        DrawText(TextFormat("x: %f, y: %f, z: %f", cam.pos.x, cam.pos.y, cam.pos.z),0,0,16,YELLOW);
        DrawText(TextFormat("yaw: %f, pitch: %f", cam.euler_angle.y, cam.euler_angle.x),0,16,16,YELLOW);
        DrawFPS(0, 48);

        for (int i = 0; i < 5; i++){
            if (select_build_color == i) DrawRectangle(i*32, SCREEN_HEIGTH-32, 32, 32, BLACK);
            DrawRectangle(i*32+4, SCREEN_HEIGTH-32+4, 24, 24, build_colors[i]);
        }
        DrawText(TextFormat("Build size: %f",build_size), 0, SCREEN_HEIGTH-64, 16, YELLOW);
        DrawText(TextFormat("Build detail: 2^%d",build_vx_size), 0, SCREEN_HEIGTH-48, 16, YELLOW);

        EndDrawing();

        f64 mpx = (mouse_pos.x - SCREEN_WIDTH / 2.0) / SCREEN_WIDTH;
        f64 mpy = (mouse_pos.y - SCREEN_HEIGTH / 2.0) / SCREEN_HEIGTH;
        cam.euler_angle.y += mpx*15.0*ROTSPEED;
        cam.euler_angle.x += mpy*15.0*ROTSPEED;

        cam.euler_angle.y += (-IsKeyDown(KEY_LEFT) + IsKeyDown(KEY_RIGHT)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        cam.euler_angle.x += (-IsKeyDown(KEY_UP) + IsKeyDown(KEY_DOWN)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));

        show_map = IsKeyPressed(KEY_M) ? !show_map : show_map;
        show_preview = IsKeyPressed(KEY_P) ? !show_preview : show_preview;

        select_build_color = (select_build_color + IsKeyPressed(KEY_O))%5;
        select_build_color = (select_build_color + IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))%5;
        build_size += (IsKeyPressed(KEY_L)-IsKeyPressed(KEY_K))*0.1;
        build_vx_size += IsKeyPressed(KEY_J)-IsKeyPressed(KEY_H);

    }
}

void generate_sphere(OctTree& otree, const Vec3& center, f64 size, i32 voxelsize, const std::vector<Color>& fill){
    f64 step = std::pow(2.0,voxelsize);
    for (f64 px = center.x - size;px <= center.x + size; px += step){
        for (f64 py = center.y - size;py <= center.y + size; py += step){
            for (f64 pz = center.z - size;pz <= center.z + size; pz += step){
                if (Vec3(px,py,pz).dist(center)>size) continue;
                u32 rsz = GetRandomValue(0, fill.size()-1);
                otree_insert_node(otree, fill[rsz], Vec3(px,py,pz), voxelsize);
            }
        }
    }
}

void insert_blocks(OctTree& otree, OctTree& otree_preview, const Transform3& cam, Color* build_colors, u32 select_build_color, f64 build_size, i32 vx) {
    Vec3 cam_forward = cam.get_forward();
    auto forward_ray = OCTTree::otree_sendray(otree, cam.pos, cam.get_forward());
    Vec3 insert_pos;
    if (forward_ray.has_value()){
        auto [ray_pos,ray_v] = forward_ray.value();
        insert_pos = ray_pos - (cam_forward*(build_size/2));
    }else {
        insert_pos = cam.pos + cam_forward * 20;
    }
    otree_preview.clear();
    if (IsKeyPressed(KEY_RIGHT_CONTROL) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Color clr1 = build_colors[select_build_color];
        Color clr2 = build_colors[select_build_color];
        Color clr3 = build_colors[select_build_color];

        clr1.r += clr1.r < 235 ? 20 : -20;
        clr1.g += clr1.g < 235 ? 20 : -20;
        clr1.b += clr1.b < 235 ? 20 : -20;

        clr2.r -= clr2.r >= 30 ? 30 : -30;
        clr2.g -= clr2.g >= 30 ? 30 : -30;
        clr2.b -= clr2.b >= 30 ? 30 : -30;

        generate_sphere(otree, insert_pos, build_size, vx, {clr1,clr2,clr3});
    }else{
        Color clr1 = build_colors[select_build_color];
        Color clr2 = build_colors[select_build_color];
        Color clr3 = build_colors[select_build_color];
        Color clr4 = build_colors[select_build_color];
        clr1.a = 200;
        clr2.a = 100;
        clr3.a = 50;
        clr4.a = 255;
        generate_sphere(otree_preview, insert_pos, build_size, vx, {clr1,clr2,clr3,clr4});
    }
}

void render_camera_view(const OctTree& otree, const Transform3& cam){
    const u32 vir_rez = VIR_REZ * SCREEN_HEIGTH /  SCREEN_WIDTH;

    f64 anglestep_v = 60.0 / vir_rez;
    f64 anglestep_h = 60.0 / VIR_REZ;

    OCTRay::OCTRayOptions opts;
    opts.max_detail(0);
    for (int angv = 0; angv <= vir_rez; angv++){
        for(int angh = 0; angh <= VIR_REZ; angh++){
            
            f64 anglh = angh * anglestep_h - 30.0;
            f64 anglv = angv * anglestep_v - 30.0;

            f64 cam_h = anglh / 180.0 * PI;
            f64 cam_v = anglv / 180.0 * PI;

            Mat3 rotcam = DTMat::from_euler_angles(Vec3(cam_v,cam_h,0));
            Mat3 rot = DTMat::from_euler_angles(cam.euler_angle);
            Vec3 cam_dir = rotcam * Vec3(0,0,1);
            Vec3 dir = rot * cam_dir;

            OCTRay::OCTRay ray = OCTRay::OCTRay(cam.pos,dir);
            auto vl = ray.send_ray(otree, opts);
            if (vl.has_value()){
                auto [vpos, vcol] = vl.value();
                f64 dst = 1.0/std::max(vpos.dist(cam.pos)/10.0,1.0);
                unsigned char clr = dst * vcol.r 
                    + GetRandomValue(-std::min(vcol.r,(unsigned char)(5)), std::min((unsigned char)(255-vcol.r),(unsigned char)(5)));
                unsigned char clg = dst * vcol.g 
                    + GetRandomValue(-std::min(vcol.g,(unsigned char)(5)), std::min((unsigned char)(255-vcol.g),(unsigned char)(5)));
                unsigned char clb = dst * vcol.b 
                    + GetRandomValue(-std::min(vcol.b,(unsigned char)(5)), std::min((unsigned char)(255-vcol.b),(unsigned char)(5)));
                auto cl = Color{clr,clg,clb,vcol.a};
                int scx = SCREEN_WIDTH / VIR_REZ;
                int scy = SCREEN_HEIGTH / vir_rez;
                DrawRectangle((angh)*scx,(angv)*scy,scx,scy,cl);
            }
        }
    }
}



void create_otree(OctTree& otree){
    Color clrs[11] = {RED,GREEN,BLUE,PINK,YELLOW,PURPLE,WHITE, DARKBLUE, DARKGREEN, DARKPURPLE, BROWN};
    int stepsz = -4;
    f64 step = std::pow(2.0,stepsz);
    for(f64 ix = 18; ix <= 32; ix+=step){
    for(f64 iy = 18; iy <= 32; iy+=step){
    for(f64 iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos + Vec3(-15,0,0), stepsz);

    }
    }
    }
    for(f64 ix = 18; ix <= 25; ix+=step){
    for(f64 iy = 18; iy <= 32; iy+=step){
    for(f64 iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, stepsz);
        
    }
    }
    }
    for(f64 ix = 25; ix <= 32; ix+=1){
    for(f64 iy = 18; iy <= 32; iy+=1){
    for(f64 iz = 18; iz <= 32; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    for(f64 ix = 8; ix <= 22; ix+=1){
    for(f64 iy = 8; iy <= 22; iy+=1){
    for(f64 iz = 8; iz <= 22; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(15,15,15))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    f64 ps1 = 0;
    f64 ps2 = 0;
    for(int i = 0; i <= 10; i++){
        int sz = 3-i;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], Vec3(35+ps1,25,50), sz);
        rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], Vec3(35+ps2,25,50), sz);
        ps1 += std::pow(2.0,sz);
        ps2 -= std::pow(2.0,sz-1);
    }
}
