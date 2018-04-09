/**
 *  Copyright 2015 Mike Reed
 */

#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPoint.h"
#include "GRect.h"
#include "tests.h"

static void setup_bitmap(GBitmap* bitmap, int w, int h) {
    size_t rb = w << 2;
    bitmap->reset(w, h, rb, (GPixel*)calloc(h, rb), GBitmap::kNo_IsOpaque);
}

static void clear(const GBitmap& bitmap) {
    memset(bitmap.pixels(), 0, bitmap.rowBytes() * bitmap.height());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool mem_eq(const void* ptr, int value, size_t size) {
    const char* cptr = (const char*)ptr;
    for (int i = 0; i < size; ++i) {
        if (cptr[i] != value) {
            return false;
        }
    }
    return true;
}

static bool bitmap_pix_eq(const GBitmap& bitmap, GPixel inside, GPixel outside) {
    const int lastX = bitmap.rowBytes() >> 2;
    const GPixel* row = bitmap.pixels();

    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            if (row[x] != inside) {
                return false;
            }
        }
        for (int x = bitmap.width(); x < lastX; ++x) {
            if (row[x] != outside) {
                return false;
            }
        }
        row += lastX;
    }
    return true;
}

static void test_clear(GTestStats* stats) {
    int w = 10, h = 10;
    size_t rb = (w + 11) * sizeof(GPixel);
    size_t size = rb * h;
    GBitmap bitmap(w, h, rb, (GPixel*)malloc(size), GBitmap::kNo_IsOpaque);

    const int wacky_component = 123;

    memset(bitmap.pixels(), wacky_component, size);
    auto canvas = GCreateCanvas(bitmap);

    // ensure that creating the canvas didn't change any pixels
    stats->expectTrue(mem_eq(bitmap.pixels(), wacky_component, size), "clear 0");

    const GPixel wacky_pixel = GPixel_PackARGB(wacky_component, wacky_component,
                                               wacky_component, wacky_component);

    canvas->clear(GColor::MakeARGB(0, 1, 1, 1));
    stats->expectTrue(bitmap_pix_eq(bitmap, 0, wacky_pixel), "clear 1");

    canvas->clear(GColor::MakeARGB(1, 1, 1, 1));
    stats->expectTrue(bitmap_pix_eq(bitmap, 0xFFFFFFFF, wacky_pixel), "clear 2");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool check9(const GBitmap& bitmap, const GPixel expected[9]) {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (*bitmap.getAddr(x, y) != expected[y * 3 + x]) {
                return false;
            }
        }
    }
    return true;
}

static void test_rect_colors(GTestStats* stats) {
    const GPixel pred = GPixel_PackARGB(0xFF, 0xFF, 0, 0);
    const GColor cred = GColor::MakeARGB(1, 1, 0, 0);

    GBitmap bitmap;
    setup_bitmap(&bitmap, 3, 3);
    auto canvas = GCreateCanvas(bitmap);

    GPixel nine[9] = { 0, 0, 0, 0, pred, 0, 0, 0, 0 };
    canvas->fillRect(GRect::MakeLTRB(1, 1, 2, 2), cred);
    stats->expectTrue(check9(bitmap, nine), "rect 0");

    nine[4] = 0;
    clear(bitmap);
    // don't expect these to draw anything
    const GRect rects[] = {
        GRect::MakeLTRB(-10, 0, 0.25f, 10),
        GRect::MakeLTRB(0, -10, 10, 0.25f),
        GRect::MakeLTRB(2.51f, 0, 10, 10),
        GRect::MakeLTRB(0, 2.51, 10, 10),

        GRect::MakeLTRB(1, 1, 1, 1),
        GRect::MakeLTRB(1.51f, 0, 2.49f, 3),
    };
    for (int i = 0; i < GARRAY_COUNT(rects); ++i) {
        canvas->fillRect(rects[i], cred);
        stats->expectTrue(check9(bitmap, nine), "rect 1");
    }

    // vertical stripe down center
    nine[1] = nine[4] = nine[7] = pred;
    canvas->fillRect(GRect::MakeLTRB(0.6f, -3, 2.3f, 2.6f), cred);
    stats->expectTrue(check9(bitmap, nine), "rect 2");

    clear(bitmap);
    memset(nine, 0, sizeof(nine));
    // don't expect anything to draw
    const GColor colors[] = {
        GColor::MakeARGB(0, 1, 0, 0),
        GColor::MakeARGB(-1, 1, 0, 0),
        GColor::MakeARGB(0.00001f, 1, 0, 0),
    };
    for (int i = 0; i < GARRAY_COUNT(colors); ++i) {
        canvas->fillRect(GRect::MakeWH(3, 3), colors[i]);
        stats->expectTrue(check9(bitmap, nine), "rect 3");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool is_filled_with(const GBitmap& bitmap, GPixel expected) {
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            if (*bitmap.getAddr(x, y) != expected) {
                return false;
            }
        }
    }
    return true;
}

