#ifndef Clipper_DEFINED
#define Clipper_DEFINED

#include <deque>

#include "GPoint.h"
#include "GRect.h"


/**
 * An edge contains information about a line segment. It contains the minimum
 * amount of information necessary for our scan converter.
 */
struct Edge {
    int topY;
    int bottomY;
    float curX;
    float dxdy;
    short wind;

    /**
     * Initialize a new edge from two points.
     *
     * Args:
     *     p0:
     *         The first point describing the edge.
     *     p1:
     *         The second point describing the edge.
     *     wind:
     *         The edges initial winding value.
     *
     * Returns:
     *     A boolean indicating if the edge is meaningful. For example, edges
     *     that don't cross a pixel center vertically are not meaningful.
     */
    bool init(GPoint p0, GPoint p1, int wind);

    /**
     * Determine if the edge is "less than" another edge.
     *
     * Edges are compared by their upper Y value, starting X value, and then
     * slope.
     *
     * Args:
     *     other:
     *         The other edge to compare the current edge to.
     *
     * Returns:
     *     A boolean indicating if the edge is "less than" the specified edge.
     */
    bool operator<(const Edge& other);
};


/**
 * Clip the line segement described by two points.
 *
 * Args:
 *     p0:
 *         The first point of the line segment to clip.
 *     p1:
 *         The second point of the line segment to clip.
 *     bounds:
 *         A rectangle describing the boundaries of the clipping region. The
 *         part of the line segment extending outside the bounds will be
 *         clipped.
 *     edge:
 *         A pointer to the next available edge in an array of clipped edges.
 *
 * Returns:
 *     A pointer to the next available edge in the array of clipped edges. The
 *     pointer is moved forward by the number of edges created in the clip
 *     process for the line, which is at most 3.
 */
Edge* clipLine(GPoint p0, GPoint p1, GRect bounds, Edge* edge);


#endif
