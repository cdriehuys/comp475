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
    bitmap->fWidth = w;
    bitmap->fHeight = h;
    bitmap->fRowBytes = w * sizeof(GPixel);
    bitmap->fPixels = (GPixel*)calloc(bitmap->rowBytes(), bitmap->height());
}

static void clear(const GBitmap& bitmap) {
    memset(bitmap.pixels(), 0, bitmap.rowBytes() * bitmap.height());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void test_bad_input(GTestStats* stats) {
    GBitmap bitmap;

    bitmap.fWidth = -1;
    bitmap.fHeight = 1;
    stats->expectNULL(GCanvas::Create(bitmap), "bad input 0");
    
    bitmap.fWidth = 5;
    bitmap.fHeight = -5;
    stats->expectNULL(GCanvas::Create(bitmap), "bad input 1");

    bitmap.fWidth = bitmap.fHeight = 10;
    bitmap.fRowBytes = (bitmap.fWidth - 1) * sizeof(GPixel);
    stats->expectNULL(GCanvas::Create(bitmap), "bad input 2");
    
    bitmap.fWidth = bitmap.fHeight = 10;
    bitmap.fRowBytes = (bitmap.fWidth + 7) * sizeof(GPixel);
    bitmap.fPixels = (GPixel*)malloc(bitmap.rowBytes() * bitmap.height());
    GCanvas* canvas = GCanvas::Create(bitmap);
    stats->expectPtr(canvas, "bad input 3");
    delete canvas;
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
    GBitmap bitmap;

    bitmap.fWidth = bitmap.fHeight = 10;
    bitmap.fRowBytes = (bitmap.fWidth + 11) * sizeof(GPixel);
    size_t size = bitmap.rowBytes() * bitmap.height();
    bitmap.fPixels = (GPixel*)malloc(size);
    
    const int wacky_component = 123;

    memset(bitmap.fPixels, wacky_component, size);
    GCanvas* canvas = GCanvas::Create(bitmap);

    // ensure that creating the canvas didn't change any pixels
    stats->expectTrue(mem_eq(bitmap.fPixels, wacky_component, size), "clear 0");

    const GPixel wacky_pixel = GPixel_PackARGB(wacky_component, wacky_component,
                                               wacky_component, wacky_component);

    canvas->clear(GColor::MakeARGB(0, 1, 1, 1));
    stats->expectTrue(bitmap_pix_eq(bitmap, 0, wacky_pixel), "clear 1");

    canvas->clear(GColor::MakeARGB(1, 1, 1, 1));
    stats->expectTrue(bitmap_pix_eq(bitmap, 0xFFFFFFFF, wacky_pixel), "clear 2");
    
    delete canvas;
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
    GCanvas* canvas = GCanvas::Create(bitmap);

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

static void test_hori_bitmap(GTestStats* stats) {
    GPixel srcStorage[4] = {
        GPixel_PackARGB(0xFF, 0xFF, 0, 0),
        GPixel_PackARGB(0xFF, 0, 0xFF, 0),
        GPixel_PackARGB(0xFF, 0, 0, 0xFF),
        GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF),
    };
    GBitmap src;
    src.fWidth = 4;
    src.fHeight = 1;
    src.fRowBytes = src.fWidth * sizeof(GPixel);
    src.fPixels = srcStorage;
    
    GPixel dstStorage[16];
    GBitmap dst;
    dst.fWidth = dst.fHeight = 4;
    dst.fRowBytes = dst.fWidth * sizeof(GPixel);
    dst.fPixels = dstStorage;
    
    GCanvas* canvas = GCanvas::Create(dst);
    canvas->fillBitmapRect(src, GRect::MakeWH(4, 4));
    
    for (int y = 0; y < dst.height(); ++y) {
        stats->expectEQ(memcmp(srcStorage, dst.getAddr(0, y), src.rowBytes()), 0, "hori_bitmap");
    }
}

static void test_vert_bitmap(GTestStats* stats) {
    GPixel srcStorage[4] = {
        GPixel_PackARGB(0xFF, 0xFF, 0, 0),
        GPixel_PackARGB(0xFF, 0, 0xFF, 0),
        GPixel_PackARGB(0xFF, 0, 0, 0xFF),
        GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF),
    };
    GBitmap src;
    src.fWidth = 1;
    src.fHeight = 4;
    src.fRowBytes = src.fWidth * sizeof(GPixel);
    src.fPixels = srcStorage;
    
    GPixel dstStorage[16];
    GBitmap dst;
    dst.fWidth = dst.fHeight = 4;
    dst.fRowBytes = dst.fWidth * sizeof(GPixel);
    dst.fPixels = dstStorage;
    
    GCanvas* canvas = GCanvas::Create(dst);
    canvas->fillBitmapRect(src, GRect::MakeWH(4, 4));
    
    for (int y = 0; y < dst.height(); ++y) {
        for (int x = 0; x < dst.width(); ++x) {
            stats->expectEQ(*dst.getAddr(x, y), *src.getAddr(0, y), "vert_bitmap");
        }
    }
}

