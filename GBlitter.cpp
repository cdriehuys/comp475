#include "GBlitter.h"
#include "GFilter.h"
#include "GPaint.h"
#include "GPixel.h"
#include "GShader.h"

#include "Blend.h"
#include "ColorUtils.h"


void GBlitter::blitRow(int y, int xLeft, int xRight) {
    GASSERT(xLeft <= xRight);
    xLeft = std::max(0, xLeft);
    xRight = std::min(this->fBitmap.width(), xRight);

    BlendProc blendProc = Blend_GetProc(this->fPaint.getBlendMode());

    GShader* shader = this->fPaint.getShader();
    if (shader == nullptr) {
        GColor color = this->fPaint.getColor().pinToUnit();
        GPixel source[1] = {colorToPixel(color)};

        if (this->fPaint.getFilter() != nullptr) {
            this->fPaint.getFilter()->filter(source, source, 1);
        }

        for (int x = xLeft; x < xRight; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendProc(source[0], *addr);
        }
    } else {
        int count = xRight - xLeft;
        GPixel* shaded = (GPixel*) malloc(count * sizeof(GPixel));
        shader->shadeRow(xLeft, y, count, shaded);

        if (this->fPaint.getFilter() != nullptr) {
            this->fPaint.getFilter()->filter(shaded, shaded, count);
        }

        for (int x = xLeft; x < xRight; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendProc(shaded[x - xLeft], *addr);
        }

        free(shaded);
    }
}
