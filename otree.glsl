#version 430
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct OctTreeNodeSer {
    int size;
    uint filled;
    int kids[8];
};


layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };
layout(std430, binding=1) buffer ssbo1 { OctTreeNodeSer nodes[]; };
layout(std430, binding=2) buffer ssbo2 { uint node_n; };

layout(std430, binding=3) buffer ssbo3 { vec4 orgin; };
layout(std430, binding=4) buffer ssbo4 { vec4 dir; };


uint is_pos_filled(vec3 poss){
    vec3 pos = poss;
    vec3 c_pos = vec3(0,0,0);
    uint indx = 0;
    float psize = pow(2,nodes[0].size);
    if(pos.x > psize || pos.y > psize || pos.z > psize){
        return 0;
    }
    while(true){
        psize = pow(2,nodes[indx].size);
        if (nodes[indx].filled==1){
            return 1;
        }
        bool ptx = !(pos.x < c_pos.x + (psize / 2));
        bool pty = !(pos.y < c_pos.y + (psize / 2));
        bool ptz = !(pos.z < c_pos.z + (psize / 2));
        uint id = ptx + pty*2 + ptz*4;
        c_pos += vec3(ptx,pty,ptz) * (psize / 2);
        if (nodes[indx].kids[id]==-1){
            break;
        }
        indx = nodes[indx].kids[id];
    }
    return 0;
}

void main(){

}
