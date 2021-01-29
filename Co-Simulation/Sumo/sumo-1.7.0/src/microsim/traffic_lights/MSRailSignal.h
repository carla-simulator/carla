/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSRailSignal.h
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @author  Jakob Erdmann
/// @date    Jan 2015
///
// A rail signal logic
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <microsim/MSRoute.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSLink;
class MSPhaseDefinition;
class MSRailSignalConstraint;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSRailSignal
 * @brief A signal for rails
 */
class MSRailSignal : public MSTrafficLightLogic {
public:
    /** @brief Constructor
     * @param[in] tlcontrol The tls control responsible for this tls
     * @param[in] id This tls' id
     * @param[in] programID This tls' sub-id (program id)
     * @param[in] parameters This tls' parameters
     * @param[in] delay The time to wait before the first switch
     */
    MSRailSignal(MSTLLogicControl& tlcontrol,
                 const std::string& id, const std::string& programID, SUMOTime delay,
                 const std::map<std::string, std::string>& parameters);


    /** @brief Initialises the rail signal with information about adjacent rail signals
     * @param[in] nb The detector builder
     * @exception ProcessError If something fails on initialisation
     */
    void init(NLDetectorBuilder& nb);


    /// @brief Destructor
    ~MSRailSignal();

    /** @brief Adds a link on building
     * @param[in] link The controlled link
     * @param[in] lane The lane this link starts at
     * @param[in] pos The link's index (signal group) within this program
     */
    void addLink(MSLink* link, MSLane* lane, int pos);

    /// @name Handling of controlled links
    /// @{

    /** @brief Applies information about controlled links and lanes from the given logic
     * @param[in] logic The logic to use the information about controlled links/lanes from
     * @see MSTrafficLightLogic::adaptLinkInformationFrom
     */
    void adaptLinkInformationFrom(const MSTrafficLightLogic& logic);
    /// @}


    /// @name Switching and setting current rows
    /// @{


    /** @brief returns the state of the signal that actually required
     *
     * Returns the state of the rail signal that is actually required based on the
     *  occupation of the adjoining blocks.
     *
     * @return The state actually required for this signal.
     */

    /// @brief updates the current phase of the signal
    void updateCurrentPhase();

    /** @brief Switches to the next phase
    * @return The time of the next switch (always the next step)
    * @see MSTrafficLightLogic::trySwitch
    */
    SUMOTime trySwitch();

    /// @}


    /// @name Static Information Retrieval
    /// @{

    /** @brief Returns the number of phases
     * @return The number of this tls program's phases (always zero)
     * @see MSTrafficLightLogic::getPhaseNumber
     */
    int getPhaseNumber() const;


    /** @brief Returns the phases of this tls program
     * @return The phases of this tls program
     * @see MSTrafficLightLogic::getPhases
     */
    const Phases& getPhases() const;

    /** @brief Returns the definition of the phase from the given position within the plan
    *
    * Returns the current phase as there does not exist a plan of the phases.
    *
    * @param[in] givenstep The index of the phase within the plan
    * @return The definition of the phase at the given position
    * @see MSTrafficLightLogic::getPhase
    */
    const MSPhaseDefinition& getPhase(int givenstep) const;
    /// @}


    /// @name Dynamic Information Retrieval
    /// @{

    /** @brief Returns the current index within the program
     * @return The index of the current phase within the tls (here, always zero will be returned)
     * @see MSTrafficLightLogic::getCurrentPhaseIndex
     */
    int getCurrentPhaseIndex() const;


    /** @brief Returns the definition of the current phase
    * @return The current phase
    */
    const MSPhaseDefinition& getCurrentPhaseDef() const;
    /// @}


    /// @name Conversion between time and phase
    /// @{

    /** @brief Returns the index of the logic at the given simulation step
     * @return The (estimated) index of the tls at the given simulation time step (here, always zero will be returned)
     * @see MSTrafficLightLogic::getPhaseIndexAtTime
     */
    SUMOTime getPhaseIndexAtTime(SUMOTime simStep) const;


    /** @brief Returns the position (start of a phase during a cycle) from of a given step
     * @param[in] index The index of the phase to return the begin of
     * @return The begin time of the phase (here, always zero will be returned)
     * @see MSTrafficLightLogic::getOffsetFromIndex
     */
    SUMOTime getOffsetFromIndex(int index) const;


