#include "qtree.hpp"
#include <cmath>
#include <memory>
#include <stack>
#include <tuple>

QuadTreeNode::QuadTreeNode(bool filled, i32 size){this->filled = filled;this->size = size;}
QuadTree::QuadTree(i32 size){
    std::unique_ptr<QuadTreeNode> root(new QuadTreeNode(false,size));
    this->root.swap(root);
}

const double __QT_RAY_e = 0.00001;

std::optional<Vec2> qtree_sendray(const QuadTree& tree, const Vec2& orgin, const Vec2& dir){
    Vec2 cpos = Vec2(0,0);

    std::stack<std::tuple<QTNcpointer,Vec2>> qu;

    qu.push({&tree.root,Vec2(0,0)});

    Vec2 ray_pos = Vec2(orgin);

    while(true){

        if (qu.empty()) break;
        auto [c_node,map_pos] = qu.top();
        if (c_node->get()->filled) return ray_pos;
        double psize = std::pow(2,c_node->get()->size);

        if(
                ray_pos.x >= map_pos.x + psize ||
                ray_pos.y >= map_pos.y + psize || 
                ray_pos.x < map_pos.x ||
                ray_pos.y < map_pos.y
                ){
            qu.pop();
            continue;
        }
        
        bool ptx = !(ray_pos.x < map_pos.x + (psize / 2));
        bool pty = !(ray_pos.y < map_pos.y + (psize / 2));
        u32 id = ptx + pty*2;

        if (c_node->get()->children[id].has_value()){
            auto nnode = &c_node->get()->children[id].value();
            auto npos = map_pos + (Vec2(ptx,pty)*(psize/2));
            qu.push({nnode,npos});
            continue;
        } else {
            Vec2 c1 = map_pos + (Vec2(ptx,pty) * (psize/2));
            Vec2 c2 = c1 + Vec2(psize/2,psize/2);
            c1 -= Vec2(__QT_RAY_e,__QT_RAY_e);
            c2 += Vec2(__QT_RAY_e,__QT_RAY_e);

            double tx = 10000;
            if (dir.x > 0 && c2.x != ray_pos.x) tx = std::min(tx,(c2.x-ray_pos.x)/dir.x);
            if (dir.x < 0 && c1.x != ray_pos.x) tx = std::min(tx,(c1.x-ray_pos.x)/dir.x);
            double ty = 10000;
            if (dir.y > 0 && c2.y != ray_pos.y) ty = std::min(ty,(c2.y-ray_pos.y)/dir.y);
            if (dir.y < 0 && c1.y != ray_pos.y) ty = std::min(ty,(c1.y-ray_pos.y)/dir.y);
            Vec2 tmv = dir * std::min(tx,ty);
            ray_pos += tmv;
            continue;
        }

    }

    return {};
}
bool qtree_is_pos_filled(const QuadTree& tree, const Vec2& position){
    Vec2 c_pos = Vec2(0,0); 
    const std::unique_ptr<QuadTreeNode>* c_node = &tree.root; 
    double psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize) return false;
    while(true){
        psize = std::pow(2,c_node->get()->size);
        if (
                c_node->get()->filled
           ){
            return true;
        }
        bool ptx = !(position.x < c_pos.x + (psize / 2));
        bool pty = !(position.y < c_pos.y + (psize / 2));
        u32 id = ptx + pty*2;
        c_pos = c_pos + Vec2((psize/2)*ptx,(psize/2)*pty);
        if (!c_node->get()->children[id].has_value()){
            break;
        }
        c_node = &c_node->get()->children[id].value();
        continue;
    }
    return false;
}
void qtree_insert_node(QuadTree& tree, const Vec2& position, const i32& size){
    Vec2 c_pos = Vec2(0,0); 
    std::unique_ptr<QuadTreeNode>* c_node = &tree.root; 
    double psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize) return;
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
        u32 id = ptx + pty*2;
        c_pos = c_pos + Vec2((psize/2)*ptx,(psize/2)*pty);
        if (!c_node->get()->children[id].has_value()){
            c_node->get()->children[id].emplace(new QuadTreeNode(false,c_node->get()->size-1));
        }
        c_node = &c_node->get()->children[id].value();
        continue;
    }
}
