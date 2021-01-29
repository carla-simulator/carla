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
/// @file    GUIPerspectiveChanger.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A virtual class that allows to steer the visual output in dependence to
/****************************************************************************/
#pragma once
#include <config.h>

#include <fx.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include "GUISUMOAbstractView.h"


// ===========================================================================
// class declarations
// ===========================================================================
class GUISUMOAbstractView;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIPerspectiveChanger
 * This is the interface for implementation of own classes that handle the
 * interaction between the user and a display field.
 * While most of our (IVF) interfaces allow zooming by choosing the rectangle
 * to show, other types of interaction are possible and have been implemented.
 * To differ between the behaviours, all types of interaction between the
 * user and the canvas are send to this class: mouse moving, mouse button
 * pressing and releasing.
 */
class GUIPerspectiveChanger {
public:
    /// @brief mouse states
    enum MouseState {
        MOUSEBTN_NONE = 0,
        MOUSEBTN_LEFT = 1,
        MOUSEBTN_RIGHT = 2,
        MOUSEBTN_MIDDLE = 4
    };

    /// @brief Constructor
    GUIPerspectiveChanger(GUISUMOAbstractView& callBack, const Boundary& viewPort);

    /// @brief Destructor
    virtual ~GUIPerspectiveChanger();

    /// @brief mouse functions
    //@{
    /// @brief called when user press left button
    virtual void onLeftBtnPress(void* data);

    /// @brief called when user releases left button
    virtual bool onLeftBtnRelease(void* data);

    /// @brief called when user press right button
    virtual void onRightBtnPress(void* data);

    /// @brief called when user releases right button
    virtual bool onRightBtnRelease(void* data);

    /// @brief called when user click two times
    virtual void onDoubleClicked(void* data);

    /// @brief called when user changes mouse wheel
    virtual void onMouseWheel(void* data);

    /// @brief called when user moves mouse
    virtual void onMouseMove(void* data);

    /// @brief called when user press a key
    virtual long onKeyPress(void* data);

    /// @brief called when user releases a key
    virtual long onKeyRelease(void* data);
    //@}

    /// @brief Returns the rotation of the canvas stored in this changer
    virtual double getRotation() const = 0;

    /// @brief Returns the x-offset of the field to show stored in this changer
    virtual double getXPos() const = 0;

    /// @brief Returns the y-offset of the field to show stored in this changer
    virtual double getYPos() const = 0;

    /// @brief Returns the zoom factor computed stored in this changer
    virtual double getZoom() const = 0;

    /// @brief Returns the camera height corresponding to the current zoom factor
    virtual double getZPos() const = 0;

    /// @brief Returns the camera height at which the given zoom level is reached
    virtual double zoom2ZPos(double zoom) const = 0;

    /// @brief Returns the zoom level that is achieved at a given camera height
    virtual double zPos2Zoom(double zPos) const = 0;

    /// @brief Centers the view to the given position, setting it to a size that covers the radius. Used for: Centering of vehicles and junctions */
    virtual void centerTo(const Position& pos, double radius, bool applyZoom = true) = 0;

    /// @brief Sets the viewport Used for: Adapting a new viewport
    virtual void setViewport(double zoom, double xPos, double yPos) = 0;

    /// @brief Alternative method for setting the viewport
    virtual void setViewportFrom(double xPos, double yPos, double zPos) = 0;

    /// @brief Sets the rotation
    virtual void setRotation(double rotation) = 0;

    /// @brief Returns the last mouse x-position an event occurred at
    FXint getMouseXPosition() const;

    /// @brief Returns the last mouse y-position an event occurred at
    FXint getMouseYPosition() const;

    /* @brief Adapts the viewport so that a change in canvass size keeps most of the
     * view intact (by showing more / less instead of zooming)
     * The canvass is clipped/enlarged on the left side of the screen
     *
     * @param[in] change The horizontal change in canvas size in pixels
     */
    virtual void changeCanvasSizeLeft(int change) = 0;

    /// @brief get viewport
    Boundary getViewport(bool fixRatio = true);

    /// @brief set viewport
    void setViewport(const Boundary& viewPort);

protected:
    /// @brief The parent window (canvas to scale)
    GUISUMOAbstractView& myCallback;

    /// @brief the current mouse position
    FXint myMouseXPosition, myMouseYPosition;

    /// @brief the intended viewport
    Boundary myViewPort;


private:
    /// @brief patched viewPort with the same aspect ratio as the canvas
    Boundary patchedViewPort();


private:
    /// @brief Invalidated copy constructor.
    GUIPerspectiveChanger(const GUIPerspectiveChanger&);

    /// @brief Invalidated assignment operator.
    GUIPerspectiveChanger& operator=(const GUIPerspectiveChanger&);
};
