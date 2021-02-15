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
/// @file    RORouteHandler.h
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 9 Jul 2001
///
// Parser and container for routes during their loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <vector>
#include <utils/distribution/RandomDistributor.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/NamedRTree.h>
#include <utils/router/PedestrianRouter.h>
#include <utils/vehicle/SUMORouteHandler.h>
#include "ROPerson.h"


// ===========================================================================
// class declarations
// ===========================================================================
class OutputDevice_String;
class ROEdge;
class ROLane;
class RONet;
class RORoute;
class RORouteDef;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class RORouteHandler
 * @brief Parser and container for routes during their loading
 *
 * RORouteHandler is the container for routes while they are build until
 * their transfering to the MSNet::RouteDict
 * The result of the operations are single MSNet::Route-instances
 */
class RORouteHandler : public SUMORouteHandler {
public:
    /// @brief standard constructor
    RORouteHandler(RONet& net, const std::string& file,
                   const bool tryRepair,
                   const bool emptyDestinationsAllowed,
                   const bool ignoreErrors,
                   const bool checkSchema);

    /// @brief standard destructor
    virtual ~RORouteHandler();

protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    virtual void myStartElement(int element,
                                const SUMOSAXAttributes& attrs);
    //@}

    /** @brief Called for parsing from and to and the corresponding taz attributes
     *
     * @param[in] element description of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     */
    void parseFromViaTo(SumoXMLTag tag, const SUMOSAXAttributes& attrs, bool& ok);

    /// @brief opens a type distribution for reading
    void openVehicleTypeDistribution(const SUMOSAXAttributes& attrs);

    /// @brief closes (ends) the building of a distribution
    void closeVehicleTypeDistribution();

    /// @brief opens a route for reading
    void openRoute(const SUMOSAXAttributes& attrs);

    /// @brief opens a flow for reading
    void openFlow(const SUMOSAXAttributes& attrs);

    /// @brief opens a route flow for reading
    void openRouteFlow(const SUMOSAXAttributes& attrs);

    /// @brief opens a trip for reading
    void openTrip(const SUMOSAXAttributes& attrs);

    /**@brief closes (ends) the building of a route.
     * @note Afterwards no edges may be added to it;
     *       this method may throw exceptions when
     *       a) the route is empty or
     *       b) another route with the same id already exists
     */
    void closeRoute(const bool mayBeDisconnected = false);

    /// @brief opens a route distribution for reading
    void openRouteDistribution(const SUMOSAXAttributes& attrs);

    /// @brief closes (ends) the building of a distribution
    void closeRouteDistribution();

    /// @brief Ends the processing of a vehicle
    void closeVehicle();

    /// @brief Ends the processing of a vehicle type
    void closeVType();

    /// @brief Ends the processing of a person
    void closePerson();

    /// @brief Ends the processing of a personFlow
    void closePersonFlow();

    /// @brief Ends the processing of a container
    void closeContainer();

    /// @brief Ends the processing of a flow
    void closeFlow();

    /// @brief Ends the processing of a trip
    void closeTrip();

    /// @brief Processing of a stop
    void addStop(const SUMOSAXAttributes& attrs);

    /// @brief Processing of a person
    void addPerson(const SUMOSAXAttributes& attrs);

    /// @brief Processing of a person from a personFlow
    void addFlowPerson(SUMOTime depart, const std::string& baseID, int i);

    /// @brief Processing of a container
    void addContainer(const SUMOSAXAttributes& attrs);

    /// @brief Processing of a ride
    void addRide(const SUMOSAXAttributes& attrs);

    /// @brief Processing of a transport
    void addTransport(const SUMOSAXAttributes& attrs);

    /// @brief Processing of a tranship
    void addTranship(const SUMOSAXAttributes& attrs);

    /// @brief Parse edges from strings
    void parseEdges(const std::string& desc, ConstROEdgeVector& into,
                    const std::string& rid, bool& ok);

    /// @brief Parse edges from coordinates
    void parseGeoEdges(const PositionVector& positions, bool geo,
                       ConstROEdgeVector& into, const std::string& rid, bool isFrom, bool& ok);

    /// @brief find closest edge within distance for the given position or nullptr
    const ROEdge* getClosestEdge(const Position& pos, double distance, SUMOVehicleClass vClass);

    /// @brief find closest junction taz given the closest edge
    const ROEdge* getJunctionTaz(const Position& pos, const ROEdge* closestEdge, SUMOVehicleClass vClass, bool isFrom);

    /// @brief add a routing request for a walking or intermodal person
    void addPersonTrip(const SUMOSAXAttributes& attrs);

    /// @brief add a fully specified walk
    void addWalk(const SUMOSAXAttributes& attrs);

    ///@ brief parse depart- and arrival positions of a walk
    void parseWalkPositions(const SUMOSAXAttributes& attrs, const std::string& personID,
                            const ROEdge* fromEdge, const ROEdge*& toEdge,
                            double& departPos, double& arrivalPos, std::string& busStopID,
                            const ROPerson::PlanItem* const lastStage, bool& ok);

    /// @brief initialize lane-RTree
    NamedRTree* getLaneTree();

protected:
    /// @brief The current route
    RONet& myNet;

    /// @brief The current route
    ConstROEdgeVector myActiveRoute;

    /// @brief number of repetitions of the active route
    int myActiveRouteRepeat;
    SUMOTime myActiveRoutePeriod;

    /// @brief The plan of the current person
    ROPerson* myActivePerson;

    /// @brief The plan of the current container
    OutputDevice_String* myActiveContainerPlan;

    /// @brief The number of stages in myActiveContainerPlan
    int myActiveContainerPlanSize;

    /// @brief Information whether routes shall be repaired
    const bool myTryRepair;

    /// @brief Information whether the "to" attribute is mandatory
    const bool myEmptyDestinationsAllowed;

    /// @brief Depending on the "ignore-errors" option different outputs are used
    MsgHandler* const myErrorOutput;

    /// @brief The begin time
    const SUMOTime myBegin;

    /// @brief whether to keep the the vtype distribution in output
    const bool myKeepVTypeDist;

    /// @brief maximum distance when map-matching
    const double myMapMatchingDistance;
    const bool myMapMatchJunctions;

    /// @brief The currently parsed distribution of vehicle types (probability->vehicle type)
    RandomDistributor<SUMOVTypeParameter*>* myCurrentVTypeDistribution;

    /// @brief The id of the currently parsed vehicle type distribution
    std::string myCurrentVTypeDistributionID;

    /// @brief The currently parsed route alternatives
    RORouteDef* myCurrentAlternatives;

    /// @brief RTree for finding lanes
    NamedRTree* myLaneTree;

private:
    /// @brief Invalidated copy constructor
    RORouteHandler(const RORouteHandler& s) = delete;

    /// @brief Invalidated assignment operator
    RORouteHandler& operator=(const RORouteHandler& s) = delete;
};
