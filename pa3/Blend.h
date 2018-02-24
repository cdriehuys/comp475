#ifndef Blend_DEFINED
#define Blend_DEFINED

#include "GPaint.h"
#include "GPixel.h"


/**
 * BlendProc is a function that blends a source and destination pixel using a
 * specific Porter-Duff blend mode.
 */
typedef GPixel (*BlendProc)(GPixel, GPixel);


GPixel Blend_Clear(const GPixel source, const GPixel dest);
GPixel Blend_Src(const GPixel source, const GPixel dest);
GPixel Blend_Dst(const GPixel source, const GPixel dest);
GPixel Blend_SrcOver(const GPixel source, const GPixel dest);
GPixel Blend_DstOver(const GPixel source, const GPixel dest);
GPixel Blend_SrcIn(const GPixel source, const GPixel dest);
GPixel Blend_DstIn(const GPixel source, const GPixel dest);
GPixel Blend_SrcOut(const GPixel source, const GPixel dest);
GPixel Blend_DstOut(const GPixel source, const GPixel dest);
GPixel Blend_SrcATop(const GPixel source, const GPixel dest);
GPixel Blend_DstATop(const GPixel source, const GPixel dest);
GPixel Blend_Xor(const GPixel source, const GPixel dest);


// If this matches the order of the GBlendMode enum, we can use the enum values
// as indices for this array.
const BlendProc Blend_PROCS[] = {
    Blend_Clear,
    Blend_Src,
    Blend_Dst,
    Blend_SrcOver,
    Blend_DstOver,
    Blend_SrcIn,
    Blend_DstIn,
    Blend_SrcOut,
    Blend_DstOut,
    Blend_SrcATop,
    Blend_DstATop,
    Blend_Xor,
};


/**
 * Get the correct blend function for the provided blend mode.
 *
 * Args:
 *     mode:
 *         The blend mode that tells us which blend function to use.
 *     src:
 *         The source pixel used for the blending operation. We can make
 *         certain optimizations based on the transparency of the source pixel.
 *
 * Returns:
 *     A BlendProc, which is a pointer to a blend function.
 */
BlendProc Blend_GetProc(const GBlendMode mode, const GPixel src);

#endif
