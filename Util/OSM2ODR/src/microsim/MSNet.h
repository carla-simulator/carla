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
/// @file    MSNet.h
/// @author  Christian Roessel
/// @author  Jakob Erdmann
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Eric Nicolay
/// @author  Clemens Honomichl
/// @author  Michael Behrisch
/// @author  Leonhard Luecken
/// @date    Mon, 12 Mar 2001
///
// The simulated network and simulation performer
/****************************************************************************/
#pragma once
#include <config.h>

#include <typeinfo>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <memory>
#include <utils/common/SUMOTime.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/NamedObjectCont.h>
#include <utils/common/NamedRTree.h>
#include <utils/router/SUMOAbstractRouter.h>
#include "MSJunction.h"

#ifdef HAVE_FOX
#include <utils/foxtools/FXConditionalLock.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSEdgeControl;
class MSEventControl;
class MSVehicleControl;
class MSJunctionControl;
class MSInsertionControl;
class SUMORouteLoaderControl;
class MSTransportableControl;
class MSVehicle;
class MSRoute;
class MSLane;
class MSTLLogicControl;
class MSTrafficLightLogic;
class MSDetectorControl;
class ShapeContainer;
class MSDynamicShapeUpdater;
class PolygonDynamics;
class MSEdgeWeightsStorage;
class SUMOVehicle;
class MSTractionSubstation;
class MSStoppingPlace;
template<class E, class L, class N, class V>
class IntermodalRouter;
template<class E, class L, class N, class V>
class PedestrianRouter;
class OptionsCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSNet
 * @brief The simulated network and simulation perfomer
 */
class MSNet {
public:
    /** @enum SimulationState
     * @brief Possible states of a simulation - running or stopped with different reasons
     */
    enum SimulationState {
        /// @brief The simulation is loading
        SIMSTATE_LOADING,
        /// @brief The simulation is running
        SIMSTATE_RUNNING,
        /// @brief The final simulation step has been performed
        SIMSTATE_END_STEP_REACHED,
        /// @brief The simulation does not contain further vehicles
        SIMSTATE_NO_FURTHER_VEHICLES,
        /// @brief The connection to a client was closed by the client
        SIMSTATE_CONNECTION_CLOSED,
        /// @brief An error occurred during the simulation step
        SIMSTATE_ERROR_IN_SIM,
        /// @brief An external interrupt occured
        SIMSTATE_INTERRUPTED,
        /// @brief The simulation had too many teleports
        SIMSTATE_TOO_MANY_TELEPORTS
    };

    typedef PedestrianRouter<MSEdge, MSLane, MSJunction, MSVehicle> MSPedestrianRouter;
    typedef IntermodalRouter<MSEdge, MSLane, MSJunction, SUMOVehicle> MSIntermodalRouter;


public:
    /** @brief Returns the pointer to the unique instance of MSNet (singleton).
     * @return Pointer to the unique MSNet-instance
     * @exception ProcessError If a network was not yet constructed
     */
    static MSNet* getInstance();

    /**
     * @brief Returns whether this is a GUI Net
     */
    virtual bool isGUINet() const {
        return false;
    }

    /// @brief Place for static initializations of simulation components (called after successful net build)
    static void initStatic();

    /// @brief Place for static initializations of simulation components (called after successful net build)
    static void cleanupStatic();


    /** @brief Returns whether the network was already constructed
    * @return whether the network was already constructed
    */
    static bool hasInstance() {
        return myInstance != nullptr;
    }


    /** @brief Constructor
     *
     * This constructor builds a net of which only some basic structures are initialised.
     * It prepares the network for being filled while loading.
     * As soon as all edge/junction/traffic lights and other containers are build, they
     *  must be initialised using "closeBuilding".
     * @param[in] vc The vehicle control to use
     * @param[in] beginOfTimestepEvents The event control to use for simulation step begin events
     * @param[in] endOfTimestepEvents The event control to use for simulation step end events
     * @param[in] insertionEvents The event control to use for insertion events
     * @param[in] shapeCont The shape container to use
     * @exception ProcessError If a network was already constructed
     * @see closeBuilding
     */
    MSNet(MSVehicleControl* vc, MSEventControl* beginOfTimestepEvents,
          MSEventControl* endOfTimestepEvents,
          MSEventControl* insertionEvents,
          ShapeContainer* shapeCont = 0);


    /// @brief Destructor
    virtual ~MSNet();


