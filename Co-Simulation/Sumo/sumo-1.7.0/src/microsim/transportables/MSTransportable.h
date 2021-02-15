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
/// @file    MSTransportable.h
/// @author  Michael Behrisch
/// @date    Tue, 21 Apr 2015
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#pragma once
#include <config.h>

#include <set>
#include <cassert>
#include <utils/common/SUMOTime.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/geom/Position.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/Boundary.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/vehicle/SUMOTrafficObject.h>
#include "MSStage.h"


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;
class MSNet;
class MSStoppingPlace;
class MSVehicleType;
class OutputDevice;
class SUMOVehicleParameter;
class SUMOVehicle;
class MSTransportableDevice;

typedef std::vector<const MSEdge*> ConstMSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
  * @class MSTransportable
  *
  * The class holds a simulated moveable object
  */
class MSTransportable : public SUMOTrafficObject {
public:
    /// @name inherited from SUMOTrafficObject
    /// @{
    bool isPerson() const {
        return myAmPerson;
    }

    bool isContainer() const {
        return !myAmPerson;
    }

    bool isStopped() const {
        return getCurrentStageType() == MSStageType::WAITING;
    }

    double getSlope() const;

    double getChosenSpeedFactor() const {
        return 1.0;
    }

    SUMOVehicleClass getVClass() const;

    double getMaxSpeed() const;

    SUMOTime getWaitingTime() const;

    double getPreviousSpeed() const {
        return getSpeed();
    }

    double getAcceleration() const {
        return 0.0;
    }

    double getPositionOnLane() const {
        return getEdgePos();
    }

    double getBackPositionOnLane(const MSLane* /*lane*/) const {
        return getEdgePos();
    }

    Position getPosition(const double /*offset*/) const {
        return getPosition();
    }
    /// @}

    /// the structure holding the plan of a transportable
    typedef std::vector<MSStage*> MSTransportablePlan;

    /// constructor
    MSTransportable(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportablePlan* plan, const bool isPerson);

    /// destructor
    virtual ~MSTransportable();

    /* @brief proceeds to the next step of the route,
     * @return Whether the transportables plan continues  */
    virtual bool proceed(MSNet* net, SUMOTime time, const bool vehicleArrived = false);

    virtual void checkAccess(const MSStage* const prior, const bool isDisembark = true) {
        UNUSED_PARAMETER(prior);
        UNUSED_PARAMETER(isDisembark);
    }

    /// @brief set the id (inherited from Named but forbidden for transportables)
    void setID(const std::string& newID);

    inline const SUMOVehicleParameter& getParameter() const {
        return *myParameter;
    }

    inline const MSVehicleType& getVehicleType() const {
        return *myVType;
    }

    /// @brief returns the associated RNG
    std::mt19937* getRNG() const;

    /// Returns the desired departure time.
    SUMOTime getDesiredDepart() const;

    /// logs depart time of the current stage
    void setDeparted(SUMOTime now);

    /// Returns the current destination.
    const MSEdge* getDestination() const {
        return (*myStep)->getDestination();
    }

    /// Returns the destination after the current destination.
    const MSEdge* getNextDestination() const {
        return (*(myStep + 1))->getDestination();
    }

    /// @brief Returns the current edge
    const MSEdge* getEdge() const {
        return (*myStep)->getEdge();
    }

    /// @brief Returns the current lane (may be nullptr)
    const MSLane* getLane() const {
        return (*myStep)->getLane();
    }

    /// @brief Returns the departure edge
    const MSEdge* getFromEdge() const {
        return (*myStep)->getFromEdge();
    }

    /// @brief Return the position on the edge
    virtual double getEdgePos() const;

    /// @brief Return the Network coordinate of the transportable
    virtual Position getPosition() const;

    /// @brief return the current angle of the transportable
    virtual double getAngle() const;

    /// @brief the time this transportable spent waiting in seconds
    virtual double getWaitingSeconds() const;

    /// @brief the current speed of the transportable
    virtual double getSpeed() const;

    /// @brief the current speed factor of the transportable (where applicable)
    virtual double getSpeedFactor() const {
        return 1;
    }

    /// @brief the current stage type of the transportable
    MSStageType getCurrentStageType() const {
        return (*myStep)->getStageType();
    }

    /// @brief the stage type for the nth next stage
    MSStageType getStageType(int next) const {
        assert(myStep + next < myPlan->end());
        assert(myStep + next >= myPlan->begin());
        return (*(myStep + next))->getStageType();
    }

    /// @brief return textual summary for the given stage
    std::string getStageSummary(int stageIndex) const;

