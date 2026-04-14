#include "otree_ray.hpp"
#include <cmath>
#include <stack>
namespace OCTTree::OCTRay {

    OCTRay::OCTRay(const Vec3& orgin, const Vec3& dir){
        this->orgin = orgin;
        this->direction = dir;
    }


const double __QT_RAY_e = 0.000001;
    std::optional<RayResult> OCTRay::send_ray(const OctTree& tree, const std::optional<OCTRayOptions>& opt) const{
        Vec3 cpos = Vec3(0,0,0);
        Vec3 dir = this->direction;

        std::stack<std::tuple<OTNcpointer,Vec3>> qu;

        qu.push({&tree.root,Vec3(0,0,0)});

        Vec3 ray_pos = Vec3(this->orgin);

        while(true){

            if (qu.empty()) break;
            auto [c_node,map_pos] = qu.top();
            if (c_node->get()->fill.has_value()){ 
                Color clr = c_node->get()->fill.value();
                std::tuple<Vec3,Color> rt = {ray_pos,clr}; 
                return rt;
            }
            double psize = std::pow(2,c_node->get()->size);

            if(
                    ray_pos.x >= map_pos.x + psize ||
                    ray_pos.y >= map_pos.y + psize || 
                    ray_pos.z >= map_pos.z + psize || 
                    ray_pos.x < map_pos.x ||
                    ray_pos.y < map_pos.y ||
                    ray_pos.z < map_pos.z
                    ){
                qu.pop();
                continue;
            }
            
            bool ptx = !(ray_pos.x < map_pos.x + (psize / 2));
            bool pty = !(ray_pos.y < map_pos.y + (psize / 2));
            bool ptz = !(ray_pos.z < map_pos.z + (psize / 2));
            u32 id = ptx + pty*2 + ptz*4;

            if (c_node->get()->children[id].has_value()){
                auto nnode = &c_node->get()->children[id].value();
                auto npos = map_pos + (Vec3(ptx,pty,ptz)*(psize/2));
                qu.push({nnode,npos});
                continue;
            } else {
                Vec3 c1 = map_pos + (Vec3(ptx,pty,ptz) * (psize/2));
                Vec3 c2 = c1 + (Vec3(1,1,1)*(psize/2));
                c1 -= Vec3(__QT_RAY_e,__QT_RAY_e,__QT_RAY_e);
                c2 += Vec3(__QT_RAY_e,__QT_RAY_e,__QT_RAY_e);

                double t = 10000;
                double tx =  ((dir.x > 0 ? c2.x : c1.x) - ray_pos.x) / dir.x;
                t = tx == 0 ? t : std::min(tx,t);
                double ty = ((dir.y > 0 ? c2.y : c1.y) - ray_pos.y) / dir.y;
                t = ty == 0 ? t : std::min(ty,t);
                double tz = ((dir.z > 0 ? c2.z : c1.z) - ray_pos.z) / dir.z;
                t = tz == 0 ? t : std::min(tz,t);

                Vec3 tmv = dir * std::min(tx,std::min(ty,tz));
                ray_pos += tmv;
                continue;
            }

        }

        return {};
    }

}
