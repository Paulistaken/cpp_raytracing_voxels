#version 430
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct PixelData{
    vec4 col;
    float deph;
};
struct ScreenData{
    PixelData pixels[60][60];
};
layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };

void main()
{
    uint ix = gl_GlobalInvocationID.x;
    uint iy = gl_GlobalInvocationID.y;
    screen_data.pixels[iy][ix].deph = 1000000;
    screen_data.pixels[iy][ix].col = vec4(0.2,0.55,0.7,1.0);
}
