/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPoint.h"
#include "GRandom.h"
#include "GRect.h"
#include "GFilter.h"
#include <string>

class RedBlueFilter : public GFilter {
public:
    RedBlueFilter() {}

    bool preservesAlpha() override { return true; }

    void filter(GPixel output[], const GPixel input[], int count) override {
        for (int i = 0; i < count; ++i) {
            unsigned a = GPixel_GetA(input[i]);
            unsigned r = GPixel_GetR(input[i]);
            unsigned g = GPixel_GetG(input[i]);
            unsigned b = GPixel_GetB(input[i]);
            output[i] = GPixel_PackARGB(a, b, g, r);
        }
    }
};

static std::unique_ptr<GShader> make_bm_shader(const char path[], const GRect& dst) {
    GBitmap bm;
    if (!bm.readFromFile(path)) {
        return nullptr;
    }

    GMatrix inv;
    inv.setTranslate(-dst.x(), -dst.y());
    inv.postScale(bm.width() / dst.width(), bm.height() / dst.height());
    return GCreateBitmapShader(bm, inv);
}

static void draw_rb_switch(GCanvas* canvas) {
    RedBlueFilter filter;
    GPaint paint;
    paint.setFilter(&filter);

    const GRect r = GRect::MakeWH(135, 135);
    const float gap = 10;

    canvas->translate(gap, gap);
    canvas->save();
    canvas->drawRect(r, paint.setColor({1, 1, 0, 0}));
    canvas->translate(r.width() + gap, 0);
    canvas->drawRect(r, paint.setColor({1, 0, 0, 1}));
    canvas->restore();

    auto spock = make_bm_shader("apps/spock.png", r);
    auto oldwell = make_bm_shader("apps/oldwell.png", r);

    canvas->translate(0, r.height() + gap);
    canvas->drawRect(r, paint.setShader(spock.get()));
    canvas->translate(r.width() + gap, 0);
    canvas->drawRect(r, paint.setShader(oldwell.get()));
}

static void draw_hole(GCanvas* canvas) {
    GRect r = GRect::MakeXYWH(50, 50, 200, 200);
    canvas->drawPaint(GPaint({1, 1, 0, 0}));
    canvas->saveLayer();
    canvas->drawPaint(GPaint({1, 0, 1, 0}));

    GPaint p;
    p.setBlendMode(GBlendMode::kClear);
    canvas->drawRect(r, p);
    canvas->restore();
}

static void draw_plus(GCanvas* canvas, float alpha) {
    canvas->drawRect(GRect::MakeXYWH(50, 110, 200, 80), GPaint({alpha, 1, 0, 0}));
    canvas->drawRect(GRect::MakeXYWH(110, 50, 80, 200), GPaint({alpha, 0, 0, 1}));
}

static void draw_pluses(GCanvas* canvas) {
    CheckerShader shader(10, 0xFFCCCCCC, 0xFFFFFFFF);
    canvas->drawPaint(GPaint(&shader));

    GRect bounds = GRect::MakeXYWH(25, 25, 250, 250);

    canvas->scale(0.5, 0.5);
    
    canvas->save();
    draw_plus(canvas, 1);
    canvas->translate(220, 0);
    draw_plus(canvas, 0.5);
    canvas->restore();

    auto filter = GCreateBlendFilter(GBlendMode::kDstIn, {0.5, 0, 0, 0});

    canvas->save();
    canvas->translate(0, 220);
    canvas->saveLayer(bounds);
    draw_plus(canvas, 0.5);
    canvas->restore();
    canvas->translate(220, 0);
    canvas->saveLayer(&bounds, GPaint(filter.get()));
    draw_plus(canvas, 1);
    canvas->restore();
    canvas->restore();
}

static void draw_mode_filter(GCanvas* canvas, const GRect& bounds, GBlendMode mode) {
    outer_frame(canvas, bounds);

    GPaint paint;
    GPoint pts[4];
    
    GPixel dstp[5] = {
        GPixel_PackARGB(0, 0, 0, 0),
        GPixel_PackARGB(0x40, 0x40, 0, 0),
        GPixel_PackARGB(0x80, 0x80, 0, 0),
        GPixel_PackARGB(0xC0, 0xC0, 0, 0),
        GPixel_PackARGB(0xFF, 0xFF, 0, 0),
    };
    GBitmap dstbm(1, 5, 1*4, dstp, false);
    auto dstsh = GCreateBitmapShader(dstbm, GMatrix::MakeScale(1.0f/bounds.width(),
                                                               5.0f/bounds.height()));

    paint.setShader(dstsh.get());
    canvas->drawConvexPolygon(rect_pts(bounds, pts), 4, paint);

    GPixel srcp[5] = {
        GPixel_PackARGB(0, 0, 0, 0),
        GPixel_PackARGB(0x40, 0, 0, 0x40),
        GPixel_PackARGB(0x80, 0, 0, 0x80),
        GPixel_PackARGB(0xC0, 0, 0, 0xC0),
        GPixel_PackARGB(0xFF, 0, 0, 0xFF),
    };
    GBitmap srcbm(5, 1, 5*4, srcp, false);
    auto srcsh = GCreateBitmapShader(srcbm, GMatrix::MakeScale(5.0f/bounds.width(),
                                                               1.0f/bounds.height()));
    paint.setShader(srcsh.get());

    float dh = bounds.height() / 5.0f;
    GRect r = bounds;
    r.fBottom = r.fTop + dh;

    for (int i = 0; i < 5; ++i) {
        float alpha = 1.0f * i / 5;
        auto filter = GCreateBlendFilter(mode, {alpha, 0, 1, 0});
        paint.setFilter(filter.get());
        canvas->drawRect(r, paint);
        r.offset(0, dh);
    }
}

static void draw_filter_blendmodes(GCanvas* canvas) {
    draw_all_blendmodes(canvas, draw_mode_filter);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static void draw_mode_paint(GCanvas* canvas, const GRect& bounds, GBlendMode mode) {
    GBitmap bm;
    {
        CheckerShader ch0(10, 0xFFFF0000, 0x00000000);
        CheckerShader ch1(20, 0xFF00FF00, 0x00000000);

        bm.alloc(100, 100);
        auto can = GCreateCanvas(bm);
        can->drawPaint(GPaint(&ch0));

        GPaint paint;
        paint.setBlendMode(mode);
        can->saveLayer(paint);
        can->drawPaint(GPaint(&ch1));
        can->restore();
    }

    outer_frame(canvas, bounds);

    GMatrix mat;
    mat.setScale(bm.width() / bounds.width(), bm.height() / bounds.height());
    mat.preTranslate(-bounds.left(), -bounds.top());

    canvas->drawRect(bounds, GPaint(GCreateBitmapShader(bm, mat).get()));
}

static void draw_paint_blendmodes(GCanvas* canvas) {
    draw_all_blendmodes(canvas, draw_mode_paint);
}
