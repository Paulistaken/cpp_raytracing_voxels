#ifndef OTREEHEADER

#include <memory>
#include <optional>

#include "dtypes.hpp"


class OctTreeNode{
    public:
    bool filled;
    Vec2 pos;
    i32 size;
    std::optional<std::unique_ptr<OctTreeNode>> children[8];
    OctTreeNode(bool filled, i32 size);
};
class OctTree{
    public:
    std::unique_ptr<OctTreeNode> root;
    OctTree(i32 size);
};
void otree_insert_node(OctTree& tree, const Vec3& position, const i32& size);
bool otree_is_pos_filled(const OctTree& tree, const Vec3& position);
std::optional<Vec3> otree_sendray(const OctTree& tree, const Vec3& orgin, const Vec3& dir);

typedef const std::unique_ptr<OctTreeNode>* OTNcpointer;

#define OTREEHEADER
#endif // !OTREEHEADER
