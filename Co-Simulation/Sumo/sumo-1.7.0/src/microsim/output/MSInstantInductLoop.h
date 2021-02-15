/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSInstantInductLoop.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    2011-09.08
///
// An instantaneous induction loop
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
class SUMOTrafficObject;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSInstantInductLoop
 * @brief An instantaneous induction loop
 *
 * @see MSMoveReminder
 * @see MSDetectorFileOutput
 */
class MSInstantInductLoop
    : public MSMoveReminder, public MSDetectorFileOutput {
public:
    /**
     * @brief Constructor.
     *
     * @param[in] id Unique id
     * @param[in] od The device to write to
     * @param[in] lane Lane where detector woks on.
     * @param[in] position Position of the detector within the lane.
     */
    MSInstantInductLoop(const std::string& id, OutputDevice& od,
                        MSLane* const lane, double positionInMeters,
                        const std::string& vTypes);


    /// @brief Destructor
    ~MSInstantInductLoop();



    /// @name Methods inherited from MSMoveReminder
    /// @{

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
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyLeave
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* enteredLane = 0);
    //@}



    /** @brief Write the generated output to the given device
     *
     * This method is not used - output is written as soon as a vehicle visits the detector.
     *
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     * @exception IOError If an error on writing occurs
     */
    void writeXMLOutput(OutputDevice& dev,
                        SUMOTime startTime, SUMOTime stopTime) {
        UNUSED_PARAMETER(dev);
        UNUSED_PARAMETER(startTime);
        UNUSED_PARAMETER(stopTime);
    }


    /** @brief Open the XML-output
     *
     * The implementing function should open an xml element using
     *  OutputDevice::writeXMLHeader.
     *
     * @param[in] dev The output device to write the root into
     * @exception IOError If an error on writing occurs
     */
    void writeXMLDetectorProlog(OutputDevice& dev) const;


protected:
    /** @brief Writes an event line
     * @param[in] state The current state to report
     * @param[in] t The event time
     * @param[in] veh The vehicle responsible for the event
     * @param[in] speed The speed of the vehicle
     * @param[in] add An optional attribute to report
     * @param[in] addValue The value of the optional attribute
     */
    void write(const char* state, double t, SUMOTrafficObject& veh, double speed, const char* add = 0, double addValue = -1);


protected:
    /// @brief The output device to use
    OutputDevice& myOutputDevice;

    /// @brief Detector's position on lane [m]
    const double myPosition;

    /// @brief The last exit time
    double myLastExitTime;

    /// @brief The last exit time
    std::map<SUMOTrafficObject*, double> myEntryTimes;

private:
    /// @brief Invalidated copy constructor.
    MSInstantInductLoop(const MSInstantInductLoop&);

    /// @brief Invalidated assignment operator.
    MSInstantInductLoop& operator=(const MSInstantInductLoop&);


};
