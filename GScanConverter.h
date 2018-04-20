#ifndef GScanConverter_DEFINED
#define GScanConverter_DEFINED

#include "Clipper.h"


class GScanConverter {
public:
    /**
     * Scan converter to blit a set of edges that form a convex figure.
     *
     * Args:
     *     edges:
     *         A pointer to the beginning of the array of edges to blit.
     *     count:
     *         The number of edges in the array.
     *     blitter:
     *         The blitter to use to actually draw each row of pixels.
     */
    static void scan(Edge* edges, int count, GBlitter& blitter);

    /**
     * Scan converter to blit a set of edges that form a closed figure. Note
     * that there is no requirement here that the shape be convex.
     *
     * Args:
     *     edges:
     *         A pointer to the beginning of the array of edges to blit.
     *     count:
     *         The number of edges in the array.
     *     blitter:
     *         The blitter to use to actually draw each row of pixels.
     */
    static void scanComplex(Edge* edges, int count, GBlitter& blitter);
};


#endif
