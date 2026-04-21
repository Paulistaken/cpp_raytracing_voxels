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


typedef struct{
    float deph;
    u32 r;
    u32 g;
    u32 b;
    u32 a;
} PixelData;

const i32 VREZ = 200;
typedef struct{
    PixelData pixels[VREZ][VREZ];
} ScreenData;

typedef struct{
    Vector4 orgin;
    Vector4 angle;
} CamData;

class RenderShader{
    public:
    RenderShader(
            const char* otree_shader,
            const char* reset_shader);

    u32 add_tree_buffer(
            const OCTTree::OctTree& otree);
    void update_tree_buffer(
            const u32 index,
            const OCTTree::OctTree& otree);
    void update_tree_buffer_data(
            const u32 index,
            const OCTTree::OctTree& otree);
    void load_screen(
           const Vox_Rend::Screen& scr
            );
    void reset_screen(Color rst_clr);
    void render_screen(
            Vox_Rend::Screen& scr
            );
    void run_raytracing(
            const u32 index,
            const DT3::Vec3 orgin,
            const DT3::Vec3 dir
            );

    private:
    int otree_shader;
    int otree_shader_p;
    int rst_shader;
    int rst_shader_p;
    int ssbo_screen_data;
    std::vector<std::tuple<int,int>> ssbo_nodes;
    int ssbo_cam;
    int vao;
    // Texture2D text;
};

#endif // !RENDERSHADER
       //
