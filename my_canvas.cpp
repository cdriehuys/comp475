#include <stack>

#include "GBitmap.h"
#include "GBlitter.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GFilter.h"
#include "GMath.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"
#include "GShader.h"

#include "Blend.h"
#include "Clipper.h"
#include "ColorUtils.h"


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device)
            : fDevice(device)
            , fBlitter(GBlitter(device)) {
        GMatrix identity;
        identity.setIdentity();
        mCTMStack.push(identity);
    }

    void concat(const GMatrix& matrix) override {
        mCTMStack.top().preConcat(matrix);
    }

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint srcPoints[], int count, const GPaint& paint) override {
        // If the paint has a shader and we can't set its context, we can't
        // draw anything.
        if (paint.getShader() != nullptr
                && !paint.getShader()->setContext(mCTMStack.top())) {
            return;
        }

        GPoint points[count];
        mCTMStack.top().mapPoints(points, srcPoints, count);

        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        Edge storage[count * 3];
        Edge* edge = storage;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];

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
            fBlitter.blitRow(curY, GRoundToInt(leftX), GRoundToInt(rightX), paint);
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
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        drawRect(bounds, paint);
    }

    /**
     * Draw a rectangular area by filling it with the provided paint.
     */
    void drawRect(const GRect& rect, const GPaint& paint) override {
        GPoint points[4] = {
            GPoint::Make(rect.left(), rect.top()),
            GPoint::Make(rect.right(), rect.top()),
            GPoint::Make(rect.right(), rect.bottom()),
            GPoint::Make(rect.left(), rect.bottom())
        };

        drawConvexPolygon(points, 4, paint);
    }

    GMatrix getCTM() {
        return mCTMStack.top();
    }

    void onSaveLayer(const GRect* bounds, const GPaint&) override {

    }

    void restore() override {
        mCTMStack.pop();
    }

    void save() override {
        GMatrix current = mCTMStack.top();
        GMatrix copy(
            current[0], current[1], current[2],
            current[3], current[4], current[5]);

        mCTMStack.push(copy);
    }

private:
    const GBitmap fDevice;
    GBlitter fBlitter;

    std::stack<GMatrix> mCTMStack;
};


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

