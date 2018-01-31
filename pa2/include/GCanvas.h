/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GPaint.h"

class GBitmap;
class GPoint;
class GRect;

class GCanvas {
public:
    virtual ~GCanvas() {}

    /**
     *  Fill the entire canvas with the specified color, using the specified blendmode.
     */
    virtual void drawPaint(const GPaint&) = 0;

    /**
     *  Fill the rectangle with the color, using the specified blendmode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     */
    virtual void drawRect(const GRect&, const GPaint&) = 0;

    /**
     *  Fill the convex polygon with the color and blendmode,
     *  following the same "containment" rule as rectangles.
     */
    virtual void drawConvexPolygon(const GPoint[], int count, const GPaint&) = 0;

    // Helpers

    void clear(const GColor& color) {
        GPaint paint(color);
        paint.setBlendMode(GBlendMode::kSrc);
        this->drawPaint(paint);
    }

    void fillRect(const GRect& rect, const GColor& color) {
        this->drawRect(rect, GPaint(color));
    }
};

/**
 *  If the bitmap is valid for drawing into, this returns a subclass that can perform the
 *  drawing. If bitmap is invalid, this returns NULL.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap);

#endif
