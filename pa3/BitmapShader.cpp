#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"


class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localInv)
        : fSourceBitmap(bitmap)
        , fLocalMatrix(localInv) {}

    bool isOpaque() override {
        return false;
    }

    bool setContext(const GMatrix& ctm) override {
        if (!ctm.invert(&fInverse)) {
            return false;
        }

        fInverse.postConcat(fLocalMatrix);

        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint local = fInverse.mapXY(x + 0.5f, y + 0.5f);

        for (int i = 0; i < count; ++i) {
            int sourceX = GRoundToInt(local.fX);
            int sourceY = GRoundToInt(local.fY);

            // Clamp values
            sourceX = std::max(0, std::min(fSourceBitmap.width() - 1, sourceX));
            sourceY = std::max(0, std::min(fSourceBitmap.height() - 1, sourceY));

            row[i] = *fSourceBitmap.getAddr(sourceX, sourceY);

            local.fX += fInverse[GMatrix::SX];
            local.fY += fInverse[GMatrix::KY];
        }
    }

private:
    GBitmap fSourceBitmap;
    GMatrix fInverse;
    GMatrix fLocalMatrix;
};


std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localInv) {
    if (!bitmap.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GShader>(new BitmapShader(bitmap, localInv));
}
