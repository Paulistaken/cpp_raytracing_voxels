#version 430
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct PixelData{
    float deph;
    uint r;
    uint g;
    uint b;
    uint a;
};
struct ScreenData{
    PixelData pixels[60][60];
};

struct OctTreeSer{
    vec4 pos;
    int size;
    uint len;
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


layout(std430, binding=0) buffer ssbo0 { ScreenData screen_data; };
layout(std430, binding=1) buffer ssbo1 { OctTreeNodeSer nodes[]; };
layout(std430, binding=2) buffer ssbo2 { OctTreeSer node_data; };

layout(std430, binding=3) buffer ssbo3 { CameraData cam; };


struct PosFill{
    bool filled;
    uint r;
    uint g;
    uint b;
    uint a;

    vec3 area_bg;
    vec3 area_en;
};

PosFill is_pos_filled(vec3 poss){
    vec3 pos = poss;
    vec3 c_pos = vec3(0,0,0);
    uint indx = 0;
    float apsize = pow(2,nodes[indx].size);
    float psize = pow(2,nodes[indx].size);
    if(pos.x > psize || pos.y > psize || pos.z > psize){
        return PosFill(
            false, 0, 0, 0, 0, vec3(0,0,0), vec3(psize,psize,psize)
        );
    }
    while(true){
        psize = pow(2,nodes[indx].size);
        if (nodes[indx].filled_r>=0){
            vec3 area_bg = c_pos;
            vec3 area_en = c_pos + vec3(psize,psize,psize);
            return PosFill(
                true,
                uint(nodes[indx].filled_r),
                uint(nodes[indx].filled_g),
                uint(nodes[indx].filled_b),
                uint(nodes[indx].filled_a),
                area_bg,
                area_en
            );
        }
        uint ptx = pos.x < c_pos.x + (psize / 2) ? 0 : 1;
        uint pty = pos.y < c_pos.y + (psize / 2) ? 0 : 1;
        uint ptz = pos.z < c_pos.z + (psize / 2) ? 0 : 1;
        uint id = ptx + pty*2 + ptz*4;
        c_pos += vec3(ptx,pty,ptz) * (psize / 2);
        if (nodes[indx].kids[id]<=0){
            vec3 area_bg = c_pos;
            vec3 area_en = c_pos + vec3(psize/2,psize/2,psize/2);
            return PosFill(
                false,
                0,
                0,
                0,
                0,
                area_bg,
                area_en
            );
        }
        indx = nodes[indx].kids[id];
    }
    return PosFill(
        false, 0, 0,0, 0, vec3(0,0,0), vec3(apsize,apsize,apsize)
    );
}

struct RayResult{
    bool hit;
    uint r;
    uint g;
    uint b;
    uint a;
    float dist;
};

RayResult do_ray_tracing(vec3 spos, vec3 dir, float maxdist){
    float dist = 0;
    vec3 pos = spos - vec3(node_data.pos.x,node_data.pos.y,node_data.pos.z);

    float apsize = pow(2,nodes[0].size);
    if (pos.x < 0 || pos.x > apsize || pos.y < 0 || pos.y > apsize || pos.z < 0 || pos.z > apsize){
        float t = 1000000;

        if (dir.x>0 && pos.x < 0.001) t = min(t, (0.001-pos.x)/dir.x);
        if (dir.y>0 && pos.y < 0.001) t = min(t, (0.001-pos.y)/dir.y);
        if (dir.z>0 && pos.z < 0.001) t = min(t, (0.001-pos.z)/dir.z);

        if (dir.x<0 && pos.x > apsize-0.001) t = min(t, (apsize-0.001 - pos.x)/dir.x);
        if (dir.y<0 && pos.y > apsize-0.001) t = min(t, (apsize-0.001 - pos.y)/dir.y);
        if (dir.z<0 && pos.z > apsize-0.001) t = min(t, (apsize-0.001 - pos.z)/dir.z);

        dist += t;
        pos += dir * t;
    }

    uint cur_n = 0;
    uint cur_r = 0;
    uint cur_g = 0;
    uint cur_b = 0;
    uint cur_a = 0;

    while(true){
        if (dist >= maxdist){
            break;
        }
        float psize = pow(2,nodes[0].size);
        if(pos.x > psize || pos.y > psize || pos.z > psize){
            break;
        }
        if(pos.x < 0 || pos.y < 0 || pos.z < 0){
            break;
        }
        PosFill posfill = is_pos_filled(pos);
        if (posfill.filled){
            cur_n += 1;
            cur_r = min( (cur_r * cur_a + posfill.r * posfill.a ) / 255,255);
            cur_g = min( (cur_g * cur_a + posfill.g * posfill.a ) / 255,255);
            cur_b = min( (cur_b * cur_a + posfill.b * posfill.a ) / 255,255);
            cur_a = max(cur_a,posfill.a);
            if (cur_a >= 255){
                return RayResult(true, cur_r, cur_g, cur_b, cur_a,dist);
            }
        }
        
        posfill.area_en += vec3(0.0001,0.0001,0.0001);
        posfill.area_bg -= vec3(0.0001,0.0001,0.0001);
        float t = 1000000;

        if (dir.x>0 && pos.x < posfill.area_en.x) t = min(t, (posfill.area_en.x-pos.x)/dir.x);
        if (dir.y>0 && pos.y < posfill.area_en.y) t = min(t, (posfill.area_en.y-pos.y)/dir.y);
        if (dir.z>0 && pos.z < posfill.area_en.z) t = min(t, (posfill.area_en.z-pos.z)/dir.z);

        if (dir.x<0 && posfill.area_bg.x < pos.x) t = min(t, (posfill.area_bg.x-pos.x)/dir.x);
        if (dir.y<0 && posfill.area_bg.y < pos.y) t = min(t, (posfill.area_bg.y-pos.y)/dir.y);
        if (dir.z<0 && posfill.area_bg.z < pos.z) t = min(t, (posfill.area_bg.z-pos.z)/dir.z);

        dist += t;
        pos += dir * t;
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
    
    float angv = (float(iy) - 30) / 180 * PI;
    float angh = (float(ix) - 30) / 180 * PI;
    
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
