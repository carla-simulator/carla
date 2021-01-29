/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    GUIDanielPerspectiveChanger.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A class that allows to steer the visual output in dependence to
/****************************************************************************/
#include <config.h>

#include <fxkeys.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/settings/GUICompleteSchemeStorage.h>
#include "GUIPerspectiveChanger.h"
#include "GUIDanielPerspectiveChanger.h"


// ===========================================================================
// method definitions
// ===========================================================================
GUIDanielPerspectiveChanger::GUIDanielPerspectiveChanger(
    GUISUMOAbstractView& callBack, const Boundary& viewPort) :
    GUIPerspectiveChanger(callBack, viewPort),
    myOrigWidth(viewPort.getWidth()),
    myOrigHeight(viewPort.getHeight()),
    myRotation(0),
    myMouseButtonState(MOUSEBTN_NONE),
    myMoveOnClick(false),
    myZoomBase(viewPort.getCenter()),
    myDragDelay(0) {
}


GUIDanielPerspectiveChanger::~GUIDanielPerspectiveChanger() {}


void
GUIDanielPerspectiveChanger::move(int xdiff, int ydiff) {
    myViewPort.moveby(myCallback.p2m(xdiff), -myCallback.p2m(ydiff));
    myCallback.update();
}


void
GUIDanielPerspectiveChanger::zoom(double factor) {
    if (myCallback.getApp()->reg().readIntEntry("gui", "zoomAtCenter", 0)) {
        myZoomBase = myViewPort.getCenter();
    }
    if (factor > 0) {
        myViewPort = Boundary(
                         myZoomBase.x() - (myZoomBase.x() - myViewPort.xmin()) / factor,
                         myZoomBase.y() - (myZoomBase.y() - myViewPort.ymin()) / factor,
                         myZoomBase.x() - (myZoomBase.x() - myViewPort.xmax()) / factor,
                         myZoomBase.y() - (myZoomBase.y() - myViewPort.ymax()) / factor);
        myCallback.update();
    }
}


void
GUIDanielPerspectiveChanger::rotate(int /* diff */) {
    /*
    if (myCallback.allowRotation()) {
        myRotation += (double) diff / (double) 10.0;
        myCallback.update();
    }
    */
}


double
GUIDanielPerspectiveChanger::getRotation() const {
    return myRotation;
}


double
GUIDanielPerspectiveChanger::getXPos() const {
    return myViewPort.getCenter().x();
}


double
GUIDanielPerspectiveChanger::getYPos() const {
    return myViewPort.getCenter().y();
}


double
GUIDanielPerspectiveChanger::getZoom() const {
    return myOrigWidth / myViewPort.getWidth() * 100;
}


double
GUIDanielPerspectiveChanger::getZPos() const {
    return myViewPort.getWidth();
}


double
GUIDanielPerspectiveChanger::zoom2ZPos(double zoom) const {
    return myOrigWidth / (zoom / 100);
}


double
GUIDanielPerspectiveChanger::zPos2Zoom(double zPos) const {
    return (myOrigWidth / zPos) * 100;
}


void
GUIDanielPerspectiveChanger::centerTo(const Position& pos, double radius,
                                      bool applyZoom) {
    if (applyZoom) {
        myViewPort = Boundary();
        myViewPort.add(pos);
        myViewPort.grow(radius);
    } else {
        myViewPort.moveby(pos.x() - getXPos(), pos.y() - getYPos());
    }
}


void
GUIDanielPerspectiveChanger::onLeftBtnPress(void* data) {
    myMouseButtonState |= MOUSEBTN_LEFT;
    FXEvent* e = (FXEvent*) data;
    myMouseXPosition = e->win_x;
    myMouseYPosition = e->win_y;
    myMoveOnClick = false;
    myMouseDownTime = FXThread::time();
}


bool
GUIDanielPerspectiveChanger::onLeftBtnRelease(void* data) {
    myMouseButtonState &= ~MOUSEBTN_LEFT;
    FXEvent* e = (FXEvent*) data;
    myMouseXPosition = e->win_x;
    myMouseYPosition = e->win_y;
    return myMoveOnClick;
}


void
GUIDanielPerspectiveChanger::onRightBtnPress(void* data) {
    myMouseButtonState |= MOUSEBTN_RIGHT;
    FXEvent* e = (FXEvent*) data;
    myMouseXPosition = e->win_x;
    myMouseYPosition = e->win_y;
    myMoveOnClick = false;
    myMouseDownTime = FXThread::time();
    myZoomBase = myCallback.getPositionInformation();
}


bool
GUIDanielPerspectiveChanger::onRightBtnRelease(void* data) {
    myMouseButtonState &= ~MOUSEBTN_RIGHT;
    if (data != nullptr) {
        FXEvent* e = (FXEvent*) data;
        myMouseXPosition = e->win_x;
        myMouseYPosition = e->win_y;
    }
    return myMoveOnClick;
}


