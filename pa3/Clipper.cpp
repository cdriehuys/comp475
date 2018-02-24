#include "Clipper.h"


/**
 * Create an edge between two points.
 *
 * If the created edge is not useful, a nullptr is returned. Otherwise, a
 * pointer to the created edge is returned.
 *
 * Args:
 *     p0:
 *         The first point of the edge.
 *     p1:
 *         The second point of the edge.
 */
Edge* makeEdge(GPoint p0, GPoint p1) {
    // Ensure that p0.y <= p1.y is true
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
    }

    int topY = GRoundToInt(p0.y());
    int bottomY = GRoundToInt(p1.y());

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
 *
 * Args:
 *     x:
 *         The x-coordinate of the vertical axis the line is being projected
 *         onto.
 *     y1:
 *         The y-coordinate of the first point being projected onto the axis.
 *     y2:
 *         The y-coordinate of the second point being projected onto the axis.
 *     results:
 *         The deque that the created edge is appended to. If the resulting
 *         edge is not useful, nothing will be added to the deque.
 */
void project(float x, float y1, float y2, std::deque<Edge>& results) {
    Edge* projected = makeEdge(GPoint::Make(x, y1), GPoint::Make(x, y2));

    if (projected != nullptr) {
        results.push_back(*projected);
    }
}


void clip(GPoint p0, GPoint p1, GRect bounds, std::deque<Edge>& results) {
    // Ensure p0.y <= p1.y
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
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
        std::swap(p0, p1);
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
