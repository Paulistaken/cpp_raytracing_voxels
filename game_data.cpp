#include "game_data.hpp"

namespace GameTypes {
    void LightSource::render(RenderShader& rd, u32 index) const{
        rd.run_light_raytracing(index, this->pos + this->orgin, this->strengh, this->dep, this->rc, this->color);

    }
}
