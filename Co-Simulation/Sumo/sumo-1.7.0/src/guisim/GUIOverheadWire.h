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
/// @file    GUIOverheadWire.h
/// @author  Jakub Sevcik (RICE)
/// @author  Jan Prikryl (RICE)
/// @date    2019-11-25
///
// The gui-version of a MSOverheadWire
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utils/common/Command.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/PositionVector.h>
#include <microsim/trigger/MSOverheadWire.h>
#include <guisim/GUIBusStop.h>
#include <utils/gui/globjects/GUIGlObject.h>
#include <utils/gui/globjects/GUIGlObject_AbstractAdd.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/geom/Position.h>
#include <gui/GUIManipulator.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSLane;
class GUIManipulator;
class GUIOverheadWire;
class GUIOverheadWireClamp;


// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class GUIOverheadWire
* @brief GUI for the overhead wire system.
*
* This gui-version of the overhead wire network extends MSOverheadWire by methods for displaying
*  and interaction. It also adds a visual representation of an overhead wire clamp.
*
* @see MSOverheadWire
* @see GUIGlObject_AbstractAdd
* @see GUIGlObject
*/
class GUIOverheadWire : public MSOverheadWire, public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
    * @param[in] id The id of the overhead wire segment
    * @param[in] lane The lane the overhead wire segment is placed on
    * @param[in] frompos Begin position of the overhead wire segment on the lane
    * @param[in] topos End position of the overhead wire segment on the lane
    * @param[in] voltageSource If the segment is the place of the connection of a traction substation
    */
    GUIOverheadWire(const std::string& id, MSLane& lane, double frompos, double topos,
                    bool voltageSource);

    /// @brief Destructor
    ~GUIOverheadWire();

    /// @name inherited from GUIGlObject
    //@{

    /** @brief Returns an own popup-menu
    *
    * @param[in] app The application needed to build the popup-menu
    * @param[in] parent The parent window needed to build the popup-menu
    * @return The built popup-menu
    * @see GUIGlObject::getPopUpMenu
    */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns an own parameter window
    *
    * Overhead wires have parameter windows showing beginning and end position of the overhead wire.
    *
    * @param[in] app The application needed to build the parameter window
    * @param[in] parent The parent window needed to build the parameter window
    * @return The built parameter window.
    * @see GUIGlObject::getParameterWindow
    */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns the boundary to which the view shall be centered in order to show the object
    *
    * @return The boundary the object is within
    * @see GUIGlObject::getCenteringBoundary
    */
    Boundary getCenteringBoundary() const;

    /** @brief Draws the object
    * @param[in] s The settings for the current view (may influence drawing)
    * @see GUIGlObject::drawGL
    */
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}

private:
    /// @brief The rotations of the shape parts
    std::vector<double> myFGShapeRotations;

    /// @brief The lengths of the shape parts
    std::vector<double> myFGShapeLengths;

    /// @brief The shape
    PositionVector myFGShape;

    /// @brief The position of the sign
    Position myFGSignPos;

    /// @brief The rotation of the sign
    double myFGSignRot;
};

class GUIOverheadWireClamp : public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
    * @param[in] id The id of the overhead wire clamp
    * @param[in] lane_start The lane, where is the overhead wire segment placed, to the start of which the overhead wire clamp is connected
    * @param[in] lane_end The lane, where is the overhead wire segment placed, to the end of which the overhead wire clamp is connected
    * @param[in] topos End position of the overhead wire segment on the lane
    */
    GUIOverheadWireClamp(const std::string& id, MSLane& lane_start, MSLane& lane_end);

    /// @brief Destructor
    ~GUIOverheadWireClamp();

    /// @name inherited from GUIGlObject
//@{

    /** @brief Returns an own popup-menu
    *
    * @param[in] app The application needed to build the popup-menu
    * @param[in] parent The parent window needed to build the popup-menu
    * @return The built popup-menu
    * @see GUIGlObject::getPopUpMenu
    */
    GUIGLObjectPopupMenu* getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns an own parameter window
    *
    * Overhead wire clamps do not display anything in their parameter window yet.
    *
    * @param[in] app The application needed to build the parameter window
    * @param[in] parent The parent window needed to build the parameter window
    * @return The built parameter window
    * @see GUIGlObject::getParameterWindow
    */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns the boundary to which the view shall be centered in order to show the object
    *
    * @return The boundary the object is within
    * @see GUIGlObject::getCenteringBoundary
    */
    Boundary getCenteringBoundary() const;

    /** @brief Draws the object
    * @param[in] s The settings for the current view (may influence drawing)
    * @see GUIGlObject::drawGL
    */
    void drawGL(const GUIVisualizationSettings& s) const;
    //@}

private:

    /// @brief The shape
    PositionVector myFGShape;
};


