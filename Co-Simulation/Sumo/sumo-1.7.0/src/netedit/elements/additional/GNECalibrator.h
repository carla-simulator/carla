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
/// @file    GNECalibrator.h
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#pragma once
#include "GNEAdditional.h"

// ===========================================================================
// class declaration
// ===========================================================================

class GNERouteProbe;
class GNECalibratorFlow;
class GNERoute;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNECalibrator
 * class for represent Calibratos in netedit
 */
class GNECalibrator : public GNEAdditional {

public:

    /**@brief Constructor using edge
    * @param[in] id The storage of gl-ids to get the one for this lane representation from
    * @param[in] net pointer to GNENet of this additional element belongs
    * @param[in] edge Edge of this calibrator belongs
    * @param[in] pos position of the calibrator on the edge (Currently not used)
    * @param[in] frequency the aggregation interval in which to calibrate the flows
    * @param[in] name Calibrator name
    * @param[in] output The output file for writing calibrator information
    * @param[in] routeprobe route probe vinculated with this calibrator
    * @param[in] calibratorRoutes vector with the calibratorRoutes of calibrator
    * @param[in] calibratorFlows vector with the calibratorFlows of calibrator
    */
    GNECalibrator(const std::string& id, GNENet* net, GNEEdge* edge, double pos, SUMOTime frequency, const std::string& name, const std::string& output, const std::string& routeprobe);

    /**@brief Constructor using lane
    * @param[in] id The storage of gl-ids to get the one for this lane representation from
    * @param[in] net pointer to GNENet of this additional element belongs
    * @param[in] lane Lane of this calibrator belongs
    * @param[in] pos position of the calibrator on the edge (Currently not used)
    * @param[in] frequency the aggregation interval in which to calibrate the flows
    * @param[in] name Calibrator name
    * @param[in] routeprobe route probe vinculated with this calibrator
    * @param[in] output The output file for writing calibrator information
    * @param[in] calibratorRoutes vector with the calibratorRoutes of calibrator
    * @param[in] calibratorFlows vector with the calibratorFlows of calibrator
    */
    GNECalibrator(const std::string& id, GNENet* net, GNELane* lane, double pos, SUMOTime frequency, const std::string& name, const std::string& output, const std::string& routeprobe);

    /// @brief Destructor
    ~GNECalibrator();

    /// @brief open Calibrator Dialog
    void openAdditionalDialog();

    /// @name Functions related with geometry of element
    /// @{
    /**@brief change the position of the element geometry without saving in undoList
     * @param[in] offset Position used for calculate new position of geometry without updating RTree
     */
    void moveGeometry(const Position& offset);

    /**@brief commit geometry changes in the attributes of an element after use of moveGeometry(...)
     * @param[in] undoList The undoList on which to register changes
     */
    void commitGeometryMoving(GNEUndoList* undoList);

    /// @brief update pre-computed geometry information
    void updateGeometry();

    /// @brief Returns position of additional in view
    Position getPositionInView() const;

    /// @brief Returns the boundary to which the view shall be centered in order to show the object
    Boundary getCenteringBoundary() const;

    /// @brief split geometry
    void splitEdgeGeometry(const double splitPosition, const GNENetworkElement* originalElement, const GNENetworkElement* newElement, GNEUndoList* undoList);
    /// @}

    /// @name inherited from GUIGlObject
    /// @{
    /**@brief Returns the name of the parent object
     * @return This object's parent id
     */
    std::string getParentName() const;

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

    /* @brief method for getting the Attribute of an XML key in double format (to avoid unnecessary parse<double>(...) for certain attributes)
     * @param[in] key The attribute key
     * @return double with the value associated to key
     */
    double getAttributeDouble(SumoXMLAttr key) const;

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

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    bool isAttributeEnabled(SumoXMLAttr key) const;

    /// @brief get PopPup ID (Used in AC Hierarchy)
    std::string getPopUpID() const;

    /// @brief get Hierarchy Name (Used in AC Hierarchy)
    std::string getHierarchyName() const;
    /// @}

protected:
    /// @brief position over Lane
    double myPositionOverLane;

    /// @brief Frequency of calibrator
    SUMOTime myFrequency;

    /// @brief output of calibrator
    std::string myOutput;

    /// @brief ID to current RouteProbe
    std::string myRouteProbe;

    /// @brief extra calibrator geometries
    std::vector<GNEGeometry::Geometry> myEdgeCalibratorGeometries;

private:
    /// @brief draw calibrator symbol
    void drawCalibratorSymbol(const GUIVisualizationSettings& s, const double exaggeration, const Position& pos, const double rot) const;

    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief Invalidated copy constructor.
    GNECalibrator(const GNECalibrator&) = delete;

    /// @brief Invalidated assignment operator.
    GNECalibrator& operator=(const GNECalibrator&) = delete;
};
