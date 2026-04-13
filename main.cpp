#include <algorithm>
#include <cmath>
#include <raylib.h>

#include "dtypes.hpp"
#include "otree.hpp"
#include <optional>
#include <utility>

const u32 SCREEN_WIDTH = 600;
const u32 SCREEN_HEIGTH = 600;
const u32 VIR_REZ = 60;

const double MOVESPEED = 0.5;
const double ROTSPEED = 0.1;

void create_otree(OctTree& otree);

int main(){

    //128x128x128
    OctTree otree(7);

    create_otree(otree);

    InitWindow(600, 600, "ray");
    SetTargetFPS(60);

    Vec3 orgin = {35,25,60};
    Vec3 mov_velocity = {0,0,0};
    double angle_h = PI;
    double angle_v = 0;
    
    bool show_map = false;

    while(!WindowShouldClose()){
        ClearBackground(Color{20,50,200,255});

        BeginDrawing();

        angle_h += (-IsKeyDown(KEY_LEFT) + IsKeyDown(KEY_RIGHT)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        angle_v += (-IsKeyDown(KEY_UP) + IsKeyDown(KEY_DOWN)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));

        const u32 vir_rez = VIR_REZ * SCREEN_HEIGTH /  SCREEN_WIDTH;
        for (int angv = 0; angv <= vir_rez; angv++){
            for(int angh = 0; angh <= VIR_REZ; angh++){
                int anglh = angh - (VIR_REZ / 2);
                int anglv = angv - (vir_rez / 2);

                double cam_h = anglh / 180.0 * PI;
                double cam_v = anglv / 180.0 * PI;

                double rotcam_v[9] = {1,0,0,0,std::cos(cam_v),std::sin(cam_v),0,-std::sin(cam_v),std::cos(cam_v)}; 
                double rotcam_h[9] = {std::cos(cam_h),0,-std::sin(cam_h),0,1,0,std::sin(cam_h),0,std::cos(cam_h)}; 

                double rot_v[9] = {1,0,0,0,std::cos(angle_v),std::sin(angle_v),0,-std::sin(angle_v),std::cos(angle_v)}; 
                double rot_h[9] = {std::cos(angle_h),0,-std::sin(angle_h),0,1,0,std::sin(angle_h),0,std::cos(angle_h)}; 
                
                Mat3 rotcam = Mat3(rotcam_v) * Mat3 (rotcam_h);
                Vec3 cam_dir = rotcam * Vec3(0,0,1);

                Mat3 rot = Mat3(rot_v) * Mat3(rot_h);
                Vec3 dir = rot * cam_dir;

                auto vl = otree_sendray(otree, orgin, dir);
                if (vl.has_value()){
                    auto [vpos, vcol] = vl.value();
                    double dst = 1.0/std::max(vpos.dist(orgin)-10.0,1.0);
                    unsigned char clr = dst * vcol.r;
                    unsigned char clg = dst * vcol.g;
                    unsigned char clb = dst * vcol.b;
                    auto cl = Color{clr,clg,clb,255};
                    int scx = SCREEN_WIDTH / VIR_REZ;
                    int scy = SCREEN_HEIGTH / vir_rez;
                    DrawRectangle((angh)*scx,(angv)*scy,scx,scy,cl);
                }
            }
        }

        double rot_h[9] = {std::cos(angle_h),0,std::sin(angle_h),0,1,0,-std::sin(angle_h),0,std::cos(angle_h)}; 
        Mat3 rot = Mat3(rot_h);
        Vec3 movedir = Vec3(IsKeyDown(KEY_D)-IsKeyDown(KEY_A),-IsKeyDown(KEY_LEFT_SHIFT)+IsKeyDown(KEY_SPACE),IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*MOVESPEED* (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        movedir=rot*movedir;
        mov_velocity = mov_velocity * 0.8;
        if (movedir.dist(Vec3(0,0,0))>=0.00001){
            mov_velocity = movedir;
        }
        orgin += mov_velocity;

        show_map = IsKeyPressed(KEY_M) ? !show_map : show_map;

        if (show_map) {
            Vec3 dir = rot*Vec3(0,0,1);
            auto avl = otree_sendray(otree, orgin, dir);
            for(i32 x = 0; x < 60; x++){
                for(i32 y = 0; y < 60; y++){
                    if (!otree_is_pos_filled(otree, Vec3(x,orgin.y,y))) continue;
                    DrawRectangle(x*10, y*10, 10, 10, BLUE);
                }
            }
            DrawCircle(orgin.x * 10, orgin.z * 10, 5, RED);
            if (avl.has_value()){
                auto [vl, vlc] = avl.value();
                DrawLine(orgin.x * 10, orgin.z * 10, vl.x * 10, vl.z * 10, RED);
            }else{
                DrawLine(orgin.x*10, orgin.z*10, orgin.x*10+dir.x*600, orgin.z*10+dir.z*600, GREEN);
            }
        }

        DrawText(TextFormat("x: %f, y: %f, z: %f", orgin.x, orgin.y, orgin.z),0,0,16,YELLOW);
        DrawText(TextFormat("yaw: %f, pitch: %f", angle_h, angle_v),0,16,16,YELLOW);
        DrawFPS(0, 32);

        EndDrawing();

    }
}

void create_otree(OctTree& otree){
    Color clrs[11] = {RED,GREEN,BLUE,PINK,YELLOW,PURPLE,WHITE, DARKBLUE, DARKGREEN, DARKPURPLE, BROWN};
    int stepsz = -4;
    double step = std::pow(2.0,stepsz);
    for(double ix = 18; ix <= 32; ix+=step){
    for(double iy = 18; iy <= 32; iy+=step){
    for(double iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos + Vec3(-15,0,0), stepsz);

    }
    }
    }
    for(double ix = 18; ix <= 25; ix+=step){
    for(double iy = 18; iy <= 32; iy+=step){
    for(double iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, stepsz);
        
    }
    }
    }
    for(double ix = 25; ix <= 32; ix+=1){
    for(double iy = 18; iy <= 32; iy+=1){
    for(double iz = 18; iz <= 32; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    for(double ix = 8; ix <= 22; ix+=1){
    for(double iy = 8; iy <= 22; iy+=1){
    for(double iz = 8; iz <= 22; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(15,15,15))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    double ps1 = 0;
    double ps2 = 0;
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
