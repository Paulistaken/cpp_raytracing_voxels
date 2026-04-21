
#include "otree.hpp"
#include "../dtypes.hpp"
#include <cmath>
#include <memory>
#include <optional>
#include <raylib.h>

namespace OCTTree{

typedef DT3::Vec3 Vec3;

OctTreeNode::OctTreeNode(std::optional<Color> fill, i32 size){this->fill = fill;this->size = size;}
OctTree::OctTree(i32 size){
    this->size=size;
    std::unique_ptr<OctTreeNode> root(new OctTreeNode({},size));
    this->root.swap(root);
}
void OctTree::clear(){
    this->root.reset(new OctTreeNode({},this->size));
}

bool ray_in_area (const Vec3& rp, const Vec3& ap, f64 as){
    return rp.x >= ap.x && rp.x < ap.x + as&&rp.y >= ap.y && rp.y < ap.y + as&&rp.z >= ap.z && rp.z < ap.z + as;
}

std::optional<Color> OctTree::is_pos_filled(const Vec3& position) const{
    const OctTree& tree = *this;
    Vec3 c_pos = Vec3(0,0,0); 
    OTNcpointer c_node = &tree.root;
    f64 psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize || position.z > psize) return {};
    while(true){
        psize = std::pow(2,c_node->get()->size);
        if (
                c_node->get()->fill.has_value()
           ){
            return c_node->get()->fill;
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
    return {};
}

void OctTree::OctTree::insert_node(const Color& fill, const Vec3& position, const i32& size){
    OctTree& tree = *this;
    Vec3 c_pos = Vec3(0,0,0); 
    OTNcpointer c_node = &tree.root;
    f64 psize = std::pow(2,c_node->get()->size);
    if (position.x > psize || position.y > psize || position.z > psize) return;
    while(true){
        psize = std::pow(2.0,(f64)c_node->get()->size);
        if (
                c_node->get()->size <= size
           ){
            c_node->get()->fill=fill;
            break;
        }
        bool ptx = !(position.x < c_pos.x + (psize / 2));
        bool pty = !(position.y < c_pos.y + (psize / 2));
        bool ptz = !(position.z < c_pos.z + (psize / 2));
        u32 id = ptx + pty*2 + ptz*4;
        c_pos += Vec3(ptx,pty,ptz) * (psize/2);
        if (!c_node->get()->children[id].has_value()){
            c_node->get()->children[id].emplace(new OctTreeNode({},c_node->get()->size-1));
        }
        c_node = &c_node->get()->children[id].value();
        continue;
    }
}

void OctTreeNode::optimize(){
    for(auto & chd : this->children){
        if (!chd.has_value()) continue;
        chd.value()->optimize();
    }
    bool allfilled = true;
    for (const auto& chd : this->children){
        if (!chd.has_value()){
            allfilled = false;
            break;
        }
        allfilled = allfilled & chd.value()->fill.has_value();
    }
    if (!allfilled) return;
    u32 colr = 0;u32 colg = 0;u32 colb = 0;u32 cola = 0;
    for (const auto& chd : this->children){
        const Color& clr = chd.value()->fill.value();
        colr+=(u32)clr.r * (u32)clr.a;
        colg+=(u32)clr.g * (u32)clr.a;
        colb+=(u32)clr.b * (u32)clr.a;
        cola = std::max(cola, (u32)clr.a);
    }
    colr /= 8; colg /= 8;colb /= 8;
    colr /= 255; colg /= 255;colb /= 255;
    this->fill = Color{(u8)colr,(u8)colg,(u8)colb,(u8)cola};
    for(auto & chd : this -> children){
        chd = {};
    }
}
void OctTree::optimize(){
    this->root->optimize();
}

    u32 octree_size(const std::unique_ptr<OctTreeNode>& node){
        u32 amt = 1;
        for(const auto& nd : node->children){
            if(!nd.has_value()) continue;
            amt += octree_size(nd.value());
        }
        return amt;
    }
    void serialize_octree_nodes(
            OctTreeNodeSer* nodes,
            u32 cindex,
            u32& avindex,
            const std::unique_ptr<OctTreeNode>& cnode
            ){
        nodes[cindex].size=cnode->size;
        nodes[cindex].filled_r=cnode->fill.has_value() ? cnode->fill.value().r : -1;
        nodes[cindex].filled_g=cnode->fill.has_value() ? cnode->fill.value().g : -1;
        nodes[cindex].filled_b=cnode->fill.has_value() ? cnode->fill.value().b : -1;
        nodes[cindex].filled_a=cnode->fill.has_value() ? cnode->fill.value().a : -1;
        for(int i = 0; i < 8; i++){
            nodes[cindex].children[i]=-1;
            if (!cnode->children[i].has_value()) continue;
            avindex += 1;
            nodes[cindex].children[i]=avindex;
            serialize_octree_nodes(nodes, avindex, avindex, cnode->children[i].value());
        }
    }
    OctTreeDataSer serialize_octree_data(const OctTree& otree){
        OctTreeDataSer data = OctTreeDataSer{
            Vector4{(f32)otree.position.x,(f32)otree.position.y,(f32)otree.position.z,0},
            otree.size,
        };
        return data;

    }
    // Malloc wymagany to interfejsowania z kartą graficzną
    OctTreeSer serialize_octtree(const OctTree& otree){
        u32 full_size = octree_size(otree.root);
        OctTreeNodeSer* nodes = (OctTreeNodeSer*)(RL_MALLOC(sizeof(OctTreeNodeSer)*full_size));
        u32 avindex = 0;
        serialize_octree_nodes(nodes, 0, avindex, otree.root);
        OctTreeDataSer data = OctTreeDataSer{
            Vector4{(f32)otree.position.x,(f32)otree.position.y,(f32)otree.position.z,0},
            otree.size,
        };
        OctTreeSer otreeser = OctTreeSer{
            otree.size,
            full_size,
            data,
            nodes
        };
        return otreeser;
    }

}