    /** @brief Returns the step (the phasenumber) of a given position of the cycle
    * @param[in] offset The offset (time) for which the according phase shall be returned
    * @return The according phase (here, always zero will be returned)
    * @see MSTrafficLightLogic::getIndexFromOffset
    */
    int getIndexFromOffset(SUMOTime offset) const;
    /// @}


    /// @name Changing phases and phase durations
    /// @{

    /** @brief Changes the current phase and her duration
     * @param[in] tlcontrol The responsible traffic lights control
     * @param[in] simStep The current simulation step
     * @param[in] step Index of the phase to use
     * @param[in] stepDuration The left duration of the phase
     * @see MSTrafficLightLogic::changeStepAndDuration
     */
    void changeStepAndDuration(MSTLLogicControl& tlcontrol, SUMOTime simStep, int step, SUMOTime stepDuration) {
        UNUSED_PARAMETER(tlcontrol);
        UNUSED_PARAMETER(simStep);
        UNUSED_PARAMETER(step);
        UNUSED_PARAMETER(stepDuration);
    }
    /// @}

    /// @brief return vehicles that block the intersection/rail signal for vehicles that wish to pass the given linkIndex
    VehicleVector getBlockingVehicles(int linkIndex);

    /// @brief return vehicles that approach the intersection/rail signal and are in conflict with vehicles that wish to pass the given linkIndex
    VehicleVector getRivalVehicles(int linkIndex);

    /// @brief return vehicles that approach the intersection/rail signal and have priority over vehicles that wish to pass the given linkIndex
    VehicleVector getPriorityVehicles(int linkIndex);

    /// @brief write rail signal block output for all links and driveways
    void writeBlocks(OutputDevice& od) const;

    void addConstraint(const std::string& tripId, MSRailSignalConstraint* constraint);

    static bool hasOncomingRailTraffic(MSLink* link);

protected:
    /// @brief whether the given vehicle is free to drive
    bool constraintsAllow(const SUMOVehicle* veh) const;

protected:

    typedef std::pair<const SUMOVehicle* const, const MSLink::ApproachingVehicleInformation> Approaching;
    typedef std::set<const MSLane*, ComparatorNumericalIdLess> LaneSet;
    typedef std::map<const MSLane*, int, ComparatorNumericalIdLess> LaneVisitedMap;

    /*  The driveways (Fahrstrassen) for each link index
     *  Each link index has at least one driveway
     *  A driveway describes one possible route that passes the signal up
     *  the next secure point
     *  When a signal guards a switch (indirect guard) that signal stores two
     *  or more driveways
     */
    struct DriveWay {

        /// @brief Constructor
        DriveWay(int index) :
            myIndex(index),
            myMaxFlankLength(0),
            myActive(nullptr)
        {}

        /// @brief index in the list of driveways
        int myIndex;

        /// @brief the maximum flank length searched while building this driveway
        double myMaxFlankLength;

        /// @brief whether the current signal is switched green for a train approaching this block
        const SUMOVehicle* myActive;

        /// @brief list of lanes for matching against train routes
        std::vector<const MSEdge*> myRoute;

        /* @brief the actual driveway part up to the next railsignal (halting position)
         * This must be free of other trains */
        std::vector<MSLane*> myForward;

        /* @brief the list of bidirectional edges that can enter the forward
         * section and  which must also be free of traffic
         * (up to the first element that could give protection) */
        std::vector<MSLane*> myBidi;

        /* @brief the list of edges that merge with the forward section
         * (found via backward search, up to the first element that could give protection) */
        std::vector<const MSLane*> myFlank;

        /// @brief the lanes that must be clear of trains before this signal can switch to green
        std::vector<const MSLane*> myConflictLanes;

        /* @brief the list of switches that threaten the driveway and for which protection must be found
         */
        std::vector<MSLink*> myFlankSwitches;

        /* @brief the list of (first) switches that could give protection from oncoming/flanking vehicles
         * if any of them fails to do so, upstream search must be performed
         * until protection or conflict is found
         */
        std::vector<MSLink*> myProtectingSwitches;

        /* The conflict links for this block
         * Conflict resolution must be performed if vehicles are approaching the
         * current link and any of the conflict links */
        std::vector<MSLink*> myConflictLinks;

        /// @brief whether any of myConflictLanes is occupied (vehicles that are the target of a join must be ignored)
        bool conflictLaneOccupied(const std::string& joinVehicle = "", bool store = true) const;

