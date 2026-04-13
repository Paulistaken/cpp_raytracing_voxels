
#include "otree.hpp"
#include <cmath>
#include <memory>
#include <stack>
#include <tuple>

OctTreeNode::OctTreeNode(bool filled, i32 size){this->filled = filled;this->size = size;}
OctTree::OctTree(i32 size){
    std::unique_ptr<OctTreeNode> root(new OctTreeNode(false,size));
    this->root.swap(root);
}

const double __QT_RAY_e = 0.00001;

std::optional<Vec3> otree_sendray(const OctTree& tree, const Vec3& orgin, const Vec3& dir){
    Vec3 cpos = Vec3(0,0,0);

    std::stack<std::tuple<OTNcpointer,Vec3>> qu;

    qu.push({&tree.root,Vec3(0,0,0)});

    Vec3 ray_pos = Vec3(orgin);

    while(true){

        if (qu.empty()) break;
        auto [c_node,map_pos] = qu.top();
        if (c_node->get()->filled) return ray_pos;
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

            double tx = 10000;
            if (dir.x > 0 && c2.x != ray_pos.x) tx = std::min(tx,(c2.x-ray_pos.x)/dir.x);
            if (dir.x < 0 && c1.x != ray_pos.x) tx = std::min(tx,(c1.x-ray_pos.x)/dir.x);
            double ty = 10000;
            if (dir.y > 0 && c2.y != ray_pos.y) ty = std::min(ty,(c2.y-ray_pos.y)/dir.y);
            if (dir.y < 0 && c1.y != ray_pos.y) ty = std::min(ty,(c1.y-ray_pos.y)/dir.y);
            double tz = 10000;
            if (dir.z > 0 && c2.z != ray_pos.z) tz = std::min(ty,(c2.z-ray_pos.z)/dir.z);
            if (dir.z < 0 && c1.z != ray_pos.z) tz = std::min(ty,(c1.z-ray_pos.z)/dir.z);
            Vec3 tmv = dir * std::min(tx,std::min(ty,tz));
            ray_pos += tmv;
            continue;
        }

    }

    return {};
}
bool otree_is_pos_filled(const OctTree& tree, const Vec3& position){
    Vec3 c_pos = Vec3(0,0,0); 
    OTNcpointer c_node = &tree.root;
    double psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize || position.z > psize) return false;
    while(true){
        psize = std::pow(2,c_node->get()->size);
        if (
                c_node->get()->filled
           ){
            return true;
        }
        bool ptx = !(position.x < c_pos.x + (psize / 2));
        bool pty = !(position.y < c_pos.y + (psize / 2));
        bool ptz = !(position.z < c_pos.z + (psize / 2));
        u32 id = ptx + pty*2 + ptz*4;
        c_pos += Vec3(ptx,pty,ptz) * (psize/2);
        if (!c_node->get()->children[id].has_value()){
            break;
        }
        c_node = &c_node->get()->children[id].value();
        continue;
    }
    return false;
}
void otree_insert_node(OctTree& tree, const Vec3& position, const i32& size){
    Vec3 c_pos = Vec3(0,0,0); 
    OTNcpointer c_node = &tree.root;
    double psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize || position.z > psize) return;
    while(true){
        psize = std::pow(2.0,(double)c_node->get()->size);
        if (
                c_node->get()->size <= size
           ){
            c_node->get()->filled=true;
            break;
        }
        bool ptx = !(position.x < c_pos.x + (psize / 2));
        bool pty = !(position.y < c_pos.y + (psize / 2));
        bool ptz = !(position.z < c_pos.z + (psize / 2));
        u32 id = ptx + pty*2 + ptz*4;
        c_pos += Vec3(ptx,pty,ptz) * (psize/2);
        if (!c_node->get()->children[id].has_value()){
            c_node->get()->children[id].emplace(new OctTreeNode(false,c_node->get()->size-1));
        }
        c_node = &c_node->get()->children[id].value();
        continue;
    }
}
