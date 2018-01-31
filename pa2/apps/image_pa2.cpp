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
#include <string>

static void draw_tri(GCanvas* canvas) {
    GPoint pts[] = {
        { 10, 10 },
        { 200, 50 },
        { 100, 200 },
    };
    canvas->drawConvexPolygon(pts, GARRAY_COUNT(pts), GPaint({1, 0, 1, 0}));
}

static void draw_tri_clipped(GCanvas* canvas) {
    GPoint pts[] = {
        { -10, -10 },
        { 300, 50 },
        { 100, 300 },
    };
    canvas->drawConvexPolygon(pts, GARRAY_COUNT(pts), GPaint({1, 1, 1, 0}));
}

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = M_PI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
        angle += deltaAngle;
    }
}

static void dr_poly(GCanvas* canvas, float dx, float dy) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        for (int i = 0; i < count; ++i) {
            storage[i].fX += dx;
            storage[i].fY += dy;
        }
        GColor c = GColor::MakeARGB(0.8f,
                                    fabs(sin(count*7)),
                                    fabs(sin(count*11)),
                                    fabs(sin(count*17)));
        canvas->drawConvexPolygon(storage, count, GPaint(c));
    }
}

static void draw_poly(GCanvas* canvas) {
    dr_poly(canvas, 0, 0);
}

static void draw_poly_center(GCanvas* canvas) {
    dr_poly(canvas, -128, -128);
}

static GPoint scale(GPoint vec, float size) {
    float scale = size / sqrt(vec.fX * vec.fX + vec.fY * vec.fY);
    return GPoint::Make(vec.fX * scale, vec.fY * scale);
}

static void draw_line(GCanvas* canvas, GPoint a, GPoint b, float width, const GColor& color) {
    GPoint norm = scale(GPoint::Make(b.fY - a.fY, a.fX - b.fX), width/2);

    GPoint pts[4];
    pts[0] = GPoint::Make(a.fX + norm.fX, a.fY + norm.fY);
    pts[1] = GPoint::Make(b.fX + norm.fX, b.fY + norm.fY);
    pts[2] = GPoint::Make(b.fX - norm.fX, b.fY - norm.fY);
    pts[3] = GPoint::Make(a.fX - norm.fX, a.fY - norm.fY);

    canvas->drawConvexPolygon(pts, 4, GPaint(color));
}

static void draw_poly_rotate(GCanvas* canvas) {
    const GPoint start = GPoint::Make(20, 20);
    const float scale = 200;

    const int N = 10;
    GColor color = GColor::MakeARGB(1, 1, 0, 0);
    const float deltaR = -1.0 / N;
    const float deltaB = 1.0 / N;

    const float width = 10;

    for (float angle = 0; angle <= M_PI/2; angle += M_PI/2/N) {
        GPoint end = GPoint::Make(start.fX + cos(angle) * scale,
                                  start.fY + sin(angle) * scale);
        draw_line(canvas, start, end, width, color);

        color.fR += deltaR;
        color.fB += deltaB;
    }
}
