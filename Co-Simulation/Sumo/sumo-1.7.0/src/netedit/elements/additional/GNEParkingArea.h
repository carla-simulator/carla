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
/// @file    GNEParkingArea.h
/// @author  Pablo Alvarez Lopez
/// @date    Feb 2018
///
// A class for visualizing ParkingArea geometry (adapted from GUILaneWrapper)
/****************************************************************************/
#pragma once
#include "GNEStoppingPlace.h"


// ===========================================================================
// class definitions
// ===========================================================================

/**
 * @class GNEParkingArea
 * @brief A lane area vehicles can park at (netedit-version)
 */
class GNEParkingArea : public GNEStoppingPlace {

public:
    /**@brief Constructor
     * @param[in] id The storage of gl-ids to get the one for this lane representation from
     * @param[in] lane Lane of this StoppingPlace belongs
     * @param[in] net pointer to GNENet of this additional element belongs
     * @param[in] startPos Start position of the StoppingPlace
     * @param[in] endPos End position of the StoppingPlace
     * @param[in] name Name of ParkingArea
     * @param[in] friendlyPos enable or disable friendly position
     * @param[in] roadSideCapacity road side capacity of ParkingArea
     * @param[in] width ParkingArea's length
     * @param[in] length ParkingArea's length
     * @param[in] angle ParkingArea's angle
     * @param[in] block movement enable or disable additional movement
     */
    GNEParkingArea(const std::string& id, GNELane* lane, GNENet* net, const double startPos, const double endPos, const int parametersSet,
                   const std::string& name, bool friendlyPosition, int roadSideCapacity, bool onRoad, double width, const std::string& length,
                   double angle, bool blockMovement);

    /// @brief Destructor
    ~GNEParkingArea();

    /// @name Functions related with geometry of element
    /// @{
    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    Boundary getCenteringBoundary() const;

    /// @brief split geometry
    void splitEdgeGeometry(const double splitPosition, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList);
    /// @}

    /// @name inherited from GUIGlObject
    /// @{
    /**@brief Draws the object
     * @param[in] s The settings for the current view (may influence drawing)
     * @see GUIGlObject::drawGL
     */
    void drawGL(const GUIVisualizationSettings& s) const;
    /// @}

    /// @name inherited from GNEAttributeCarrier
    /// @{
    /* @brief method for getting the Attribute of an XML key
     * @param[in] key The attribute key
     * @return string with the value associated to key
     */
    std::string getAttribute(SumoXMLAttr key) const;

    /* @brief method for setting the attribute and letting the object perform additional changes
     * @param[in] key The attribute key
     * @param[in] value The new value
     * @param[in] undoList The undoList on which to register changes
     */
    void setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList);

    /* @brief method for checking if the key and their correspond attribute are valids
     * @param[in] key The attribute key
     * @param[in] value The value asociated to key key
     * @return true if the value is valid, false in other case
     */
    bool isValid(SumoXMLAttr key, const std::string& value);

    /// @}

protected:
    /// @brief roadside capacity of Parking Area
    int myRoadSideCapacity;

    /// @brief Whether vehicles stay on the road
    bool myOnRoad;

    /// @brief width of Parking Area
    double myWidth;

    /// @brief Length of Parking Area (by default (endPos - startPos) / roadsideCapacity
    std::string myLength;

    /// @brief Angle of Parking Area
    double myAngle;

private:
    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNEParkingArea(const GNEParkingArea&) = delete;

    /// @brief Invalidated assignment operator.
    GNEParkingArea& operator=(const GNEParkingArea&) = delete;
};


