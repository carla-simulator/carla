/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_BTreceiver.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    14.08.2013
///
// A BT receiver
/****************************************************************************/
#pragma once
#include <config.h>

#include <random>
#include "MSVehicleDevice.h"
#include "MSDevice_BTsender.h"
#include <utils/common/SUMOTime.h>
#include <utils/common/Command.h>
#include <utils/common/RandHelper.h>


// ===========================================================================
// class declarations
// ===========================================================================
class SUMOVehicle;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSDevice_BTreceiver
 * @brief A BT receiver
 *
 * @see MSDevice
 */
class MSDevice_BTreceiver : public MSVehicleDevice {
public:
    /** @brief Inserts MSDevice_BTreceiver-options
     * @param[filled] oc The options container to add the options to
     */
    static void insertOptions(OptionsCont& oc);


    /** @brief Build devices for the given vehicle, if needed
     *
     * The options are read and evaluated whether a bt-receiver-device shall be built
     *  for the given vehicle.
     *
     * The built device is stored in the given vector.
     *
     * @param[in] v The vehicle for which a device may be built
     * @param[filled] into The vector to store the built device in
     */
    static void buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into);


    /** @brief Returns the configured range
     * @return the device range
     */
    static double getRange() {
        return myRange;
    }


    static std::mt19937* getRNG() {
        return &sRecognitionRNG;
    }

public:
    /// @brief Destructor.
    ~MSDevice_BTreceiver();



    /// @name Methods inherited from MSMoveReminder.
    /// @{

    /** @brief Adds the vehicle to running vehicles if it (re-) enters the network
     *
     * @param[in] veh The entering vehicle.
     * @param[in] reason how the vehicle enters the lane
     * @return Always true
     * @see MSMoveReminder::notifyEnter
     * @see MSMoveReminder::Notification
     */
    bool notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* enteredLane = 0);


    /** @brief Checks whether the reminder still has to be notified about the vehicle moves
     *
     * Indicator if the reminders is still active for the passed
     * vehicle/parameters. If false, the vehicle will erase this reminder
     * from it's reminder-container.
     *
     * @param[in] veh Vehicle that asks this reminder.
     * @param[in] oldPos Position before move.
     * @param[in] newPos Position after move with newSpeed.
     * @param[in] newSpeed Moving speed.
     *
     * @return True if vehicle hasn't passed the reminder completely.
     */
    bool notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed);


    /** @brief Moves (the known) vehicle from running to arrived vehicles' list
     *
     * @param[in] veh The leaving vehicle.
     * @param[in] lastPos Position on the lane when leaving.
     * @param[in] isArrival whether the vehicle arrived at its destination
     * @param[in] isLaneChange whether the vehicle changed from the lane
     * @see MSMoveReminder
     * @see MSMoveReminder::notifyLeave
     */
    bool notifyLeave(SUMOTrafficObject& veh, double lastPos, Notification reason, const MSLane* enteredLane = 0);
    ///@}

    /// @brief return the name for this type of device
    const std::string deviceName() const {
        return "btreceiver";
    }


    /** @class MeetingPoint
     * @brief Holds the information about exact positions/speeds/time of the begin/end of a meeting
     */
    class MeetingPoint {
    public:
        /** @brief Constructor
         * @param[in] _t The time of the meeting
         * @param[in] _observerState The position, speed, lane etc. the observer had at the time
         * @param[in] _seenState The position, speed, lane etc. the seen vehicle had at the time
         */
        MeetingPoint(double _t, const MSDevice_BTsender::VehicleState& _observerState,
                     const MSDevice_BTsender::VehicleState& _seenState)
            : t(_t), observerState(_observerState), seenState(_seenState) {}

        /// @brief Destructor
        ~MeetingPoint() {}

    public:
        /// @brief The time of the meeting
        const double t;
        /// @brief The state the observer had at the time
        const MSDevice_BTsender::VehicleState observerState;
        /// @brief The state the seen vehicle had at the time
        const MSDevice_BTsender::VehicleState seenState;

    private:
        /// @brief Invalidated assignment operator.
        MeetingPoint& operator=(const MeetingPoint&) = delete;

    };



    /** @class SeenDevice
     * @brief Class representing a single seen device
     */
    class SeenDevice {
    public:
        /** @brief Constructor
         * @param[in] meetingBegin_ Description of the meeting's begin
         */
        SeenDevice(const MeetingPoint& meetingBegin_)
            : meetingBegin(meetingBegin_), meetingEnd(0), lastView(meetingBegin_.t), nextView(-1.) {}

        /// @brief Destructor
        ~SeenDevice() {
            delete meetingEnd;
            for (std::vector<MeetingPoint*>::iterator i = recognitionPoints.begin(); i != recognitionPoints.end(); ++i) {
                delete *i;
            }
            recognitionPoints.clear();
        }


    public:
        /// @brief Description of the meeting's begin
        const MeetingPoint meetingBegin;
        /// @brief Description of the meeting's end
        MeetingPoint* meetingEnd;
        /// @brief Last recognition point
        double lastView;
        /// @brief Next possible recognition point
        double nextView;
        /// @brief List of recognition points
        std::vector<MeetingPoint*> recognitionPoints;
        /// @brief string of travelled receiver edges
        std::string receiverRoute;
        /// @brief string of travelled sender edges
        std::string senderRoute;

    private:
        /// @brief Invalidated assignment operator.
        SeenDevice& operator=(const SeenDevice&);

    };



    /** @brief Clears the given containers deleting the stored items
     * @param[in] c The currently seen container to clear
     * @param[in] s The seen container to clear
     */
    static void cleanUp(std::map<std::string, SeenDevice*>& c, std::map<std::string, std::vector<SeenDevice*> >& s);



