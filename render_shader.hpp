#pragma once
#include <raylib.h>
#include <rlgl.h>
#include "otree/otree.hpp"
#include "stdlib.h"
#include "vox_render.hpp"
#include <tuple>
#include <vector>

#ifndef RENDERSHADER
#define RENDERSHADER



const i32 VREZ = 200;

class RenderShader{
    public:
    RenderShader(
            const char* raytracing_shader,
            const char* screen_reset_shader,
            const char* light_raytracing_shader,
            const char* light_reset_shader);

    //returns octtree's buffer index
    u32 add_tree_buffer(
            const OCTTree::OctTree& otree);
    void update_tree_buffer_voxels(
            const u32 tree_index,
            const OCTTree::OctTree& otree);
    void update_tree_buffer_data(
            const u32 tree_index,
            const OCTTree::OctTree& otree);
    void load_camera_data(const DT3::Transform3 cam_transform);
    void reset_octtree_light(const u32 tree_index, float light_level);
    void run_light_raytracing(const u32 tree_index, const DT3::Vec3 orgin, const f32 light_str, f32 ligh_dip, i32 raycount, DT3::Vec3 light_color);
    void load_screen_data(
           const Vox_Rend::Screen& screen
            );
    void reset_screen_data(Color background_color);
    void render_screen(
            Vox_Rend::Screen& screen
            );
    void run_raytracing(
            const u32 tree_index
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
};

#endif // !RENDERSHADER
       //