class GSurface {
public:
    GSurface(int width, int height) {
        fBitmap.alloc(width, height);
        fCanvas = GCreateCanvas(fBitmap);
    }
    ~GSurface() {
        free(fBitmap.pixels());
    }

    GCanvas* canvas() const { return fCanvas.get(); }
    const GBitmap& bitmap() const { return fBitmap; }

private:
    std::unique_ptr<GCanvas>    fCanvas;
    GBitmap                     fBitmap;
};

static void test_bad_input_poly(GTestStats* stats) {
    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();

    canvas->clear({1, 1, 1, 1});
    const GPixel white = GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_invalid_clear");

    const GPaint paint({1, 0, 0, 0});  // black
    const GPixel black = GPixel_PackARGB(0xFF, 0, 0, 0);

    // inside the top/left corner
    const GPoint pts[] = {
        GPoint::Make(0, 0), GPoint::Make(5, 10), GPoint::Make(10, 5)
    };

    // Now draw some polygons that shouldn't actually draw any pixels
    const struct {
        int fCount;
        const char* fMsg;
    } recs[] = {
        { -1, "bad_ploly_count_-1" },
        {  0, "bad_ploly_count_0" },
        {  1, "bad_ploly_count_1" },
        {  2, "bad_ploly_count_2" },
    };
    for (int i = 0; i < GARRAY_COUNT(recs); ++i) {
        canvas->drawConvexPolygon(pts, recs[i].fCount, paint);
        stats->expectTrue(is_filled_with(surface.bitmap(), white), recs[i].fMsg);
    }
}

static void test_offscreen_poly(GTestStats* stats) {
    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();

    canvas->clear({1, 1, 1, 1});
    const GPixel white = GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_offscreen_clear");

    const GPaint paint({1, 0, 0, 0});  // black
    const GPixel black = GPixel_PackARGB(0xFF, 0, 0, 0);

    // draw a valid polygon, but it is "offscreen", so nothing should get drawn

    // triange up and to the left of the top/left corner
    const GPoint pts[] = {
        GPoint::Make(-10, -10), GPoint::Make(5, -10), GPoint::Make(-10, 5)
    };
    canvas->drawConvexPolygon(pts, 3, paint);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_offscreen");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static bool ie_eq(float a, float b) {
    return fabs(a - b) <= 0.00001f;
}

static bool is_eq(const GMatrix& m, float a, float b, float c, float d, float e, float f) {
    //    printf("%g %g %g    %g %g %g\n", m[0], m[1], m[2], m[3], m[4], m[5]);
    return ie_eq(m[0], a) && ie_eq(m[1], b) && ie_eq(m[2], c) &&
    ie_eq(m[3], d) && ie_eq(m[4], e) && ie_eq(m[5], f);
}

