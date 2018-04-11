#include <stack>

#include "GBitmap.h"
#include "GBlitter.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GFilter.h"
#include "GLayer.h"
#include "GMath.h"
#include "GMatrix.h"
#include "GPath.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"
#include "GShader.h"

#include "Blend.h"
#include "Clipper.h"
#include "ColorUtils.h"


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) {
        GMatrix identity;
        identity.setIdentity();
        mLayers.push(GLayer(device, identity));
    }

    void concat(const GMatrix& matrix) override {
        mLayers.top().getCTM()->preConcat(matrix);
    }

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint srcPoints[], int count, const GPaint& paint) override {
        GLayer layer = mLayers.top();

        // If the paint has a shader and we can't set its context, we can't
        // draw anything.
        if (paint.getShader() != nullptr
                && !paint.getShader()->setContext(*layer.getCTM())) {
            return;
        }

        GPoint points[count];
        layer.getCTM()->mapPoints(points, srcPoints, count);

        GRect bounds = GRect::MakeWH(
            layer.fBitmap.width(),
            layer.fBitmap.height());
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

        GBlitter blitter = GBlitter(layer.fBitmap);

        // Loop through all the possible y-coordinates that could be drawn
        while (curY < lastY) {
            blitter.blitRow(curY, GRoundToInt(leftX), GRoundToInt(rightX), paint);
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
        GBitmap bm = mLayers.top().fBitmap;
        GRect bounds = GRect::MakeWH(bm.width(), bm.height());
        drawRect(bounds, paint);
    }

    void drawPath(const GPath& path, const GPaint& paint) override {

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

    void onSaveLayer(const GRect* boundsPtr, const GPaint& paint) override {
        GLayer layer = mLayers.top();
        GBitmap oldBitmap = layer.fBitmap;
        GRect bounds;

        if (boundsPtr == nullptr) {
            bounds = GRect::MakeWH(oldBitmap.width(), oldBitmap.height());
        } else {
            bounds = *boundsPtr;
        }

        GPoint pts[4] = {
            GPoint::Make(bounds.left(), bounds.top()),
            GPoint::Make(bounds.right(), bounds.top()),
            GPoint::Make(bounds.right(), bounds.bottom()),
            GPoint::Make(bounds.left(), bounds.bottom())
        };
        layer.getCTM()->mapPoints(pts, pts, 4);

        float left = std::min(pts[0].fX, std::min(pts[1].fX, std::min(pts[2].fX, pts[3].fX)));
        float right = std::max(pts[0].fX, std::max(pts[1].fX, std::max(pts[2].fX, pts[3].fX)));
        float top = std::min(pts[0].fY, std::min(pts[1].fY, std::min(pts[2].fY, pts[3].fY)));
        float bottom = std::max(pts[0].fY, std::max(pts[1].fY, std::max(pts[2].fY, pts[3].fY)));

        GIRect transformedBounds = GRect::MakeLTRB(left, top, right, bottom).round();
        transformedBounds.setLTRB(
            std::max(transformedBounds.left(), 0),
            std::max(transformedBounds.top(), 0),
            std::min(transformedBounds.right(), oldBitmap.width() - 1),
            std::min(transformedBounds.bottom(), oldBitmap.height() - 1));

        // Bitmap initialization taken from:
        // apps/image.cpp::setup_bitmap
        GBitmap bitmap;
        size_t rb = transformedBounds.width() * sizeof(GPixel);
        bitmap.reset(
            transformedBounds.width(),
            transformedBounds.height(),
            rb,
            (GPixel*)calloc(transformedBounds.height(), rb),
            GBitmap::kNo_IsOpaque);

        mLayers.push(GLayer(bitmap, *layer.getCTM(), transformedBounds, paint));
    }

    void restore() override {
        GLayer layer = mLayers.top();
        mLayers.pop();
        GBitmap base = mLayers.top().fBitmap;

        if (layer.fIsLayer) {
            layer.draw(base);
        }
    }

    void save() override {
        GLayer current = mLayers.top();
        mLayers.push(GLayer(current.fBitmap, *current.getCTM()));
    }

private:
    std::stack<GLayer> mLayers;
};


std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

