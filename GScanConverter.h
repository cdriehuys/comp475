#ifndef GScanConverter_DEFINED
#define GScanConverter_DEFINED

#include "Clipper.h"


class GScanConverter {
public:
    static void scan(Edge* edges, int count, GBlitter& blitter);
};


#endif