void
GUIDanielPerspectiveChanger::onMouseWheel(void* data) {
    FXEvent* e = (FXEvent*) data;
    // catch empty ghost events after scroll (seem to occur only on Ubuntu)
    if (e->code == 0) {
        return;
    }
    // zoom scale relative delta and its inverse; is optimized (all literals)
    const double zScale_rDelta_norm = 0.1;
    const double zScale_rDelta_inv = -zScale_rDelta_norm / (1. + zScale_rDelta_norm);
    double zScale_rDelta = zScale_rDelta_norm ;
    if (e->code < 0) {
        // for inverse zooming direction
        zScale_rDelta = zScale_rDelta_inv;
    }
    // keyboard modifier: slow, fast mouse-zoom
    if ((e->state & CONTROLMASK) != 0) {
        zScale_rDelta /= 4;
    } else if ((e->state & SHIFTMASK) != 0) {
        zScale_rDelta *= 4;
    }
    myZoomBase = myCallback.getPositionInformation();
    zoom(1.0 + zScale_rDelta);
    myCallback.updateToolTip();
}


void
GUIDanielPerspectiveChanger::onMouseMove(void* data) {
    FXEvent* e = (FXEvent*) data;
    myCallback.setWindowCursorPosition(e->win_x, e->win_y);
    const int xdiff = myMouseXPosition - e->win_x;
    const int ydiff = myMouseYPosition - e->win_y;
    const bool moved = xdiff != 0 || ydiff != 0;
    const bool pastDelay = !gSchemeStorage.getDefault().gaming && FXThread::time() > (myMouseDownTime + myDragDelay);
    switch (myMouseButtonState) {
        case MOUSEBTN_LEFT:
            if (pastDelay) {
                if (myRotation != 0) {
                    Position diffRot = Position(xdiff, ydiff).rotateAround2D(
                                           DEG2RAD(myRotation), Position(0, 0));
                    move((int)diffRot.x(), (int)diffRot.y());
                } else {
                    move(xdiff, ydiff);
                }
                if (moved) {
                    myMoveOnClick = true;
                }
            }
            break;
        case MOUSEBTN_RIGHT:
            if (pastDelay) {
                zoom(1 + 10.0 * ydiff / myCallback.getWidth());
                rotate(xdiff);
                if (moved) {
                    myMoveOnClick = true;
                }
            }
            break;
        default:
            if (moved) {
                myCallback.updateToolTip();
            }
            break;
    }
    myMouseXPosition = e->win_x;
    myMouseYPosition = e->win_y;
}


void
GUIDanielPerspectiveChanger::setViewport(double zoom,
        double xPos, double yPos) {
    const double zoomFactor = zoom / 50; // /100 to normalize, *2 because growth is added on both sides
    myViewPort = Boundary();
    myViewPort.add(Position(xPos, yPos));
    myViewPort.growHeight(myOrigHeight / zoomFactor);
    myViewPort.growWidth(myOrigWidth / zoomFactor);
    myCallback.update();
}


void
GUIDanielPerspectiveChanger::setViewportFrom(double xPos, double yPos, double zPos) {
    setViewport(zPos2Zoom(zPos), xPos, yPos);
}


void
GUIDanielPerspectiveChanger::setRotation(double rotation) {
    myRotation = rotation;
}

void
GUIDanielPerspectiveChanger::changeCanvasSizeLeft(int change) {
    myViewPort = Boundary(
                     myViewPort.xmin() - myCallback.p2m(change),
                     myViewPort.ymin(),
                     myViewPort.xmax(),
                     myViewPort.ymax());
}


long
GUIDanielPerspectiveChanger::onKeyPress(void* data) {
    // ignore key events in gaming mode
    if (gSchemeStorage.getDefault().gaming) {
        return 0;
    }
    FXEvent* e = (FXEvent*) data;
    double zoomDiff = 0.1;
    double moveX = 0;
    double moveY = 0;
    double moveFactor = 1;
    bool pageVertical = true;
    if (e->state & CONTROLMASK) {
        zoomDiff /= 2;
        moveFactor /= 10;
    } else if (e->state & SHIFTMASK) {
        pageVertical = false;
        zoomDiff *= 2;
    }
    switch (e->code) {
        case FX::KEY_Left:
            moveX = -1;
            moveFactor /= 10;
            break;
        case FX::KEY_Right:
            moveX = 1;
            moveFactor /= 10;
            break;
        case FX::KEY_Up:
            moveY = -1;
            moveFactor /= 10;
            break;
        case FX::KEY_Down:
            moveY = 1;
            moveFactor /= 10;
            break;
        case FX::KEY_Page_Up:
            if (pageVertical) {
                moveY = -1;
            } else {
                moveX = -1;
            }
            break;
        case FX::KEY_Page_Down:
            if (pageVertical) {
                moveY = 1;
            } else {
                moveX = 1;
            }
            break;
        case FX::KEY_plus:
        case FX::KEY_KP_Add:
            myZoomBase = myCallback.getPositionInformation();
            zoom(1.0 + zoomDiff);
            myCallback.updateToolTip();
            return 1;
        case FX::KEY_minus:
        case FX::KEY_KP_Subtract:
            zoomDiff = -zoomDiff;
            myZoomBase = myCallback.getPositionInformation();
            zoom(1.0 + zoomDiff);
            myCallback.updateToolTip();
            return 1;
        case FX::KEY_Home:
        case FX::KEY_KP_Home:
            myCallback.recenterView();
            myCallback.update();
            return 1;
        default:
            return 0;
    }
    myViewPort.moveby(moveX * moveFactor * myViewPort.getWidth(),
                      -moveY * moveFactor * myViewPort.getHeight());
    myCallback.update();
    return 1;
}


/****************************************************************************/
