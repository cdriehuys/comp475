#include "GBlitter.h"
#include "GMath.h"
#include "GScanConverter.h"

#include "Clipper.h"


void GScanConverter::scan(Edge* edges, int count, GBlitter& blitter) {
    GASSERT(count >= 2);

    std::sort(edges, edges + count);

    int lastY = edges[count - 1].bottomY;

    // Set up our initial left and right boundary edges
    Edge left = edges[0];
    Edge right = edges[1];

    // Track index of next edge position
    int next = 2;

    float curY = left.topY;

    float leftX = left.curX;
    float rightX = right.curX;

    // Loop through all the possible y-coordinates that could be drawn
    while (curY < lastY) {
        blitter.blitRow(curY, GRoundToInt(leftX), GRoundToInt(rightX));
        curY++;

        // After drawing, we check to see if we've completed either the
        // left or right edge. If we have, we replace it with the next
        // edge.

        if (curY > left.bottomY) {
            left = edges[next];
            next++;

            leftX = left.curX;
        } else {
            leftX += left.dxdy;
        }

        if (curY > right.bottomY) {
            right = edges[next];
            next++;

            rightX = right.curX;
        } else {
            rightX += right.dxdy;
        }
    }
}
