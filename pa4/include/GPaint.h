/*
 *  Copyright 2016 Mike Reed
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GColor.h"
#include "GBlendMode.h"

class GFilter;
class GShader;

class GPaint {
public:
    GPaint() {}
    GPaint(const GColor& c) : fColor(c) {}
    GPaint(GShader* sh) : fShader(sh) {}
    GPaint(GFilter* fl) : fFilter(fl) {}

    const GColor& getColor() const { return fColor; }
    GPaint& setColor(GColor c) { fColor = c; return *this; }

    float getAlpha() const { return fColor.fA; }
    GPaint& setAlpha(float alpha) {
        fColor.fA = alpha;
        return *this;
    }

    GBlendMode getBlendMode() const { return fMode; }
    GPaint& setBlendMode(GBlendMode m) { fMode = m; return *this; }

    GShader* getShader() const { return fShader; }
    GPaint&  setShader(GShader* s) { fShader = s; return *this; }

    GFilter* getFilter() const { return fFilter; }
    GPaint&  setFilter(GFilter* filter) { fFilter = filter; return *this; }

private:
    GColor      fColor = GColor::MakeARGB(1, 0, 0, 0);
    GShader*    fShader = nullptr;
    GFilter*    fFilter = nullptr;
    GBlendMode  fMode = GBlendMode::kSrcOver;
};

#endif
