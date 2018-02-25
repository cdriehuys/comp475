#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GMath.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"

#include "Blend.h"
#include "Clipper.h"


/**
 * Convert a float in the range [0.0 ... 1.0] to an 8-bit integer.
 */
int floatToPixelValue(float value) {
    GASSERT(0 <= value && value <= 1);

    return GRoundToInt(value * 255);
}


/**
 * Convert a GColor to a GPixel that can actually be used.
 *
 * The color components are pre-multiplied by the alpha value before being
 * compacted.
 */
GPixel colorToPixel(const GColor& color) {
    int alpha = floatToPixelValue(color.fA);
    int red   = floatToPixelValue(color.fR * color.fA);
    int green = floatToPixelValue(color.fG * color.fA);
    int blue  = floatToPixelValue(color.fB * color.fA);

    return GPixel_PackARGB(alpha, red, green, blue);
}


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {}

    void concat(const GMatrix& matrix) override {

    }

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint srcPoints[], int count, const GPaint& paint) override {
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        Edge storage[count * 3];
        Edge* edge = storage;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = srcPoints[i];
            GPoint p1 = srcPoints[(i + 1) % count];

            edge = clipLine(p0, p1, bounds, edge);
        }

        int edgeCount = edge - storage;
        if (edgeCount == 0) {
            return;
        }

        GASSERT(edgeCount >= 2);

        std::sort(storage, storage + edgeCount);

        int lastY = storage[edgeCount - 1].bottomY;

        // Set up our initial left and right boundary edges
        Edge left = storage[0];
        Edge right = storage[1];

        // Track index of next edge position
        int next = 2;

        float curY = left.topY;

        float leftX = left.curX;
        float rightX = right.curX;

        // Loop through all the possible y-coordinates that could be drawn
        while (curY < lastY) {
            GRect row = GRect::MakeLTRB(leftX, curY, rightX, curY + 1);
            drawRect(row, paint);

            curY++;

            // After drawing, we check to see if we've completed either the
            // left or right edge. If we have, we replace it with the next
            // edge.

            if (curY > left.bottomY) {
                left = storage[next];
                next++;

                leftX = left.curX;
            } else {
                leftX += left.dxdy;
            }

            if (curY > right.bottomY) {
                right = storage[next];
                next++;

                rightX = right.curX;
            } else {
                rightX += right.dxdy;
            }
        }
    }

    /**
     * Fill the entire canvas with a particular paint.
     */
    void drawPaint(const GPaint& paint) override {
        GColor color = paint.getColor().pinToUnit();
        GPixel pixel = colorToPixel(color);

        for (int y = 0; y < fDevice.height(); ++y) {
            for (int x = 0; x < fDevice.width(); ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = pixel;
            }
        }
    }

    /**
     * Draw a rectangular area by filling it with the provided paint.
     */
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GColor color = paint.getColor().pinToUnit();
        GIRect rounded = rect.round();

        // Limit the rectangle's area to the boundaries of the canvas.
        rounded.fLeft = std::max(rounded.fLeft, 0);
        rounded.fTop = std::max(rounded.fTop, 0);
        rounded.fRight = std::min(rounded.fRight, fDevice.width());
        rounded.fBottom = std::min(rounded.fBottom, fDevice.height());

        GPixel source = colorToPixel(color);

        BlendProc blendProc = Blend_GetProc(paint.getBlendMode(), source);

        for (int y = rounded.fTop; y < rounded.fBottom; ++y) {
            for (int x = rounded.fLeft; x < rounded.fRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel dest = *addr;

                *addr = blendProc(source, dest);
            }
        }
    }

    void restore() override {

    }

    void save() override {

    }

private:
    const GBitmap fDevice;
};


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

