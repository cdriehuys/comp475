#include <stack>

#include "GBitmap.h"
#include "GBlitter.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GFilter.h"
#include "GLayer.h"
#include "GMatrix.h"
#include "GPath.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"
#include "GScanConverter.h"
#include "GShader.h"

#include "Blend.h"
#include "Clipper.h"
#include "ColorUtils.h"
#include "MathUtils.h"


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) {
        GMatrix identity;
        identity.setIdentity();

        GIRect bounds = GIRect::MakeWH(device.width(), device.height());

        mLayers.push(GLayer(&device, identity, bounds));
    }

    void concat(const GMatrix& matrix) override {
        mLayers.top().getCTM().preConcat(matrix);
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
                && !paint.getShader()->setContext(layer.getCTM())) {
            return;
        }

        GPoint points[count];
        layer.getCTM().mapPoints(points, srcPoints, count);

        GRect bounds = GRect::MakeWH(
            layer.getBitmap().width(),
            layer.getBitmap().height());
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

        GBlitter blitter = GBlitter(layer.getBitmap(), paint);

        GScanConverter::scan(storage, edgeCount, blitter);
    }

    /**
     * Fill the entire canvas with a particular paint.
     */
    void drawPaint(const GPaint& paint) override {
        GBitmap bm = mLayers.top().getBitmap();
        GRect bounds = GRect::MakeWH(bm.width(), bm.height());
        drawRect(bounds, paint);
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        GLayer layer = mLayers.top();

        // If the paint has a shader and we can't set its context, we can't
        // draw anything.
        if (paint.getShader() != nullptr
                && !paint.getShader()->setContext(layer.getCTM())) {
            return;
        }

        GPoint points[2 * path.countPoints()];
        int edgeCount = 0;
        GPath::Edger edger = GPath::Edger(path);

        GPath::Verb verb;
        do {
            verb = edger.next(&points[edgeCount * 2]);

            if (verb == GPath::Verb::kLine) {
                edgeCount++;
            }
        } while (verb != GPath::Verb::kDone);

        layer.getCTM().mapPoints(points, points, 2 * edgeCount);

        GRect bounds = GRect::MakeWH(
            layer.getBitmap().width(),
            layer.getBitmap().height());
        Edge storage[3 * edgeCount];
        Edge* edge = storage;

        for (int i = 0; i < edgeCount; ++i) {
            GPoint p0 = points[2 * i];
            GPoint p1 = points[2 * i + 1];

            edge = clipLine(p0, p1, bounds, edge);
        }

        // Recalculate edge count after clipping
        edgeCount = edge - storage;
        if (edgeCount == 0) {
            return;
        }

        GBlitter blitter = GBlitter(layer.getBitmap(), paint);

        GScanConverter::scan(storage, edgeCount, blitter);
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
        GIRect bounds;
        GIRect prevBounds = mLayers.top().getBounds();
        if (boundsPtr == nullptr) {
            bounds = GIRect::MakeWH(prevBounds.width(), prevBounds.height());
        } else {
            GPoint points[4] = {
                GPoint::Make(boundsPtr->left(), boundsPtr->top()),
                GPoint::Make(boundsPtr->right(), boundsPtr->top()),
                GPoint::Make(boundsPtr->right(), boundsPtr->bottom()),
                GPoint::Make(boundsPtr->left(), boundsPtr->bottom())
            };
            mLayers.top().getCTM().mapPoints(points, points, 4);

            float xVals[4], yVals[4];
            for (int i = 0; i < 4; ++i) {
                xVals[i] = points[i].fX;
                yVals[i] = points[i].fY;
            }

            float left = manyMin(xVals, 4);
            float top = manyMin(yVals, 4);
            float right = manyMax(xVals, 4);
            float bottom = manyMax(yVals, 4);

            GRect floatBounds = GRect::MakeLTRB(left, top, right, bottom);
            bounds = floatBounds.round();

            if (!bounds.intersect(prevBounds)) {
                bounds = GIRect::MakeWH(0, 0);
            }
        }

        // Bitmap initialization taken from:
        // apps/image.cpp::setup_bitmap
        GBitmap bitmap;
        size_t rowBytes = bounds.width() * sizeof(GPixel);
        bitmap.reset(
            bounds.width(),
            bounds.height(),
            rowBytes,
            (GPixel*)calloc(bounds.height(), rowBytes),
            GBitmap::kNo_IsOpaque);

        GMatrix oldCTM = mLayers.top().getCTM();
        GMatrix newCTM = GMatrix(
            oldCTM[0], oldCTM[1], oldCTM[2],
            oldCTM[3], oldCTM[4], oldCTM[5]);
        newCTM.postTranslate(
            prevBounds.left() - bounds.left(),
            prevBounds.top() - bounds.top());

        GLayer newLayer = GLayer(bitmap, newCTM, bounds, paint);
        mLayers.push(newLayer);
    }

    void restore() override {
        GLayer top = mLayers.top();
        mLayers.pop();
        GLayer base = mLayers.top();

        if (top.isLayer()) {
            top.draw(&base.getBitmap());
        }
    }

    void save() override {
        GLayer current = mLayers.top();
        mLayers.push(GLayer(&current.getBitmap(), current.getCTM(), current.getBounds()));
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