    /** @brief Closes the network's building process
     *
     * Assigns the structures built while loading to this network.
     * @param[in] oc The options to use
     * @param[in] edges The control of edges which belong to this network
     * @param[in] junctions The control of junctions which belong to this network
     * @param[in] routeLoaders The route loaders used
     * @param[in] tlc The control of traffic lights which belong to this network
     * @param[in] stateDumpTimes List of time steps at which state shall be written
     * @param[in] stateDumpFiles Filenames for states
     * @param[in] hasInternalLinks Whether the network actually contains internal links
     * @param[in] version The network version
     * @todo Try to move all this to the constructor?
     */
    void closeBuilding(const OptionsCont& oc, MSEdgeControl* edges, MSJunctionControl* junctions,
                       SUMORouteLoaderControl* routeLoaders, MSTLLogicControl* tlc,
                       std::vector<SUMOTime> stateDumpTimes, std::vector<std::string> stateDumpFiles,
                       bool hasInternalLinks, bool hasNeighs,
                       double version);


    /** @brief Returns whether the network has specific vehicle class permissions
     * @return whether permissions are present
     */
    bool hasPermissions() const {
        return myHavePermissions;
    }


    /// @brief Labels the network to contain vehicle class permissions
    void setPermissionsFound() {
        myHavePermissions = true;
    }


    /** @brief Adds a restriction for an edge type
     * @param[in] id The id of the type
     * @param[in] svc The vehicle class the restriction refers to
     * @param[in] speed The restricted speed
     */
    void addRestriction(const std::string& id, const SUMOVehicleClass svc, const double speed);


    /** @brief Returns the restrictions for an edge type
     * If no restrictions are present, 0 is returned.
     * @param[in] id The id of the type
     * @return The mapping of vehicle classes to maximum speeds
     */
    const std::map<SUMOVehicleClass, double>* getRestrictions(const std::string& id) const;


    /** @brief Clears all dictionaries
     * @todo Try to move all this to the destructor
     */
    static void clearAll();


    /** @brief Simulates from timestep start to stop
     * @param[in] start The begin time step of the simulation
     * @param[in] stop The end time step of the simulation
     * @return Returns always 0
     * @todo Recheck return value
     * @todo What exceptions may occure?
     */
    SimulationState simulate(SUMOTime start, SUMOTime stop);


    /** @brief Performs a single simulation step
     * @todo Which exceptions may occur?
     */
    void simulationStep();


    /** @brief loads routes for the next few steps */
    void loadRoutes();


    /** @brief Writes performance output and running vehicle stats
     *
     * @param[in] start The step the simulation was started with
     */
    const std::string generateStatistics(SUMOTime start);

    /// @brief write statistic output to (xml) file
    void writeStatistics() const;

    /** @brief Closes the simulation (all files, connections, etc.)
     *
     * Writes also performance output
     *
     * @param[in] start The step the simulation was started with
     */
    void closeSimulation(SUMOTime start, const std::string& reason = "");


    /** @brief This method returns the current simulation state. It should not modify status.
     * @param[in] stopTime The time the simulation shall stop at
     * @return The current simulation state
     * @see SimulationState
     */
    SimulationState simulationState(SUMOTime stopTime) const;


    /** @brief Called after a simulation step, this method adapts the current simulation state if necessary
     * @param[in] state The current simulation state
     * @return The new simulation state
     * @see SimulationState
     */
    SimulationState adaptToState(const SimulationState state) const;


    /** @brief Returns the message to show if a certain state occurs
     * @return Readable description of the state
     */
    static std::string getStateMessage(SimulationState state);


    /** @brief Returns the current simulation step
     * @return the current simulation step
     */
    inline SUMOTime getCurrentTimeStep() const {
        return myStep;
    }


    /** @brief Sets the current simulation step (used by state loading)
     * @param step the current simulation step
     */
    inline void setCurrentTimeStep(const SUMOTime step) {
        myStep = step;
    }


    /** @brief Resets events when quick-loading state
     * @param step The new simulation step
     */
    void clearState(const SUMOTime step);

    /** @brief Write netstate, summary and detector output
     * @todo Which exceptions may occur?
     */
    void writeOutput();


    /** @brief Returns whether duration shall be logged
     * @return Whether duration shall be logged
     */
    bool logSimulationDuration() const;



    /// @name Output during the simulation
    //@{

    /** @brief Prints the current step number
     *
     * Called on the begin of a simulation step
     */
    void preSimStepOutput() const;


    /** @brief Prints the statistics of the step at its end
     *
     * Called on the end of a simulation step
     */
    void postSimStepOutput() const;
    //}



