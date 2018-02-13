#include <iostream>
#include <vector>

#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"

#include "Blend.h"


int round(float value) {
    return floor(value + 0.5);
}


/**
 * Convert a float in the range [0.0 ... 1.0] to an 8-bit integer.
 */
int floatToPixelValue(float value) {
    GASSERT(0 <= value && value <= 1);

    return round(value * 255);
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


struct Edge {
    int topY;
    int bottomY;
    float curX;
    float slope;
};


Edge* makeEdge(GPoint p0, GPoint p1) {
    if (p0.y() > p1.y()) {
        GPoint temp = p1;
        p1 = p0;
        p0 = temp;
    }

    int topY = round(p0.y());
    int bottomY = round(p1.y());

    if (topY == bottomY) {
        return nullptr;
    }

    float slope = (p1.x() - p0.x()) / (p1.y() - p0.y());

    float deltaY = floor(p0.y()) - p0.y() + .5;
    float curX = p0.x() + deltaY * slope;

    Edge* edge = new Edge();
    edge->topY = topY;
    edge->bottomY = bottomY;
    edge->curX = curX;
    edge->slope = slope;

    return edge;
}


void clip(GPoint p0_, GPoint p1_, GRect bounds, std::vector<Edge>& results) {
    GPoint p0, p1;

    // Sort points based on y-coordinate
    if (p0_.y() > p1_.y()) {
        p0 = p1_;
        p1 = p0_;
    } else {
        p0 = p0_;
        p1 = p1_;
    }

    // We can discard edges that are completely out of the clipping region in
    // the vertical direction.
    if (p1.y() <= bounds.top() || p0.y() >= bounds.bottom()) {
        return;
    }

    if (p0.y() < bounds.top()) {
        float newX = p0.x() + (p1.x() - p0.x()) * (bounds.top() - p0.y()) / (p1.y() - p0.y());

        p0.set(newX, bounds.top());
    }

    if (p1.y() > bounds.bottom()) {
        float newX = p1.x() - (p1.x() - p0.x()) * (bounds.top() - p0.y()) / (p1.y() - p0.y());

        p1.set(newX, bounds.bottom());
    }

    // Sort points by x-coordinate
    if (p0.x() > p1.x()) {
        GPoint temp = p0;
        p0 = p1;
        p1 = temp;
    }

    if (p0.x() < bounds.left()) {
        float newY = p0.y() + (bounds.left() - p0.x()) * (p1.y() - p0.y()) / (p1.x() - p0.x());

        // Make the projected edge
        Edge* projected = makeEdge(
            GPoint::Make(bounds.left(), p0.y()),
            GPoint::Make(bounds.left(), newY));
        if (projected != nullptr) {
            results.push_back(*projected);
        }

        p0.set(bounds.left(), newY);
    }

    if (p1.x() > bounds.right()) {
        float newY = p1.y() - (p1.x() - bounds.right()) * (p1.y() - p0.y()) / (p1.x() - p0.x());

        // Make the projected edge
        Edge* projected = makeEdge(
            GPoint::Make(bounds.right(), p1.y()),
            GPoint::Make(bounds.right(), newY));
        if (projected != nullptr) {
            results.push_back(*projected);
        }

        p1.set(bounds.right(), newY);
    }

    // Now that we've clipped the segment, we make an edge from what's left.
    Edge* edge = makeEdge(p0, p1);
    if (edge != nullptr) {
        results.push_back(*edge);
    }
}


bool isEdgeGreater(Edge e1, Edge e2) {
    if (e1.topY < e2.topY) {
        return true;
    }

    if (e2.topY < e1.topY) {
        return false;
    }

    if (e1.curX < e2.curX) {
        return true;
    }

    if (e2.curX < e1.curX) {
        return false;
    }

    return e1.slope <= e2.slope;
}


class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {}

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        std::vector<Edge> edges;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];

            clip(p0, p1, bounds, edges);
        }

        std::sort(edges.begin(), edges.end(), isEdgeGreater);

        // std::cout << "After clipping, " << count << " edge(s) became " << edges.size() << "\n";

        // If we clipped all the edges away, we don't have to do anything.
        if (edges.size() == 0) {
            return;
        }

        GASSERT(edges.size() >= 2);

        Edge left = edges[0];
        Edge right = edges[1];

        int next = 2;

        float curY = left.topY;

        float leftX = left.curX;
        float rightX = right.curX;

        while (curY <= edges.back().bottomY) {
            GRect row = GRect::MakeLTRB(leftX, curY, rightX, curY + 1);
            drawRect(row, paint);

            if (curY >= left.bottomY) {
                left = edges[next];
                next++;

                if (next == edges.size()) {
                    break;
                }

                leftX = left.curX;
            } else {
                leftX += left.slope;
            }

            if (curY >= right.bottomY) {
                right = edges[next];
                next++;

                rightX = right.curX;
            } else {
                rightX += right.slope;
            }

            curY++;
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

        BlendProc blendProc = getBlendProc(paint.getBlendMode(), source);

        for (int y = rounded.fTop; y < rounded.fBottom; ++y) {
            for (int x = rounded.fLeft; x < rounded.fRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel dest = *addr;

                *addr = blendProc(source, dest);
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

