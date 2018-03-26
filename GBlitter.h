#ifndef GBlitter_DEFINED
#define GBlitter_DEFINED

#include "GBitmap.h"
#include "GPaint.h"


class GBlitter {
public:
    /**
     * Construct a new blitter that draws to the provided bitmap.
     *
     * Args:
     *     bitmap:
     *         The bitmap that gets drawn to.
     */
    GBlitter(const GBitmap& bitmap) : fBitmap(bitmap) {}

    /**
     * Draw a row to the blitter's bitmap.
     *
     * Args:
     *     y:
     *         The y-coordinate of the row to draw.
     *     xLeft:
     *         The x-coordinate of the beginning of the row.
     *     xRight:
     *         The x-coordinate of the end of the row.
     *     paint:
     *         The paint to draw the row with.
     */
    void blitRow(int y, int xLeft, int xRight, const GPaint& paint);

private:
    const GBitmap fBitmap;
};


#endif
