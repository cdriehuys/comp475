#include "GBitmap.h"
#include "GShader.h"


class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap) {}
};


std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localInv) {
    return nullptr;
}
