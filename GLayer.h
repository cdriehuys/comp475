#ifndef GLayer_DEFINED
#define GLayer_DEFINED

#include "GBitmap.h"
#include "GMatrix.h"
#include "GPaint.h"
#include "GRect.h"


class GLayer {
public:
    GLayer(const GMatrix& ctm);
    GLayer(const GMatrix& ctm, GRect bounds, const GPaint& paint);

    GMatrix* getCTM() { return &fCTM; }

private:
    bool fIsLayer;

    GBitmap fBitmap;
    GMatrix fCTM;
    const GPaint fPaint;
    GRect bounds;
};


#endif
