#include <algorithm>
#include <cmath>
#include <iostream>
#include <raylib.h>

#include "dtypes.hpp"
#include "otree.hpp"
#include <optional>
#include <utility>

const double MOVESPEED = 0.5;
const double ROTSPEED = 0.1;

int main(){

    //64x64
    // QuadTree qtree(6);
    OctTree otree(7);


    Color clrs[7] = {RED,GREEN,BLUE,PINK,YELLOW,PURPLE,WHITE};
    int stepsz = -4;
    double step = std::pow(2.0,stepsz);
    for(double ix = 18; ix <= 32; ix+=step){
    for(double iy = 18; iy <= 32; iy+=step){
    for(double iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], cpos + Vec3(-15,0,0), stepsz);

    }
    }
    }
    for(double ix = 18; ix <= 25; ix+=step){
    for(double iy = 18; iy <= 32; iy+=step){
    for(double iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], cpos, stepsz);
        
    }
    }
    }
    for(double ix = 25; ix <= 32; ix+=1){
    for(double iy = 18; iy <= 32; iy+=1){
    for(double iz = 18; iz <= 32; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    for(double ix = 8; ix <= 22; ix+=1){
    for(double iy = 8; iy <= 22; iy+=1){
    for(double iz = 8; iz <= 22; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(15,15,15))>5)continue;
        int rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], cpos, 0);
        
    }
    }
    }

    double ps1 = 0;
    double ps2 = 0;
    for(int i = 0; i <= 10; i++){
        int sz = 3-i;
        int rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], Vec3(35+ps1,25,50), sz);
        rind = GetRandomValue(0, 6);
        otree_insert_node(otree, clrs[rind], Vec3(35+ps2,25,50), sz);
        ps1 += std::pow(2.0,sz);
        ps2 -= std::pow(2.0,sz-1);
    }

    InitWindow(600, 600, "ray");
    SetTargetFPS(60);

    Vec3 orgin = {35,25,60};
    double angle_h = PI;
    double angle_v = 0;
    
    bool show_map = false;

    while(!WindowShouldClose()){
        ClearBackground(BLACK);

        BeginDrawing();
        DrawText("Hello World", 16, 16, 32, RED);
        DrawText("Hello World", 16, 48, 32, WHITE);



        angle_h += (-IsKeyDown(KEY_LEFT) + IsKeyDown(KEY_RIGHT)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        angle_v += (-IsKeyDown(KEY_UP) + IsKeyDown(KEY_DOWN)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));

        for (int angv = -30; angv <= 30; angv++){
            for(int angh = -30; angh <= 30; angh++){
                double cam_h = angh / 180.0 * PI;
                double cam_v = angv / 180.0 * PI;

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
                    DrawRectangle((angh+30)*10,(angv+30)*10,10,10,cl);
                }
            }
        }

        double rot_h[9] = {std::cos(angle_h),0,std::sin(angle_h),0,1,0,-std::sin(angle_h),0,std::cos(angle_h)}; 
        Mat3 rot = Mat3(rot_h);
        Vec3 movedir = Vec3(IsKeyDown(KEY_D)-IsKeyDown(KEY_A),IsKeyDown(KEY_LEFT_SHIFT)-IsKeyDown(KEY_SPACE),IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*MOVESPEED;
        movedir = rot * movedir;
        orgin += movedir;

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

        EndDrawing();

    }
}
