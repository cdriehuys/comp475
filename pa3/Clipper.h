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
    float slope;
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
 *     results:
 *         A deque containing the edges resulting from the clipping process.
 *         Each clip operation can result in up to 3 new edges.
 */
void clip(GPoint p0, GPoint p1, GRect bounds, std::deque<Edge>& results);


#endif
