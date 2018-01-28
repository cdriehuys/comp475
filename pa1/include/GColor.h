/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GColor_DEFINED
#define GColor_DEFINED

#include "GMath.h"
#include "GPixel.h"

/**
 *  Defines an un-premultiplied color, where alpha, red, green, blue are all independent of
 *  each other. Legal values for each component are [0.0 .. 1.0].
 */
class GColor {
public:
    float   fA;
    float   fR;
    float   fG;
    float   fB;

    static GColor MakeARGB(float a, float r, float g, float b) {
        GColor c = { a, r, g, b };
        return c;
    }

    GColor pinToUnit() const {
        return MakeARGB(GPinToUnit(fA), GPinToUnit(fR), GPinToUnit(fG), GPinToUnit(fB));
    }
};

#endif
