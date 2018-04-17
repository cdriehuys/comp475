#include "GPath.h"
#include "GRect.h"


GPath& GPath::addPolygon(const GPoint pts[], int count) {
    return *this;
}


GPath& GPath::addRect(const GRect& rect, Direction dir) {
    return *this;
}


GRect GPath::bounds() const {
    return GRect::MakeWH(0, 0);
}


void GPath::transform(const GMatrix&) {

}
