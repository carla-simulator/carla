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
/// @file    MSEdgeControl.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christoph Sommer
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Mon, 09 Apr 2001
///
// Stores edges and lanes, performs moving of vehicle
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <queue>
#include <utils/common/SUMOTime.h>
#include <utils/common/Named.h>
#include <utils/router/SUMOAbstractRouter.h>
#include <utils/router/RouterProvider.h>
#include <utils/vehicle/SUMOVehicle.h>

#include <utils/foxtools/FXSynchQue.h>
#include <utils/foxtools/FXSynchSet.h>
#ifdef HAVE_FOX
#include <utils/foxtools/FXWorkerThread.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSLane;
class MSJunction;
class OutputDevice;

typedef std::vector<MSEdge*> MSEdgeVector;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSEdgeControl
 * @brief Stores edges and lanes, performs moving of vehicle
 *
 * In order to avoid touching all lanes, even the empty ones, this class stores
 *  and updates the information about "active" lanes, those that have at least
 *  one vehicle on them. During longitudinal movement, this can be simply
 *  achieved through return values of the MSLane-methods, signalling either
 *  that the lane got active or inactive. This is but not possible when
 *  changing lanes, we have to go through the lanes, here. Also, we have to
 *  add lanes on which a vehicle was inserted, separately, doing this into
 *  ("myChangedStateLanes") which entries are integrated at the begin of is step
 *  in "patchActiveLanes".
 */
class MSEdgeControl {

public:
    typedef RouterProvider<MSEdge, MSLane, MSJunction, SUMOVehicle> MSRouterProvider;

    /** @brief Constructor
     *
     * Builds LaneUsage information for each lane and assigns them to lanes.
     *
     * @param[in] edges The loaded edges
     * @todo Assure both containers are not 0
     */
    MSEdgeControl(const std::vector< MSEdge* >& edges);


    /// @brief Destructor.
    ~MSEdgeControl();


    /** @brief Resets information whether a lane is active for all lanes
     *
     * For each lane in "myChangedStateLanes": if the lane has at least one vehicle
     *  and is not marked as being active, it is added to the list og active lanes
     *  and marked as being active.
     */
    void patchActiveLanes();


    /// @name Interfaces for longitudinal vehicle movement
    /// @{

    /** @brief Compute safe velocities for all vehicles based on positions and
     * speeds from the last time step. Also registers
     * ApproachingVehicleInformation for all links
     *
     * This method goes through all active lanes calling their "planMovements" method.
     * @see MSLane::planMovements
     */
    void planMovements(SUMOTime t);

    /** @brief Register junction approaches for all vehicles after velocities
     * have been planned. This is a prerequisite for executeMovements
     *
     * This method goes through all active lanes calling their "setJunctionApproaches" method.
     */
    void setJunctionApproaches(SUMOTime t);


    /** @brief Executes planned vehicle movements with regards to right-of-way
     *
     * This method goes through all active lanes calling their executeMovements
     * method which causes vehicles to update their positions and speeds.
     * Lanes which receive new vehicles are stored in myWithVehicles2Integrate
     * After movements are executed the vehicles in myWithVehicles2Integrate are
     * put onto their new lanes
     * This method also updates the "active" status of lanes
     *
     * @see MSLane::executeMovements
     * @see MSLane::integrateNewVehicle
     */
    void executeMovements(SUMOTime t);

    void needsVehicleIntegration(MSLane* const l) {
        myWithVehicles2Integrate.push_back(l);
    }
    /// @}


    /** @brief Moves (precomputes) critical vehicles
     *
     * Calls "changeLanes" of each of the multi-lane edges. Check then for this
     *  edge whether a lane got active, adding it to "myActiveLanes" and marking
     *  it as active in such cases.
     *
     * @see MSEdge::changeLanes
     */
    void changeLanes(const SUMOTime t);


