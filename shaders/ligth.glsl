#version 430

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}
float random( float f ) {
    const uint mantissaMask = 0x007FFFFFu;
    const uint one          = 0x3F800000u;
   
    uint h = hash( floatBitsToUint( f ) );
    h &= mantissaMask;
    h |= one;
    
    float  r2 = uintBitsToFloat( h );
    return r2 - 1.0;
}

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
};

struct LightData{
    vec4 orgin;
    float strengh;
    float disp;
};


layout(std430, binding=0) buffer ssbo0 { OctTreeNodeSer nodes[]; };
layout(std430, binding=1) buffer ssbo1 { OctTreeSer node_data; };
layout(std430, binding=2) buffer ssbo2 { LightData lightsource; };

bool point_in_area(vec3 point, vec3 bg, vec3 en){
    if (point.x <= bg.x || point.y <= bg.y || point.z <= bg.z) return false;
    if (point.x >= en.x || point.y >= en.y || point.z >= en.z) return false;
    return true;
}

const int LightDetail = 0;

vec3 translate_ray(vec3 pos){
    mat3x3 cpitch = mat3x3(1,0,0,0,cos(-node_data.angle.x),sin(-node_data.angle.x),0,-sin(-node_data.angle.x),cos(-node_data.angle.x));
    mat3x3 cyaw = mat3x3(cos(-node_data.angle.y),0,-sin(-node_data.angle.y),0,1,0,sin(-node_data.angle.y),0,cos(-node_data.angle.y));
    mat3x3 crot = cyaw * cpitch;

    vec3 rpos = pos - node_data.pos.xyz - node_data.orgin.xyz;
    rpos = crot * rpos;
    rpos += node_data.orgin.xyz;
    return rpos;
}

