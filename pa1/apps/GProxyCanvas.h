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

private:
    GCanvas* fProxy;
};

#endif
