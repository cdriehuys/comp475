/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GShader_DEFINED
#define GShader_DEFINED

#include <memory>
#include "GPixel.h"

class GBitmap;
class GMatrix;

/**
 *  GShaders create colors to fill whatever geometry is being drawn to a GCanvas.
 */
class GShader {
public:
    virtual ~GShader() {}

    // Return true iff all of the GPixels that may be returned by this shader will be opaque.
    virtual bool isOpaque() = 0;

    // The draw calls in GCanvas must call this with the CTM before any calls to shadeSpan().
    virtual bool setContext(const GMatrix& ctm) = 0;

    /**
     *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
     *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
     *  can hold at least [count] entries.
     */
    virtual void shadeRow(int x, int y, int count, GPixel row[]) = 0;
};

/**
 *  Return a subclass of GShader that draws the specified bitmap and the inverse of a local matrix.
 *  Returns null if the either parameter is invalid.
 */
std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap&, const GMatrix& localInv);

#endif
