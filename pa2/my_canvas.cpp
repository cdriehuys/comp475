#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GPixel.h"
#include "GRect.h"


/**
 * Convert a float in the range [0.0 ... 1.0] to an 8-bit integer.
 */
int floatToPixelValue(float value) {
    GASSERT(0 <= value && value <= 1);

    return floor(value * 255 + 0.5);
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


/**
 * Multiply bytes the same way numbers in the range [0.0 ... 1.0] are
 * multiplied.
 */
int multiplyBytes(int x, int y) {
    int result = x * y;

    return (result + 127) / 255;
}


/**
 * Blend two pixels together using the SrcOver method.
 */
GPixel blend(const GPixel& source, const GPixel& dest) {
    int sAlpha = GPixel_GetA(source);
    int sRed = GPixel_GetR(source);
    int sGreen = GPixel_GetG(source);
    int sBlue = GPixel_GetB(source);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int alpha = sAlpha + multiplyBytes(255 - sAlpha, dAlpha);
    int red = sRed + multiplyBytes(255 - sAlpha, dRed);
    int green = sGreen + multiplyBytes(255 - sAlpha, dGreen);
    int blue = sBlue + multiplyBytes(255 - sAlpha, dBlue);

    return GPixel_PackARGB(alpha, red, green, blue);
}


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {}

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint[], int count, const GPaint&) override {

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

        for (int y = rounded.fTop; y < rounded.fBottom; ++y) {
            for (int x = rounded.fLeft; x < rounded.fRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel dest = *addr;

                *addr = blend(source, dest);
            }
        }
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