    /// @name Retrieval of references to substructures
    /// @{

    /** @brief Returns the vehicle control
     * @return The vehicle control
     * @see MSVehicleControl
     * @see myVehicleControl
     */
    MSVehicleControl& getVehicleControl() {
        return *myVehicleControl;
    }


    /** @brief Returns the person control
     *
     * If the person control does not exist, yet, it is created.
     *
     * @return The person control
     * @see MSPersonControl
     * @see myPersonControl
     */
    virtual MSTransportableControl& getPersonControl();

    /** @brief Returns whether persons are simulated
     */
    bool hasPersons() const {
        return myPersonControl != nullptr;
    }

    /** @brief Returns the container control
     *
     * If the container control does not exist, yet, it is created.
     *
     * @return The container control
     * @see MSContainerControl
     * @see myContainerControl
     */
    virtual MSTransportableControl& getContainerControl();

    /** @brief Returns whether containers are simulated
    */
    bool hasContainers() const {
        return myContainerControl != nullptr;
    }


    /** @brief Returns the edge control
     * @return The edge control
     * @see MSEdgeControl
     * @see myEdges
     */
    MSEdgeControl& getEdgeControl() {
        return *myEdges;
    }


    /** @brief Returns the insertion control
     * @return The insertion control
     * @see MSInsertionControl
     * @see myInserter
     */
    MSInsertionControl& getInsertionControl() {
        return *myInserter;
    }


    /** @brief Returns the detector control
     * @return The detector control
     * @see MSDetectorControl
     * @see myDetectorControl
     */
    MSDetectorControl& getDetectorControl() {
        return *myDetectorControl;
    }


    /** @brief Returns the tls logics control
     * @return The tls logics control
     * @see MSTLLogicControl
     * @see myLogics
     */
    MSTLLogicControl& getTLSControl() {
        return *myLogics;
    }


    /** @brief Returns the junctions control
     * @return The junctions control
     * @see MSJunctionControl
     * @see myJunctions
     */
    MSJunctionControl& getJunctionControl() {
        return *myJunctions;
    }


    /** @brief Returns the event control for events executed at the begin of a time step
     * @return The control responsible for events that are executed at the begin of a time step
     * @see MSEventControl
     * @see myBeginOfTimestepEvents
     */
    MSEventControl* getBeginOfTimestepEvents() {
        return myBeginOfTimestepEvents;
    }


    /** @brief Returns the event control for events executed at the end of a time step
     * @return The control responsible for events that are executed at the end of a time step
     * @see MSEventControl
     * @see myEndOfTimestepEvents
     */
    MSEventControl* getEndOfTimestepEvents() {
        return myEndOfTimestepEvents;
    }


    /** @brief Returns the event control for insertion events
     * @return The control responsible for insertion events
     * @see MSEventControl
     * @see myInsertionEvents
     */
    MSEventControl* getInsertionEvents() {
        return myInsertionEvents;
    }


    /** @brief Returns the shapes container
     * @return The shapes container
     * @see ShapeContainer
     * @see myShapeContainer
     */
    ShapeContainer& getShapeContainer() {
        return *myShapeContainer;
    }

    /** @brief Returns the dynamic shapes updater
     * @see PolygonDynamics
     */
    MSDynamicShapeUpdater* getDynamicShapeUpdater() {
        return myDynamicShapeUpdater.get();
    }

    /** @brief Creates and returns a dynamic shapes updater
     * @see PolygonDynamics
     */
    MSDynamicShapeUpdater* makeDynamicShapeUpdater();

    /** @brief Returns the net's internal edge travel times/efforts container
     *
     * If the net does not have such a container, it is built.
     * @return The net's knowledge about edge weights
     */
    MSEdgeWeightsStorage& getWeightsStorage();
    /// @}

    /// @name Insertion and retrieval of stopping places
    /// @{

    /** @brief Adds a stopping place
     *
     * If another stop with the same id and category exists, false is returned.
     *  Otherwise, the stop is added to the internal stopping place container.
     *
     * This control gets responsible for deletion of the added stop.
     *
     * @param[in] stop The stop to add
     * @return Whether the stop could be added
     */
    bool addStoppingPlace(const SumoXMLTag category, MSStoppingPlace* stop);


    /** @brief Adds a traction substation
     *
     * If another traction substation with the same id and category exists, false is returned.
     *  Otherwise, the traction substation is added to the internal substations container.
     *
     * @param[in] substation The traction substation to add
     * @return Whether the stop could be added
     */
    bool addTractionSubstation(MSTractionSubstation* substation);


