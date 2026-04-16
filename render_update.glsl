#version 430
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct PixelData{
    vec4 col;
    float deph;
};
struct ScreenData{
    PixelData pixels[60][60];
}


layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };
layout(std430, binding=1) buffer ssbo1 { ScreenData screen_data2; };

void main(){
    uint ix = gl_GlobalInvocationID.x;
    uint iy = gl_GlobalInvocationID.y;
    if (screen_data.pixels[iy][ix].deph > screen_data2.pixels[iy][ix].deph){
        screen_data.pixels[iy][ix].deph = 
            screen_data2.pixels[iy][ix].deph;
        screen_data.pixels[iy][ix].col = 
            screen_data2.pixels[iy][ix].col;
    }
}
