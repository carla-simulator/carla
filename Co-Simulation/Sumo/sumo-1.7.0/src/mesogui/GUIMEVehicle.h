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
/// @file    GUIMEVehicle.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// A MSVehicle extended by some values for usage within the gui
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <string>
#include <guisim/GUIBaseVehicle.h>
#include <mesosim/MEVehicle.h>


// ===========================================================================
// class declarations
// ===========================================================================
class GUISUMOAbstractView;
class GUIVisualizationSettings;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIMEVehicle
 * @brief A MSVehicle extended by some values for usage within the gui
 *
 * A visualisable MSVehicle. Extended by the possibility to retrieve names
 * of all available vehicles (static) and the possibility to retrieve the
 * color of the vehicle which is available in different forms allowing an
 * easier recognition of done actions such as lane changing.
 */
class GUIMEVehicle : public MEVehicle, public GUIBaseVehicle {
public:
    /** @brief Constructor
     * @param[in] pars The vehicle description
     * @param[in] route The vehicle's route
     * @param[in] type The vehicle's type
     * @param[in] speedFactor The factor for driven lane's speed limits
     * @exception ProcessError If a value is wrong
     */
    GUIMEVehicle(SUMOVehicleParameter* pars, const MSRoute* route,
                 MSVehicleType* type, const double speedFactor);


    /// @brief destructor
    ~GUIMEVehicle();


    /** @brief Return current position (x/y, cartesian)
    *
    * @note implementation of abstract method does not work otherwise
    */
    Position getPosition(const double offset = 0) const {
        return MEVehicle::getPosition(offset);
    }

    /** @brief Return current angle
    *
    * @note implementation of abstract method does not work otherwise
    */
    double getAngle() const {
        return MEVehicle::getAngle();
    }

    /// @brief gets the color value according to the current scheme index
    double getColorValue(const GUIVisualizationSettings& s, int activeScheme) const;

    /// @brief draws the given guiShape with distinct carriages/modules
    void drawAction_drawCarriageClass(const GUIVisualizationSettings& s, bool asImage) const;

    /** @brief Returns the time since the last lane change in seconds
     * @see MSVehicle::myLastLaneChangeOffset
     * @return The time since the last lane change in seconds
     */
    double getLastLaneChangeOffset() const;

    /** @brief Draws the route
     * @param[in] r The route to draw
     */
    void drawRouteHelper(const GUIVisualizationSettings& s, const MSRoute& r, bool future, bool noLoop, const RGBColor& col) const;

    /// @brief retrieve information about the current stop state
    std::string getStopInfo() const;

    std::string getEdgeID() const;

    int getSegmentIndex() const;

    /// @brief adds the blocking foes to the current selection
    void selectBlockingFoes() const;

    /** @brief Returns an own parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     * @see GUIGlObject::getParameterWindow
     */
    GUIParameterTableWindow* getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

    /** @brief Returns an own type parameter window
     *
     * @param[in] app The application needed to build the parameter window
     * @param[in] parent The parent window needed to build the parameter window
     * @return The built parameter window
     */
    GUIParameterTableWindow* getTypeParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& parent);

};
