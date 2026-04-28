
#include "render_shader.hpp"

#include <algorithm>
#include <cmath>
#include <raylib.h>
#include "vox_render.hpp"

#include "dtypes.hpp"
#include "otree/otree.hpp"
#include "otree/otree_ray.hpp"
#include <optional>
#include <vector>
#include "game_data.hpp"


const bool DO_LIGHT_RAYTRACING = true;
const u32 SCREEN_WIDTH = 600;
const u32 SCREEN_HEIGTH = SCREEN_WIDTH;
const u32 VIR_REZ = VREZ;
const f64 MOVESPEED = 0.5;
const f64 ROTSPEED = 0.1;
const u32 LIGHT_UPDATE_TICK = 10;

using namespace::OCTTree;

typedef DT3::Vec3 Vec3;
typedef DT3::Transform3 Transform3;
typedef DTMat::Mat3 Mat3;
typedef GameTypes::LightSource LightSource;

typedef struct {
    bool do_light;
    bool do_flashlight;
} LightSettings;
typedef struct{
    bool do_build;
    u32 select_build_color;
    i32 build_detail_size;
    f32 build_size;
} BuildSettings;

void create_otree(OctTree& otree);
void generate_sphere(OctTree& octree, const Vec3& center, f64 size, i32 voxelsize,const std::vector<Color>& fill);
void render_camera_view(const OctTree& otree, const Transform3& cam);
void insert_blocks(OctTree& otree, RenderShader& rnd, OctTree& otree_preview, const Transform3& cam, const std::vector<Color>& build_colors, u32 select_build_color, f64 build_size, i32 build_vx);

