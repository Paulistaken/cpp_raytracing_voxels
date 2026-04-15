#include <optional>
#include <tuple>
#ifndef OCTTREE_RAY

#include "otree.hpp"

namespace OCTTree::OCTRay{

typedef std::tuple<Vec3, Color> RayResult;

class OCTRayOptions{
    public:
    std::optional<f64> range;
    std::optional<i32> max_coll;
    OCTRayOptions() {};
    fn max_range(f64 v) -> void;
    fn max_detail(i32 v) -> void; 
};
class OCTRay{
    public:
    Vec3 orgin;
    Vec3 direction;
    OCTRay(const Vec3&, const Vec3&);
    fn send_ray(const OctTree&, const std::optional<OCTRayOptions>&) const -> std::optional<RayResult>;
};

}

#define OCTTREE_RAY
#endif // !OCTTREE_RAY
