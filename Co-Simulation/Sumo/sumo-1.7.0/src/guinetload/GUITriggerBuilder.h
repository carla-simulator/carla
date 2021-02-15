/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    GUITriggerBuilder.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 26.04.2004
///
// Builds trigger objects for guisim
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <netload/NLTriggerBuilder.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSTrigger;
class MSNet;
class MSLaneSpeedTrigger;
class MSCalibrator;
class MSTriggerControl;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUITriggerBuilder
 * @brief Builds trigger objects for guisim
 */
class GUITriggerBuilder : public NLTriggerBuilder {
public:
    /// @brief Constructor
    GUITriggerBuilder();


    /// @brief Destructor
    ~GUITriggerBuilder();


protected:
    /// @name building methods
    ///
    /// Override NLTriggerBuilder-methods for building guisim-objects
    /// @see NLTriggerBuilder
    //@{

    /** @brief Builds a lane speed trigger
     *
     * Simply calls the GUILaneSpeedTrigger constructor.
     *
     * @param[in] net The net the lane speed trigger belongs to
     * @param[in] id The id of the lane speed trigger
     * @param[in] destLanes List of lanes affected by this speed trigger
     * @param[in] file Name of the file to read the speeds to set from
     * @see MSLaneSpeedTrigger
     * @exception ProcessError If the XML definition file is errornous
     */
    virtual MSLaneSpeedTrigger* buildLaneSpeedTrigger(MSNet& net,
            const std::string& id, const std::vector<MSLane*>& destLanes,
            const std::string& file);


    /** @brief builds an rerouter
     *
     * Simply calls the GUITriggeredRerouter constructor.
     *
     * @param[in] net The net the rerouter belongs to
     * @param[in] id The id of the rerouter
     * @param[in] edges The edges the rerouter is placed at
     * @param[in] prob The probability the rerouter reoutes vehicles with
     * @param[in] file The file to read the reroute definitions from
     */
    virtual MSTriggeredRerouter* buildRerouter(MSNet& net,
            const std::string& id, MSEdgeVector& edges,
            double prob, const std::string& file, bool off,
            SUMOTime timeThreshold,
            const std::string& vTypes);


    /** @brief Builds a bus stop
     *
     * Simply calls the GUIBusStop / GUIContainerStop constructor.
     *
     * @param[in] net The net the stop belongs to
     * @param[in] id The id of the stop
     * @param[in] lines Names of the lines that halt on this stop
     * @param[in] lane The lane the stop is placed on
     * @param[in] frompos Begin position of the stop on the lane
     * @param[in] topos End position of the stop on the lane
     * @param[in] element which kind of stop is to be built
     * @exception InvalidArgument If the stop can not be added to the net (is duplicate)
     */
    virtual void buildStoppingPlace(MSNet& net, std::string id, std::vector<std::string> lines, MSLane* lane,
                                    double frompos, double topos, const SumoXMLTag element, std::string string,
                                    int personCapacity, double parkingLength);


    /** @brief Builds a parking area
     *
     * Simply calls the GUIParkingArea constructor.
     *
     * @param[in] net The net the parking area belongs to
     * @param[in] id The id of the parking area
     * @param[in] lines Names of the lines that halt on this parking area
     * @param[in] lane The lane the parking area is placed on
     * @param[in] frompos Begin position of the parking area on the lane
     * @param[in] topos End position of the parking area on the lane
     * @param[in] capacity Capacity of the parking area
     * @param[in] width Width of the default lot rectangle
     * @param[in] length Length of the default lot rectangle
     * @param[in] angle Angle of the default lot rectangle
     * @exception InvalidArgument If the parking area can not be added to the net (is duplicate)
     */
    virtual void beginParkingArea(MSNet& net, const std::string& id,
                                  const std::vector<std::string>& lines, MSLane* lane,
                                  double frompos, double topos,
                                  unsigned int capacity,
                                  double width, double length, double angle, const std::string& name,
                                  bool onRoad);


    /** @brief Builds a charging station
     *
     * Simply calls the GUIChargingStation constructor.
     *
     * @param[in] net The net the charging station belongs to
     * @param[in] id The id of the charging station
     * @param[in] lane The lane the charging station is placed on
     * @param[in] frompos Begin position of the charging station on the lane
     * @param[in] topos End position of the charging station on the lane
     * @param[in] chargingPower energy charged in every timeStep
     * @param[in] efficiency efficiency of the charge
     * @param[in] chargeInTransit enable or disable charge in transit
     * @param[in] chargeDelay delay in the charge
     * @exception InvalidArgument If the charging station can not be added to the net (is duplicate)
     */
    virtual void buildChargingStation(MSNet& net, const std::string& id, MSLane* lane,
                                      double frompos, double topos, const std::string& name,
                                      double chargingPower, double efficiency,
                                      bool chargeInTransit, double chargeDelay);

    /** @brief Builds an overhead wire segment
    *
    * Simply calls the GUIOverheadWire constructor and adds the result to the network.
    *
    * @param[in] net The net the overhead wire segment belongs to
    * @param[in] id The id of the overhead wire segment
    * @param[in] lane The lane the overhead wire segment is placed on
    * @param[in] frompos Begin position of the overhead wire segment on the lane
    * @param[in] topos End position of the overhead wire segment  on the lane
    * @param[in] voltageSource If the segment is the place of the connection of a traction substation
    * @exception InvalidArgument If the overhead wire segment can not be added to the net (is duplicate according to the id)
    */
    virtual void buildOverheadWireSegment(MSNet& net, const std::string& id, MSLane* lane,
                                          double frompos, double topos, bool voltageSource);

    /** @brief Builds an overhead wire clamp
    *
    * Simply calls the GUIOverheadWireClamp constructor and adds the result to the network.
    *
    * @param[in] net The net the overhead wire clamp belongs to
    * @param[in] id The id of the overhead wire clamp
    * @param[in] lane_start The lane, where is the overhead wire segment placed, to the start of which the overhead wire clamp is connected
    * @param[in] lane_end The lane, where is the overhead wire segment placed, to the end of which the overhead wire clamp is connected
    */
    virtual void buildOverheadWireClamp(MSNet& net, const std::string& id, MSLane* lane_start, MSLane* lane_end);
    /// @}


    /** @brief End a parking area
     * (it must be added to the SUMORTree after all parking spaces are loaded
     */
    virtual void endParkingArea();

    /** @brief End a stopping place
     *
     * @exception InvalidArgument If the current stopping place is 0
     */
    virtual void endStoppingPlace();
};
