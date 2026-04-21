#ifndef OTREEHEADER

#include <raylib.h>
#include <memory>
#include <optional>
#include "dtypes.hpp"


namespace OCTTree{

    typedef DT3::Vec3 Vec3;

    class OctTreeNode{
        public:
        std::optional<Color> fill;
        i32 size;
        std::optional<std::unique_ptr<OctTreeNode>> children[8];
        OctTreeNode(std::optional<Color> fill, i32 size);
        fn optimize() -> void;
    };
    class OctTree{
        public:
        Vec3 position;
        i32 size;
        std::unique_ptr<OctTreeNode> root;
        OctTree(i32 size);
        fn clear() -> void;
        fn insert_node(const Color& fill, const Vec3& position, const i32& size) -> void;
        fn is_pos_filled(const Vec3& position) const -> std::optional<Color>;
        fn optimize() -> void;
    };

    typedef const std::unique_ptr<OctTreeNode>* OTNcpointer;

    typedef struct {
        i32 size;
        i32 filled_r;
        i32 filled_g;
        i32 filled_b;
        i32 filled_a;
        i32 children[8];
    } OctTreeNodeSer;
    typedef struct {
        Vector4 pos;
        i32 size;
        u32 len;
    } OctTreeDataSer;
    typedef struct {
        i32 size;
        u32 lengh;
        OctTreeDataSer data;
        OctTreeNodeSer* nodes;
    } OctTreeSer;

    OctTreeSer serialize_octtree(const OctTree& octtree);

}

#define OTREEHEADER
#endif // !OTREEHEADER