    /** @brief Returns the named stopping place of the given category
     * @param[in] id The id of the stop to return.
     * @param[in] category The type of stop
     * @return The named stop, or 0 if no such stop exists
     */
    MSStoppingPlace* getStoppingPlace(const std::string& id, const SumoXMLTag category) const;

    /** @brief Returns the stop of the given category close to the given position
     * @param[in] lane the lane of the stop to return.
     * @param[in] pos the position of the stop to return.
     * @param[in] category The type of stop
     * @return The stop id on the location, or "" if no such stop exists
     */
    std::string getStoppingPlaceID(const MSLane* lane, const double pos, const SumoXMLTag category) const;
    /// @}

    const NamedObjectCont<MSStoppingPlace*>& getStoppingPlaces(SumoXMLTag category) const;

    /// @brief write charging station output
    void writeChargingStationOutput() const;

    /// @brief write rail signal block output
    void writeRailSignalBlocks() const;

    /// @brief creates a wrapper for the given logic (see GUINet)
    virtual void createTLWrapper(MSTrafficLightLogic*) {};

    /// @brief write the output generated by an overhead wire segment
    void writeOverheadWireSegmentOutput() const;

    /// @brief write electrical substation output
    void writeSubstationOutput() const;

    /// @brief return wheter the given logic (or rather it's wrapper) is selected in the GUI
    virtual bool isSelected(const MSTrafficLightLogic*) const {
        return false;
    }

    /// @name Notification about vehicle state changes
    /// @{

    /// @brief Definition of a vehicle state
    enum VehicleState {
        /// @brief The vehicle was built, but has not yet departed
        VEHICLE_STATE_BUILT,
        /// @brief The vehicle has departed (was inserted into the network)
        VEHICLE_STATE_DEPARTED,
        /// @brief The vehicle started to teleport
        VEHICLE_STATE_STARTING_TELEPORT,
        /// @brief The vehicle ended being teleported
        VEHICLE_STATE_ENDING_TELEPORT,
        /// @brief The vehicle arrived at his destination (is deleted)
        VEHICLE_STATE_ARRIVED,
        /// @brief The vehicle got a new route
        VEHICLE_STATE_NEWROUTE,
        /// @brief The vehicles starts to park
        VEHICLE_STATE_STARTING_PARKING,
        /// @brief The vehicle ends to park
        VEHICLE_STATE_ENDING_PARKING,
        /// @brief The vehicles starts to stop
        VEHICLE_STATE_STARTING_STOP,
        /// @brief The vehicle ends to stop
        VEHICLE_STATE_ENDING_STOP,
        /// @brief The vehicle is involved in a collision
        VEHICLE_STATE_COLLISION,
        /// @brief The vehicle had to brake harder than permitted
        VEHICLE_STATE_EMERGENCYSTOP,
        /// @brief Vehicle maneuvering either entering or exiting a parking space
        VEHICLE_STATE_MANEUVERING
    };


    /** @class VehicleStateListener
     * @brief Interface for objects listening to vehicle state changes
     */
    class VehicleStateListener {
    public:
        /// @brief Constructor
        VehicleStateListener() { }

        /// @brief Destructor
        virtual ~VehicleStateListener() { }

        /** @brief Called if a vehicle changes its state
         * @param[in] vehicle The vehicle which changed its state
         * @param[in] to The state the vehicle has changed to
         * @param[in] info Additional information on the state change
         */
        virtual void vehicleStateChanged(const SUMOVehicle* const vehicle, VehicleState to, const std::string& info = "") = 0;

    };


    /** @brief Adds a vehicle states listener
     * @param[in] listener The listener to add
     */
    void addVehicleStateListener(VehicleStateListener* listener);


    /** @brief Removes a vehicle states listener
     * @param[in] listener The listener to remove
     */
    void removeVehicleStateListener(VehicleStateListener* listener);


    /** @brief Informs all added listeners about a vehicle's state change
     * @param[in] vehicle The vehicle which changed its state
     * @param[in] to The state the vehicle has changed to
     * @param[in] info Information regarding the replacement
     * @see VehicleStateListener:vehicleStateChanged
     */
    void informVehicleStateListener(const SUMOVehicle* const vehicle, VehicleState to, const std::string& info = "");
    /// @}



