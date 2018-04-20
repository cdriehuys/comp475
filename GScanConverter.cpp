#include <algorithm>
#include <vector>

#include "GBlitter.h"
#include "GMath.h"
#include "GScanConverter.h"
#include "GTypes.h"

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


int getEdgeIndex(Edge* edge, std::vector<Edge*> edges) {
    int pos = std::find(edges.begin(), edges.end(), edge) - edges.begin();

    if (pos >= edges.size()) {
        // Couldn't find edge
        GASSERT(false);
    }

    return pos;
}


bool isEdgeLess(Edge* first, Edge* second) {
    return first->curX <= second->curX;
}


void resortBackwards(Edge* edge, std::vector<Edge*>& edges) {
    int pos = getEdgeIndex(edge, edges);

    std::sort(edges.begin(), edges.begin() + pos + 1, isEdgeLess);
}


// Note that unlike the simple scan converter, this one is "destructive"
// because it manipulates each edge's 'curX' property. This is unlikely to
// matter since we don't do anything with the edges after drawing them.
void GScanConverter::scanComplex(Edge* edges, int count, GBlitter& blitter) {
    GASSERT(count >= 2);

    std::sort(edges, edges + count);

    std::vector<Edge*> edgeVec;
    for (int i = 0; i < count; ++i) {
        edgeVec.push_back(&edges[i]);
    }

    int minY = edgeVec.front()->topY;
    int maxY = edgeVec.back()->bottomY;

    for (int y = minY; y < maxY;) {
        int wind = 0;
        Edge* edge = edgeVec.front();
        Edge* next;

        int x0, x1;

        while (edge->topY <= y) {
            if (wind == 0) {
                x0 = GRoundToInt(edge->curX);
            }

            wind += edge->wind;

            if (wind == 0) {
                x1 = GRoundToInt(edge->curX);
                blitter.blitRow(y, x0, x1);
            }

            int nextIndex = getEdgeIndex(edge, edgeVec) + 1;
            if (nextIndex >= edgeVec.size()) {
                break;
            }
            next = edgeVec[nextIndex];

            if (edge->bottomY == y + 1) {
                edgeVec.erase(edgeVec.begin() + getEdgeIndex(edge, edgeVec));
            } else {
                edge->curX += edge->dxdy;
                resortBackwards(edge, edgeVec);
            }

            edge = next;
        }

        y++;

        while (edge->topY == y) {
            int nextIndex = getEdgeIndex(edge, edgeVec) + 1;
            if (nextIndex >= edgeVec.size()) {
                break;
            }
            next = edgeVec[nextIndex];
            resortBackwards(edge, edgeVec);
            edge = next;
        }
    }
}
