#ifndef QTREEHEADER

#include <memory>
#include <optional>

#include "dtypes.hpp"


class QuadTreeNode{
    public:
    bool filled;
    Vec2 pos;
    i32 size;
    std::optional<std::unique_ptr<QuadTreeNode>> children[4];
    QuadTreeNode(bool filled, i32 size);
};
class QuadTree{
    public:
    std::unique_ptr<QuadTreeNode> root;
    QuadTree(i32 size);
};
void qtree_insert_node(QuadTree& tree, const Vec2& position, const i32& size);
bool qtree_is_pos_filled(const QuadTree& tree, const Vec2& position);
std::optional<Vec2> qtree_sendray(const QuadTree& tree, const Vec2& orgin, const Vec2& dir);

typedef const std::unique_ptr<QuadTreeNode>* QTNcpointer;

#define QTREEHEADER
#endif // !QTREEHEADER

