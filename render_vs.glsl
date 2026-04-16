#version 430

struct PixelData{
    vec4 col;
    float deph;
};
struct ScreenData{
    PixelData pixels[60][60];
}

layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };

in vec4 fragColor;
out vec4 finalColor;

void main()
{
    uint id = gl_InstanceID;
    if (id == 0){
        gl_Position.x = 0;
        gl_Position.y = 0;
        return;
    }
    if (id == 1){
        gl_Position.x = 1;
        gl_Position.y = 1;
        return;
    }
    if (id == 2){
        gl_Position.x = 0;
        gl_Position.y = 1;
        return;
    }

    // There is only one thing to do.
    // finalColor = fragColor;
}
