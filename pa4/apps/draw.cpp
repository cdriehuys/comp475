/**
 *  Copyright 2015 Mike Reed
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GFilter.h"
#include "GRandom.h"
#include "GRect.h"
#include "GShader.h"

#include <vector>

static const float CORNER_SIZE = 9;

template <typename T> int find_index(const std::vector<T*>& list, T* target) {
    for (int i = 0; i < list.size(); ++i) {
        if (list[i] == target) {
            return i;
        }
    }
    return -1;
}

static GRandom gRand;

static GColor rand_color() {
    return GColor::MakeARGB(0.5f, gRand.nextF(), gRand.nextF(), gRand.nextF());
}

static GRect make_from_pts(const GPoint& p0, const GPoint& p1) {
    return GRect::MakeLTRB(std::min(p0.fX, p1.fX), std::min(p0.fY, p1.fY),
                           std::max(p0.fX, p1.fX), std::max(p0.fY, p1.fY));
}

static bool contains(const GRect& rect, float x, float y) {
    return rect.left() < x && x < rect.right() && rect.top() < y && y < rect.bottom();
}

static GRect offset(const GRect& rect, float dx, float dy) {
    return GRect::MakeLTRB(rect.left() + dx, rect.top() + dy,
                           rect.right() + dx, rect.bottom() + dy);
}

static bool hit_test(float x0, float y0, float x1, float y1) {
    const float dx = fabs(x1 - x0);
    const float dy = fabs(y1 - y0);
    return std::max(dx, dy) <= CORNER_SIZE;
}

static bool in_resize_corner(const GRect& r, float x, float y, GPoint* anchor) {
    if (hit_test(r.left(), r.top(), x, y)) {
        anchor->set(r.right(), r.bottom());
        return true;
    } else if (hit_test(r.right(), r.top(), x, y)) {
        anchor->set(r.left(), r.bottom());
        return true;
    } else if (hit_test(r.right(), r.bottom(), x, y)) {
        anchor->set(r.left(), r.top());
        return true;
    } else if (hit_test(r.left(), r.bottom(), x, y)) {
        anchor->set(r.right(), r.top());
        return true;
    }
    return false;
}

static void draw_corner(GCanvas* canvas, const GColor& c, float x, float y, float dx, float dy) {
    canvas->fillRect(make_from_pts(GPoint::Make(x, y - 1), GPoint::Make(x + dx, y + 1)), c);
    canvas->fillRect(make_from_pts(GPoint::Make(x - 1, y), GPoint::Make(x + 1, y + dy)), c);
}

static void draw_hilite(GCanvas* canvas, const GRect& r) {
    const float size = CORNER_SIZE;
    GColor c = GColor::MakeARGB(1, 0, 0, 0);
    draw_corner(canvas, c, r.fLeft, r.fTop, size, size);
    draw_corner(canvas, c, r.fLeft, r.fBottom, size, -size);
    draw_corner(canvas, c, r.fRight, r.fTop, -size, size);
    draw_corner(canvas, c, r.fRight, r.fBottom, -size, -size);
}

static void constrain_color(GColor* c) {
    c->fA = std::max(std::min(c->fA, 1.f), 0.1f);
    c->fR = std::max(std::min(c->fR, 1.f), 0.f);
    c->fG = std::max(std::min(c->fG, 1.f), 0.f);
    c->fB = std::max(std::min(c->fB, 1.f), 0.f);
}

class Shape {
public:
    virtual ~Shape() {}
    virtual void draw(GCanvas* canvas) {}
    virtual GRect getRect() = 0;
    virtual void setRect(const GRect&) {}
    virtual GColor getColor() = 0;
    virtual void setColor(const GColor&) {}
};

class RectShape : public Shape {
public:
    RectShape(GColor c) : fColor(c) {
        fRect = GRect::MakeXYWH(0, 0, 0, 0);
    }

    void draw(GCanvas* canvas) override {
        canvas->fillRect(fRect, fColor);
    }

    GRect getRect() override { return fRect; }
    void setRect(const GRect& r) override { fRect = r; }
    GColor getColor() override { return fColor; }
    void setColor(const GColor& c) override { fColor = c; }

private:
    GRect   fRect;
    GColor  fColor;
};

class BitmapShape : public Shape {
public:
    BitmapShape(const GBitmap& bm) : fBM(bm) {
        fRect = GRect::MakeXYWH(20, 20, 150, 150);
    }

    void draw(GCanvas* canvas) override {
        GPaint paint;
#if 0
        GMatrix inv, m = GMatrix::MakeScale(fRect.width() / fBM.width(),
                                            fRect.height() / fBM.height());
        m.postTranslate(fRect.left(), fRect.top());
        if (m.invert(&inv)) {
            auto sh = GCreateBitmapShader(fBM, inv);
            paint.setShader(sh.get());
            canvas->drawRect(fRect, paint);
        }
#else
        auto sh = GCreateBitmapShader(fBM, GMatrix());
        paint.setShader(sh.get());
        auto fl = GCreateBlendFilter(GBlendMode::kDstIn, fColor);
        paint.setFilter(fl.get());

        canvas->save();
        canvas->translate(fRect.left(), fRect.top());
        canvas->scale(fRect.width() / fBM.width(),
                      fRect.height() / fBM.height());
        canvas->drawRect(GRect::MakeWH(fBM.width(), fBM.height()), paint);
        canvas->restore();
#endif
    }

    GRect getRect() override { return fRect; }
    void setRect(const GRect& r) override { fRect = r; }
    GColor getColor() override { return fColor; }
    void setColor(const GColor& c) override { fColor = c; }

private:
    GRect   fRect;
    GColor  fColor = { 1, 0, 0, 0 };
    GBitmap fBM;
};

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float rx, float ry) {
    float angle = 0;
    const float deltaAngle = M_PI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * rx, cy + sin(angle) * ry);
        angle += deltaAngle;
    }
}

class ConvexShape : public Shape {
public:
    ConvexShape(GColor c, int sides) : fPaint(c), fN(sides) {
        fBounds.setXYWH(100, 100, 150, 150);
    }

    void draw(GCanvas* canvas) override {
        float sx = fBounds.width() * 0.5f;
        float sy = fBounds.height() * 0.5f;
        float cx = (fBounds.left() + fBounds.right()) * 0.5f;
        float cy = (fBounds.top() + fBounds.bottom()) * 0.5f;

        GPoint* pts = new GPoint[fN];
        make_regular_poly(pts, fN, cx, cy, sx, sy);
        canvas->drawConvexPolygon(pts, fN, fPaint);
        delete[] pts;
    }
    
    GRect getRect() override { return fBounds; }
    void setRect(const GRect& r) override { fBounds = r; }
    GColor getColor() override { return fPaint.getColor(); }
    void setColor(const GColor& c) override { fPaint.setColor(c); }

private:
    GPaint  fPaint;
    int     fN;
    GRect   fBounds;
};

static Shape* cons_up_shape(unsigned index) {
    const char* names[] = {
        "apps/spock.png", "apps/oldwell.png",
    };
    if (index < GARRAY_COUNT(names)) {
        GBitmap bm;
        if (bm.readFromFile(names[index])) {
            return new BitmapShape(bm);
        }
    }
    if (index == 2) {
        int n = (int)(3 + gRand.nextF() * 12);
        return new ConvexShape(rand_color(), n);
    }
    return nullptr;
}

class TestWindow : public GWindow {
    std::vector<Shape*> fList;
    Shape* fShape;
    GColor fBGColor;

public:
    TestWindow(int w, int h) : GWindow(w, h) {
        fBGColor = GColor::MakeARGB(1, 1, 1, 1);
        fShape = NULL;
    }

    virtual ~TestWindow() {}
    
protected:
    void onDraw(GCanvas* canvas) override {
        canvas->fillRect(GRect::MakeXYWH(0, 0, 10000, 10000), fBGColor);

        for (int i = 0; i < fList.size(); ++i) {
            fList[i]->draw(canvas);
        }
        if (fShape) {
            draw_hilite(canvas, fShape->getRect());
        }
    }

    bool onKeyPress(uint32_t sym) override {
        {
            Shape* s = cons_up_shape(sym - '1');
            if (s) {
                fList.push_back(fShape = s);
                this->updateTitle();
                this->requestDraw();
                return true;
            }
        }

        if (fShape) {
            switch (sym) {
                case SDLK_UP: {
                    int index = find_index(fList, fShape);
                    if (index < fList.size() - 1) {
                        std::swap(fList[index], fList[index + 1]);
                        this->requestDraw();
                        return true;
                    }
                    return false;
                }
                case SDLK_DOWN: {
                    int index = find_index(fList, fShape);
                    if (index > 0) {
                        std::swap(fList[index], fList[index - 1]);
                        this->requestDraw();
                        return true;
                    }
                    return false;
                }
                case SDLK_DELETE:
                case SDLK_BACKSPACE:
                    this->removeShape(fShape);
                    fShape = NULL;
                    this->updateTitle();
                    this->requestDraw();
                    return true;
                default:
                    break;
            }
        }

        GColor c = fShape ? fShape->getColor() : fBGColor;
        const float delta = 0.1f;
        switch (sym) {
            case 'a': c.fA -= delta; break;
            case 'A': c.fA += delta; break;
            case 'r': c.fR -= delta; break;
            case 'R': c.fR += delta; break;
            case 'g': c.fG -= delta; break;
            case 'G': c.fG += delta; break;
            case 'b': c.fB -= delta; break;
            case 'B': c.fB += delta; break;
            default:
                return false;
        }
        constrain_color(&c);
        if (fShape) {
            fShape->setColor(c);
        } else {
            c.fA = 1;   // need the bg to stay opaque
            fBGColor = c;
        }
        this->updateTitle();
        this->requestDraw();
        return true;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        if (fShape) {
            GPoint anchor;
            if (in_resize_corner(fShape->getRect(), loc.x(), loc.y(), &anchor)) {
                return new GClick(loc, [this, anchor](GClick* click) {
                    fShape->setRect(make_from_pts(click->curr(), anchor));
                    this->updateTitle();
                    this->requestDraw();
                });
            }
        }

        for (int i = fList.size() - 1; i >= 0; --i) {
            if (contains(fList[i]->getRect(), loc.x(), loc.y())) {
                fShape = fList[i];
                this->updateTitle();
                return new GClick(loc, [this](GClick* click) {
                    const GPoint curr = click->curr();
                    const GPoint prev = click->prev();
                    fShape->setRect(offset(fShape->getRect(), curr.x() - prev.x(), curr.y() - prev.y()));
                    this->updateTitle();
                    this->requestDraw();
                });
            }
        }
        
        // else create a new shape
        fShape = new RectShape(rand_color());
        fList.push_back(fShape);
        this->updateTitle();
        return new GClick(loc, [this](GClick* click) {
            if (fShape && GClick::kUp_State == click->state()) {
                if (fShape->getRect().isEmpty()) {
                    this->removeShape(fShape);
                    fShape = NULL;
                    return;
                }
            }
            fShape->setRect(make_from_pts(click->orig(), click->curr()));
            this->updateTitle();
            this->requestDraw();
        });
    }

private:
    void removeShape(Shape* target) {
        GASSERT(target);

        std::vector<Shape*>::iterator it = std::find(fList.begin(), fList.end(), target);
        if (it != fList.end()) {
            fList.erase(it);
        } else {
            GASSERT(!"shape not found?");
        }
    }

    void updateTitle() {
        char buffer[100];
        buffer[0] = ' ';
        buffer[1] = 0;

        GColor c = fBGColor;
        if (fShape) {
            c = fShape->getColor();
        }

        sprintf(buffer, "%02X %02X %02X %02X",
                int(c.fA * 255), int(c.fR * 255), int(c.fG * 255), int(c.fB * 255));
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow(640, 480);

    return wind->run();
}

