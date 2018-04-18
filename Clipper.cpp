#include "Clipper.h"


bool Edge::init(GPoint p0, GPoint p1) {
    this->wind = (p0.y() <= p1.y()) ? 1 : -1;

    // Ensure p0.y <= p1.y
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
        this->wind = -this->wind;
    }

    this->topY = GRoundToInt(p0.y());
    this->bottomY = GRoundToInt(p1.y());

    if (topY == bottomY) {
        return false;
    }

    this->dxdy = (p1.x() - p0.x()) / (bottomY - topY);

    float dx = this->dxdy * (this->topY - p0.y() + 0.5f);
    this->curX = p0.x() + dx;

    return true;
}


bool Edge::operator<(const Edge& other) {
    if (this->topY < other.topY) {
        return true;
    }

    if (other.topY < this->topY) {
        return false;
    }

    if (this->curX < other.curX) {
        return true;
    }

    if (other.curX < this->curX) {
        return false;
    }

    return this->dxdy <= other.dxdy;
}


Edge* clipLine(GPoint p0, GPoint p1, GRect bounds, Edge* edge) {
    // Ensure p0.y <= p1.y
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
    }

    // We can discard edges that are completely out of the clipping region in
    // the vertical direction.
    if (p1.y() <= bounds.top() || p0.y() >= bounds.bottom()) {
        return edge;
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
        p0.fX = p1.fX = bounds.left();
        return edge + edge->init(p0, p1);
    }

    // Similarly, if we're entirely outside the right edge, we only add the
    // projection back onto the right bound.
    if (p0.x() >= bounds.right()) {
        p0.fX = p1.fX = bounds.right();
        return edge + edge->init(p0, p1);
    }

    // Handle clipping and projection onto the left boundary
    if (p0.x() < bounds.left()) {
        float newY = p0.y() + (bounds.left() - p0.x()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        edge += edge->init(
            GPoint::Make(bounds.left(), p0.y()),
            GPoint::Make(bounds.left(), newY));

        p0.set(bounds.left(), newY);
    }

    // Handle clipping and projection onto the right boundary
    if (p1.x() > bounds.right()) {
        float newY = p1.y() - (p1.x() - bounds.right()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        edge += edge->init(
            GPoint::Make(bounds.right(), newY),
            GPoint::Make(bounds.right(), p1.y()));

        p1.set(bounds.right(), newY);
    }

    // Now that we've clipped the segment, we make an edge from what's left.
    return edge + edge->init(p0, p1);
}
