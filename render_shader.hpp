#pragma once
#include <algorithm>
#include <raylib.h>
#include <rlgl.h>
#include "otree/otree.hpp"
#include "stdlib.h"
#include "vox_render.hpp"
#include <iostream>
#include <tuple>
#include <vector>

#ifndef RENDERSHADER
#define RENDERSHADER



const i32 VREZ = 200;

class RenderShader{
    public:
    RenderShader(
            const char* otree_shader,
            const char* reset_shader,
            const char* light_shader,
            const char* light_rs_shader);

    u32 add_tree_buffer(
            const OCTTree::OctTree& otree);
    void update_tree_buffer(
            const u32 index,
            const OCTTree::OctTree& otree);
    void update_tree_buffer_data(
            const u32 index,
            const OCTTree::OctTree& otree);
    void load_camera(const DT3::Vec3 orgin, const DT3::Vec3 dir);
    void reset_light(const u32 index, float light_level);
    void run_light(const u32 index, const DT3::Vec3 orgin, const f32 light_str, f32 ligh_dip, i32 raycount, DT3::Vec3 light_color);
    void load_screen(
           const Vox_Rend::Screen& scr
            );
    void reset_screen(Color rst_clr);
    void render_screen(
            Vox_Rend::Screen& scr
            );
    void run_raytracing(
            const u32 index
            );

    private:
    int otree_shader;
    int otree_shader_p;
    int rst_shader;
    int rst_shader_p;
    int lt_shader;
    int lt_shader_p;
    int ltr_shader;
    int ltr_shader_p;
    int ssbo_screen_data;
    std::vector<std::tuple<i32,i32,u32>> ssbo_nodes;
    int ssbo_cam;
    int vao;
    // Texture2D text;
};

#endif // !RENDERSHADER
       //
