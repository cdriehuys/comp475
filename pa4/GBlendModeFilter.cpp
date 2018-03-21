#include "GBlendMode.h"
#include "GColor.h"
#include "GFilter.h"
#include "GPixel.h"

#include "Blend.h"
#include "ColorUtils.h"


class GBlendModeFilter : public GFilter {
public:
    GBlendModeFilter(GBlendMode mode, const GColor& src)
            : fMode(mode)
            , fSrc(colorToPixel(src)) {}

    bool preservesAlpha() override {
        return false;
    }

    void filter(GPixel output[], const GPixel input[], int count) override {
        BlendProc blendProc = Blend_GetProc(fMode, fSrc);

        for (int i = 0; i < count; ++i) {
            output[i] = blendProc(fSrc, input[i]);
        }
    }

private:
    GBlendMode fMode;
    GPixel fSrc;
};


std::unique_ptr<GFilter> GCreateBlendFilter(GBlendMode mode, const GColor& src) {
    return std::unique_ptr<GFilter>(new GBlendModeFilter(mode, src));
}
