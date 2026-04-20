
#include "otree.hpp"
#include "dtypes.hpp"
#include <cmath>
#include <memory>
#include <optional>
#include <raylib.h>
#include <stack>
#include <tuple>

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
        nodes[cindex].filled=cnode->fill.has_value();
        for(int i = 0; i < 8; i++){
            if (!cnode->children[i].has_value()){
                nodes[cindex].children[i]=-1;
                continue;
            }
            avindex += 1;
            serialize_octree_nodes(nodes, avindex, avindex, cnode->children[i].value());
        }
    }
    // Malloc wymagany to interfejsowania z kartą graficzną
    OctTreeSer serialize_octtree(const OctTree& otree){
        u32 full_size = octree_size(otree.root);
        OctTreeNodeSer* nodes = (OctTreeNodeSer*)(RL_MALLOC(sizeof(OctTreeNodeSer)*full_size));
        u32 avindex = 0;
        serialize_octree_nodes(nodes, 0, avindex, otree.root);
        OctTreeSer otreeser = OctTreeSer{
            otree.size,
            full_size,
            nodes
        };
        return otreeser;
    }

}
