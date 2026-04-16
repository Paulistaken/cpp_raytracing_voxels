#pragma once
#include "raylib.h"
#include "rlgl.h"
#include "stdlib.h"
#include "vox_render.hpp"
#include <iostream>


typedef struct{
    Vector4 color;
    float deph;
} PixelData;

typedef struct{
    PixelData pixels[60][60];
} ScreenData;


class RenderShader{
    public:
    RenderShader(const char* vs, const char* fs,const char* upd, const char* rst);
    void load_screen_data(const Vox_Rend::Screen& scr);
    void run_screen_reset();
    void run_screen_render();
    void run_screen_update(const Vox_Rend::Screen& scr2);
    private:
    Shader fs_shader;
    int update_shader;
    int reset_shader;
    int ssbo0;
    int ssbo1;
    int vao;
};

RenderShader::RenderShader(const char* vs,const char* fs, const char* cls, const char* rst){
#ifdef GRAPHICS_API_OPENGL_43
    std::cout<<"YES!";

#endif // GRAPHICS_API_OPENGL_43
    char* rst_code = LoadFileText(rst);
    // std::cout<<rst_code<<"\n";
    std::cout<<"glsl:"<<rlGlVersion()<<"\n";
    std::cout<<"glsl:"<<rlGetVersion()<<"\n";
    UnloadFileText(rst_code);
}
void RenderShader::load_screen_data(const Vox_Rend::Screen& scr){
    ScreenData shader_scr;
    for(int y = 0; y < 60; y++){
        for(int x = 0; x < 60; x++){
            const auto& pixel = scr.pixels[y][x];
            auto& pxl = shader_scr.pixels[y][x];
            pxl.deph = pixel.deph;
            pxl.color = Vector4{(float)pixel.col.r / (float)255.0,(float)pixel.col.g / (float)255.0, (float)pixel.col.b / (float)255.0, 0.0};
        }
    }
    this->ssbo0 = rlLoadShaderBuffer(sizeof(shader_scr), &shader_scr, RL_DYNAMIC_COPY);
    this->ssbo1 = rlLoadShaderBuffer(sizeof(shader_scr), &shader_scr, RL_DYNAMIC_COPY);
    this->vao = rlLoadVertexArray();
    rlEnableVertexArray(vao);
}
void RenderShader::run_screen_reset(){
    rlEnableShader(this->reset_shader);
    rlBindShaderBuffer(this->ssbo0, 0);
    rlComputeShaderDispatch(60, 60, 1);
    rlDisableShader();
}
void RenderShader::run_screen_update(const Vox_Rend::Screen& scr2){
    ScreenData shader_scr;
    for(int y = 0; y < 60; y++){
        for(int x = 0; x < 60; x++){
            const auto& pixel = scr2.pixels[y][x];
            auto& pxl = shader_scr.pixels[y][x];
            pxl.deph = pixel.deph;
            pxl.color = Vector4{(float)pixel.col.r / (float)255.0,(float)pixel.col.g / (float)255.0, (float)pixel.col.b / (float)255.0, 0.0};
        }
    }
    rlUpdateShaderBuffer(this->ssbo1, &shader_scr, sizeof(shader_scr), 0);
    rlEnableShader(this->update_shader);
    rlBindShaderBuffer(this->ssbo0, 0);
    rlBindShaderBuffer(this->ssbo1, 1);
    rlComputeShaderDispatch(60, 60, 1);
    rlDisableShader();
}
void RenderShader::run_screen_render(){
    rlEnableShader(this->fs_shader.id);
    rlBindShaderBuffer(this->ssbo0, 0);

    rlEnableVertexArray(this->vao);
    rlDrawVertexArrayInstanced(0, 3, 6);
    rlDisableVertexArray();

    rlDisableShader();

}
