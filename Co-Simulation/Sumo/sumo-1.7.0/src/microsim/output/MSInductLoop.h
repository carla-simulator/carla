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
/// @file    MSInductLoop.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    2004-11-23
///
// An unextended detector measuring at a fixed position on a fixed lane.
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <deque>
#include <map>
#include <functional>
#include <microsim/MSMoveReminder.h>
#include <microsim/output/MSDetectorFileOutput.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSVehicle;
class OutputDevice;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSInductLoop
 * @brief An unextended detector measuring at a fixed position on a fixed lane.
 *
 * Only vehicles that passed the entire detector are counted. We
 *  ignore vehicles that are emitted onto the detector and vehicles
 *  that change their lane while they are on the detector, because we
 *  cannot determine a meaningful enter/leave-times.
 *
 * This detector uses the MSMoveReminder mechanism, i.e. the vehicles
 *  call the detector if they pass it.
 *
 * @see MSMoveReminder
 * @see MSDetectorFileOutput
 */
class MSInductLoop
    : public MSMoveReminder, public MSDetectorFileOutput {
public:
    /**
     * @brief Constructor.
     *
     * Adds reminder to MSLane.
     *
     * @param[in] id Unique id
     * @param[in] lane Lane where detector works on
     * @param[in] position Position of the detector within the lane
     * @param[in] vTypes which vehicle types are considered
     * @param[in] needLocking whether internals need to be guarded against concurrent access (GUI)
     */
    MSInductLoop(const std::string& id, MSLane* const lane,
                 double positionInMeters,
                 const std::string& vTypes,
                 const bool needLocking);


    /// @brief Destructor
    ~MSInductLoop();


    /** @brief Resets all generated values to allow computation of next interval
     */
    virtual void reset();


    /** @brief Returns the position of the detector on the lane
     * @return The detector's position in meters
     */
    double getPosition() const {
        return myPosition;
    }


    /// @name Methods inherited from MSMoveReminder
    /// @{
    /** @brief Checks whether the reminder is activated by a vehicle entering the lane
     *
     * Lane change means in this case that the vehicle changes to the lane
     *  the reminder is placed at.
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return True if vehicle enters the induction loop
     * @see Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* enteredLane = 0);

    /** @brief Checks whether the vehicle shall be counted and/or shall still touch this MSMoveReminder
     *
     * As soon a vehicle enters the detector, its entry time is computed and stored
     *  in myVehiclesOnDet via enterDetectorByMove. If it passes the detector, the
     *  according leaving time is computed and stored, too, using leaveDetectorByMove.
     *
     * @param[in] veh Vehicle that asks this remider.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     * @return True if vehicle hasn't passed the detector completely.
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyMove
     * @see enterDetectorByMove
     * @see leaveDetectorByMove
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed);


    /** @brief Dismisses the vehicle if it is on the detector due to a lane change
     *
     * If the vehicle is on the detector, it will be dismissed by incrementing
     *  myDismissedVehicleNumber and removing this vehicle's entering time from
     *  myVehiclesOnDet.
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @see discardVehicle
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyLeave
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);


    //@}



    /// @name Methods returning current values
    /// @{

    /** @brief Returns the speed of the vehicle on the detector
     *
     * If no vehicle is on the detector, -1 is returned, otherwise
     *  this vehicle's current speed.
     *
     * @return The speed [m/s] of the vehicle if one is on the detector, -1 otherwise
     */
    double getSpeed(const int offset) const;


    /** @brief Returns the length of the vehicle on the detector
     *
     * If no vehicle is on the detector, -1 is returned, otherwise
     *  this vehicle's length.
     *
     * @return The length [m] of the vehicle if one is on the detector, -1 otherwise
     */
    double getVehicleLength(const int offset) const;


    /** @brief Returns the current occupancy
     *
     * If a vehicle is on the detector, 1 is returned. If a vehicle has passed the detector
     *  in this timestep, its occupancy value is returned. If no vehicle has passed,
     *  0 is returned.
     *
     * @return This detector's current occupancy
     * @todo recheck (especially if more than one vehicle has passed)
     */
    double getOccupancy() const;


    /** @brief Returns the number of vehicles that have passed the detector
     *
     * If a vehicle is on the detector, 1 is returned. If a vehicle has passed the detector
     *  in this timestep, 1 is returned. If no vehicle has passed,
     *  0 is returned.
     *
     * @return The number of vehicles that have passed the detector
     * @todo recheck (especially if more than one vehicle has passed)
     */
    double getEnteredNumber(const int offset) const;


    /** @brief Returns the ids of vehicles that have passed the detector
     *
     * @return The ids of vehicles that have passed the detector
     * @todo recheck (especially if more than one vehicle has passed)
     */
    std::vector<std::string> getVehicleIDs(const int offset) const;


    /** @brief Returns the time since the last vehicle left the detector
     *
     * @return seconds from last leaving (detection) of the detector
     */
    double getTimeSinceLastDetection() const;

    ///@brief return last time a vehicle was on the detector
    SUMOTime getLastDetectionTime() const;
    //@}



    /// @name Methods inherited from MSDetectorFileOutput.
    /// @{

    /** @brief Writes collected values into the given stream
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @see MSDetectorFileOutput::writeXMLOutput
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime);


    /** @brief Opens the XML-output using "detector" as root element
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     * @exception IOError If an error on writing occurs (!!! not yet implemented)
     */
    void writeXMLDetectorProlog(OutputDevice& dev) const;
    /// @}


    /** @brief Struct to store the data of the counted vehicle internally.
     *
     * These data is fed into a container.
     *
     * @see myVehicleDataCont
     */
    struct VehicleData {
        /** @brief Constructor
         *
         * Used if the vehicle has left the induction loop completely
         *
         * @param[in] vehLength The length of the vehicle
         * @param[in] entryTimestep The time at which the vehicle entered the detector
         * @param[in] leaveTimestep The time at which the vehicle left the detector
         */
        VehicleData(const SUMOTrafficObject& v, double entryTimestep,
                    double leaveTimestep, const bool leftEarly);

        /// @brief The id of the vehicle
        std::string idM;
        /// @brief Length of the vehicle
        double lengthM;
        /// @brief Entry-time of the vehicle in [s]
        double entryTimeM;
        /// @brief Leave-time of the vehicle in [s]
        double leaveTimeM;
        /// @brief Speed of the vehicle in [m/s]
        double speedM;
        /// @brief Type of the vehicle
        std::string typeIDM;
        /// @brief whether the vehicle left the detector with a lane change / teleport etc.
        bool leftEarlyM;
    };


    /** @brief Returns vehicle data for vehicles that have been on the detector starting at the given time
     *
     * @param[in] t The time from which vehicles shall be counted
     * @param[in] leaveTime Whether entryTime or leaveTime shall be compared against t
     *            (the latter gives a more complete picture but may include vehicles in multiple steps even if they did not stay on the detector)
     * @return The list of vehicles
     */
    std::vector<VehicleData> collectVehiclesOnDet(SUMOTime t, bool includeEarly = false, bool leaveTime = false, bool forOccupancy = false) const;

    /// @brief allows for special color in the gui version
    virtual void setSpecialColor(const RGBColor* /*color*/) {};

    virtual void setVisible(bool /*show*/) {};

    /** @brief Remove all vehicles before quick-loading state */
    virtual void clearState();

protected:
    /// @name Function for summing up values
    ///@{

    /// @brief Adds up VehicleData::speedM
    static inline double speedSum(double sumSoFar, const MSInductLoop::VehicleData& data) {
        return sumSoFar + data.speedM;
    }

    /// @brief Adds up VehicleData::lengthM
    static inline double lengthSum(double sumSoFar, const MSInductLoop::VehicleData& data) {
        return sumSoFar + data.lengthM;
    }
    ///@}


protected:
    /// @brief Detector's position on lane [m]
    const double myPosition;

    /// @brief whether internals need to be guarded against concurrent access (GUI or multi threading)
    const bool myNeedLock;

    /// @brief Leave-time of the last vehicle detected [s]
    double myLastLeaveTime;

    /// @brief The number of entered vehicles
    int myEnteredVehicleNumber;

    /// @brief Type of myVehicleDataCont.
    typedef std::deque< VehicleData > VehicleDataCont;

    /// @brief Data of vehicles that have completely passed the detector
    VehicleDataCont myVehicleDataCont;

    /// @brief Data of vehicles that have completely passed the detector in the last time interval
    VehicleDataCont myLastVehicleDataCont;

    /// @brief Data for vehicles that have entered the detector (vehicle -> enter time)
    std::map<SUMOTrafficObject*, double> myVehiclesOnDet;

private:
    /// @brief Invalidated copy constructor.
    MSInductLoop(const MSInductLoop&);

    /// @brief Invalidated assignment operator.
    MSInductLoop& operator=(const MSInductLoop&);


};
