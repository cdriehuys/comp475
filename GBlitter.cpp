#include "GBlitter.h"
#include "GFilter.h"
#include "GPaint.h"
#include "GPixel.h"
#include "GShader.h"

#include "Blend.h"
#include "ColorUtils.h"


void GBlitter::blitRow(int y, int xLeft, int xRight, const GPaint& paint) {
    xLeft = std::max(0, xLeft);
    xRight = std::min(this->fBitmap.width(), xRight);

    BlendProc blendProc = Blend_GetProc(paint.getBlendMode());

    GShader* shader = paint.getShader();
    if (shader == nullptr) {
        GColor color = paint.getColor().pinToUnit();
        GPixel source[1] = {colorToPixel(color)};

        if (paint.getFilter() != nullptr) {
            paint.getFilter()->filter(source, source, 1);
        }

        for (int x = xLeft; x < xRight; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendProc(source[0], *addr);
        }
    } else {
        int count = xRight - xLeft;
        GPixel shaded[count];
        shader->shadeRow(xLeft, y, count, shaded);

        if (paint.getFilter() != nullptr) {
            paint.getFilter()->filter(shaded, shaded, count);
        }

        for (int x = xLeft; x < xRight; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendProc(shaded[x - xLeft], *addr);
        }
    }
}
