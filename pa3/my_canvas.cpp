#include <deque>

#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GRect.h"

#include "Blend.h"


/**
 * Round a float to the nearest integer.
 */
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


/**
 * An edge contains information about a line segment. It contains the minimum
 * amount of information necessary for our scan converter.
 */
struct Edge {
    int topY;
    int bottomY;
    float curX;
    float slope;
};


/**
 * Create an edge between two points.
 *
 * If the created edge is not useful, a nullptr is returned. Otherwise, a
 * pointer to the created edge is returned.
 */
Edge* makeEdge(GPoint p0, GPoint p1) {
    // Ensure that p0.y <= p1.y is true
    if (p0.y() > p1.y()) {
        GPoint temp = p1;
        p1 = p0;
        p0 = temp;
    }

    int topY = round(p0.y());
    int bottomY = round(p1.y());

    // If the edge doesn't cross any pixel centers vertically, it can be
    // ignored.
    if (topY == bottomY) {
        return nullptr;
    }

    // Our slope is really dx/dy for the purposes of our scan converter.
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


/**
 * Creates an edge that is the projection of two points onto a vertical axis.
 */
void project(float x, float y1, float y2, std::deque<Edge>& results) {
    Edge* projected = makeEdge(GPoint::Make(x, y1), GPoint::Make(x, y2));

    if (projected != nullptr) {
        results.push_back(*projected);
    }
}


/**
 * Clip the line segment between two points into up to 3 edges.
 */
void clip(GPoint p0, GPoint p1, GRect bounds, std::deque<Edge>& results) {
    // Ensure p0.y <= p1.y
    if (p0.y() > p1.y()) {
        GPoint temp = p0;
        p0 = p1;
        p1 = temp;
    }

    // We can discard edges that are completely out of the clipping region in
    // the vertical direction.
    if (p1.y() <= bounds.top() || p0.y() >= bounds.bottom()) {
        return;
    }

    // Handle clipping the top boundary
    if (p0.y() < bounds.top()) {
        float newX = p0.x() + (p1.x() - p0.x()) * (bounds.top() - p0.y()) / (p1.y() - p0.y());

        p0.set(newX, bounds.top());
    }

    // Clip the bottom boundary
    if (p1.y() > bounds.bottom()) {
        float newX = p1.x() - (p1.x() - p0.x()) * (p1.y() - bounds.bottom()) / (p1.y() - p0.y());

        p1.set(newX, bounds.bottom());
    }

    // Ensure p0.x <= p1.x
    if (p0.x() > p1.x()) {
        GPoint temp = p0;
        p0 = p1;
        p1 = temp;
    }

    // If we're entirely outside the left edge, we simply add the projection of
    // the edge onto the left bound.
    if (p1.x() <= bounds.left()) {
        project(bounds.left(), p0.y(), p1.y(), results);

        return;
    }

    // Similarly, if we're entirely outside the right edge, we only add the
    // projection back onto the right bound.
    if (p0.x() >= bounds.right()) {
        project(bounds.right(), p0.y(), p1.y(), results);

        return;
    }

    // Handle clipping and projection onto the left boundary
    if (p0.x() < bounds.left()) {
        float newY = p0.y() + (bounds.left() - p0.x()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        project(bounds.left(), p0.y(), newY, results);

        p0.set(bounds.left(), newY);
    }

    // Handle clipping and projection onto the right boundary
    if (p1.x() > bounds.right()) {
        float newY = p1.y() - (p1.x() - bounds.right()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        project(bounds.right(), newY, p1.y(), results);

        p1.set(bounds.right(), newY);
    }

    // Now that we've clipped the segment, we make an edge from what's left.
    Edge* edge = makeEdge(p0, p1);
    if (edge != nullptr) {
        results.push_back(*edge);
    }
}


/**
 * A predicate function for sorting two edges.
 *
 * Edges are first compared by top y-coordinate, then starting x-coordinate,
 * and finally their "slope".
 *
 * Returns true if the first edge should be sorted before the second edge and
 * false otherwise.
 */
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

    void concat(const GMatrix& matrix) override {

    }

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        std::deque<Edge> edges;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];

            clip(p0, p1, bounds, edges);
        }

        std::sort(edges.begin(), edges.end(), isEdgeGreater);

        // If we clipped all the edges away, we don't have to do anything.
        if (edges.size() == 0) {
            return;
        }

        GASSERT(edges.size() >= 2);

        int lastY = edges.back().bottomY;

        // Set up our initial left and right boundary edges
        Edge left = edges.front();
        edges.pop_front();

        Edge right = edges.front();
        edges.pop_front();

        float curY = left.topY;

        float leftX = left.curX;
        float rightX = right.curX;

        // Loop through all the possible y-coordinates that could be drawn
        while (curY <= lastY) {
            GRect row = GRect::MakeLTRB(leftX, curY, rightX, curY + 1);
            drawRect(row, paint);

            // After drawing, we check to see if we've completed either the
            // left or right edge. If we have, we replace it with the next
            // edge.

            if (curY >= left.bottomY) {
                left = edges.front();
                edges.pop_front();

                leftX = left.curX;
            } else {
                leftX += left.slope;
            }

            if (curY >= right.bottomY) {
                right = edges.front();
                edges.pop_front();

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

