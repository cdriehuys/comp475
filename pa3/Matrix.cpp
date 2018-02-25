#include <math.h>

#include "GMatrix.h"
#include "GPoint.h"


void GMatrix::setIdentity() {
    this->set6(1, 0, 0, 0, 1, 0);
}


void GMatrix::setTranslate(float tx, float ty) {
    this->setIdentity();
    this->fMat[GMatrix::TX] = tx;
    this->fMat[GMatrix::TY] = ty;
}


void GMatrix::setScale(float sx, float sy) {
    this->setIdentity();
    this->fMat[GMatrix::SX] = sx;
    this->fMat[GMatrix::SY] = sy;
}


void GMatrix::setRotate(float radians) {
    this->set6(
        cos(radians), -sin(radians), 0,
        sin(radians), cos(radians), 0);
}


void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo) {
    this->set6(
        primo[0] * secundo[0] + primo[3] * secundo[1],
        primo[1] * secundo[0] + primo[4] * secundo[1],
        primo[2] * secundo[0] + primo[5] * secundo[1] + secundo[2],
        primo[0] * secundo[3] + primo[3] * secundo[4],
        primo[1] * secundo[3] + primo[4] * secundo[4],
        primo[2] * secundo[3] + primo[5] * secundo[4] + secundo[5]);
}


bool GMatrix::invert(GMatrix* inverse) const {
    float a = this->fMat[0];
    float b = this->fMat[1];
    float c = this->fMat[2];
    float d = this->fMat[3];
    float e = this->fMat[4];
    float f = this->fMat[5];

    float determinant = a * e - b * d;
    if (determinant == 0) {
        return false;
    }

    float divisor = 1 / determinant;

    inverse->set6(
        e * divisor, -b * divisor, -(c * e - b * f) * divisor,
        -d * divisor, a * divisor, (c * d - a * f) * divisor);

    return true;
}


void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        GPoint point = src[i];

        float x0 = point.x();
        float y0 = point.y();

        float x = this->fMat[GMatrix::SX] * x0 + this->fMat[GMatrix::KX] * y0 + this->fMat[GMatrix::TX];
        float y = this->fMat[GMatrix::SY] * y0 + this->fMat[GMatrix::KY] * x0 + this->fMat[GMatrix::TY];

        dst[i] = GPoint::Make(x, y);
    }
}