    /// Returns the current stage description as a string
    std::string getCurrentStageDescription() const {
        return (*myStep)->getStageDescription(myAmPerson);
    }

    /// @brief Return the current stage
    MSStage* getCurrentStage() const {
        return *myStep;
    }

    /// @brief Return the current stage
    MSStage* getNextStage(int next) const {
        assert(myStep + next >= myPlan->begin());
        assert(myStep + next < myPlan->end());
        return *(myStep + next);
    }

    /// @brief Return the edges of the nth next stage
    ConstMSEdgeVector getEdges(int next) const {
        assert(myStep + next < myPlan->end());
        assert(myStep + next >= myPlan->begin());
        return (*(myStep + next))->getEdges();
    }

    /// @brief Return the number of remaining stages (including the current)
    int getNumRemainingStages() const;

    /// @brief Return the total number stages in this persons plan
    int getNumStages() const;

    /** @brief Called on writing tripinfo output
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     */
    void tripInfoOutput(OutputDevice& os) const;

    /** @brief Called on writing vehroute output
     *
     * @param[in] os The stream to write the information into
     * @exception IOError not yet implemented
     */
    void routeOutput(OutputDevice& os, const bool withRouteLength) const;

    /// Whether the transportable waits for the given vehicle in the current step
    bool isWaitingFor(const SUMOVehicle* vehicle) const {
        return (*myStep)->isWaitingFor(vehicle);
    }

    /// @brief Whether the transportable waits for a vehicle
    bool isWaiting4Vehicle() const {
        return (*myStep)->isWaiting4Vehicle();
    }

    /// @brief The vehicle associated with this transportable
    SUMOVehicle* getVehicle() const {
        return (*myStep)->getVehicle();
    }

    /// @brief Appends the given stage to the current plan
    void appendStage(MSStage* stage, int next = -1);

    /// @brief removes the nth next stage
    void removeStage(int next);

    /// sets the walking speed (ignored in other stages)
    void setSpeed(double speed);

    /// @brief returns the final arrival pos
    double getArrivalPos() const {
        return myPlan->back()->getArrivalPos();
    }

    /// @brief returns the final arrival edge
    const MSEdge* getArrivalEdge() const {
        return myPlan->back()->getEdges().back();
    }

    /** @brief Replaces the current vehicle type by the one given
    *
    * If the currently used vehicle type is marked as being used by this vehicle
    *  only, it is deleted, first. The new, given type is then assigned to
    *  "myVType".
    * @param[in] type The new vehicle type
    * @see MSTransportable::myVType
    */
    void replaceVehicleType(MSVehicleType* type);


    /** @brief Replaces the current vehicle type with a new one used by this vehicle only
    *
    * If the currently used vehicle type is already marked as being used by this vehicle
    *  only, no new type is created.
    * @return The new modifiable vehicle type
    * @see MSTransportable::myVType
    */
    MSVehicleType& getSingularType();


    /// @brief return the bounding box of the person
    PositionVector getBoundingBox() const;

    /// @brief return whether the person has reached the end of its plan
    bool hasArrived() const;

    /// @brief return whether the transportable has started it's plan
    bool hasDeparted() const;

    /// @brief adapt plan when the vehicle reroutes and now stops at replacement instead of orig
    void rerouteParkingArea(MSStoppingPlace* orig, MSStoppingPlace* replacement);

    /// @brief Returns a device of the given type if it exists or 0
    MSTransportableDevice* getDevice(const std::type_info& type) const;

    /** @brief Returns this vehicle's devices
     * @return This vehicle's devices
     */
    inline const std::vector<MSTransportableDevice*>& getDevices() const {
        return myDevices;
    }

    /** @brief Saves the current state into the given stream
     */
    void saveState(OutputDevice& out);

    /** @brief Reconstructs the current state
     */
    void loadState(const std::string& state);

protected:
    /// the plan of the transportable
    const SUMOVehicleParameter* myParameter;

    /// @brief This transportable's type. (mainly used for drawing related information
    /// Note sure if it is really necessary
    MSVehicleType* myVType;

    /// @brief Whether events shall be written
    bool myWriteEvents;

    /// the plan of the transportable
    MSTransportablePlan* myPlan;

    /// the iterator over the route
    MSTransportablePlan::iterator myStep;

    /// @brief The devices this transportable has
    std::vector<MSTransportableDevice*> myDevices;

private:
    const bool myAmPerson;

private:
    /// @brief Invalidated copy constructor.
    MSTransportable(const MSTransportable&);

    /// @brief Invalidated assignment operator.
    MSTransportable& operator=(const MSTransportable&);

};
