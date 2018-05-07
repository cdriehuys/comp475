/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GMatrix.h"
#include "GPaint.h"

class GBitmap;
class GPath;
class GPoint;
class GRect;

class GCanvas {
public:
    virtual ~GCanvas() {}

    //////////// Beginning of Final methods

    /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center point (cx, cy) and a radius.
     *  It supports an array colors (count >= 2) where
     *      color[0]       is the color at the center
     *      color[count-1] is the color at the outer edge of the circle
     *      the other colors (if any) are evenly distributed along the radius
     *
     *  e.g. If there are 4 colors and a radius of 90 ...
     *
     *      color[0] is at the center
     *      color[1] is at a distance of 30 from the center
     *      color[2] is at a distance of 60 from the center
     *      color[3] is at a distance of 90 from the center
     *
     *  Positions outside of the radius are clamped to color[count - 1].
     *  Positions inside the radius are linearly interpolated between the two nearest colors.
     *
     *  Interpolation occurs between GColors, and then it is premultiplied to a GPixel.
     */
    virtual std::unique_ptr<GShader> final_createRadialGradient(GPoint center, float radius,
                                                                const GColor colors[], int count);

    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width. Note that "width" is the distance from one side of the stroke to the other.
     *
     *  If roundCap is true, the path should also include a circular cap at each end of the line,
     *  where the circle has radius of width/2 and its center is positioned at p0 and p1.
     */
    virtual void final_addStrokedLine(GPath* path, GPoint p0, GPoint p1, float width, bool roundCap);

    /**
     *  Return a subclass of GShader that implements a triangle-gradient. This linearly
     *  interpolates the colors at each vertex across the interior of the triangle.
     *
     *  Each vertex (pts[i]) has an associated color (colors[i]).
     *
     *  Interpolation occurs between GColors, and then it is premultiplied to a GPixel.
     */
    virtual std::unique_ptr<GShader> final_createTriangleGradient(const GPoint pts[3],
                                                                  const GColor colors[3]);

    //////////// End of Final methods

    /**
     *  Save off a copy of the canvas state (CTM), to be later used if the balancing call to
     *  restore() is made. Calls to save/restore can be nested:
     *  save();
     *      save();
     *          concat(...);    // this modifies the CTM
     *          .. draw         // these are drawn with the modified CTM
     *      restore();          // now the CTM is as it was when the 2nd save() call was made
     *      ..
     *  restore();              // now the CTM is as it was when the 1st save() call was made
     */
    virtual void save() = 0;

    /**
     *  Also saves the CTM state (just like save()) but this additionally creates a temporary
     *  surface for subsequent drawing, initially set to transparent. When the balancing call to
     *  restore() is made, that surface (layer) is blended against the previous surface.
     *
     *  If bounds is not null, it is conservative bounds of the area that will be drawn, in local
     *  coordinates. Implementations can use this to minimize the size of the offscreen layer.
     *  The bounds should be run through the CTM, and then the rounded bounds of that (in device
     *  coordintaes) is the extent of the needed layer. Note: this can be clipped to the bounds of
     *  the previous surface (or the root bitmap if there is no previous call to saveLayer).
     *
     *  If the paint's filter is not null, then the surface is first "filtered" before it is drawn
     *  onto the previous layer during restore, using the paint's blendmode.
     */
    void saveLayer(const GRect* bounds, const GPaint& paint) { this->onSaveLayer(bounds, paint); }
    void saveLayer(const GPaint& paint) { this->onSaveLayer(nullptr, paint); }
    void saveLayer(const GRect& bounds) { this->onSaveLayer(&bounds, GPaint()); }
    void saveLayer() { this->onSaveLayer(nullptr, GPaint()); }

    /**
     *  Copy the canvas state (CTM) that was record in the correspnding call to save() back into
     *  the canvas. It is an error to call restore() if there has been no previous call to save().
     */
    virtual void restore() = 0;

    /**
     *  Modifies the CTM by preconcatenating the specified matrix with the CTM. The canvas
     *  is constructed with an identity CTM.
     *
     *  CTM' = CTM * matrix
     */
    virtual void concat(const GMatrix& matrix) = 0;

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
     *  Fill the convex polygon with the paint, following the same "containment" rule as rects.
     */
    virtual void drawConvexPolygon(const GPoint[], int count, const GPaint&) = 0;

    /**
     *  Fill the path with the paint, interpreting the path using winding-fill (non-zero winding).
     */
    virtual void drawPath(const GPath&, const GPaint&) = 0;

    // Helpers

    void translate(float x, float y) {
        this->concat(GMatrix::MakeTranslate(x, y));
    }

    void scale(float x, float y) {
        this->concat(GMatrix::MakeScale(x, y));
    }

    void rotate(float radians) {
        this->concat(GMatrix::MakeRotate(radians));
    }

    void clear(const GColor& color) {
        GPaint paint(color);
        paint.setBlendMode(GBlendMode::kSrc);
        this->drawPaint(paint);
    }

    void fillRect(const GRect& rect, const GColor& color) {
        this->drawRect(rect, GPaint(color));
    }

protected:
    virtual void onSaveLayer(const GRect* bounds, const GPaint&) = 0;
};

/**
 *  If the bitmap is valid for drawing into, this returns a subclass that can perform the
 *  drawing. If bitmap is invalid, this returns NULL.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap);

#endif
