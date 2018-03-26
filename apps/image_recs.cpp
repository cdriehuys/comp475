/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"

#include "image_pa1.cpp"
#include "image_pa2.cpp"

const GDrawRec gDrawRecs[] = {
    { draw_solid_ramp,  256, 7*28,  "solid_ramp",   1   },
    { draw_graphs,      256, 256,   "rect_graphs",  1   },
    { draw_blend_black, 200, 200,   "blend_black",  1   },

    { draw_poly,        512, 512,   "poly",         2 },
    { draw_poly_center, 256, 256,   "poly_center",  2 },
    { draw_blendmodes,  450, 340,   "blendmodes",   2 },

    { nullptr, 0, 0, nullptr },
};
