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
    };

    typedef const std::unique_ptr<OctTreeNode>* OTNcpointer;

}

#define OTREEHEADER
#endif // !OTREEHEADER
