#include "GLayer.h"
#include "GMatrix.h"


GLayer::GLayer(const GBitmap& bitmap, const GMatrix& ctm)
        : fBitmap(bitmap) {
    this->fIsLayer = false;
    this->fCTM = GMatrix(
        ctm[0], ctm[1], ctm[2],
        ctm[3], ctm[4], ctm[5]);
}


GLayer::GLayer(const GBitmap& bitmap, const GMatrix& ctm, GRect bounds, const GPaint& paint) {

}
