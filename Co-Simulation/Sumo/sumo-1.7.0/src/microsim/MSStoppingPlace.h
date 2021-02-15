/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSStoppingPlace.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 13.12.2005
///
// A lane area vehicles can halt at
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLane;
class MSEdge;
class SUMOVehicle;
class MSTransportable;
class Position;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSStoppingPlace
 * @brief A lane area vehicles can halt at
 *
 * The stop tracks the last free space a vehicle may halt at by being
 *  informed about a vehicle's entering and depart. It keeps the information
 *  about entered vehicles' begin and end position within an internal
 *  container ("myEndPositions") and is so able to compute the last free space.
 *
 * Please note that using the last free space disallows vehicles to enter a
 *  free space in between other vehicles.
 */
class MSStoppingPlace : public Named, public Parameterised {
public:
    /** @brief Constructor
     *
     * @param[in] id The id of the stop
     * @param[in] net The net the stop belongs to
     * @param[in] lines Names of the lines that halt on this stop
     * @param[in] lane The lane the stop is placed on
     * @param[in] begPos Begin position of the stop on the lane
     * @param[in] endPos End position of the stop on the lane
     */
    MSStoppingPlace(const std::string& id,
                    const std::vector<std::string>& lines, MSLane& lane,
                    double begPos, double endPos, const std::string name = "",
                    int capacity = 0,
                    double parkingLength = 0);



    /// @brief Destructor
    virtual ~MSStoppingPlace();


    /** @brief Returns the lane this stop is located at
     *
     * @return Reference to the lane the stop is located at
     */
    const MSLane& getLane() const;


    /** @brief Returns the begin position of this stop
     *
     * @return The position the stop begins at
     */
    double getBeginLanePosition() const;


    /** @brief Returns the end position of this stop
     *
     * @return The position the stop ends at
     */
    double getEndLanePosition() const;


    /** @brief Called if a vehicle enters this stop
     *
     * Stores the position of the entering vehicle in myEndPositions.
     *
     * Recomputes the free space using "computeLastFreePos" then.
     *
     * @param[in] what The vehicle that enters the bus stop
     * @param[in] beg The begin halting position of the vehicle
     * @param[in] what The end halting position of the vehicle
     * @see computeLastFreePos
     */
    void enter(SUMOVehicle* veh, bool parking);


    /** @brief Called if a vehicle leaves this stop
     *
     * Removes the position of the vehicle from myEndPositions.
     *
     * Recomputes the free space using "computeLastFreePos" then.
     *
     * @param[in] what The vehicle that leaves the bus stop
     * @see computeLastFreePos
     */
    void leaveFrom(SUMOVehicle* what);


    /** @brief Returns the last free position on this stop
     *
     * @return The last free position of this bus stop
     */
    double getLastFreePos(const SUMOVehicle& forVehicle) const;

    /// @brief return whether the given vehicle fits at the given position
    bool fits(double pos, const SUMOVehicle& veh) const;

    /** @brief Returns the next free waiting place for pedestrians / containers
     *
     * @return The next free waiting place for pedestrians / containers
     */
    virtual Position getWaitPosition(MSTransportable* person) const;

    /** @brief Returns the lane position corresponding to getWaitPosition()
     *
     * @return The waiting position along the stop lane
     */
    double getWaitingPositionOnLane(MSTransportable* t) const;


    /** @brief For vehicles at the stop this gives the the actual stopping
     *         position of the vehicle. For all others the last free stopping position
     *
     */
    double getStoppingPosition(const SUMOVehicle* veh) const;

    /** @brief Returns the number of transportables waiting on this stop
    */
    int getTransportableNumber() const {
        return (int)myWaitingTransportables.size();
    }

    /** @brief Returns the tranportables waiting on this stop
     */
    std::vector<MSTransportable*> getTransportables() const;

    /** @brief Returns the number of stopped vehicles waiting on this stop
    */
    int getStoppedVehicleNumber() const {
        return (int)myEndPositions.size();
    }

    double getLastFreePos() const {
        return myLastFreePos;
    }

    /// @brief whether there is still capacity for more transportables
    bool hasSpaceForTransportable() const;

    /// @brief adds a transportable to this stop
    bool addTransportable(MSTransportable* p);

    /// @brief Removes a transportable from this stop
    void removeTransportable(MSTransportable* p);

    /// @brief adds an access point to this stop
    virtual bool addAccess(MSLane* lane, const double pos, const double length);

    /// @brief lanes and positions connected to this stop
    const std::vector<std::tuple<MSLane*, double, double> >& getAllAccessPos() const {
        return myAccessPos;
    }

    /// @brief the position on the given edge which is connected to this stop, -1 on failure
    double getAccessPos(const MSEdge* edge) const;

    /// @brief the distance from the access on the given edge to the stop, -1 on failure
    double getAccessDistance(const MSEdge* edge) const;

    const std::string& getMyName() const;

    static int getPersonsAbreast(double length);

    /// @brief get list of vehicles waiting at this stop
    std::vector<const SUMOVehicle*> getStoppedVehicles() const;

    /// @brief get number of persons waiting at this stop
    inline int getNumWaitingPersons() const {
        return (int)myWaitingTransportables.size();
    }

    /// @brief get IDs of persons waiting at this stop
    void getWaitingPersonIDs(std::vector<std::string>& into) const;

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

protected:
    /** @brief Computes the last free position on this stop
     *
     * The last free position is the one, the last vehicle ends at.
     * It is stored in myLastFreePos. If no vehicle halts, the last free
     *  position gets the value of myEndPos.
     */
    void computeLastFreePos();

    int getPersonsAbreast() const;

protected:
    /// @brief The list of lines that are assigned to this stop
    std::vector<std::string> myLines;

    /// @brief A map from objects (vehicles) to the areas they acquire after entering the stop
    std::map<const SUMOVehicle*, std::pair<double, double>, ComparatorNumericalIdLess> myEndPositions;

    /// @brief The lane this bus stop is located at
    const MSLane& myLane;

    /// @brief The begin position this bus stop is located at
    const double myBegPos;

    /// @brief The end position this bus stop is located at
    const double myEndPos;

    /// @brief The last free position at this stop (variable)
    double myLastFreePos;

    /// @brief The name of the stopping place
    const std::string myName;

    /// @brief The number of transportables that can wait here
    const int myTransportableCapacity;

    /// @brief the scaled space capacity for parking vehicles
    const double myParkingFactor;

protected:

    /// @brief Persons waiting at this stop (mapped to waiting position)
    std::map<MSTransportable*, int> myWaitingTransportables;
    std::set<int> myWaitingSpots;

    /// @brief lanes and positions connected to this stop
    std::vector<std::tuple<MSLane*, double, double> > myAccessPos;

private:
    /// @brief Invalidated copy constructor.
    MSStoppingPlace(const MSStoppingPlace&);

    /// @brief Invalidated assignment operator.
    MSStoppingPlace& operator=(const MSStoppingPlace&);


};
