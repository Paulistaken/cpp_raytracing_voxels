#include "dtypes.hpp"
#include "render_shader.hpp"

#ifndef GAMETYPES

namespace GameTypes{

    typedef DT3::Vec3 Vec3;

    struct LightSource{
        public:
        Vec3 pos;
        Vec3 orgin;
        Vec3 color;
        float strengh;
        float dep;
        u32 rc;
        void render(RenderShader&, u32 index) const;
    };
}

#define GAMETYPES
#endif // !GAMEDATA