    /** @brief Returns the travel time to pass an edge
     * @param[in] e The edge for which the travel time to be passed shall be returned
     * @param[in] v The vehicle that is rerouted
     * @param[in] t The time for which the travel time shall be returned [s]
     * @return The travel time for an edge
     * @see DijkstraRouter_ByProxi
     */
    static double getTravelTime(const MSEdge* const e, const SUMOVehicle* const v, double t);


    /** @brief Returns the effort to pass an edge
     * @param[in] e The edge for which the effort to be passed shall be returned
     * @param[in] v The vehicle that is rerouted
     * @param[in] t The time for which the effort shall be returned [s]
     * @return The effort (abstract) for an edge
     * @see DijkstraRouter_ByProxi
     */
    static double getEffort(const MSEdge* const e, const SUMOVehicle* const v, double t);


    /* @brief get the router, initialize on first use
     * @param[in] prohibited The vector of forbidden edges (optional)
     */
    SUMOAbstractRouter<MSEdge, SUMOVehicle>& getRouterTT(const int rngIndex,
            const MSEdgeVector& prohibited = MSEdgeVector()) const;
    SUMOAbstractRouter<MSEdge, SUMOVehicle>& getRouterEffort(const int rngIndex,
            const MSEdgeVector& prohibited = MSEdgeVector()) const;
    MSPedestrianRouter& getPedestrianRouter(const int rngIndex, const MSEdgeVector& prohibited = MSEdgeVector()) const;
    MSIntermodalRouter& getIntermodalRouter(const int rngIndex, const int routingMode = 0, const MSEdgeVector& prohibited = MSEdgeVector()) const;

    static void adaptIntermodalRouter(MSIntermodalRouter& router);


    /// @brief return whether the network contains internal links
    bool hasInternalLinks() const {
        return myHasInternalLinks;
    }

    /// @brief return whether the network contains elevation data
    bool hasElevation() const {
        return myHasElevation;
    }

    /// @brief return whether the network contains walkingareas and crossings
    bool hasPedestrianNetwork() const {
        return myHasPedestrianNetwork;

    }
    /// @brief return whether the network contains bidirectional rail edges
    bool hasBidiEdges() const {
        return myHasBidiEdges;
    }

    /// @brief return the network version
    double getNetworkVersion() const {
        return myVersion;
    }

    /// @brief return whether a warning regarding the given object shall be issued
    bool warnOnce(const std::string& typeAndID);

    void interrupt() {
        myAmInterrupted = true;
    }

    bool isInterrupted() const {
        return myAmInterrupted;
    }

    /// @brief find electrical substation by its id
    MSTractionSubstation* findTractionSubstation(const std::string& substationId);

    /// @brief return whether given electrical substation exists in the network
    bool existTractionSubstation(const std::string& substationId);

protected:
    /// @brief check all lanes for elevation data
    bool checkElevation();

    /// @brief check all lanes for type walkingArea
    bool checkWalkingarea();

    /// @brief check wether bidirectional edges occur in the network
    bool checkBidiEdges();

protected:
    /// @brief Unique instance of MSNet
    static MSNet* myInstance;

    /// @brief Route loader for dynamic loading of routes
    SUMORouteLoaderControl* myRouteLoaders;

    /// @brief Current time step.
    SUMOTime myStep;

    /// @brief Maximum number of teleports.
    int myMaxTeleports;

    /// @brief whether an interrupt occured
    bool myAmInterrupted;



    /// @name Substructures
    /// @{

    /// @brief Controls vehicle building and deletion; @see MSVehicleControl
    MSVehicleControl* myVehicleControl;
    /// @brief Controls person building and deletion; @see MSTransportableControl
    MSTransportableControl* myPersonControl;
    /// @brief Controls container building and deletion; @see MSTransportableControl
    MSTransportableControl* myContainerControl;
    /// @brief Controls edges, performs vehicle movement; @see MSEdgeControl
    MSEdgeControl* myEdges;
    /// @brief Controls junctions, realizes right-of-way rules; @see MSJunctionControl
    MSJunctionControl* myJunctions;
    /// @brief Controls tls logics, realizes waiting on tls rules; @see MSJunctionControl
    MSTLLogicControl* myLogics;
    /// @brief Controls vehicle insertion; @see MSInsertionControl
    MSInsertionControl* myInserter;
    /// @brief Controls detectors; @see MSDetectorControl
    MSDetectorControl* myDetectorControl;
    /// @brief Controls events executed at the begin of a time step; @see MSEventControl
    MSEventControl* myBeginOfTimestepEvents;
    /// @brief Controls events executed at the end of a time step; @see MSEventControl
    MSEventControl* myEndOfTimestepEvents;
    /// @brief Controls insertion events; @see MSEventControl
    MSEventControl* myInsertionEvents;
    /// @brief A container for geometrical shapes; @see ShapeContainer
    ShapeContainer* myShapeContainer;
    /// @brief The net's knowledge about edge efforts/travel times; @see MSEdgeWeightsStorage
    MSEdgeWeightsStorage* myEdgeWeights;
    /// @}



