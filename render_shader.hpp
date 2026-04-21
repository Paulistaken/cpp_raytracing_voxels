#pragma once
#include <algorithm>
#include <raylib.h>
#include <rlgl.h>
#include "otree.hpp"
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

void unser_screen_data(Vox_Rend::Screen& scr, const ScreenData& shader_scr){
    for(int y = 0; y < VREZ; y++){
        for(int x = 0; x < VREZ; x++){
            auto& pxl = scr.pixels[y][x];
            const auto& pixel = shader_scr.pixels[y][x];
            pxl.deph = pixel.deph;
            pxl.col.r = (u8)(pixel.r);
            pxl.col.g = (u8)(pixel.g);
            pxl.col.b = (u8)(pixel.b);
            pxl.col.a = (u8)(pixel.a);
        }
    }
}
ScreenData get_screen_data_ser(const Vox_Rend::Screen& scr){
    ScreenData shader_scr;
    for(int y = 0; y < VREZ; y++){
        for(int x = 0; x < VREZ; x++){
            const auto& pixel = scr.pixels[y][x];
            auto& pxl = shader_scr.pixels[y][x];
            pxl.deph = pixel.deph;
            pxl.r = pixel.col.r;
            pxl.g = pixel.col.g;
            pxl.b = pixel.col.b;
            pxl.a = pixel.col.a;
        }
    }
    return shader_scr;
}

class RenderShader{
    public:
    RenderShader(const char* otree_shader);

    u32 add_tree_buffer(
            const OCTTree::OctTree& otree);
    void update_tree_buffer(
            const u32 index,
            const OCTTree::OctTree& otree);
    void run_raytracing(
            Vox_Rend::Screen& scr,
            const u32 index,
            const DT3::Vec3 orgin,
            const DT3::Vec3 dir
            );

    private:
    int otree_shader;
    int otree_shader_p;
    int ssbo_screen_data;
    std::vector<std::tuple<int,int>> ssbo_nodes;
    int ssbo_cam;
    int vao;
    Texture2D text;
};

RenderShader::RenderShader(const char* os){
    char* shader_code = LoadFileText(os);
    this->otree_shader = rlLoadShader(shader_code, RL_COMPUTE_SHADER);
    this->otree_shader_p = rlLoadShaderProgramCompute(this->otree_shader);
    UnloadFileText(shader_code);
    this->text = LoadTextureFromImage(LoadImage("white.png"));
}
u32 RenderShader::add_tree_buffer(
        const OCTTree::OctTree& otree
        ){
    OCTTree::OctTreeSer otreeser = OCTTree::serialize_octtree(otree);
    i32 c_ssbo_nodes = rlLoadShaderBuffer(sizeof(OCTTree::OctTreeNodeSer) * otreeser.lengh, otreeser.nodes, RL_DYNAMIC_COPY);
    i32 c_ssbo_nodeN = rlLoadShaderBuffer(sizeof(otreeser.data), &otreeser.data, RL_DYNAMIC_COPY);
    free(otreeser.nodes);
    this->ssbo_nodes.push_back({c_ssbo_nodes,c_ssbo_nodeN});
    return this->ssbo_nodes.size() - 1;
}
void RenderShader::update_tree_buffer(
        const u32 index,
        const OCTTree::OctTree& otree
        ){
    OCTTree::OctTreeSer otreeser = OCTTree::serialize_octtree(otree);
    auto& [nodes, nodeN] = this->ssbo_nodes[index];
    rlUnloadShaderBuffer(nodes);
    rlUnloadShaderBuffer(nodeN);
    nodes = rlLoadShaderBuffer(sizeof(OCTTree::OctTreeNodeSer) * otreeser.lengh, otreeser.nodes, RL_DYNAMIC_COPY);
    nodeN = rlLoadShaderBuffer(sizeof(otreeser.data), &otreeser.data, RL_DYNAMIC_COPY);

    // rlUpdateShaderBuffer(nodes, otreeser.nodes, sizeof(OCTTree::OctTreeNodeSer)*otreeser.lengh, 0);
    // rlUpdateShaderBuffer(nodeN, &otreeser.lengh, sizeof(otreeser.lengh), 0);
    free(otreeser.nodes);
}
void RenderShader::run_raytracing(
        Vox_Rend::Screen& scr,
        const u32 index,
        const DT3::Vec3 orgin,
        const DT3::Vec3 dir
        ){
    auto [c_ssbo_nodes, c_ssbo_nodeN] = this->ssbo_nodes[index];
    Vector4 orgin_ser = Vector4{(f32)orgin.x, (f32)orgin.y,(f32)orgin.z,0.0};
    Vector4 dir_ser = Vector4{(f32)dir.x, (f32)dir.y,(f32)dir.z,0.0};
    CamData cam_data = CamData{orgin_ser,dir_ser};
    this->ssbo_cam = rlLoadShaderBuffer(sizeof(cam_data), &cam_data, RL_DYNAMIC_COPY);
    ScreenData shader_scr = get_screen_data_ser(scr);
    this->ssbo_screen_data = rlLoadShaderBuffer(sizeof(shader_scr), &shader_scr, RL_DYNAMIC_COPY);


    rlEnableShader(this->otree_shader_p);

    rlBindShaderBuffer(this->ssbo_screen_data, 0);
    rlBindShaderBuffer(c_ssbo_nodes, 1);
    rlBindShaderBuffer(c_ssbo_nodeN, 2);
    rlBindShaderBuffer(this->ssbo_cam, 3);

    rlComputeShaderDispatch(VREZ, VREZ, 1);

    rlReadShaderBuffer(this->ssbo_screen_data, &shader_scr, sizeof(ScreenData), 0);
    rlReadShaderBuffer(this->ssbo_screen_data, &shader_scr, sizeof(ScreenData), 0);
    unser_screen_data(scr, shader_scr);

    rlDisableShader();

    rlUnloadShaderBuffer(this->ssbo_screen_data);
    rlUnloadShaderBuffer(this->ssbo_cam);

}

#endif // !RENDERSHADER
       //
