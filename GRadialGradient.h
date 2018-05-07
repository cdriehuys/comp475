#ifndef GRadialGradient_DEFINED
#define GRadialGradient_DEFINED


#include "GColor.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GShader.h"


class GRadialGradient : public GShader {
public:
    GRadialGradient(GPoint center, float radius, const GColor colors[], int count);

    bool isOpaque();
    bool setContext(const GMatrix& ctm);
    void shadeRow(int x, int y, int count, GPixel row[]);

private:
    GPoint fCenter;
    float fRadius;

    GColor* fColors;
    int fColorCount;

    GMatrix fLocalMatrix;
};


#endif
