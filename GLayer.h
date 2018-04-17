#ifndef GLayer_DEFINED
#define GLayer_DEFINED

#include "GBitmap.h"
#include "GMatrix.h"
#include "GPaint.h"
#include "GRect.h"


/**
 * A layer can be thought of as a drawing context. Drawing operations performed
 * on a layer are independent of other layers until they are merged.
 */
class GLayer {
public:
    /**
     * Create a new "layer" that is only used to store a transformation matrix.
     *
     * Args:
     *     bitmap:
     *         A pointer to the bitmap that the layer should draw on.
     *     matrix:
     *         The matrix that the layer should use as its CTM.
     *     bounds:
     *         The global boundaries of the layer. Even though a layer that
     *         only holds a matrix doesn't need the bounds, they might be
     *         required by the next layer.
     */
    GLayer(const GBitmap* bitmap, GMatrix matrix, GIRect bounds);

    /**
     * Create a new layer to draw onto. The layer can then be drawn onto a
     * bitmap using the specified paint.
     *
     * Args:
     *     bitmap:
     *         The bitmap that the layer draws to before it is merged with its
     *         base layer.
     *     matrix:
     *         The layer's CTM.
     *     bounds:
     *         The layer's bounds relative to its parent layer.
     *     paint:
     *         The paint to use when drawing this layer onto another. Only the
     *         paint's filter and blend mode matter.
     */
    GLayer(GBitmap& bitmap, GMatrix matrix, GIRect bounds, GPaint paint);

    /**
     * Draw the layer onto another bitmap.
     *
     * When drawing the layers, the filter attached to the layer's paint is
     * first applied to the whole layer. Then each pixel from the layer is
     * drawn to the base offset by the amount given in the layer's bounds.
     *
     * Args:
     *     base:
     *         A pointer to the bitmap that the layer should be drawn to.
     */
    void draw(GBitmap* base);

    bool isLayer() { return fIsLayer; }
    GBitmap& getBitmap() { return fBitmap; }
    GIRect getBounds() { return fBounds; }
    GMatrix& getCTM() { return fCTM; }

private:
    bool fIsLayer = false;
    GBitmap fBitmap;
    GIRect fBounds;
    GMatrix fCTM;
    GPaint fPaint;
};


#endif