        /// @brief attempt reserve this driveway for the given vehicle
        bool reserve(const Approaching& closest, MSEdgeVector& occupied);

        /// @brief Whether the approaching vehicle is prevent from driving by another vehicle approaching the given link
        bool hasLinkConflict(const Approaching& closest, MSLink* foeLink) const;

        /// @brief Whether veh must yield to the foe train
        bool mustYield(const Approaching& veh, const Approaching& foe) const;

        /// @brief Whether any of the conflict linkes have approaching vehicles
        bool conflictLinkApproached() const;

        /// @brief find protection for the given vehicle  starting at a switch
        bool findProtection(const Approaching& veh, MSLink* link) const;

        /// @brief Wether this driveway overlaps with the given one
        bool overlap(const DriveWay& other) const;

        /// @brief Write block items for this driveway
        void writeBlocks(OutputDevice& od) const;

        /* @brief determine route that identifies this driveway (a subset of the
         * vehicle route)
         * collects:
         *   myRoute
         *   myForward
         *   myBidi
         *   myFlankSwitches
         *   myProtectingSwitches (at most 1 at the end of the bidi block)
         */
        void buildRoute(MSLink* origin, double length, MSRouteIterator next, MSRouteIterator end,
                        LaneVisitedMap& visited);

        /// @brief find switches that threathen this driveway
        void checkFlanks(const std::vector<MSLane*>& lanes, const LaneVisitedMap& visited, bool allFoes);

        /// @brief find links that cross the driveway without entering it
        void checkCrossingFlanks(MSLink* dwLink, const LaneVisitedMap& visited);

        /// @brief find upstream protection from the given link
        void findFlankProtection(MSLink* link, double length, LaneVisitedMap& visited, MSLink* origLink);
    };

    /* The driveways for each link
     */
    struct LinkInfo {
        /// @brief constructor
        LinkInfo(MSLink* link);

        MSLink* myLink;

        /// @brief whether there is only a single DriveWay following this link
        bool myUniqueDriveWay;

        /// @brief all driveways immediately following this link
        std::vector<DriveWay> myDriveways;

        /// @brief return id for this railsignal-link
        std::string getID() const;

        /// @brief retrieve an existing Driveway or construct a new driveway based on the vehicles route
        DriveWay& getDriveWay(const SUMOVehicle*);

        /// @brief construct a new driveway by searching along the given route until all block structures are found
        DriveWay& buildDriveWay(MSRouteIterator first, MSRouteIterator end);

        /// @brief try rerouting vehicle if reservation failed
        void reroute(SUMOVehicle* veh, const MSEdgeVector& occupied);

        SUMOTime myLastRerouteTime;
        SUMOVehicle* myLastRerouteVehicle;
    };

    /// @brief data storage for every link at this node (more than one when directly guarding a switch)
    std::vector<LinkInfo> myLinkInfos;

    /// @brief get the closest vehicle approaching the given link
    static Approaching getClosest(MSLink* link);

    /// @brief return logicID_linkIndex
    static std::string getTLLinkID(MSLink* link);

    /// @brief return junctionID_junctionLinkIndex
    static std::string getJunctionLinkID(MSLink* link);

    /// @brief return logicID_linkIndex in a way that allows clicking in sumo-gui
    static std::string getClickableTLLinkID(MSLink* link);

    /// @brief print link descriptions
    static std::string describeLinks(std::vector<MSLink*> links);

    /// @brief print link descriptions
    static std::string formatVisitedMap(const LaneVisitedMap& visited);

protected:

    /** @brief The list of phases this logic uses
    *
    *   This vector is always empty and only constructed because the superclass MSTrafficLightLogic requires it.
    */
    Phases myPhases;

    /// @brief The current phase
    MSPhaseDefinition myCurrentPhase;

    /// @brief MSTrafficLightLogic requires that the phase index changes whenever signals change their state
    int myPhaseIndex;

    /// @brief map from tripId to constraint list
    std::map<std::string, std::vector<MSRailSignalConstraint*> > myConstraints;

    static int myNumWarnings;

protected:
    /// @brief update vehicle lists for traci calls
    void storeTraCIVehicles(int linkIndex);

    /// @name traci result storage
    //@{
    static bool myStoreVehicles;
    static VehicleVector myBlockingVehicles;
    static VehicleVector myRivalVehicles;
    static VehicleVector myPriorityVehicles;
    //@}


};
