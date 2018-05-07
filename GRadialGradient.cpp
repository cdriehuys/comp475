#include <math.h>

#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRadialGradient.h"

#include "ColorUtils.h"
#include "MathUtils.h"


GRadialGradient::GRadialGradient(GPoint center, float radius, const GColor colors[], int count) {
    fCenter = center;
    fRadius = radius;

    fColors = (GColor*) malloc(count * sizeof(GColor));
    memcpy(fColors, colors, count * sizeof(GColor));
    fColorCount = count;
}


bool GRadialGradient::isOpaque() {
    return false;
}


bool GRadialGradient::setContext(const GMatrix& ctm) {
    return ctm.invert(&fLocalMatrix);
}


void GRadialGradient::shadeRow(int x, int y, int count, GPixel row[]) {
    for (int i = 0; i < count; ++i) {
        GPoint point = fLocalMatrix.mapXY(x + i, y);

        float dx = point.fX - fCenter.fX;
        float dy = point.fY - fCenter.fY;
        float distance = sqrtf(dx * dx + dy * dy);

        float t = distance / fRadius;
        t = clamp(t, 0.0f, 1.0f);

        if (t == 0) {
            row[i] = colorToPixel(fColors[0].pinToUnit());
        } else if (t == 1) {
            row[i] = colorToPixel(fColors[fColorCount - 1].pinToUnit());
        } else {
            int index = floor(t * (fColorCount - 1));
            float span = 1.0f / (fColorCount - 1);
            float start = index * span;

            GColor c1 = fColors[index].pinToUnit();
            GColor c2 = fColors[index + 1].pinToUnit();

            t = clamp((t - start) / span, 0.0f, 1.0f);

            GColor color = GColor::MakeARGB(
                c1.fA * (1 - t) + c2.fA * t,
                c1.fR * (1 - t) + c2.fR * t,
                c1.fG * (1 - t) + c2.fG * t,
                c1.fB * (1 - t) + c2.fB * t);

            row[i] = colorToPixel(color);
        }
    }
}
