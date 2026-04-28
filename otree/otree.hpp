#ifndef OTREEHEADER

#include <raylib.h>
#include <memory>
#include <optional>
#include "../dtypes.hpp"


namespace OCTTree{

    typedef DT3::Vec3 Vec3;

    class OctTreeNode{
        public:
        std::optional<Color> fill;
        bool reflective;
        i32 size;
        std::optional<std::unique_ptr<OctTreeNode>> children[8];
        OctTreeNode(std::optional<Color> fill, i32 size);
        fn optimize() -> bool;
    };
    class OctTree{
        public:
        Vec3 position;
        Vec3 orgin;
        Vec3 angle;
        i32 size;
        std::unique_ptr<OctTreeNode> root;
        OctTree(i32 size);
        fn clear() -> void;
        fn insert_node(const Color& fill, const Vec3& position, const i32& size) -> void;
        fn insert_node(const Color& fill, const Vec3& position, const i32& size, const bool reflective) -> void;
        fn remove_node(const Vec3& position, const i32& size) -> void;
        fn is_pos_filled(const Vec3& position) const -> std::optional<Color>;
        fn optimize() -> void;
    };

    typedef const std::unique_ptr<OctTreeNode>* OTNcpointer;

    typedef struct {
        i32 children[8];
        i32 size;
        i32 filled_r;
        i32 filled_g;
        i32 filled_b;
        i32 filled_a;
        float light;
        float light_r;
        float light_g;
        float light_b;
        u32 ref;
    } OctTreeNodeSer;
    typedef struct {
        Vector4 pos;
        Vector4 orgin;
        Vector4 angle;
        i32 size;
    } OctTreeDataSer;
    typedef struct {
        i32 size;
        u32 lengh;
        OctTreeDataSer data;
        OctTreeNodeSer* nodes;
    } OctTreeSer;

    OctTreeDataSer serialize_octree_data(const OctTree& otree);
    OctTreeSer serialize_octtree(const OctTree& octtree);

}

#define OTREEHEADER
#endif // !OTREEHEADER