int main(){

    LightSettings light_settings = LightSettings{DO_LIGHT_RAYTRACING,true};
    BuildSettings build_settings = BuildSettings{true,0,0,0.9};

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "ray");

    Vox_Rend::Screen screen = Vox_Rend::Screen(SCREEN_WIDTH,SCREEN_HEIGTH,VIR_REZ);

    RenderShader rnd = RenderShader(
            "shaders/otree.glsl",
            "shaders/reset_screen.glsl",
            "shaders/ligth.glsl",
            "shaders/ligth_rs.glsl"
            );

    std::vector<Color> build_colors = {WHITE,RED,GREEN,BLUE,PURPLE, Color{255,255,255,170}, Color{255,0,0,170}, Color{0,255,0,170}, Color{0,0,255,170}};


    //128x128x128
    OctTree otree(7);
    OctTree otree_sun(6);
    OctTree otree_preview(7);

    OctTree otree_mars(6);

    create_otree(otree);

    generate_sphere(otree_sun, Vec3(10,10,10), 3, -2, {WHITE});
    generate_sphere(otree_sun, Vec3(10,10,5), 1, -5, {DARKBLUE,DARKGREEN,BLUE,GREEN});
    generate_sphere(otree_mars, Vec3(10,10,20), 0.5, -5, {RED,ORANGE,BROWN,WHITE,RED,BROWN,RED,RED});
    otree_sun.orgin = Vec3(10,10,10);
    otree_mars.orgin = Vec3(10,10,10);

    otree.optimize();
    otree_sun.optimize();

    SetTargetFPS(60);

    Transform3 cam = Transform3({35,25,60},{0.,PI,0.});
    Vec3 mov_velocity = {0,0,0};
    

    f64 o2yaw = 0.0;
    f64 o2yawo = 0.0;
    f64 o2yawmars = 0.0;

    rnd.add_tree_buffer(otree);
    rnd.add_tree_buffer(otree_sun);
    rnd.add_tree_buffer(otree_preview);
    rnd.add_tree_buffer(otree_mars);

    rnd.load_screen_data(screen);

    // LightSource light_flash = LightSource{Vec3(),Vec3(),Vec3(1,0.9,0.1),25,1,100};
    LightSource light_flash = LightSource{Vec3(),Vec3(),Vec3(1,1,1),25,1,100};
    LightSource light_sun = LightSource{Vec3(),Vec3(),Vec3(1,1,1),100,1,500};

    light_sun.orgin = otree_sun.orgin;

    Vec3 lamp_location = Vec3(10,10,10);

    u32 light_tick = 0;

    while(!WindowShouldClose()){

        rnd.update_tree_buffer_data(0, otree);
        rnd.update_tree_buffer_data(1, otree_sun);
        rnd.update_tree_buffer_data(3, otree_mars);

        lamp_location = Vec3(25,25,25) + DTMat::from_euler_angles(Vec3(0,o2yaw,0)) * Vec3(0,0,30);
        otree_sun.position = lamp_location;
        otree_sun.angle = Vec3(0,o2yawo,0);
        otree_mars.position = otree_sun.position;
        otree_mars.angle = Vec3(0,o2yawmars,0);
        o2yaw += 0.0125;
        o2yawo+= 0.073;
        o2yawmars-= 0.1;
        ClearBackground(Color{20,125,200,255});

        auto mouse_pos = GetMousePosition();

        DisableCursor();
        EnableCursor();
        HideCursor();


        BeginDrawing();


        rnd.reset_screen_data(Color{20,125,200,255});

        rnd.load_camera_data(cam);

        if (IsKeyPressed(KEY_G)){
            light_settings.do_light = !light_settings.do_light;
            rnd.reset_octtree_light(0, 1.0);
        }
        if (IsKeyPressed(KEY_F)) light_settings.do_flashlight = !light_settings.do_flashlight;

        light_tick += 1;
        if (light_settings.do_light && light_tick >= LIGHT_UPDATE_TICK) {
            light_tick=0;
            light_flash.pos = cam.pos;
            light_sun.pos = otree_sun.position;
            rnd.reset_octtree_light(0, 0.0);
            if (light_settings.do_flashlight) {
                light_flash.render(rnd, 0);
                // rnd.run_light(0, cam.pos, 25.0, 0.25, GetRandomValue(50, 100),Vec3(1,0.9,0.1));
            }
            light_sun.render(rnd, 0);
            // rnd.run_light(0, lamp_location + Vec3(10,10,10), 200.0, 1.0, GetRandomValue(50, 100),Vec3(1,1,1));
        }

        rnd.run_raytracing(0);
        rnd.run_raytracing(1);
        rnd.run_raytracing(3);
        if (build_settings.do_build) rnd.run_raytracing(2);
        if (build_settings.do_build) 
            insert_blocks(otree, rnd, otree_preview, cam, build_colors, 
                    build_settings.select_build_color, build_settings.build_size, build_settings.build_detail_size);

        rnd.render_screen(screen);

        screen.__cpu__render_scr();

        Vec3 move_axis = Vec3(IsKeyDown(KEY_D)-IsKeyDown(KEY_A),-IsKeyDown(KEY_LEFT_SHIFT)+IsKeyDown(KEY_SPACE),IsKeyDown(KEY_W)-IsKeyDown(KEY_S))*MOVESPEED*(IsKeyDown(KEY_E) ? 0.2 : 1);
        Vec3 move_dir = DTMat::from_euler_angles(Vec3(0,cam.euler_angle.y,0))*move_axis;

        mov_velocity = mov_velocity * (IsKeyDown(KEY_E) ? 0.5 : 0.8);

        if (move_dir.dist(Vec3(0,0,0))>=0.00001){
            mov_velocity = move_dir;
        }
        cam.pos += mov_velocity;

        DrawRectangle(0, SCREEN_HEIGTH-68, build_colors.size()*32+8, 68, BLACK);
        DrawRectangle(0, 0, 300, 48+8, BLACK);

        DrawText(TextFormat("x: %f, y: %f, z: %f", cam.pos.x, cam.pos.y, cam.pos.z),0,0,16,YELLOW);
        DrawText(TextFormat("yaw: %f, pitch: %f", cam.euler_angle.y, cam.euler_angle.x),0,16,16,YELLOW);
        DrawFPS(0, 32);

        for (int i = 0; i < build_colors.size(); i++){
            if (build_settings.select_build_color == i) DrawRectangle(i*32, SCREEN_HEIGTH-32, 32, 32, BLACK);
            DrawRectangle(i*32+4, SCREEN_HEIGTH-32+4, 24, 24, build_colors[i]);
        }
        DrawText(TextFormat("Build size: %f",build_settings.build_size), 0, SCREEN_HEIGTH-64, 16, YELLOW);
        DrawText(TextFormat("Build detail: 2^%d",build_settings.build_detail_size), 0, SCREEN_HEIGTH-48, 16, YELLOW);

        EndDrawing();

        f64 mpx = (mouse_pos.x - SCREEN_WIDTH / 2.0) / SCREEN_WIDTH;
        f64 mpy = (mouse_pos.y - SCREEN_HEIGTH / 2.0) / SCREEN_HEIGTH;
        cam.euler_angle.y += mpx*15.0*ROTSPEED;
        cam.euler_angle.x += mpy*15.0*ROTSPEED;

        cam.euler_angle.y += (-IsKeyDown(KEY_LEFT) + IsKeyDown(KEY_RIGHT)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));
        cam.euler_angle.x += (-IsKeyDown(KEY_UP) + IsKeyDown(KEY_DOWN)) * ROTSPEED * (1.0 / (IsKeyDown(KEY_E)*4.0+1.0));

		if (IsKeyPressed(KEY_P)) build_settings.do_build = !build_settings.do_build;

        if (IsKeyPressed(KEY_O)){
            build_settings.select_build_color = (build_settings.select_build_color+1)%build_colors.size();
        }

        u32 sbc = 0;
        if (IsKeyPressed(KEY_ONE)) sbc = 1;
		if (IsKeyPressed(KEY_TWO)) sbc = 2;
		if (IsKeyPressed(KEY_THREE)) sbc = 3;
		if (IsKeyPressed(KEY_FOUR)) sbc = 4;
		if (IsKeyPressed(KEY_FIVE)) sbc = 5;
		if (IsKeyPressed(KEY_SIX)) sbc = 6;
		if (IsKeyPressed(KEY_SEVEN)) sbc = 7;
        if (sbc > 0) build_settings.select_build_color = sbc-1;

        build_settings.build_size += (IsKeyPressed(KEY_I)-IsKeyPressed(KEY_K))*0.1;
        build_settings.build_detail_size += IsKeyPressed(KEY_U)-IsKeyPressed(KEY_J);

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
        rnd.update_tree_buffer_voxels(0, otree);
        
    }else if (IsKeyPressed(KEY_M) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        insert_pos -= (cam_forward * (build_size/2));
        rm_sphere(otree, insert_map_pos, build_size, vx);
        otree.optimize();
        rnd.update_tree_buffer_voxels(0, otree);
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
        rnd.update_tree_buffer_voxels(2, otree_preview);
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
