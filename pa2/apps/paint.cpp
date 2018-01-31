/**
 *  Copyright 2018 Mike Reed
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"

#include <vector>

static GRandom gRand;

static GColor rand_color() {
    return GColor::MakeARGB(0.5f, gRand.nextF(), gRand.nextF(), gRand.nextF());
}

class Brush {
public:
    virtual ~Brush() {}

    virtual void draw(GCanvas*, GPoint loc) = 0;
};

static GRect offset(const GRect& r, GPoint offset) {
    return GRect::MakeXYWH(r.left() + offset.x(), r.top() + offset.y(), r.width(), r.height());
}

class RectBrush : public Brush {
public:
    RectBrush(const GRect& r, const GColor& c) : fR(r), fC(c) {}

    void draw(GCanvas* canvas, GPoint loc) override {
        canvas->fillRect(offset(fR, loc), fC);
    }

private:
    GRect   fR;
    GColor  fC;
};

class TestWindow : public GWindow {
    GBitmap fBitmap;

public:
    TestWindow(int w, int h) : GWindow(w, h) {
        fBitmap.alloc(1024, 768);
        memset(fBitmap.getAddr(0, 0), 0xFF, fBitmap.height() * fBitmap.rowBytes());
    }

    virtual ~TestWindow() {}
    
protected:
    void onUpdate(const GBitmap& dst, GCanvas* canvas) override {
        int w = std::min(dst.width(), fBitmap.width());
        int h = std::min(dst.height(), fBitmap.height());
        for (int y = 0; y < h; ++y) {
            memcpy(dst.getAddr(0, y), fBitmap.getAddr(0, y), w << 2);
        }
    }

    bool onKeyPress(uint32_t sym) override {
        switch (sym) {
            case 'c':
            case 'C':
                GCreateCanvas(fBitmap)->clear({1, 1, 1, 1});
                this->requestDraw();
                return true;
        }
        return false;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        GRect r = GRect::MakeWH(20, 20);
        Brush* b = new RectBrush(r, rand_color());
        GCanvas* c = GCreateCanvas(fBitmap).release();
        return new GClick(loc, [this, c, b](GClick* click) {
            if (click->state() == GClick::kUp_State) {
                delete c;
                delete b;
            } else {
                b->draw(c, click->curr());
                this->requestDraw();
            }
        });
    }

private:
    void updateTitle() {
        char buffer[100];
        buffer[0] = ' ';
        buffer[1] = 0;
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow(640, 480);

    return wind->run();
}

