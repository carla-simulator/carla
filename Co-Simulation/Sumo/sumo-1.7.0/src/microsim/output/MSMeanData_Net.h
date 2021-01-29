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
/// @file    MSMeanData_Net.h
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Mon, 10.05.2004
///
// Network state mean data collector for edges/lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <limits>
#include "MSMeanData.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class MSEdgeControl;
class MSEdge;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMeanData_Net
 * @brief Network state mean data collector for edges/lanes
 *
 * This structure does not contain the data itself, it is stored within
 *  MSLaneMeanDataValues-MoveReminder objects.
 * This class is used to build the output, optionally, in the case
 *  of edge-based dump, aggregated over the edge's lanes.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSMeanData_Net : public MSMeanData {
public:
    /**
     * @class MSLaneMeanDataValues
     * @brief Data structure for mean (aggregated) edge/lane values
     *
     * Structure holding values that describe the flow and other physical
     *  properties aggregated over some seconds.
     */
    class MSLaneMeanDataValues : public MSMeanData::MeanDataValues {
    public:
        /** @brief Constructor
         * @param[in] length The length of the object for which the data gets collected
         */
        MSLaneMeanDataValues(MSLane* const lane, const double length, const bool doAdd,
                             const MSMeanData_Net* parent);

        /** @brief Destructor */
        virtual ~MSLaneMeanDataValues();

        /** @brief Resets values so they may be used for the next interval
         */
        void reset(bool afterWrite = false);

        /** @brief Add the values of this to the given one and store them there
         *
         * @param[in] val The meandata to add to
         */
        void addTo(MSMeanData::MeanDataValues& val) const;

        /// @name Methods inherited from MSMoveReminder
        /// @{

        /** @brief Called if the vehicle leaves the reminder's lane
         *
         * @param veh The leaving vehicle.
         * @param[in] lastPos Position on the lane when leaving.
         * @param[in] isArrival whether the vehicle arrived at its destination
         * @param[in] isLaneChange whether the vehicle changed from the lane
         * @see MSMoveReminder
         * @see MSMoveReminder::notifyLeave
         */
        bool notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


        /** @brief Computes current values and adds them to their sums
         *
         * The fraction of time the vehicle is on the lane is computed and
         *  used as a weight for the vehicle's current values.
         *  The "emitted" field is incremented, additionally.
         *
         * @param[in] veh The vehicle that enters the lane
         * @param[in] veh The entering vehicle.
         * @param[in] reason how the vehicle enters the lane
         * @return Always true
         * @see MSMoveReminder::notifyEnter
         * @see MSMoveReminder::Notification
         */
        bool notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
        //@}

        bool isEmpty() const;

        /** @brief Writes output values into the given stream
         *
         * @param[in] dev The output device to write the data into
         * @param[in] period Length of the period the data were gathered
         * @param[in] numLanes The total number of lanes for which the data was collected
         * @exception IOError If an error on writing occurs (!!! not yet implemented)
         */
        void write(OutputDevice& dev, long long int attributeMask, const SUMOTime period,
                   const double numLanes, const double defaultTravelTime,
                   const int numVehicles = -1) const;

    protected:
        /** @brief Internal notification about the vehicle moves
         *  @see MSMoveReminder::notifyMoveInternal
         */
        void notifyMoveInternal(const SUMOTrafficObject& veh,
                                const double frontOnLane, const double timeOnLane, const double,
                                const double meanSpeedVehicleOnLane,
                                const double travelledDistanceFrontOnLane,
                                const double travelledDistanceVehicleOnLane,
                                const double meanLengthOnLane);

    private:
//        /// @brief Calculate the vehicle front's distance to myLane's end for a vehicle that called notifyMoveInternal()
//        ///        maxDist gives the maximal distance to search back from the vehicle's current lane to myLane
//        ///        returns INVALID_DOUBLE if myLane wasn't found in that range
//        double getVehicleDistToMyLane(const SUMOVehicle& veh, double maxDist);

    public:
        /// @name Collected values
        /// @{
        /// @brief The number of vehicles that were emitted on the lane
        int nVehDeparted;

        /// @brief The number of vehicles that finished on the lane
        int nVehArrived;

        /// @brief The number of vehicles that entered this lane within the sample interval
        int nVehEntered;

        /// @brief The number of vehicles that left this lane within the sample interval
        int nVehLeft;

        /// @brief The number of vehicles that left this lane within the sample interval
        int nVehVaporized;

        /// @brief The number of vehicle probes with small speed
        double waitSeconds;

    private:
        /// @brief The number of vehicles that changed from this lane
        int nVehLaneChangeFrom;

        /// @brief The number of vehicles that changed to this lane
        int nVehLaneChangeTo;

        /// @brief The number of vehicle probes regarding the vehicle front
        double frontSampleSeconds;

        /// @brief The travelled distance regarding the vehicle front
        double frontTravelledDistance;

        /// @brief The sum of the lengths the vehicles had
        double vehLengthSum;

        /// @brief The sum of the occupation of the lane
        double occupationSum;

        /// @brief minimal vehicle length in the current interval (used to determine a maximal density, see #3265)
        double minimalVehicleLength;

        //@}

        /// @brief The meandata parent
        const MSMeanData_Net* myParent;

    };


public:
    /** @brief Constructor
     *
     * @param[in] id The id of the detector
     * @param[in] dumpBegin Begin time of dump
     * @param[in] dumpEnd End time of dump
     * @param[in] useLanes Information whether lane-based or edge-based dump shall be generated
     * @param[in] withEmpty Information whether empty lanes/edges shall be written
     * @param[in] printDefaults Information whether defaults for empty lanes/edges shall be written
     * @param[in] withInternal Information whether internal lanes/edges shall be written
     * @param[in] trackVehicles Information whether vehicles shall be tracked
     * @param[in] detectPersons Whether pedestrians shall be detected instead of vehicles
     * @param[in] maxTravelTime the maximum travel time to output
     * @param[in] minSamples the minimum number of sample seconds before the values are valid
     * @param[in] haltSpeed the maximum speed to consider a vehicle waiting
     * @param[in] vTypes the set of vehicle types to consider
     */
    MSMeanData_Net(const std::string& id,
                   const SUMOTime dumpBegin, const SUMOTime dumpEnd,
                   const bool useLanes, const bool withEmpty, const bool printDefaults,
                   const bool withInternal, const bool trackVehicles, const int detectPersons,
                   const double maxTravelTime, const double minSamples,
                   const double haltSpeed, const std::string& vTypes,
                   const std::string& writeAttributes);


    /// @brief Destructor
    virtual ~MSMeanData_Net();

protected:
    /** @brief Create an instance of MeanDataValues
     *
     * @param[in] lane The lane to create for
     * @param[in] doAdd whether to add the values as reminder to the lane
     */
    MSMeanData::MeanDataValues* createValues(MSLane* const lane, const double length, const bool doAdd) const;

    /** @brief Resets network value in order to allow processing of the next interval
     *
     * Goes through the lists of edges and starts "resetOnly" for each edge.
     * @param [in] edge The last time step that is reported
     */
    void resetOnly(SUMOTime stopTime);

private:
    /// @brief the minimum sample seconds
    const double myHaltSpeed;

    /// @brief Invalidated copy constructor.
    MSMeanData_Net(const MSMeanData_Net&);

    /// @brief Invalidated assignment operator.
    MSMeanData_Net& operator=(const MSMeanData_Net&);

};
