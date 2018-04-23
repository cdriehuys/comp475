#include "GMatrix.h"
#include "GPath.h"
#include "GPoint.h"
#include "GRect.h"

#include "MathUtils.h"


void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
    dst[4] = src[3];
    dst[5] = src[3];
    dst[6] = src[3];
}


void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[2];
    dst[4] = src[2];
}


GPath& GPath::addCircle(GPoint center, float radius, Direction dir) {
    GRect rect = GRect::MakeLTRB(
        center.x() - radius / 2,
        center.y() - radius / 2,
        center.x() + radius / 2,
        center.y() + radius/ 2);

    return this->addRect(rect, dir);
}


GPath& GPath::addPolygon(const GPoint pts[], int count) {
    if (count <= 1) { return *this; }

    this->moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }

    return *this;
}


GPath& GPath::addRect(const GRect& rect, Direction dir) {
    this->moveTo(GPoint::Make(rect.left(), rect.top()));

    if (dir == Direction::kCW_Direction) {
        this->lineTo(GPoint::Make(rect.right(), rect.top()));
        this->lineTo(GPoint::Make(rect.right(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.left(), rect.bottom()));
    } else {
        this->lineTo(GPoint::Make(rect.left(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.right(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.right(), rect.top()));
    }

    return *this;
}


GRect GPath::bounds() const {
    int count = this->fPts.size();

    if (count == 0) {
        return GRect::MakeWH(0, 0);
    }

    float xVals[count], yVals[count];
    for (int i = 0; i < count; ++i) {
        xVals[i] = fPts[i].fX;
        yVals[i] = fPts[i].fY;
    }

    return GRect::MakeLTRB(
        manyMin(xVals, count),
        manyMin(yVals, count),
        manyMax(xVals, count),
        manyMax(yVals, count));
}


void GPath::transform(const GMatrix& matrix) {
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}
