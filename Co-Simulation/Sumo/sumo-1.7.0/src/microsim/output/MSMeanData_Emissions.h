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
/// @file    MSMeanData_Emissions.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Emission data collector for edges/lanes
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <set>
#include <limits>
#include <utils/emissions/PollutantsInterface.h>
#include "MSMeanData.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice;
class MSLane;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSMeanData_Emissions
 * @brief Emission data collector for edges/lanes
 *
 * This structure does not contain the data itself, it is stored within
 *  MSLaneMeanDataValues-MoveReminder objects.
 *
 * This class is used to build the output, optionally, in the case
 *  of edge-based dump, aggregated over the edge's lanes.
 *
 * @todo consider error-handling on write (using IOError)
 */
class MSMeanData_Emissions : public MSMeanData {
public:
    /**
     * @class MSLaneMeanDataValues
     * @brief Data structure for mean (aggregated) edge/lane values
     *
     * Structure holding values that describe the emissions aggregated
     *  over some seconds.
     */
    class MSLaneMeanDataValues : public MSMeanData::MeanDataValues {
    public:
        /** @brief Constructor */
        MSLaneMeanDataValues(MSLane* const lane, const double length, const bool doAdd,
                             const MSMeanData_Emissions* parent);

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


        /** @brief Writes output values into the given stream
         *
         * @param[in] dev The output device to write the data into
         * @param[in] prefix The xml prefix to write (mostly the lane / edge id)
         * @param[in] numLanes The total number of lanes for which the data was collected
         * @param[in] length The length of the object for which the data was collected
         * @exception IOError If an error on writing occurs (!!! not yet implemented)
         */
        void write(OutputDevice& dev, long long int attributeMask, const SUMOTime period,
                   const double numLanes, const double defaultTravelTime,
                   const int numVehicles = -1) const;


    protected:
        /** @brief Internal notification about the vehicle moves
         *  @see MSMoveReminder::notifyMoveInternal()
         */
        void notifyMoveInternal(const SUMOTrafficObject& veh, const double /* frontOnLane */, const double timeOnLane, const double /*meanSpeedFrontOnLane*/, const double meanSpeedVehicleOnLane, const double travelledDistanceFrontOnLane, const double travelledDistanceVehicleOnLane, const double /* meanLengthOnLane */);

        /** @brief Computes idling emission values and adds them to the aggregate emission sums
        *
        * Idling implied by zero velocity, acceleration and slope
        *
        * @param[in] veh The vehicle
        *
        * @see MSMoveReminder::notifyMove
        * @see PollutantsInterface
        */
        bool notifyIdle(SUMOTrafficObject& veh);


    private:
        /// @brief Collected values
        PollutantsInterface::Emissions myEmissions;
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
     * @param[in] maxTravelTime the maximum travel time to use when calculating per vehicle output
     * @param[in] minSamples the minimum number of sample seconds before the values are valid
     * @param[in] vTypes the set of vehicle types to consider
     */
    MSMeanData_Emissions(const std::string& id,
                         const SUMOTime dumpBegin, const SUMOTime dumpEnd,
                         const bool useLanes, const bool withEmpty,
                         const bool printDefaults, const bool withInternal,
                         const bool trackVehicles,
                         const double minSamples, const double maxTravelTime,
                         const std::string& vTypes,
                         const std::string& writeAttributes);


    /// @brief Destructor
    virtual ~MSMeanData_Emissions();



protected:
    /** @brief Create an instance of MeanDataValues
     *
     * @param[in] lane The lane to create for
     * @param[in] doAdd whether to add the values as reminder to the lane
     */
    MSMeanData::MeanDataValues* createValues(MSLane* const lane, const double length, const bool doAdd) const;



private:
    /// @brief Invalidated copy constructor.
    MSMeanData_Emissions(const MSMeanData_Emissions&);

    /// @brief Invalidated assignment operator.
    MSMeanData_Emissions& operator=(const MSMeanData_Emissions&);

};
