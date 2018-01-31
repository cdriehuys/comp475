/**
 *  Copyright 2015 Mike Reed
 *
 *  COMP 575 -- Fall 2015
 */

#ifndef GWindow_DEFINED
#define GWindow_DEFINED

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "GBitmap.h"
#include "GPoint.h"

class GCanvas;
class GClick;

class GWindow {
public:
    int run();

protected:
    GWindow(int initial_width, int initial_height);
    virtual ~GWindow();

    virtual void onDraw(GCanvas*) {}
    virtual void onResize(int w, int h) {}
    virtual bool onKeyPress(const XEvent&, KeySym) { return false; }
    virtual GClick* onFindClickHandler(GPoint) { return NULL; }
    virtual void onHandleClick(GClick*) {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    
    void setTitle(const char title[]);
    void requestDraw();
    void setReadyToQuit() { fReadyToQuit = true; }
    
private:
    Display*    fDisplay;
    Window      fWindow;
    GC          fGC;
    GClick*     fClick;
    
    GBitmap fBitmap;
    GCanvas* fCanvas;
    int fWidth;
    int fHeight;
    bool fReadyToQuit;
    bool fNeedDraw;

    bool handleEvent(XEvent*);
    void drawCanvasToWindow();
    void setupBitmap(int w, int h);
};

class GClick {
public:
    GClick(GPoint, const char* name = NULL);
    
    enum State {
        kDown_State,
        kMove_State,
        kUp_State
    };
    
    State state() const { return fState; }
    GPoint curr() const { return fCurr; }
    GPoint prev() const { return fPrev; }
    GPoint orig() const { return fOrig; }

    const char* name() { return fName; }
    bool isName(const char name[]) const { return !strcmp(name, fName); }

private:
    GPoint  fCurr, fPrev, fOrig;
    State   fState;
    const char* fName;

    friend class GWindow;
};

#endif
