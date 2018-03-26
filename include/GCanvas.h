/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GTypes.h"

class GBitmap;
class GColor;
class GRect;

class GCanvas {
public:
    virtual ~GCanvas() {}

    /**
     *  Fill the entire canvas with the specified color, using SRC porter-duff mode.
     */
    virtual void clear(const GColor&) = 0;

    /**
     *  Fill the rectangle with the color, using SRC_OVER porter-duff mode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     *
     *  Any area in the rectangle that is outside of the bounds of the canvas is ignored.
     */
    virtual void fillRect(const GRect&, const GColor&) = 0;
};

/**
 *  If the bitmap is valid for drawing into, this returns a subclass that can perform the
 *  drawing. If bitmap is invalid, this returns NULL.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap);

#endif
