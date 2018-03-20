/**
 *  Copyright 2018 Mike Reed
 */

#include "GCanvas.h"
#include "GFilter.h"
#include "GShader.h"
#include "tests.h"

class got_called_filter : public GFilter {
public:
    GPixel fPixel = 0;

    // sets fPixel to an illegal value (not premul)
    void reset() { fPixel = 0x00FF0000; }

    bool preservesAlpha() override { return true; }
    void filter(GPixel output[], const GPixel input[], int count) override {
        if (count > 0) {
            memcpy(output, input, count * sizeof(GPixel));
            fPixel = input[0];
        }
    }
};

class pixel_shader : public GShader {
    GPixel fPixel;
public:
    pixel_shader(GPixel p) : fPixel(p) {}

    bool isOpaque() override { return GPixel_GetA(fPixel) == 0xFF; }
    bool setContext(const GMatrix&) override { return true; }
    void shadeRow(int, int, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            row[i] = fPixel;
        }
    }
};

static void test_filters(GTestStats* stats) {
    got_called_filter filter;
    GPixel pixel = GPixel_PackARGB(0xFF, 0xFF, 0, 0);   // red
    pixel_shader shader(pixel);

    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();
    GPaint paint;
    paint.setFilter(&filter);

    GShader* const shaders[] = { nullptr, &shader };
    for (GShader* sh : shaders) {
        paint.setColor({1,1,0,0});
        paint.setShader(sh);
        // set to black if we have a shader, so we know the shader got called
        if (sh) {
            paint.setColor({1,0,0,0});
        }

        filter.reset();
        canvas->drawPaint(paint);
        stats->expectTrue(filter.fPixel == pixel, "paint-called-filter");

        filter.reset();
        canvas->drawRect(GRect::MakeWH(5, 5), paint);
        stats->expectTrue(filter.fPixel == pixel, "rect-called-filter");

        GPoint pts[] = {{0,0}, {5,0}, {0,5}};
        filter.reset();
        canvas->drawConvexPolygon(pts, 3, paint);
        stats->expectTrue(filter.fPixel == pixel, "poly-called-filter");
    }
}

static bool expect_one_color(const GBitmap& bm, GPixel color) {
    bool valid = true;
    visit_pixels(bm, [&valid, color](int x, int y, GPixel* addr) {
        valid &= *addr == color;
    });
    return valid;
}

static void test_savelayesr(GTestStats* stats) {
    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();
    
    int n = 1000;
    canvas->saveLayer(GRect::MakeXYWH(-n, -n, n, n));
    canvas->clear({1,1,1,1});
    canvas->restore();
    stats->expectTrue(expect_one_color(surface.bitmap(), 0), "offscreen layer0");

    canvas->clear({0,0,0,0});
    stats->expectTrue(expect_one_color(surface.bitmap(), 0), "offscreen layer1");
    canvas->saveLayer(GRect::MakeXYWH(-n, -n, n+1, n+1));
    canvas->clear({1,1,1,1});
    stats->expectTrue(expect_one_color(surface.bitmap(), 0), "offscreen layer2");
    canvas->restore();
    
    bool valid = true;
    visit_pixels(surface.bitmap(), [&valid](int x, int y, GPixel* addr) {
        valid &= (*addr != 0 && x == 0 && y == 0) || (*addr == 0 && (x != 0 || y != 0));
    });
    stats->expectTrue(valid, "offscreen layer3");
    
}

