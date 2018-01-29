#include <iostream>

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


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {}

    void clear(const GColor& color) override {
        GPixel pixel = colorToPixel(color);

        for (int y = 0; y < fDevice.height(); ++y) {
            for (int x = 0; x < fDevice.width(); ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = pixel;
            }
        }
    }

    void fillRect(const GRect& rect, const GColor& color) override {
        std::cout << "Initial Rectangle: (" << rect.fLeft << ", " << rect.fTop << ", " << rect.fRight << ", " << rect.fBottom << ")\n";

        GIRect rounded = rect.round();

        rounded.fLeft = std::max(rounded.fLeft, 0);
        rounded.fTop = std::max(rounded.fTop, 0);
        rounded.fRight = std::min(rounded.fRight, fDevice.width());
        rounded.fBottom = std::min(rounded.fBottom, fDevice.height());

        std::cout << "Rounded Rectangle: (" << rounded.fLeft << ", " << rounded.fTop << ", " << rounded.fRight << ", " << rounded.fBottom << ")\n";

        GPixel pixel = colorToPixel(color);

        std::cout << "Pixel Components: " << GPixel_GetA(pixel) << ", " << GPixel_GetR(pixel) << ", " << GPixel_GetG(pixel) << ", " << GPixel_GetB(pixel) << "\n";

        for (int y = rounded.fTop; y < rounded.fBottom; ++y) {
            for (int x = rounded.fLeft; x < rounded.fRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);

                GPixel dest = *addr;

                int sAlpha = GPixel_GetA(pixel);
                int sRed = GPixel_GetR(pixel);
                int sGreen = GPixel_GetG(pixel);
                int sBlue = GPixel_GetB(pixel);

                int dAlpha = GPixel_GetA(dest);
                int dRed = GPixel_GetR(dest);
                int dGreen = GPixel_GetG(dest);
                int dBlue = GPixel_GetB(dest);

                int alpha = sAlpha + ((255 - sAlpha) * dAlpha + 127) / 255;
                int red = sRed + ((255 - sAlpha) * dRed + 127) / 255;
                int blue = sBlue + ((255 - sAlpha) * dBlue + 127) / 255;
                int green = sGreen + ((255 - sAlpha) * dGreen + 127) / 255;

                *addr = GPixel_PackARGB(alpha, red, green, blue);
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

