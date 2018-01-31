#ifndef Blend_DEFINED
#define Blend_DEFINED

#include "GPixel.h"

static inline int multiplyBytes(int x, int y) {
    int result = x * y;

    return (result + 127) / 255;
}


static inline GPixel Blend_Clear(const GPixel& source, const GPixel& dest) {
    return GPixel_PackARGB(0, 0, 0, 0);
}


static inline GPixel Blend_Src(const GPixel& source, const GPixel& dest) {
    return source;
}


static inline GPixel Blend_Dst(const GPixel& source, const GPixel& dest) {
    return dest;
}


static inline GPixel Blend_SrcOver(const GPixel& source, const GPixel& dest) {
    int sAlpha = GPixel_GetA(source);
    int sRed = GPixel_GetR(source);
    int sGreen = GPixel_GetG(source);
    int sBlue = GPixel_GetB(source);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int alpha = sAlpha + multiplyBytes(255 - sAlpha, dAlpha);
    int red = sRed + multiplyBytes(255 - sAlpha, dRed);
    int green = sGreen + multiplyBytes(255 - sAlpha, dGreen);
    int blue = sBlue + multiplyBytes(255 - sAlpha, dBlue);

    return GPixel_PackARGB(alpha, red, green, blue);
}


static inline GPixel Blend_DstOver(const GPixel& source, const GPixel& dest) {
    int sAlpha = GPixel_GetA(source);
    int sRed = GPixel_GetR(source);
    int sGreen = GPixel_GetG(source);
    int sBlue = GPixel_GetB(source);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int alpha = dAlpha + multiplyBytes(255 - dAlpha, sAlpha);
    int red = dRed + multiplyBytes(255 - dAlpha, sRed);
    int green = dGreen + multiplyBytes(255 - dAlpha, sGreen);
    int blue = dBlue + multiplyBytes(255 - dAlpha, sBlue);

    return GPixel_PackARGB(alpha, red, green, blue);
}


static inline GPixel Blend_SrcIn(const GPixel& source, const GPixel& dest) {
    int sAlpha = GPixel_GetA(source);
    int sRed = GPixel_GetR(source);
    int sGreen = GPixel_GetG(source);
    int sBlue = GPixel_GetB(source);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int alpha = multiplyBytes(sAlpha, dAlpha);
    int red = multiplyBytes(dAlpha, sRed);
    int green = multiplyBytes(dAlpha, sGreen);
    int blue = multiplyBytes(dAlpha, sBlue);

    return GPixel_PackARGB(alpha, red, green, blue);
}


static inline GPixel Blend_DstIn(const GPixel& source, const GPixel& dest) {
    int sAlpha = GPixel_GetA(source);
    int sRed = GPixel_GetR(source);
    int sGreen = GPixel_GetG(source);
    int sBlue = GPixel_GetB(source);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int alpha = multiplyBytes(dAlpha, sAlpha);
    int red = multiplyBytes(sAlpha, dRed);
    int green = multiplyBytes(sAlpha, dGreen);
    int blue = multiplyBytes(sAlpha, dBlue);

    return GPixel_PackARGB(alpha, red, green, blue);
}


typedef GPixel (*BlendProc)(GPixel, GPixel);


// If this matches the order of the GBlendMode enum, we can use the enum values
// as indices for this array.
BlendProc BLEND_PROCS = {
    Blend_Clear,
    Blend_Src,
    Blend_Dst,
    Blend_SrcOver,
    Blend_DstOver,
    Blend_SrcIn,
    Blend_DstIn,
};

#endif
