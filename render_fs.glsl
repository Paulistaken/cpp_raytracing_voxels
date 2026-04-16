#version 430

// struct PixelData{
//     vec4 col;
//     float deph;
// };
// struct ScreenData{
//     PixelData pixels[60][60];
// };

// layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };

in vec4 fragColor;
out vec4 finalColor;

void main()
{
    // vec4 fgpos = gl_FragCoord;
    // float px = fgpos.x;
    // float py = fgpos.y;
    //
    // uint ix = (uint)(px / 10.0);
    // uint iy = (uint)(py / 10.0);
    //
    // finalColor = screen_data.pixels[iy][ix];

    // There is only one thing to do.
    // finalColor = fragColor;
}
