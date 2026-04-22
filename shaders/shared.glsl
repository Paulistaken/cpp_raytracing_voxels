
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

struct OctTreeSer{
    vec4 pos;
    vec4 orgin;
    vec4 angle;
    int size;
};
struct OctTreeNodeSer {
    int size;
    int filled_r;
    int filled_g;
    int filled_b;
    int filled_a;
    int kids[8];
};

struct CameraData{
    vec4 orgin;
    vec4 angle;
};
