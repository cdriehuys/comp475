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

    { draw_tri,         256, 256,   "tri",          2 },
    { draw_tri_clipped, 256, 256,   "tri_clipped",  2 },
    { draw_poly,        512, 512,   "poly",         2 },
    { draw_poly_center, 256, 256,   "poly_center",  2 },
    { draw_poly_rotate, 230, 230,   "poly_rotate",  2 },

    { nullptr, 0, 0, nullptr },
};
