#version 440

const int VREZ = 200;

layout (local_size_x = VREZ, local_size_y = 1, local_size_z = 1) in;

const float FOV = 60;


#define LGT

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

struct CameraData{
    vec4 orgin;
    vec4 angle;
};


layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };
layout(std430, binding=1) buffer ssbo1 { OctTreeNodeSer nodes[]; };
layout(std430, binding=2) buffer ssbo2 { OctTreeSer node_data; };

layout(std430, binding=3) buffer ssbo3 { CameraData cam; };


struct RayResult{
    bool hit;
    uint r;
    uint g;
    uint b;
    uint a;
    float dist;
};

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

RayResult do_ray_tracing(vec3 pos, vec3 dir, float maxdist){
    float dist = 0;
    int qu_indx = 0;
    int qu_node[32];
    vec3 qu_pos[32];

    #ifdef LGT
        float qu_light[32];
        vec3 qu_light_c[32];
        qu_light[qu_indx]=0;
        qu_light_c[qu_indx]=vec3(1,1,1);
    #endif

    qu_node[qu_indx] = 0;
    qu_pos[qu_indx] = vec3(0,0,0);

    uint cur_n = 0;
    uint cur_r = 0;
    uint cur_g = 0;
    uint cur_b = 0;
    uint cur_a = 0;

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

        float bg = 0.5;
        float en = apsize - 0.5;

        vec3 vbg = vec3(0,0,0);
        vec3 ven = vec3(apsize,apsize,apsize);

        float t = 1000000;

        float tx = dir.x == 0 ? t : (((dir.x > 0) ? bg : en) - ray_pos.x) / dir.x;
        float ty = dir.y == 0 ? t : (((dir.y > 0) ? bg : en) - ray_pos.y) / dir.y;
        float tz = dir.z == 0 ? t : (((dir.z > 0) ? bg : en) - ray_pos.z) / dir.z;

        if (tx > 0 && point_in_area(ray_pos + dir * tx,vbg,ven)) t = min(tx,t);
        if (ty > 0 && point_in_area(ray_pos + dir * ty,vbg,ven)) t = min(ty,t);
        if (tz > 0 && point_in_area(ray_pos + dir * tz,vbg,ven)) t = min(tz,t);

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

        #ifdef LGT
            if (nodes[c_indx].size <= LightDetail){
                qu_light[qu_indx] = max(qu_light[qu_indx],nodes[c_indx].light);
                qu_light_c[qu_indx] = vec3(nodes[c_indx].light_r,nodes[c_indx].light_g,nodes[c_indx].light_b);
            }
        #endif


        if (nodes[c_indx].filled_r >= 0){
            cur_n += 1;
            int new_r = nodes[c_indx].filled_r;
            int new_g = nodes[c_indx].filled_g;
            int new_b = nodes[c_indx].filled_b;
            #ifdef LGT 
                qu_light[qu_indx] = max(qu_light[qu_indx],nodes[c_indx].light);
                float c_light = qu_light[qu_indx];
                vec3 c_light_c = qu_light_c[qu_indx] * c_light;
                new_r = int(float(new_r) * c_light_c.x);
                new_g = int(float(new_g) * c_light_c.y);
                new_b = int(float(new_b) * c_light_c.z);
            #endif
            cur_r = min( (cur_r * cur_a + new_r * nodes[c_indx].filled_a ) / 255,255);
            cur_g = min( (cur_g * cur_a + new_g * nodes[c_indx].filled_a ) / 255,255);
            cur_b = min( (cur_b * cur_a + new_b * nodes[c_indx].filled_a ) / 255,255);
            cur_a = max(cur_a,nodes[c_indx].filled_a);
            if (cur_a >= 255){
                return RayResult(true, cur_r, cur_g, cur_b, cur_a,dist);
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

            #ifdef LGT
                float c_light = qu_light[qu_indx];
                vec3 c_light_c = qu_light_c[qu_indx];
            #endif

            qu_indx += 1;
            vec3 npos = c_map_pos + vec3(ptx,pty,ptz)*(psize/2);
            qu_node[qu_indx] = nodes[c_indx].kids[id];
            qu_pos[qu_indx]=npos;

            #ifdef LGT
                qu_light[qu_indx] = c_light;
                qu_light_c[qu_indx] = c_light_c;
            #endif
            
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
    if (cur_n != 0){
        return RayResult(true, cur_r, cur_g, cur_b, cur_a,dist);
    }
    return RayResult(false, 0, 0, 0, 0, 0);
}

const float PI = 3.14159265359;

void main(){
    uint ix = gl_GlobalInvocationID.x;
    uint iy = gl_GlobalInvocationID.y;
    if (ix>=VREZ||iy>=VREZ) return;

    float angstep = FOV / float(VREZ);
    
    float angv = (angstep * float(iy) - (FOV/2)) / 180 * PI;
    float angh = (angstep * float(ix) - (FOV/2)) / 180 * PI;
    
    mat3x3 lpitch = mat3x3(1,0,0,0,cos(angv),sin(angv),0,-sin(angv),cos(angv));
    mat3x3 lyaw = mat3x3(cos(angh),0,-sin(angh),0,1,0,sin(angh),0,cos(angh));

    mat3x3 cpitch = mat3x3(1,0,0,0,cos(cam.angle.x),sin(cam.angle.x),0,-sin(cam.angle.x),cos(cam.angle.x));
    mat3x3 cyaw = mat3x3(cos(cam.angle.y),0,-sin(cam.angle.y),0,1,0,sin(cam.angle.y),0,cos(cam.angle.y));

    vec3 dir = vec3(0,0,1);
    dir = lpitch * lyaw * dir;

    dir = cyaw * cpitch * dir;

    vec3 pos = vec3(cam.orgin.x,cam.orgin.y,cam.orgin.z);
    float maxdist = screen_data.pixels[iy][ix].deph >= 0 ? screen_data.pixels[iy][ix].deph : 500;

    RayResult rayhit = do_ray_tracing(pos,dir,maxdist);
    
    if (!rayhit.hit) return;

    if (rayhit.a >= 255){
        screen_data.pixels[iy][ix].r = rayhit.r;
        screen_data.pixels[iy][ix].g = rayhit.g;
        screen_data.pixels[iy][ix].b = rayhit.b;
        screen_data.pixels[iy][ix].a = rayhit.a;
        screen_data.pixels[iy][ix].deph = rayhit.dist;
        return;
    }
    screen_data.pixels[iy][ix].r = min( (screen_data.pixels[iy][ix].r * screen_data.pixels[iy][ix].a + rayhit.r * rayhit.a ) / 255, 255);
    screen_data.pixels[iy][ix].g = min( (screen_data.pixels[iy][ix].g * screen_data.pixels[iy][ix].a + rayhit.g * rayhit.a ) / 255, 255);
    screen_data.pixels[iy][ix].b = min( (screen_data.pixels[iy][ix].b * screen_data.pixels[iy][ix].a + rayhit.b * rayhit.a ) / 255, 255);
    screen_data.pixels[iy][ix].a = max(screen_data.pixels[iy][ix].a,rayhit.a);


}