protected:
    /** @brief Constructor
     *
     * @param[in] holder The vehicle that holds this device
     * @param[in] id The ID of the device
     */
    MSDevice_BTreceiver(SUMOVehicle& holder, const std::string& id);



private:
    /// @brief Whether the bt-system was already initialised
    static bool myWasInitialised;

    /// @brief The range of the device
    static double myRange;

    /// @brief The offtime of the device
    static double myOffTime;


    /** @class VehicleInformation
     * @brief Stores the information of a vehicle
     */
    class VehicleInformation : public MSDevice_BTsender::VehicleInformation {
    public:
        /** @brief Constructor
         * @param[in] id The id of the vehicle
         * @param[in] range Recognition range of the vehicle
         */
        VehicleInformation(const std::string& id, const double _range) : MSDevice_BTsender::VehicleInformation(id), range(_range) {}

        /// @brief Destructor
        ~VehicleInformation() {
            std::map<std::string, SeenDevice*>::iterator i;
            for (i = currentlySeen.begin(); i != currentlySeen.end(); i++) {
                delete i->second;
            }
            std::map<std::string, std::vector<SeenDevice*> >::iterator j;
            std::vector<SeenDevice*>::iterator k;
            for (j = seen.begin(); j != seen.end(); j++) {
                for (k = j->second.begin(); k != j->second.end(); k++) {
                    delete *k;
                }
            }
        }

        /// @brief Recognition range of the vehicle
        const double range;

        /// @brief The map of devices seen by the vehicle at removal time
        std::map<std::string, SeenDevice*> currentlySeen;

        /// @brief The past episodes of removed vehicle
        std::map<std::string, std::vector<SeenDevice*> > seen;

    private:
        /// @brief Invalidated copy constructor.
        VehicleInformation(const VehicleInformation&);

        /// @brief Invalidated assignment operator.
        VehicleInformation& operator=(const VehicleInformation&);

    };



    /** @class BTreceiverUpdate
     * @brief A global update performer
     */
    class BTreceiverUpdate : public Command {
    public:
        /// @brief Constructor
        BTreceiverUpdate();

        /// @brief Destructor
        ~BTreceiverUpdate();

        /** @brief Performs the update
         * @param[in] currentTime The current simulation time
         * @return Always DELTA_T - the time to being called back
         */
        SUMOTime execute(SUMOTime currentTime);


        /** @brief Rechecks the visibility for a given receiver/sender pair
         * @param[in] receiver Definition of the receiver vehicle
         * @param[in] sender Definition of the sender vehicle
         */
        void updateVisibility(VehicleInformation& receiver, MSDevice_BTsender::VehicleInformation& sender);


        /** @brief Informs the receiver about a sender entering it's radius
         * @param[in] atOffset The time offset to the current time step
         * @param[in] receiverState The position, speed, lane etc. the observer had at the time
         * @param[in] senderID The ID of the entering sender
         * @param[in] senderState The position, speed, lane etc. the seen vehicle had at the time
         * @param[in] currentlySeen The container storing episodes
         */
        void enterRange(double atOffset, const MSDevice_BTsender::VehicleState& receiverState,
                        const std::string& senderID, const MSDevice_BTsender::VehicleState& senderState,
                        std::map<std::string, SeenDevice*>& currentlySeen);


        /** @brief Removes the sender from the currently seen devices to past episodes
         * @param[in] receiverInfo The static information of the observer (id, route, etc.)
         * @param[in] receiverState The position, speed, lane etc. the observer had at the time
         * @param[in] senderInfo The static information of the seen vehicle (id, route, etc.)
         * @param[in] senderState The position, speed, lane etc. the seen vehicle had at the time
         * @param[in] tOffset The time offset to the current time step
         */
        void leaveRange(VehicleInformation& receiverInfo, const MSDevice_BTsender::VehicleState& receiverState,
                        MSDevice_BTsender::VehicleInformation& senderInfo, const MSDevice_BTsender::VehicleState& senderState,
                        double tOffset);




        /** @brief Adds a point of recognition
         * @param[in] tEnd The time of the recognition
         * @param[in] receiverState The position, speed, lane etc. the observer had at the time
         * @param[in] senderState The position, speed, lane etc. the seen vehicle had at the time
         * @param[in] senderDevice The device of the entering sender
         */
        void addRecognitionPoint(const double tEnd, const MSDevice_BTsender::VehicleState& receiverState,
                                 const MSDevice_BTsender::VehicleState& senderState,
                                 SeenDevice* senderDevice) const;


        /** @brief Writes the output
         * @param[in] id The id of the receiver
         * @param[in] seen The information about seen senders
         * @param[in] allRecognitions Whether all recognitions shall be written
         */
        void writeOutput(const std::string& id, const std::map<std::string, std::vector<SeenDevice*> >& seen,
                         bool allRecognitions);




    };


    static double inquiryDelaySlots(const int backoffLimit);

    /// @brief A random number generator used to determine whether the opposite was recognized
    static std::mt19937 sRecognitionRNG;

    /// @brief The list of arrived receivers
    static std::map<std::string, VehicleInformation*> sVehicles;



private:
    /// @brief Invalidated copy constructor.
    MSDevice_BTreceiver(const MSDevice_BTreceiver&);

    /// @brief Invalidated assignment operator.
    MSDevice_BTreceiver& operator=(const MSDevice_BTreceiver&);


};
