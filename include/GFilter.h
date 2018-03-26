/*
 *  Copyright 2018 Mike Reed
 */

#ifndef GFilter_DEFINED
#define GFilter_DEFINED

#include <memory>
#include "GBlendMode.h"
#include "GPixel.h"

class GBitmap;
class GMatrix;

/**
 *  GShaders create colors to fill whatever geometry is being drawn to a GCanvas.
 */
class GFilter {
public:
    virtual ~GFilter() {}

    /**
     *  Return true iff this filter preserves the input colors alpha. i.e. each output[i] alpha
     *  is the same as the input[i] alpha.
     */
    virtual bool preservesAlpha() = 0;

    /**
     *  Filter each input[i] pixel, and return the new value in output[i]
     */
    virtual void filter(GPixel output[], const GPixel input[], int count) = 0;
};

/**
 *  Apply the specified blendmode and src color to the filter colors. This is similar to
 *  drawing with the src color onto the input colors.
 *
 *  e.g. in filter(output, input, count)
 *      loop(count)
 *         output[i] = blend(src, input[i])
 */
std::unique_ptr<GFilter> GCreateBlendFilter(GBlendMode mode, const GColor& src);

#endif
