#include "GFilter.h"
#include "GLayer.h"
#include "GMatrix.h"
#include "GPaint.h"
#include "GPixel.h"
#include "GRect.h"

#include "Blend.h"


// Only used to store a CTM
GLayer::GLayer(const GBitmap* bitmap, GMatrix matrix, GIRect bounds) {
    this->fBitmap = *bitmap;
    this->fCTM = matrix;
    this->fBounds = bounds;

    this->fIsLayer = false;
}


// Used as a separate drawing surface
GLayer::GLayer(GBitmap& bitmap, GMatrix matrix, GIRect bounds, GPaint paint) {
    this->fBitmap = bitmap;
    this->fCTM = matrix;
    this->fBounds = bounds;
    this->fPaint = paint;

    this->fIsLayer = true;
}


void GLayer::draw(GBitmap* base) {
    GIRect bounds = this->fBounds;
    int xOffset = bounds.left();
    int yOffset = bounds.top();

    BlendProc blendProc = Blend_GetProc(this->fPaint.getBlendMode());
    GFilter* filter = this->fPaint.getFilter();

    for (int y = 0; y < this->fBitmap.height(); ++y) {
        int width = this->fBitmap.width();
        GPixel* row = (GPixel*) malloc(width * sizeof(GPixel));

        if (filter == nullptr) {
            row = this->fBitmap.getAddr(0, y);
        } else {
            filter->filter(row, this->fBitmap.getAddr(0, y), width);
        }

        for (int x = 0; x < width; ++x) {
            GPixel src = row[x];
            GPixel* dest = base->getAddr(x + xOffset, y + yOffset);

            *dest = blendProc(src, *dest);
        }
    }
}