void do_ray_tracing(vec3 pos, vec3 dir, float maxdist){
    float dist = 0;
    int qu_indx = 0;
    int qu_node[32];
    vec3 qu_pos[32];

    qu_node[qu_indx] = 0;
    qu_pos[qu_indx] = vec3(0,0,0);

    vec3 ray_pos = pos;

    mat3x3 cpitch = mat3x3(1,0,0,0,cos(node_data.angle.x),sin(node_data.angle.x),0,-sin(node_data.angle.x),cos(node_data.angle.x));
    mat3x3 cyaw = mat3x3(cos(-node_data.angle.y),0,-sin(-node_data.angle.y),0,1,0,sin(-node_data.angle.y),0,cos(-node_data.angle.y));
    mat3x3 crot = cyaw * cpitch;

    vec3 rpos1 = translate_ray(ray_pos);
    vec3 rpos2 = translate_ray(ray_pos + dir);

    ray_pos = rpos1;
    dir = rpos2 - rpos1;

    float apsize = pow(2,nodes[0].size);
    if (!point_in_area(ray_pos,vec3(0,0,0),vec3(apsize,apsize,apsize))){

        float bg = 0.1;
        float en = apsize - 0.1;

        float t = 1000000;
        float pt=t;
        if (dir.x>0 && ray_pos.x < bg) t = min(t, (bg-ray_pos.x)/dir.x);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;
        if (dir.x<0 && ray_pos.x > en) t = min(t, (en-ray_pos.x)/dir.x);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;
        if (dir.y>0 && ray_pos.y < bg) t = min(t, (bg-ray_pos.y)/dir.y);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;
        if (dir.z>0 && ray_pos.z < bg) t = min(t, (bg-ray_pos.z)/dir.z);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;
        if (dir.y<0 && ray_pos.y > en) t = min(t, (en-ray_pos.y)/dir.y);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;
        if (dir.z<0 && ray_pos.z > en) t = min(t, (en-ray_pos.z)/dir.z);
        pt=t;
        if(!point_in_area(ray_pos+dir*t,vec3(0,0,0),vec3(apsize,apsize,apsize))) t=pt;

        dist += t;
        ray_pos += dir * t;
    }

    while(true){
        if (dist >= maxdist){
            break;
        }

        if (qu_indx < 0) break;

        int c_indx = qu_node[qu_indx];
        vec3 c_map_pos = qu_pos[qu_indx];

        if (nodes[c_indx].size <= LightDetail){
            nodes[c_indx].light = max(nodes[c_indx].light, lightsource.strengh / (dist*dist*lightsource.disp));
            nodes[c_indx].light = min(nodes[c_indx].light,1.3);
        }
        if (nodes[c_indx].filled_r >= 0){
            nodes[c_indx].light = max(nodes[c_indx].light, lightsource.strengh / (dist*dist*lightsource.disp));
            nodes[c_indx].light = min(nodes[c_indx].light,1.3);
            if (nodes[c_indx].filled_a >= 255){
                return;
            }
        }

        float psize = pow(2.0, float(nodes[c_indx].size));

        if (ray_pos.x < c_map_pos.x || ray_pos.y < c_map_pos.y || ray_pos.z < c_map_pos.z){
            qu_indx -= 1;
            continue;
        }
        if (ray_pos.x > c_map_pos.x + psize || ray_pos.y > c_map_pos.y + psize || ray_pos.z > c_map_pos.z + psize){
            qu_indx -= 1;
            continue;
        }

        uint ptx = ray_pos.x < c_map_pos.x + (psize / 2) ? 0 : 1;
        uint pty = ray_pos.y < c_map_pos.y + (psize / 2) ? 0 : 1;
        uint ptz = ray_pos.z < c_map_pos.z + (psize / 2) ? 0 : 1;
        uint id = ptx + pty * 2 + ptz * 4;

        if (nodes[c_indx].kids[id]>0) {
            qu_indx += 1;
            vec3 npos = c_map_pos + vec3(ptx,pty,ptz)*(psize/2);
            qu_node[qu_indx] = nodes[c_indx].kids[id];
            qu_pos[qu_indx]=npos;
            continue;
        }else{
            vec3 area_bg = c_map_pos + vec3(ptx,pty,ptz)*(psize/2);
            vec3 area_en = area_bg + vec3(psize/2,psize/2,psize/2);

            area_bg -= vec3(0.0001,0.0001,0.0001);
            area_en += vec3(0.0001,0.0001,0.0001);
            float t = 1000000;

            if (dir.x>0 && ray_pos.x < area_en.x) t = min(t, (area_en.x-ray_pos.x)/dir.x);
            if (dir.y>0 && ray_pos.y < area_en.y) t = min(t, (area_en.y-ray_pos.y)/dir.y);
            if (dir.z>0 && ray_pos.z < area_en.z) t = min(t, (area_en.z-ray_pos.z)/dir.z);

            if (dir.x<0 && area_bg.x < ray_pos.x) t = min(t, (area_bg.x-ray_pos.x)/dir.x);
            if (dir.y<0 && area_bg.y < ray_pos.y) t = min(t, (area_bg.y-ray_pos.y)/dir.y);
            if (dir.z<0 && area_bg.z < ray_pos.z) t = min(t, (area_bg.z-ray_pos.z)/dir.z);

            dist += t;
            ray_pos += dir * t;
        }

    }
}

const float PI = 3.14159265359;

void main(){
    float rand1 = random(float(gl_GlobalInvocationID.x + gl_GlobalInvocationID.x % 1000));
    float rand2 = random(float((gl_GlobalInvocationID.x+17) + (gl_GlobalInvocationID.x-25)/2));
    float angv = rand1 * 2 * PI;
    float angh = rand2 * 2 * PI;
    
    mat3x3 lpitch = mat3x3(1,0,0,0,cos(angv),sin(angv),0,-sin(angv),cos(angv));
    mat3x3 lyaw = mat3x3(cos(angh),0,-sin(angh),0,1,0,sin(angh),0,cos(angh));


    vec3 dir = vec3(0,0,1);
    dir = lpitch * lyaw * dir;

    vec3 pos = vec3(lightsource.orgin.x,lightsource.orgin.y,lightsource.orgin.z);
    float maxdist = 500;
    do_ray_tracing(pos,dir,maxdist);
}