    /// @name data needed for computing performance values
    /// @{

    /// @brief Information whether the simulation duration shall be logged
    bool myLogExecutionTime;

    /// @brief Information whether the number of the simulation step shall be logged
    bool myLogStepNumber;
    /// @brief Period between successive step-log outputs
    int myLogStepPeriod;

    /// @brief The last simulation step duration
    long myTraCIStepDuration = 0, mySimStepDuration = 0;

    /// @brief The overall simulation duration
    long mySimBeginMillis;

    /// @brief The overall number of vehicle movements
    long long int myVehiclesMoved;
    long long int myPersonsMoved;
    //}



    /// @name State output variables
    /// @{

    /// @brief Times at which a state shall be written
    std::vector<SUMOTime> myStateDumpTimes;
    /// @brief The names for the state files
    std::vector<std::string> myStateDumpFiles;
    /// @brief The period for writing state
    SUMOTime myStateDumpPeriod;
    /// @brief name components for periodic state
    std::string myStateDumpPrefix;
    std::string myStateDumpSuffix;
    /// @}



    /// @brief Whether the network contains edges which not all vehicles may pass
    bool myHavePermissions;

    /// @brief The vehicle class specific speed restrictions
    std::map<std::string, std::map<SUMOVehicleClass, double> > myRestrictions;

    /// @brief Whether the network contains internal links/lanes/edges
    bool myHasInternalLinks;

    /// @brief Whether the network contains elevation data
    bool myHasElevation;

    /// @brief Whether the network contains pedestrian network elements
    bool myHasPedestrianNetwork;

    /// @brief Whether the network contains bidirectional rail edges
    bool myHasBidiEdges;

    /// @brief Whether the network was built for left-hand traffic
    bool myLefthand;

    /// @brief the network version
    double myVersion;

    /// @brief end of loaded edgeData
    SUMOTime myEdgeDataEndTime;

    /// @brief Dictionary of bus / container stops
    std::map<SumoXMLTag, NamedObjectCont<MSStoppingPlace*> > myStoppingPlaces;

    /// @brief Dictionary of traction substations
    std::vector<MSTractionSubstation*> myTractionSubstations;

    /// @brief Container for vehicle state listener
    std::vector<VehicleStateListener*> myVehicleStateListeners;

#ifdef HAVE_FOX
    /// @brief to avoid concurrent access to the state update function
    FXMutex myStateListenerMutex;
#endif

    /// @brief container to record warnings that shall only be issued once
    std::map<std::string, bool> myWarnedOnce;

    /* @brief The router instance for routing by trigger and by traci
     * @note MSDevice_Routing has its own instance since it uses a different weight function
     * @note we provide one member for every switchable router type
     * because the class structure makes it inconvenient to use a superclass
     */
    mutable std::map<int, SUMOAbstractRouter<MSEdge, SUMOVehicle>*> myRouterTT;
    mutable std::map<int, SUMOAbstractRouter<MSEdge, SUMOVehicle>*> myRouterEffort;
    mutable std::map<int, MSPedestrianRouter*> myPedestrianRouter;
    mutable std::map<int, MSIntermodalRouter*> myIntermodalRouter;

    /// @brief An RTree structure holding lane IDs
    mutable std::pair<bool, NamedRTree> myLanesRTree;

    /// @brief Updater for dynamic shapes that are tracking traffic objects
    ///        (ensures removal of shape dynamics when the objects are removed)
    /// @see utils/shapes/PolygonDynamics
    std::unique_ptr<MSDynamicShapeUpdater> myDynamicShapeUpdater;


    /// @brief string constants for simstep stages
    static const std::string STAGE_EVENTS;
    static const std::string STAGE_MOVEMENTS;
    static const std::string STAGE_LANECHANGE;
    static const std::string STAGE_INSERTIONS;

private:
    /// @brief Invalidated copy constructor.
    MSNet(const MSNet&);

    /// @brief Invalidated assignment operator.
    MSNet& operator=(const MSNet&);


};
