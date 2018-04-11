#include "GLayer.h"
#include "GMatrix.h"

#include "Blend.h"


GLayer::GLayer(const GBitmap& bitmap, const GMatrix& ctm)
        : fBitmap(bitmap) {
    this->fIsLayer = false;
    this->fCTM = GMatrix(
        ctm[0], ctm[1], ctm[2],
        ctm[3], ctm[4], ctm[5]);
}


GLayer::GLayer(const GBitmap& bitmap, const GMatrix& ctm, GIRect bounds, const GPaint& paint)
        : fBitmap(bitmap)
        , fBounds(bounds)
        , fPaint(paint) {
    this->fIsLayer = true;
    this->fCTM = GMatrix(
        ctm[0], ctm[1], ctm[2],
        ctm[3], ctm[4], ctm[5]);
}


void GLayer::draw(const GBitmap& base) {
    GIRect bounds = this->fBounds;
    int xOffset = bounds.left();
    int yOffset = bounds.top();

    BlendProc blendProc = Blend_GetProc(this->fPaint.getBlendMode());

    for (int y = 0; y < this->fBitmap.height(); ++y) {
        for (int x = 0; x < this->fBitmap.width(); ++x) {
            GPixel* addr = base.getAddr(x + xOffset, y + yOffset);
            *addr = blendProc(*this->fBitmap.getAddr(x, y), *addr);
        }
    }
}
