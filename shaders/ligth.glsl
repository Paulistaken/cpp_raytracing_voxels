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

struct LightData{
    vec4 orgin;
    float strengh;
    float r;
    float g;
    float b;
    float disp;
};
const uint RANDSZ=2048;
struct RandData{
    float data_x[RANDSZ];
    float data_y[RANDSZ];
};


layout(std430, binding=0) buffer ssbo0 { OctTreeNodeSer nodes[]; };
layout(std430, binding=1) buffer ssbo1 { OctTreeSer node_data; };
layout(std430, binding=2) buffer ssbo2 { LightData lightsource; };
layout(std430, binding=3) buffer ssbo3 { RandData rdata; };

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

    uint bounc = 0;
    
    vec3 clight = vec3(lightsource.r,lightsource.g,lightsource.b);

    while(true){
        if (dist >= maxdist){
            break;
        }

        if (qu_indx < 0) break;

        int c_indx = qu_node[qu_indx];
        vec3 c_map_pos = qu_pos[qu_indx];
        float psize = pow(2.0, float(nodes[c_indx].size));

        if (ray_pos.x < c_map_pos.x || ray_pos.y < c_map_pos.y || ray_pos.z < c_map_pos.z){
            qu_indx -= 1;
            continue;
        }
        if (ray_pos.x > c_map_pos.x + psize || ray_pos.y > c_map_pos.y + psize || ray_pos.z > c_map_pos.z + psize){
            qu_indx -= 1;
            continue;
        }

        if (nodes[c_indx].filled_r >= 0){
            nodes[c_indx].light = max(nodes[c_indx].light, lightsource.strengh / (dist*dist*lightsource.disp));
            nodes[c_indx].light = min(nodes[c_indx].light,1.3);

            nodes[c_indx].light_r=clight.x;
            nodes[c_indx].light_g=clight.y;
            nodes[c_indx].light_b=clight.z;

            if (nodes[c_indx].ref >= 10){
                nodes[c_indx].filled_r = int(clight.x * 255);
                nodes[c_indx].filled_g = int(clight.y * 255);
                nodes[c_indx].filled_b = int(clight.z * 255);
            }else{
                clight.x = clight.x + float(nodes[c_indx].filled_r) * float(nodes[c_indx].filled_a / 255) / 255;
                clight.y = clight.y + float(nodes[c_indx].filled_g) * float(nodes[c_indx].filled_a / 255) / 255;
                clight.z = clight.z + float(nodes[c_indx].filled_b) * float(nodes[c_indx].filled_a / 255) / 255;
            }

            if (nodes[c_indx].filled_a >= 255){
                if (bounc >= 0) return;
                dir *= -1;
                bounc += 1;
                clight.x = nodes[c_indx].filled_r / 255;
                clight.y = nodes[c_indx].filled_g / 255;
                clight.z = nodes[c_indx].filled_b / 255;
            }
        }
        else if (nodes[c_indx].size <= LightDetail){

            float c_str = nodes[c_indx].light;
            float n_str = min(lightsource.strengh / (dist*dist*lightsource.disp),1.3);

            vec3 c_light = vec3(nodes[c_indx].light_r,nodes[c_indx].light_g, nodes[c_indx].light_b) * c_str;
            vec3 n_light = vec3(clight.x,clight.y,clight.z) * n_str;

            vec3 w_light = c_light + n_light;
            float d = sqrt(pow(w_light.x,2)+pow(w_light.y,2)+pow(w_light.z,2));
            w_light /= d;

            nodes[c_indx].light_r = w_light.x;
            nodes[c_indx].light_g = w_light.y;
            nodes[c_indx].light_b = w_light.z;

            // nodes[c_indx].light = c_str + n_str;
            nodes[c_indx].light = min(max(c_str,n_str),1.3);
            // nodes[c_indx].light = min(nodes[c_indx].light,1.3);


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
    uint id1 = (gl_GlobalInvocationID.x + gl_GlobalInvocationID.x % 13 + gl_GlobalInvocationID.x % 7) % RANDSZ;
    uint id2 = (gl_GlobalInvocationID.x + gl_GlobalInvocationID.x % 7) % RANDSZ;
    float rnumbx = rdata.data_x[id1] + rdata.data_x[id2];
    float rnumby = rdata.data_y[id2] + rdata.data_y[id1];
    float angv = rnumbx * 2 * PI;
    float angh = rnumby * 2 * PI;
    
    mat3x3 lpitch = mat3x3(1,0,0,0,cos(angv),sin(angv),0,-sin(angv),cos(angv));
    mat3x3 lyaw = mat3x3(cos(angh),0,-sin(angh),0,1,0,sin(angh),0,cos(angh));


    vec3 dir = vec3(0,0,1);
    dir = lpitch * lyaw * dir;

    vec3 pos = vec3(lightsource.orgin.x,lightsource.orgin.y,lightsource.orgin.z);
    float maxdist = 200;
    do_ray_tracing(pos,dir,maxdist);
}
