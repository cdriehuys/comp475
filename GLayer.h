#ifndef GLayer_DEFINED
#define GLayer_DEFINED

#include "GBitmap.h"
#include "GMatrix.h"
#include "GPaint.h"
#include "GRect.h"


class GLayer {
public:
    const GBitmap fBitmap;
    bool fIsLayer;

    GLayer(const GBitmap& bitmap, const GMatrix& ctm);
    GLayer(const GBitmap& bitmap, const GMatrix& ctm, GIRect bounds, const GPaint& paint);

    GMatrix* getCTM() { return &fCTM; }

    void draw(const GBitmap& base);

private:
    const GPaint fPaint;

    GMatrix fCTM;
    GIRect fBounds;
};


#endif