static void test_shrink_bitmap(GTestStats* stats) {
    GPixel srcStorage[9];
    for (int i = 0; i < 9; ++i) {
        srcStorage[i] = GPixel_PackARGB(0xFF, 0xFF, 0, 0);  // red
    }
    srcStorage[4] = GPixel_PackARGB(0xFF, 0, 0xFF, 0);  // green center
    
    GBitmap src;
    src.fWidth = src.fHeight = 3;
    src.fRowBytes = src.fWidth * sizeof(GPixel);
    src.fPixels = srcStorage;
    
    GPixel dstStorage[9];
    GBitmap dst;
    dst.fWidth = dst.fHeight = 3;
    dst.fRowBytes = dst.fWidth * sizeof(GPixel);
    dst.fPixels = dstStorage;
    
    GCanvas* canvas = GCanvas::Create(dst);
    
    memset(dst.fPixels, 0, sizeof(dstStorage));
    canvas->fillBitmapRect(src, GRect::MakeXYWH(1, 1, 1, 1));
    // expect dst to still be zeros, exept its center, which should be green since we shrank down
    // src from 3x3 to 1x1 and placed it at dst's center.
    
    for (int y = 0; y < dst.height(); ++y) {
        for (int x = 0; x < dst.width(); ++x) {
            GPixel expected = 0;
            if (1 == x && 1 == y) {
                expected = GPixel_PackARGB(0xFF, 0, 0xFF, 0);
            }
            stats->expectEQ(*dst.getAddr(x, y), expected, "shrink_bitmap");
        }
    }
}

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
        fBitmap.fWidth = width;
        fBitmap.fHeight = height;
        fBitmap.fRowBytes = width * sizeof(GPixel);
        const size_t size = fBitmap.rowBytes() * fBitmap.height();
        fBitmap.fPixels = (GPixel*)malloc(size);

        fCanvas = GCanvas::Create(fBitmap);
    }
    ~GSurface() {
        delete fCanvas;
        free(fBitmap.fPixels);
    }

    GCanvas* canvas() const { return fCanvas; }
    const GBitmap& bitmap() const { return fBitmap; }

private:
    GCanvas*    fCanvas;
    GBitmap     fBitmap;
};

static void test_bad_input_poly(GTestStats* stats) {
    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();

    canvas->clear(GColor::MakeARGB(1, 1, 1, 1));
    const GPixel white = GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_invalid_clear");

    const GColor color = GColor::MakeARGB(1, 0, 0, 0);  // black
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
        canvas->fillConvexPolygon(pts, recs[i].fCount, color);
        stats->expectTrue(is_filled_with(surface.bitmap(), white), recs[i].fMsg);
    }
}

static void test_offscreen_poly(GTestStats* stats) {
    GSurface surface(10, 10);
    GCanvas* canvas = surface.canvas();
    
    canvas->clear(GColor::MakeARGB(1, 1, 1, 1));
    const GPixel white = GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_offscreen_clear");
    
    const GColor color = GColor::MakeARGB(1, 0, 0, 0);  // black
    const GPixel black = GPixel_PackARGB(0xFF, 0, 0, 0);
    
    // draw a valid polygon, but it is "offscreen", so nothing should get drawn

    // triange up and to the left of the top/left corner
    const GPoint pts[] = {
        GPoint::Make(-10, -10), GPoint::Make(5, -10), GPoint::Make(-10, 5)
    };
    canvas->fillConvexPolygon(pts, 3, color);
    stats->expectTrue(is_filled_with(surface.bitmap(), white), "poly_offscreen");
}

///////////////////////////////////////////////////////////////////////////////////////////////////

const GTestRec gTestRecs[] = {
    { test_bad_input,   "bad_input"     },

    { test_clear,       "clear"         },
    { test_rect_colors, "rect_colors"   },

    { test_hori_bitmap, "hori_bitmap" },
    { test_vert_bitmap, "vert_bitmap" },
    { test_shrink_bitmap, "shrink_bitmap" },

    { test_bad_input_poly, "poly_bad_input" },
    { test_offscreen_poly, "poly_offscreen" },

    { NULL, NULL },
};

bool gTestSuite_Verbose;
bool gTestSuite_CrashOnFailure;
