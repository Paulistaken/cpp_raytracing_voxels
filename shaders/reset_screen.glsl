#version 430
layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

const int VREZ = 200;
struct PixelData{
    float deph;
    uint r;
    uint g;
    uint b;
    uint a;
};
struct ScreenData{
    PixelData pixels[VREZ][VREZ];
};
layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };
layout(std430, binding=1) buffer ssbo1 { uint reset_clr[]; };

void main(){
    uint ix = gl_GlobalInvocationID.x;
    uint iy = gl_GlobalInvocationID.y;
    screen_data.pixels[iy][ix].deph = -1;
    screen_data.pixels[iy][ix].r=reset_clr[0];
    screen_data.pixels[iy][ix].g=reset_clr[1];
    screen_data.pixels[iy][ix].b=reset_clr[2];
    screen_data.pixels[iy][ix].a=reset_clr[3];
}
