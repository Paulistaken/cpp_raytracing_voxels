#include "render_shader.hpp"

typedef struct{
    float deph;
    u32 r;
    u32 g;
    u32 b;
    u32 a;
} PixelData;
typedef struct{
    PixelData pixels[VREZ][VREZ];
} ScreenData;

typedef struct{
    Vector4 orgin;
    Vector4 angle;
} CamData;
typedef struct{
    Vector4 orgin;
    f32 str;
} LightData;

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
RenderShader::RenderShader(const char* os, const char* rs, const char* ls){
    char* shader_code = LoadFileText(os);
    char* r_shader_code = LoadFileText(rs);
    char* l_shader_code = LoadFileText(ls);
    this->otree_shader = rlLoadShader(shader_code, RL_COMPUTE_SHADER);
    this->otree_shader_p = rlLoadShaderProgramCompute(this->otree_shader);
    this->rst_shader = rlLoadShader(r_shader_code, RL_COMPUTE_SHADER);
    this->rst_shader_p = rlLoadShaderProgramCompute(this->rst_shader);
    this->lt_shader = rlLoadShader(l_shader_code, RL_COMPUTE_SHADER);
    this->lt_shader_p = rlLoadShaderProgramCompute(this->lt_shader);
    UnloadFileText(shader_code);
    UnloadFileText(r_shader_code);
    UnloadFileText(l_shader_code);
    this->ssbo_screen_data = -1;
    this->ssbo_cam = -1;
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
void RenderShader::update_tree_buffer_data(
        const u32 index,
        const OCTTree::OctTree& otree
        ){
    OCTTree::OctTreeDataSer otreeser = OCTTree::serialize_octree_data(otree);
    auto& [nodes, nodeN] = this->ssbo_nodes[index];
    rlUnloadShaderBuffer(nodeN);
    nodeN = rlLoadShaderBuffer(sizeof(otreeser), &otreeser, RL_DYNAMIC_COPY);
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
    free(otreeser.nodes);
}
void RenderShader::load_camera(
        const DT3::Vec3 orgin, const DT3::Vec3 dir
        ){
    if (this->ssbo_cam != -1){
        rlUnloadShaderBuffer(this->ssbo_cam);
    }
    Vector4 orgin_ser = Vector4{(f32)orgin.x, (f32)orgin.y,(f32)orgin.z,0.0};
    Vector4 dir_ser = Vector4{(f32)dir.x, (f32)dir.y,(f32)dir.z,0.0};
    CamData cam_data = CamData{orgin_ser,dir_ser};
    this->ssbo_cam = rlLoadShaderBuffer(sizeof(cam_data), &cam_data, RL_DYNAMIC_COPY);
}
void RenderShader::run_light(const u32 index, const DT3::Vec3 orgin, const f32 light_str){
    Vector4 orgin_ser = Vector4{(f32)orgin.x, (f32)orgin.y,(f32)orgin.z,0.0};
    LightData light_data = LightData{orgin_ser,light_str};
    i32 ssbo0_light = rlLoadShaderBuffer(sizeof(light_data), &light_data, RL_DYNAMIC_COPY);


    auto [c_ssbo_nodes, c_ssbo_nodeN] = this->ssbo_nodes[index];


    rlEnableShader(this->lt_shader_p);

    rlBindShaderBuffer(c_ssbo_nodes, 0);
    rlBindShaderBuffer(c_ssbo_nodeN, 1);
    rlBindShaderBuffer(ssbo0_light, 2);

    rlComputeShaderDispatch(720/20, 720/20, 1);

    rlDisableShader();
    
    rlUnloadShaderBuffer(ssbo0_light);
}
void RenderShader::load_screen(
        const Vox_Rend::Screen& scr
        ){
    if (this->ssbo_screen_data != -1){
        rlUnloadShaderBuffer(this->ssbo_screen_data);
    }
    ScreenData shader_scr = get_screen_data_ser(scr);
    this->ssbo_screen_data = rlLoadShaderBuffer(sizeof(shader_scr), &shader_scr, RL_DYNAMIC_COPY);
}
void RenderShader::render_screen(Vox_Rend::Screen& scr){
    ScreenData shader_scr;
    rlReadShaderBuffer(this->ssbo_screen_data, &shader_scr, sizeof(ScreenData), 0);
    unser_screen_data(scr, shader_scr);
}

void RenderShader::reset_screen(Color rst_clr){
    u32 reset_color[4] = {rst_clr.r,rst_clr.g,rst_clr.b,rst_clr.a};
    
    i32 clr_buf = rlLoadShaderBuffer(sizeof(reset_color), reset_color, RL_DYNAMIC_COPY);

    rlEnableShader(this->rst_shader_p);

    rlBindShaderBuffer(this->ssbo_screen_data, 0);
    rlBindShaderBuffer(clr_buf, 1);
    rlComputeShaderDispatch(VREZ/10, VREZ/10, 1);

    rlDisableShader();

    rlUnloadShaderBuffer(clr_buf);

}

void RenderShader::run_raytracing(
        const u32 index
        ){
    auto [c_ssbo_nodes, c_ssbo_nodeN] = this->ssbo_nodes[index];


    rlEnableShader(this->otree_shader_p);

    rlBindShaderBuffer(this->ssbo_screen_data, 0);
    rlBindShaderBuffer(c_ssbo_nodes, 1);
    rlBindShaderBuffer(c_ssbo_nodeN, 2);
    rlBindShaderBuffer(this->ssbo_cam, 3);

    rlComputeShaderDispatch(VREZ/20, VREZ/20, 1);

    rlDisableShader();
}
