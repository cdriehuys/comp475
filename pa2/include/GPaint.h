/*
 *  Copyright 2016 Mike Reed
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GColor.h"

enum class GBlendMode {
    kClear,    //!< [0, 0]
    kSrc,      //!< [Sa, Sc]
    kDst,      //!< [Da, Dc]
    kSrcOver,  //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
    kDstOver,  //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
    kSrcIn,    //!< [Sa * Da, Sc * Da]
    kDstIn,    //!< [Da * Sa, Dc * Sa]
    kSrcOut,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
    kDstOut,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    kSrcATop,  //!< [Da, Sc * Da + Dc * (1 - Sa)]
    kDstATop,  //!< [Sa, Dc * Sa + Sc * (1 - Da)]
    kXor,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
};

class GPaint {
public:
    GPaint() : fColor(GColor::MakeARGB(1, 0, 0, 0)) {}
    GPaint(const GColor& c) : fColor(c) {}

    const GColor& getColor() const { return fColor; }
    GPaint& setColor(GColor c) { fColor = c; return *this; }

    float getAlpha() const { return fColor.fA; }
    GPaint& setAlpha(float alpha) {
        fColor.fA = alpha;
        return *this;
    }

    GBlendMode  getBlendMode() const { return fMode; }
    GPaint& setBlendMode(GBlendMode m) { fMode = m; return *this; }

private:
    GColor      fColor;
    GBlendMode  fMode = GBlendMode::kSrcOver;
};

#endif
