
#include "render_shader.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <raylib.h>
#include "vox_render.hpp"

#include "dtypes.hpp"
#include "otree/otree.hpp"
#include "otree/otree_ray.hpp"
#include <optional>
#include <utility>
#include <vector>


const bool DO_LIGHT = true;


const u32 SCREEN_WIDTH = 600;
const u32 SCREEN_HEIGTH = SCREEN_WIDTH;
const u32 VIR_REZ = VREZ;

const f64 MOVESPEED = 0.5;
const f64 ROTSPEED = 0.1;

using namespace::OCTTree;

typedef DT3::Vec3 Vec3;
typedef DT3::Transform3 Transform3;
typedef DTMat::Mat3 Mat3;

void create_otree(OctTree& otree);
void generate_sphere(OctTree& octree, const Vec3& center, f64 size, i32 voxelsize,const std::vector<Color>& fill);
void render_camera_view(const OctTree& otree, const Transform3& cam);
void insert_blocks(OctTree& otree, RenderShader& rnd, OctTree& otree_preview, const Transform3& cam, const std::vector<Color>& build_colors, u32 select_build_color, f64 build_size, i32 build_vx);

int main(){
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "ray");

    Vox_Rend::Screen screen = Vox_Rend::Screen(SCREEN_WIDTH,SCREEN_HEIGTH,VIR_REZ);

    RenderShader rnd = RenderShader(
            "shaders/otree.glsl",
            "shaders/reset_screen.glsl",
            "shaders/ligth.glsl",
            "shaders/ligth_rs.glsl"
            );

    std::vector<Color> build_colors = {WHITE,RED,GREEN,BLUE,PURPLE, Color{255,255,255,100}, Color{255,0,0,100}, Color{0,255,0,100}, Color{0,0,255,100}};
    u32 select_build_color = 0;

    i32 build_vx_size = 0;
    f64 build_size = std::pow(2.0,build_vx_size)-0.1;

    //128x128x128
    OctTree otree(7);
    OctTree otree2(6);
    OctTree otree_preview(7);

    create_otree(otree);

    generate_sphere(otree2, Vec3(10,10,10), 1, -2, {WHITE});
    generate_sphere(otree2, Vec3(10,10,5), 0.5, -5, {DARKBLUE,DARKGREEN,BLUE,GREEN});
    otree2.orgin = Vec3(10,10,10);

    otree.optimize();
    otree2.optimize();

    SetTargetFPS(60);

    Transform3 cam = Transform3({35,25,60},{0.,PI,0.});
    Vec3 mov_velocity = {0,0,0};
    
    bool show_preview = true;

    f64 o2yaw = 0.0;
    f64 o2yawo = 0.0;

    rnd.add_tree_buffer(otree);
    rnd.add_tree_buffer(otree2);
    rnd.add_tree_buffer(otree_preview);

    rnd.load_screen(screen);

    Vec3 lamp_location = Vec3(10,10,10);

    while(!WindowShouldClose()){

        rnd.update_tree_buffer_data(0, otree);
        rnd.update_tree_buffer_data(1, otree2);

        lamp_location = Vec3(25,25,25) + DTMat::from_euler_angles(Vec3(0,o2yaw,0)) * Vec3(0,0,30);
        otree2.position = lamp_location;
        otree2.angle = Vec3(0,o2yawo,0);
        o2yaw += 0.0125;
        o2yawo+= 0.073;
        ClearBackground(Color{20,125,200,255});

        auto mouse_pos = GetMousePosition();

        DisableCursor();
        EnableCursor();
        HideCursor();


        BeginDrawing();


        rnd.reset_screen(Color{20,125,200,255});

        rnd.load_camera(cam.pos, cam.euler_angle);

        if (DO_LIGHT) {
            rnd.reset_light(0, 0.0);
            rnd.run_light(0, Vec3(35,25,60), 100.0, 1.0, GetRandomValue(50,70));
            rnd.run_light(0, cam.pos, 25.0, 0.25, GetRandomValue(50, 70));
            rnd.run_light(0, lamp_location + Vec3(10,10,10), 200.0, 1.0, GetRandomValue(50, 70));
        }

        rnd.run_raytracing(0);
        rnd.run_raytracing(1);
        if (show_preview) rnd.run_raytracing(2);
        if (show_preview) insert_blocks(otree, rnd, otree_preview, cam, build_colors, select_build_color, build_size, build_vx_size);

        rnd.render_screen(screen);

        screen.__cpu__render_scr();

        Vec3 move_axis = Vec3(IsKeyDown(KEY_D)-IsKeyDown(KEY_A),-IsKeyDown(KEY_LEFT_SHIFT)+IsKeyDown(KEY_SPACE),IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*MOVESPEED*(IsKeyDown(KEY_E) ? 0.2 : 1);
        Vec3 move_dir = DTMat::from_euler_angles(Vec3(0,cam.euler_angle.y,0))*move_axis;

        mov_velocity = mov_velocity * (IsKeyDown(KEY_E) ? 0.5 : 0.8);

        if (move_dir.dist(Vec3(0,0,0))>=0.00001){
            mov_velocity = move_dir;
        }
        cam.pos += mov_velocity;

        DrawText(TextFormat("x: %f, y: %f, z: %f", cam.pos.x, cam.pos.y, cam.pos.z),0,0,16,YELLOW);
        DrawText(TextFormat("yaw: %f, pitch: %f", cam.euler_angle.y, cam.euler_angle.x),0,16,16,YELLOW);
        DrawFPS(0, 48);

        for (int i = 0; i < build_colors.size(); i++){
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

        show_preview = IsKeyPressed(KEY_P) ? !show_preview : show_preview;

        select_build_color = (select_build_color + IsKeyPressed(KEY_O))%build_colors.size();

        select_build_color = IsKeyPressed(KEY_ONE) ? 0 : select_build_color;
        select_build_color = IsKeyPressed(KEY_TWO) ? 1 : select_build_color;
        select_build_color = IsKeyPressed(KEY_THREE) ? 2 : select_build_color;
        select_build_color = IsKeyPressed(KEY_FOUR) ? 3 : select_build_color;
        select_build_color = IsKeyPressed(KEY_FIVE) ? 4 : select_build_color;
        select_build_color = IsKeyPressed(KEY_SIX) ? 5 : select_build_color;
        select_build_color = IsKeyPressed(KEY_SEVEN) ? 6 : select_build_color;

        build_size += (IsKeyPressed(KEY_I)-IsKeyPressed(KEY_K))*0.1;
        build_vx_size += IsKeyPressed(KEY_U)-IsKeyPressed(KEY_J);

    }
}

void rm_sphere(OctTree& otree, const Vec3& center, f64 size, i32 voxelsize){
    f64 step = std::pow(2.0,voxelsize);
    for (f64 px = center.x - size;px <= center.x + size; px += step){
        for (f64 py = center.y - size;py <= center.y + size; py += step){
            for (f64 pz = center.z - size;pz <= center.z + size; pz += step){
                if (Vec3(px,py,pz).dist(center)>size) continue;
                otree.remove_node(Vec3(px,py,pz), voxelsize);
            }
        }
    }
}
void generate_sphere(OctTree& otree, const Vec3& center, f64 size, i32 voxelsize, const std::vector<Color>& fill){
    f64 step = std::pow(2.0,voxelsize);
    for (f64 px = center.x - size;px <= center.x + size; px += step){
        for (f64 py = center.y - size;py <= center.y + size; py += step){
            for (f64 pz = center.z - size;pz <= center.z + size; pz += step){
                if (Vec3(px,py,pz).dist(center)>size) continue;
                if (Vec3(px,py,pz).dist(center)<size-step*2) continue;
                u32 rsz = GetRandomValue(0, fill.size()-1);
                otree.insert_node(fill[rsz], Vec3(px,py,pz), voxelsize);
            }
        }
    }
}

void insert_blocks(
        OctTree& otree, 
        RenderShader& rnd,
        OctTree& otree_preview, 
        const Transform3& cam, 
        const std::vector<Color>& build_colors, 
        u32 select_build_color, 
        f64 build_size, 
        i32 vx) {
    Vec3 cam_forward = cam.get_forward();
    auto forward_ray = OCTTree::OCTRay::OCTRay(cam.pos,cam.get_forward()).cpu_send_ray(otree, {});
    Vec3 insert_pos;
    Vec3 insert_map_pos;
    if (forward_ray.has_value()){
        auto [ray_pos,map_pos,ray_v] = forward_ray.value();
        insert_pos = ray_pos;
        insert_map_pos = map_pos;
    }else {
        insert_pos = cam.pos + cam_forward * 20;
        insert_map_pos = insert_pos;
    }
    otree_preview.clear();
    if (IsKeyPressed(KEY_RIGHT_CONTROL) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        insert_pos -= (cam_forward * (build_size/2));
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
        otree.optimize();
        rnd.update_tree_buffer(0, otree);
        
    }else if (IsKeyPressed(KEY_M) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        rm_sphere(otree, insert_map_pos, build_size, vx);
        otree.optimize();
        rnd.update_tree_buffer(0, otree);
    }else{
        insert_pos -= (cam_forward * (build_size/2));
        Color clr1 = build_colors[select_build_color];
        Color clr2 = build_colors[select_build_color];
        Color clr3 = build_colors[select_build_color];
        clr1.a = 100;
        clr2.a = 50;
        clr3.a = 150;
        i32 p_vx = std::max(vx,-2);
        f64 p_buildsize = std::max(std::pow(2.0,-2),build_size);
        generate_sphere(otree_preview, insert_pos, p_buildsize, p_vx, {clr1,clr2,clr3});
        otree_preview.optimize();
        rnd.update_tree_buffer(2, otree_preview);
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
        otree.insert_node(clrs[rind], cpos+Vec3(-15,0,0), stepsz);

    }
    }
    }
    for(f64 ix = 18; ix <= 25; ix+=step){
    for(f64 iy = 18; iy <= 32; iy+=step){
    for(f64 iz = 18; iz <= 32; iz+=step){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree.insert_node(clrs[rind], cpos, stepsz);
        
    }
    }
    }
    for(f64 ix = 25; ix <= 32; ix+=1){
    for(f64 iy = 18; iy <= 32; iy+=1){
    for(f64 iz = 18; iz <= 32; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(25,25,25))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree.insert_node(clrs[rind], cpos, 0);
        
    }
    }
    }

    for(f64 ix = 8; ix <= 22; ix+=1){
    for(f64 iy = 8; iy <= 22; iy+=1){
    for(f64 iz = 8; iz <= 22; iz+=1){
        Vec3 cpos = Vec3(ix,iy,iz);
        if (cpos.dist(Vec3(15,15,15))>5)continue;
        int rind = GetRandomValue(0, 10);
        otree.insert_node(clrs[rind], cpos, 0);
        
    }
    }
    }

    f64 ps1 = 0;
    f64 ps2 = 0;
    for(int i = 0; i <= 10; i++){
        int sz = 3-i;
        int rind = GetRandomValue(0, 10);
        otree.insert_node(clrs[rind], Vec3(35+ps1,25,50), sz);
        rind = GetRandomValue(0, 10);
        otree.insert_node(clrs[rind], Vec3(35+ps2,25,50), sz);
        ps1 += std::pow(2.0,sz);
        ps2 -= std::pow(2.0,sz-1);
    }
}
