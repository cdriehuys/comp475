#ifndef GLayer_DEFINED
#define GLayer_DEFINED

#include "GBitmap.h"
#include "GMatrix.h"
#include "GPaint.h"
#include "GRect.h"


class GLayer {
public:
    const GBitmap fBitmap;

    GLayer(const GBitmap& bitmap, const GMatrix& ctm);
    GLayer(const GBitmap& bitmap, const GMatrix& ctm, GRect bounds, const GPaint& paint);

    GMatrix* getCTM() { return &fCTM; }

private:
    const GPaint fPaint;

    bool fIsLayer;

    GMatrix fCTM;
    GRect bounds;
};


#endif
