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
/// @file    GUIChargingStation.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Tamas Kurczveil
/// @author  Pablo Alvarez Lopez
/// @date    20-12-13
///
// A lane area vehicles can halt at (gui-version)
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <string>
#include <utils/common/Command.h>
#include <utils/common/VectorHelper.h>
#include <utils/geom/PositionVector.h>
#include <microsim/trigger/MSChargingStation.h>
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


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIChargingStation
 * @brief A lane area vehicles can halt at (gui-version)
 *
 * This gui-version of a bus-stop extends MSChargingStation by methods for displaying
 *  and interaction.
 *
 * @see MSChargingStation
 * @see GUIGlObject_AbstractAdd
 * @see GUIGlObject
 */
class GUIChargingStation : public MSChargingStation, public GUIGlObject_AbstractAdd {
public:
    /** @brief Constructor
     * @param[in] id The id of the Charging Station
     * @param[in] lane The lane the charging station is placed on
     * @param[in] frompos Begin position of the charging station on the lane
     * @param[in] topos End position of the charging station on the lane
     * @param[in] chargingPower energy charged in every timeStep
     * @param[in] efficiency efficiency of the charge
     * @param[in] chargeInTransit enable or disable charge in transit
     * @param[in] chargeDelay delay in the charge
     */
    GUIChargingStation(const std::string& id, MSLane& lane, double frompos, double topos,
                       const std::string& name,
                       double chargingPower, double efficiency, bool chargeInTransit, double chargeDelay);

    /// @brief Destructor
    ~GUIChargingStation();

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
     * Bus stops have no parameter windows (yet).
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window (always 0 in this case)
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns the boundary to which the view shall be centered in order to show the object
     *
     * @return The boundary the object is within
     * @see GUIGlObject::getCenteringBoundary
     */
    Boundary getCenteringBoundary() const;

    /// @brief Returns the stopping place name
    const std::string getOptionalName() const;

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


