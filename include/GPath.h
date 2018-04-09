#ifndef GPath_DEFINED
#define GPath_DEFINED

#include <vector>
#include "GPoint.h"
#include "GRect.h"

class GMatrix;

class GPath {
public:
    GPath();
    ~GPath();

    GPath& operator=(const GPath&);

    GPath& reset();
    
    GPath& moveTo(GPoint);
    GPath& moveTo(float x, float y) { return this->moveTo({x, y}); }

    GPath& lineTo(GPoint);
    GPath& lineTo(float x, float y) { return this->lineTo({x, y}); }

    enum Direction {
        kCW_Direction,
        kCCW_Direction,
    };
    
    /**
     *  Adds 4 points to the path, traversing the rect in the specified direction, beginning
     *  with the top-left corner.
     */
    GPath& addRect(const GRect&, Direction = kCW_Direction);

    // moveTo(pts[0]), lineTo(pts[1..count-1])
    GPath& addPolygon(const GPoint pts[], int count);

    int countPoints() const { return (int)fPts.size(); }

    /**
     *  Return the bounds of all of the control-points in the path.
     *
     *  If there are no points, return {0, 0, 0, 0}
     */
    GRect bounds() const;

    /**
     *  Transform the path in-place by the specified matrix.
     */
    void transform(const GMatrix&);

    enum Verb {
        kMove,  // returns pts[0] from Iter
        kLine,  // returns pts[0]..pts[1] from Iter
        kDone   // returns nothing in pts, Iter is done
    };

    /**
     *  Walks the path, returning each verb that was entered.
     *  e.g.    moveTo() returns kMove
     *          lineTo() returns kLine
     */
    class Iter {
    public:
        Iter(const GPath&);
        Verb next(GPoint pts[]);

    private:
        const GPoint* fPrevMove;
        const GPoint* fCurrPt;
        const Verb*   fCurrVb;
        const Verb*   fStopVb;
    };

    /**
     *  Walks the path, returning "edges" only. Thus it does not return kMove, but will return
     *  the final closing "edge" for each contour.
     *  e.g.
     *      path.moveTo(A).lineTo(B).lineTo(C).moveTo(D).lineTo(E)
     *  will return
     *      kLine   A..B
     *      kLine   B..C
     *      kLine   C..A
     *      kLine   D..E
     *      kLine   E..D
     *      kDone
     */
    class Edger {
    public:
        Edger(const GPath&);
        Verb next(GPoint pts[]);

    private:
        const GPoint* fPrevMove;
        const GPoint* fCurrPt;
        const Verb*   fCurrVb;
        const Verb*   fStopVb;
        Verb fPrevVerb;
    };

private:
    std::vector<GPoint> fPts;
    std::vector<Verb>   fVbs;
};

#endif

