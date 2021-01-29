/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2003-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSVehicleTransfer.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Sep 2003
///
// A mover of vehicles that got stucked due to grid locks
// This class also serves as container for parking vehicles
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utils/foxtools/FXSynchQue.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;
class MSVehicle;
class MSVehicleControl;
class SUMOSAXAttributes;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSVehicleTransfer
 * This object (each simulation owns exactly one) is responsible for the
 *  transfer of vehicles that got stuck within the network due to grid locks.
 *  It also manages vehicles that are removed from the network because of stops
 *  with the parking attribute.
 *
 * The method add is called by a lane if a vehicle stood to long at this
 *  lane's end. After being added to this transfer object and removed from the
 *  lane, it is moved over the consecutive edges. On each edge, it is tried to
 *  insert the vehicle again. The lanes are of course chosen by examining the
 *  vehicle's real route.
 *
 * This object is used as a singleton
 */
class MSVehicleTransfer {
public:
    /// @brief Destructor
    virtual ~MSVehicleTransfer();


    /** @brief Adds a vehicle to this transfer object
     *
     * The vehicle is removed from the network as it would end the trip.
     * If the vehicle's next edge is his last one, the vehicle is also
     *  removed from the vehicle control.
     *
     * @param[in] veh The vehicle to add
     */
    void add(const SUMOTime t, MSVehicle* veh);


    /** @brief Remove a vehicle from this transfer object
     *
     * The vehicle is removed from the transfer if present.
     * This should be necessary only in the context of TraCI removals.
     *
     * @param[in] veh The vehicle to remove
     */
    void remove(MSVehicle* veh);


    /** @brief Checks "movement" of stored vehicles
     *
     * Checks whether one of the stored vehicles may be inserted back into
     *  the network. If not, the vehicle may move virtually to the next lane
     *  of it's route
     *
     * @param[in] time The current simulation time
     */
    void checkInsertions(SUMOTime time);


    /** @brief Saves the current state into the given stream */
    void saveState(OutputDevice& out);

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

    /** @brief Loads one transfer vehicle state from the given descriptionn */
    void loadState(const SUMOSAXAttributes& attrs, const SUMOTime offset, MSVehicleControl& vc);

    /** @brief Returns the instance of this object
     * @return The singleton instance
     */
    static MSVehicleTransfer* getInstance();

    /// @brief The minimum speed while teleporting
    static const double TeleportMinSpeed;

private:
    /// @brief Constructor
    MSVehicleTransfer();


protected:
    /**
     * @struct VehicleInformation
     * @brief Holds the information needed to move the vehicle over the network
     */
    struct VehicleInformation {
        /// @brief the time at which this vehicle was removed from the network
        SUMOTime myTransferTime;
        /// @brief The vehicle itself
        MSVehicle* myVeh;
        /// @brief The time at which the vehicle should be moved virtually one edge further
        SUMOTime myProceedTime;
        /// @brief whether the vehicle is or was parking
        bool myParking;

        /** @brief Constructor
         * @param[in] veh The teleported vehicle
         * @param[in] insertTime The time the vehicle was inserted at
         * @param[in] proceedTime The time at which the vehicle should be moved virtually one edge further
         */
        VehicleInformation(SUMOTime t, MSVehicle* veh, SUMOTime proceedTime, bool parking)
            : myTransferTime(t), myVeh(veh), myProceedTime(proceedTime), myParking(parking) { }

        /// @brief sort by vehicle ID for repeatable parallel simulation
        bool operator<(const VehicleInformation& v2) const;
    };


    /// @brief The information about stored vehicles to move virtually
    FXSynchQue<VehicleInformation, std::vector<VehicleInformation> > myVehicles;

    /// @brief The static singleton-instance
    static MSVehicleTransfer* myInstance;

};
