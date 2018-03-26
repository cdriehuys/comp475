/*
 *  Copyright 2017 Mike Reed
 */

#ifndef GProxyCanvas_DEFINED
#define GProxyCanvas_DEFINED

#include "GCanvas.h"

class GProxyCanvas : public GCanvas {
public:
    GProxyCanvas(GCanvas* proxy) : fProxy(proxy) {}

    bool virtual allowDraw() { return true; }

    void clear(const GColor& c) override {
        if (this->allowDraw()) {
            fProxy->clear(c);
        }
    }

    void fillRect(const GRect& r, const GColor& c) override {
        if (this->allowDraw()) {
            fProxy->fillRect(r, c);
        }
    }

    void fillBitmapRect(const GBitmap& b, const GRect& r) override {
        if (this->allowDraw()) {
            fProxy->fillBitmapRect(b, r);
        }
    }

    void fillConvexPolygon(const GPoint pts[], int count, const GColor& color) override {
        if (this->allowDraw()) {
            fProxy->fillConvexPolygon(pts, count, color);
        }
    }

private:
    GCanvas* fProxy;
};

#endif
