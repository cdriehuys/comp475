#ifndef ColorUtils_DEFINED
#define ColorUtils_DEFINED


#include "GColor.h"


/**
 * Convert a float in the range [0.0 ... 1.0] to an 8-bit integer.
 */
static inline int floatToPixelValue(float value) {
    GASSERT(0 <= value && value <= 1);

    return GRoundToInt(value * 255);
}


/**
 * Convert a GColor to a GPixel that can actually be used.
 *
 * The color components are pre-multiplied by the alpha value before being
 * compacted.
 */
static inline GPixel colorToPixel(const GColor& color) {
    int alpha = floatToPixelValue(color.fA);
    int red   = floatToPixelValue(color.fR * color.fA);
    int green = floatToPixelValue(color.fG * color.fA);
    int blue  = floatToPixelValue(color.fB * color.fA);

    return GPixel_PackARGB(alpha, red, green, blue);
}


#endif