    /** @brief Detect collisions
     *
     * Calls "detectCollisions" of each lane.
     * Shouldn't be necessary if model-implementation is correct.
     * The parameter is simply passed to the lane-instance for reporting.
     *
     * @param[in] timestep The current time step
     * @param[in] stage The current stage within the simulation step
     * @note see MSNet::simulationStep
     */
    void detectCollisions(SUMOTime timestep, const std::string& stage);


    /** @brief Returns loaded edges
     *
     * @return the container storing one-lane edges
     * @todo Check: Is this secure?
     */
    const MSEdgeVector& getEdges() const {
        return myEdges;
    }


    /** @brief Informs the control that the given lane got active
     *
     * @param[in] l The activated lane
     * @todo Check for l==0?
     */
    void gotActive(MSLane* l);

    /// @brief trigger collision checking for inactive lane
    void checkCollisionForInactive(MSLane* l);

    /// @brief apply additional restrictions
    void setAdditionalRestrictions();

#ifdef HAVE_FOX
    FXWorkerThread::Pool& getThreadPool() {
        return myThreadPool;
    }
#endif

public:
    /**
     * @struct LaneUsage
     * @brief A structure holding some basic information about a simulated lane
     *
     * To fasten up speed, this structure holds the number of vehicles using
     *  a lane and the lane's neighbours. Only lanes that are occupied are
     *  forced to compute the vehicles longitunidal movement.
     *
     * The information about a lane's neighbours speed up the computation
     *  of the lane changing.
     */
    struct LaneUsage {
        /// @brief The described lane
        MSLane* lane;
        /// @brief Information whether this lane is active
        bool amActive;
        /// @brief Information whether this lane belongs to a multi-lane edge
        bool haveNeighbors;
    };

#ifdef HAVE_FOX
    /**
     * @class WorkerThread
     * @brief the thread which provides the router instance as context
     */
    class WorkerThread : public FXWorkerThread {
    public:
        WorkerThread(FXWorkerThread::Pool& pool)
            : FXWorkerThread(pool), myRouterProvider(nullptr) {}

        bool setRouterProvider(MSRouterProvider* routerProvider) {
            if (myRouterProvider == nullptr) {
                myRouterProvider = routerProvider;
                return true;
            }
            return false;
        }
        SUMOAbstractRouter<MSEdge, SUMOVehicle>& getRouter(SUMOVehicleClass svc) const {
            return myRouterProvider->getVehicleRouter(svc);
        }
        virtual ~WorkerThread() {
            stop();
            delete myRouterProvider;
        }
    private:
        MSRouterProvider* myRouterProvider;
    };
#endif

private:
    /// @brief Loaded edges
    MSEdgeVector myEdges;

    /// @brief Definition of a container about a lane's number of vehicles and neighbors
    typedef std::vector<LaneUsage> LaneUsageVector;

    /// @brief Information about lanes' number of vehicles and neighbors
    LaneUsageVector myLanes;

    /// @brief The list of active (not empty) lanes
    std::list<MSLane*> myActiveLanes;

    /// @brief A storage for lanes which shall be integrated because vehicles have moved onto them
    FXSynchQue<MSLane*, std::vector<MSLane*> > myWithVehicles2Integrate;

    /// @brief Lanes which changed the state without informing the control
    std::set<MSLane*, ComparatorNumericalIdLess> myChangedStateLanes;

    /// @brief The list of active (not empty) lanes
    std::vector<SUMOTime> myLastLaneChange;

    /// @brief Additional lanes for which collision checking must be performed
    FXSynchSet<MSLane*, std::set<MSLane*, ComparatorNumericalIdLess> > myInactiveCheckCollisions;

    double myMinLengthGeometryFactor;

#ifdef HAVE_FOX
    FXWorkerThread::Pool myThreadPool;
#endif

    std::priority_queue<std::pair<int, int> > myRNGLoad;

private:
    /// @brief Copy constructor.
    MSEdgeControl(const MSEdgeControl&);

    /// @brief Assignment operator.
    MSEdgeControl& operator=(const MSEdgeControl&);

};
