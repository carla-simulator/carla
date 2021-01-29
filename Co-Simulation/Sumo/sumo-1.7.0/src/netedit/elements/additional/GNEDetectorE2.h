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
/// @file    GNEDetectorE2.h
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2015
///
//
/****************************************************************************/
#pragma once
#include "GNEDetector.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GNEDetectorE2
 * class for detector of type E2
 */
class GNEDetectorE2 : public GNEDetector {

public:
    /**@brief Constructor for Single-Lane E2 detectors
     * @param[in] id The storage of gl-ids to get the one for this lane representation from
     * @param[in] lane Lane of this StoppingPlace belongs
     * @param[in] net pointer to GNENet of this additional element belongs
     * @param[in] pos position of the detector on the lane
     * @param[in] length The length of the detector in meters.
     * @param[in] freq the aggregation period the values the detector collects shall be summed up.
     * @param[in] trafficLight The traffic light that triggers aggregation when switching.
     * @param[in] filename The path to the output file.
     * @param[in] vehicleTypes space separated list of vehicle type ids to consider
     * @param[in] name E2 detector name
     * @param[in] timeThreshold The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting
     * @param[in] speedThreshold The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting
     * @param[in] speedThreshold The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam
     * @param[in] friendlyPos enable or disable friendly positions
     * @param[in] block movement enable or disable additional movement
     */
    GNEDetectorE2(const std::string& id, GNELane* lane, GNENet* net, double pos, double length, const std::string& freq, const std::string& trafficLight, const std::string& filename,
                  const std::string& vehicleTypes, const std::string& name, SUMOTime timeThreshold, double speedThreshold, double jamThreshold, bool friendlyPos, bool blockMovement);

    /**@brief Constructor for Multi-Lane detectors
     * @param[in] id The storage of gl-ids to get the one for this lane representation from
     * @param[in] lanes vector of lanes Lane of this StoppingPlace belongs
     * @param[in] net pointer to GNENet of this additional element belongs
     * @param[in] pos position of the detector on the first lane
     * @param[in] endPos position of the detector on the last lane
     * @param[in] freq the aggregation period the values the detector collects shall be summed up.
     * @param[in] trafficLight The traffic light that triggers aggregation when switching.
     * @param[in] filename The path to the output file.
     * @param[in] vehicleTypes space separated list of vehicle type ids to consider
     * @param[in] name E2 detector name
     * @param[in] timeThreshold The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting
     * @param[in] speedThreshold The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting
     * @param[in] speedThreshold The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam
     * @param[in] friendlyPos enable or disable friendly positions
     * @param[in] block movement enable or disable additional movement
     */
    GNEDetectorE2(const std::string& id, std::vector<GNELane*> lanes, GNENet* net, double pos, double endPos, const std::string& freq, const std::string& trafficLight, const std::string& filename,
                  const std::string& vehicleTypes, const std::string& name, SUMOTime timeThreshold, double speedThreshold, double jamThreshold, bool friendlyPos, bool blockMovement);

    /// @brief Destructor
    ~GNEDetectorE2();

    /// @name members and functions relative to write additionals into XML
    /// @{
    /// @brief check if current additional is valid to be writed into XML
    bool isAdditionalValid() const;

    /// @brief return a string with the current additional problem
    std::string getAdditionalProblem() const;

    /// @brief fix additional problem
    void fixAdditionalProblem();
    /// @}

    /// @brief get length of E2 Detector
    double getLength() const;

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

    /* @brief method for check if the value for certain attribute is set
     * @param[in] key The attribute key
     */
    bool isAttributeEnabled(SumoXMLAttr key) const;
    /// @}

protected:
    /// @brief E2 detector length
    double myLength;

    /// @brief end position over lane (only for Multilane E2 detectors)
    double myEndPositionOverLane;

    /// @brief The time-based threshold that describes how much time has to pass until a vehicle is recognized as halting
    SUMOTime myTimeThreshold;

    /// @brief The speed-based threshold that describes how slow a vehicle has to be to be recognized as halting
    double mySpeedThreshold;

    /// @brief The minimum distance to the next standing vehicle in order to make this vehicle count as a participant to the jam
    double myJamThreshold;

    /// @brief Traffic light vinculated with this E2 Detector
    std::string myTrafficLight;

private:
    /// @brief set attribute after validation
    void setAttribute(SumoXMLAttr key, const std::string& value);

    /// @brief check if lanes are consecutives
    bool areLaneConsecutives() const;

    /// @brief Invalidated copy constructor.
    GNEDetectorE2(const GNEDetectorE2&) = delete;

    /// @brief Invalidated assignment operator.
    GNEDetectorE2& operator=(const GNEDetectorE2&) = delete;
};
