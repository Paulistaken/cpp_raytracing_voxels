#include "otree_ray.hpp"
#include "otree.hpp"
#include <algorithm>
#include <cmath>
#include <stack>
namespace OCTTree::OCTRay {

    fn OCTRayOptions::max_range(f64 v) -> void {this->range = v;}
    fn OCTRayOptions::max_detail(i32 v) -> void {this->max_coll = v;}
    OCTRay::OCTRay(const Vec3& orgin, const Vec3& dir){
        this->orgin = orgin;
        this->direction = dir;
    }


    const f64 __QT_RAY_e = 0.000001;

    auto ray_in_area (const Vec3& rp, const Vec3& ap, f64 as) -> bool{
        return rp.x >= ap.x && rp.x < ap.x + as&&rp.y >= ap.y && rp.y < ap.y + as&&rp.z >= ap.z && rp.z < ap.z + as;
    }

    auto translate_ray(const OctTree& tree, const Vec3& orgin, const Vec3& dir) -> std::optional<Vec3>{
        f64 size = std::pow(2.0,tree.size);
        if (ray_in_area(orgin,Vec3(),size)) return orgin;

        if ( (orgin.x < tree.position.x && dir.x <= 0) || (orgin.x >= tree.position.x + size && dir.x >= 0) ) return {};
        if ( (orgin.y < tree.position.y && dir.y <= 0) || (orgin.y >= tree.position.y + size && dir.y >= 0) ) return {};
        if ( (orgin.z < tree.position.z && dir.z <= 0) || (orgin.z >= tree.position.z + size && dir.z >= 0) ) return {};

        Vec3 c1 = tree.position;
        Vec3 c2 = c1 + Vec3(size,size,size);
        c1 += Vec3(__QT_RAY_e,__QT_RAY_e,__QT_RAY_e);
        c2 -= Vec3(__QT_RAY_e,__QT_RAY_e,__QT_RAY_e);

        f64 t = 10000;
        f64 tx = ((dir.x < 0 ? c2.x : c1.x) - orgin.x) / dir.x; 
        f64 ty = ((dir.y < 0 ? c2.y : c1.y) - orgin.y) / dir.y; 
        f64 tz = ((dir.z < 0 ? c2.z : c1.z) - orgin.z) / dir.z; 
        t = std::min(t, tx > 0 ? tx : t);
        t = std::min(t, ty > 0 ? tx : t);
        t = std::min(t, tz > 0 ? tx : t);

        Vec3 norgin = orgin + (dir * t);
        if (ray_in_area(norgin,Vec3(),size)) return norgin;

        return {};
    }



    auto OCTRay::send_ray(const OctTree& tree, const std::optional<OCTRayOptions>& opt) const -> std::optional<RayResult>{
        Vec3 cpos = Vec3(0,0,0);
        Vec3 dir = this->direction;

        std::stack<std::tuple<OTNcpointer,Vec3>> qu;

        qu.push({&tree.root,Vec3(0,0,0)});

        Vec3 ray_pos = Vec3(this->orgin);
        auto nray_pos = translate_ray(tree, ray_pos, dir);
        if (nray_pos.has_value()) ray_pos = nray_pos.value(); else return {};

        while(true){

            if (qu.empty()) break;
            auto [c_node,map_pos] = qu.top();
            if (c_node->get()->fill.has_value()){ 
                Color clr = c_node->get()->fill.value();
                std::tuple<Vec3,Color> rt = {ray_pos,clr}; 
                return rt;
            }
            f64 psize = std::pow(2,c_node->get()->size);

            if (!ray_in_area(ray_pos, map_pos, psize)){
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

                f64 t = 10000;
                f64 tx =  ((dir.x > 0 ? c2.x : c1.x) - ray_pos.x) / dir.x;
                t = tx == 0 ? t : std::min(tx,t);
                f64 ty = ((dir.y > 0 ? c2.y : c1.y) - ray_pos.y) / dir.y;
                t = ty == 0 ? t : std::min(ty,t);
                f64 tz = ((dir.z > 0 ? c2.z : c1.z) - ray_pos.z) / dir.z;
                t = tz == 0 ? t : std::min(tz,t);

                Vec3 tmv = dir * std::min(tx,std::min(ty,tz));
                ray_pos += tmv;
                continue;
            }

        }

        return {};
    }

}
