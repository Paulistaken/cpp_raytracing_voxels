#include "dtypes.hpp"
#include <vector>
#include "otree/otree.hpp"
#include "raylib.h"

#ifndef VOXRENDER_H

namespace Vox_Rend{

    Color color_darken(Color col, u8 amt);
    Color mix_colors_a(Color cola, Color colb);
    Color color_add_noise(Color col, u8 noise);

    class PixelData{
        public:
        f64 deph;
        Color col;
        PixelData();
        PixelData(Color, f64);
    };

    class Screen{
        public:
        u32 screen_width;
        u32 screen_heigth;
        u32 vir_rez_w;
        u32 vir_rez_h;
        std::vector<std::vector<PixelData>> pixels;
        Screen(u32 SCREEN_WIDTH,u32 SCREEN_HEIGTH,u32 VIR_REZ_WIDTH);
        fn update_rez(u32 SCREEN_WIDTH, u32 SCREEN_HEIGTH, u32 VIR_REZ_WIDTH) -> void;
        fn __cpu__reset_scr(Color background_col) -> void;
        fn set_pixel(u32 x, u32 y,Color col, f64 deph) -> void;
        fn get_pixel(u32 x, u32 y) const -> std::optional<PixelData>;
        fn __cpu__render_scr() const -> void;

        fn __cpu__render_otree(const OCTTree::OctTree& otree, const DT3::Transform3& cam)->void;

        private:
        u32 rez_h;
        u32 rez_v;
    };

}

#define VOXRENDER_H
#endif // !VOXRENDER_H
