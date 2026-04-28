#version 430

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct OctTreeSer{
    vec4 pos;
    vec4 orgin;
    vec4 angle;
    int size;
};
struct OctTreeNodeSer {
    int kids[8];
    int size;
    int filled_r;
    int filled_g;
    int filled_b;
    int filled_a;
    float light;
    float light_r;
    float light_g;
    float light_b;
    uint ref;
};


layout(std430, binding=0) buffer ssbo0 { OctTreeNodeSer nodes[]; };
layout(std430, binding=1) buffer ssbo1 { OctTreeSer node_data; };
layout(std430, binding=2) buffer ssbo2 { uint octree_size; };
layout(std430, binding=3) buffer ssbo3 { float light_level; };

void main(){
    uint id = gl_GlobalInvocationID.x;

    if (id >= octree_size) return;
    nodes[id].light = light_level;
    nodes[id].light_r = 1.0;
    nodes[id].light_g = 1.0;
    nodes[id].light_b = 1.0;
    if (nodes[id].ref >= 10){
        nodes[id].filled_r = 1;
        nodes[id].filled_g = 1;
        nodes[id].filled_b = 1;
    }
}