static void test_matrix(GTestStats* stats) {
    GMatrix m;
    stats->expectTrue(is_eq(m, 1, 0, 0, 0, 1, 0), "matrix_identity");
    m.setTranslate(2.5, -4);
    stats->expectTrue(is_eq(m, 1, 0, 2.5, 0, 1, -4), "matrix_setTranslate");
    m.setScale(2.5, -4);
    stats->expectTrue(is_eq(m, 2.5, 0, 0, 0, -4, 0), "matrix_setScale");
    m.setRotate(M_PI);
    stats->expectTrue(is_eq(m, -1, 0, 0, 0, -1, 0), "matrix_setRotate");
    
    GMatrix m2, m3;
    m.setScale(2, 3);
    m2.setScale(-1, -2);
    m3.setConcat(m, m2);
    stats->expectTrue(is_eq(m3, -2, 0, 0, 0, -6, 0), "matrix_setConcat0");
    m2.setTranslate(5, 6);
    m3.setConcat(m2, m);
    stats->expectTrue(is_eq(m3, 2, 0, 5, 0, 3, 6), "matrix_setConcat1");
}

static void test_matrix_inv(GTestStats* stats) {
    GMatrix m, m2, m3;

    stats->expectTrue(m.invert(&m2) && m == m2, "matrix_invert0");
    m.setScale(0.5f, 4);
    stats->expectTrue(m.invert(&m2) && is_eq(m2, 2, 0, 0, 0, 0.25f, 0), "matrix_invert1");
    // check for alias detection of src and dst
    stats->expectTrue(m.invert(&m)  && is_eq(m,  2, 0, 0, 0, 0.25f, 0), "matrix_invert2");

    m.setRotate(M_PI/3);
    m.postTranslate(3, 4);
    stats->expectTrue(m.invert(&m2), "matrix_invert3");
    m3.setConcat(m, m2);
    stats->expectTrue(is_eq(m3,  1, 0, 0, 0, 1, 0), "matrix_invert4");
}

static void test_matrix_map(GTestStats* stats) {
    const GPoint src[] = { {0, 0}, {1, 1}, {-3, 4}, {0.5f, -0.125} };
    GPoint dst[4], dst2[4];

    GMatrix m;
    m.setScale(2, 2);
    m.postTranslate(3, 4);
    m.mapPoints(dst, src, 4);
    const GPoint expected[] = { {3, 4}, {5, 6}, {-3, 12}, {4, 3.75f} };
    bool equal = true;
    for (int i = 0; i < 4; ++i) {
        equal &= dst[i].x() == expected[i].x();
        equal &= dst[i].y() == expected[i].y();
    }
    stats->expectTrue(equal, "matrix_mappts0");
    
    m.setRotate(M_PI/3);
    m.postTranslate(3, 4);
    m.mapPoints(dst, src, 4);
    memcpy(dst2, src, sizeof(src));
    m.mapPoints(dst2, dst2, 4);
    stats->expectTrue(memcmp(dst, dst2, sizeof(src)) == 0, "matrix_mappts1");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "tests_pa4.cpp"
#include "tests_pa5.cpp"

const GTestRec gTestRecs[] = {
    { test_clear,       "clear"         },
    { test_rect_colors, "rect_colors"   },

    { test_bad_input_poly, "poly_bad_input" },
    { test_offscreen_poly, "poly_offscreen" },
    
    { test_matrix,      "matrix_setters"    },
    { test_matrix_inv,  "matrix_inv"        },
    { test_matrix_map,  "matrix_map"        },

    { test_filters,     "filters",          },
    { test_savelayesr,  "savelayer",        },

    { test_path,        "path",             },
    { test_path_rect,   "path_rect",        },
    { test_path_rect,   "test_path_poly",   },
    { test_path_transform, "path_transform" },

    { nullptr, nullptr },
};

bool gTestSuite_Verbose;
bool gTestSuite_CrashOnFailure;
