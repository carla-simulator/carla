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
/// @file    MSLane.h
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christoph Sommer
/// @author  Tino Morenz
/// @author  Michael Behrisch
/// @author  Mario Krumnow
/// @author  Leonhard Luecken
/// @date    Mon, 12 Mar 2001
///
// Representation of a lane in the micro simulation
/****************************************************************************/
#pragma once
#include <config.h>

#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <cassert>
#include <utils/common/Named.h>
#include <utils/common/Parameterised.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/common/NamedRTree.h>
#include <utils/geom/PositionVector.h>
#include "MSLinkCont.h"
#include "MSLeaderInfo.h"
#include "MSMoveReminder.h"
#include <libsumo/Helper.h>

#include <utils/foxtools/FXSynchQue.h>
#ifdef HAVE_FOX
#include <utils/foxtools/FXWorkerThread.h>
#endif


// ===========================================================================
// class declarations
// ===========================================================================
class MSEdge;
class MSVehicle;
class MSLaneChanger;
class MSLink;
class MSVehicleTransfer;
class MSVehicleControl;
class OutputDevice;
class MSLeaderInfo;


// ===========================================================================
// type definitions
// ===========================================================================
/// Coverage info
typedef std::map<const MSLane*, std::pair<double, double> >  LaneCoverageInfo; // also declared in libsumo/Helper.h!

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSLane
 * @brief Representation of a lane in the micro simulation
 *
 * Class which represents a single lane. Somekind of the main class of the
 *  simulation. Allows moving vehicles.
 */
class MSLane : public Named, public Parameterised {
public:
    /// needs access to myTmpVehicles (this maybe should be done via double-buffering!!!)
    friend class MSLaneChanger;
    friend class MSLaneChangerSublane;

    friend class MSQueueExport;
    friend class AnyVehicleIterator;

    /// Container for vehicles.
    typedef std::vector<MSVehicle*> VehCont;

    /** Function-object in order to find the vehicle, that has just
        passed the detector. */
    struct VehPosition : public std::binary_function < const MSVehicle*, double, bool > {
        /// compares vehicle position to the detector position
        bool operator()(const MSVehicle* cmp, double pos) const;
    };

    // TODO: Better documentation
    /// @brief AnyVehicleIterator is a structure, which manages the iteration through all vehicles on the lane,
    ///        that may be of importance for the car-following dynamics along that lane. The relevant types of vehicles are:
    ///        1) vehicles with their front on the lane (myVehicles),
    ///        2) vehicles intersecting the lane but with front on another lane (myPartialVehicles)
    ///
    ///        In the context of retrieving linkLeaders during lane changing a third group of vehicles is checked:
    ///        3) vehicles processed during lane changing (myTmpVehicles)
    class AnyVehicleIterator {
    public:
        AnyVehicleIterator(
            const MSLane* lane,
            int i1,
            int i2,
            int i3,
            const int i1End,
            const int i2End,
            const int i3End,
            bool downstream = true) :
            myLane(lane),
            myI1(i1),
            myI2(i2),
            myI3(i3),
            myI1End(i1End),
            myI2End(i2End),
            myI3End(i3End),
            myDownstream(downstream),
            myDirection(downstream ? 1 : -1) {
        }

        bool operator== (AnyVehicleIterator const& other) const {
            return (myI1 == other.myI1
                    && myI2 == other.myI2
                    && myI3 == other.myI3
                    && myI1End == other.myI1End
                    && myI2End == other.myI2End
                    && myI3End == other.myI3End);
        }

        bool operator!= (AnyVehicleIterator const& other) const {
            return !(*this == other);
        }

        const MSVehicle* operator->() {
            return **this;
        }

        const MSVehicle* operator*();

        AnyVehicleIterator& operator++();

    private:
        bool nextIsMyVehicles() const;

        /// @brief the lane that is being iterated
        const MSLane* myLane;
        /// @brief index for myVehicles
        int myI1;
        /// @brief index for myPartialVehicles
        int myI2;
        /// @brief index for myTmpVehicles
        int myI3;
        /// @brief end index for myVehicles
        int myI1End;
        /// @brief end index for myPartialVehicles
        int myI2End;
        /// @brief end index for myTmpVehicles
        int myI3End;
        /// @brief iteration direction
        bool myDownstream;
        /// @brief index delta
        int myDirection;

    };


public:
    /** @enum ChangeRequest
     * @brief Requests set via TraCI
     */
    enum CollisionAction {
        COLLISION_ACTION_NONE,
        COLLISION_ACTION_WARN,
        COLLISION_ACTION_TELEPORT,
        COLLISION_ACTION_REMOVE
    };

    /** @brief Constructor
     *
     * @param[in] id The lane's id
     * @param[in] maxSpeed The speed allowed on this lane
     * @param[in] length The lane's length
     * @param[in] edge The edge this lane belongs to
     * @param[in] numericalID The numerical id of the lane
     * @param[in] shape The shape of the lane
     * @param[in] width The width of the lane
     * @param[in] permissions Encoding of the Vehicle classes that may drive on this lane
     * @param[in] index The index of this lane within its parent edge
     * @param[in] isRampAccel Whether this lane is an acceleration lane
     * @see SUMOVehicleClass
     */
    MSLane(const std::string& id, double maxSpeed, double length, MSEdge* const edge,
           int numericalID, const PositionVector& shape, double width,
           SVCPermissions permissions, int index, bool isRampAccel,
           const std::string& type);


    /// @brief Destructor
    virtual ~MSLane();

    /// @brief sets the associated RNG index
    void setRNGIndex(const int rngIndex) {
        myRNGIndex = rngIndex;
    }

    /// @brief returns the associated RNG index
    int getRNGIndex() const {
        return myRNGIndex;
    }

    /// @brief return the associated RNG
    std::mt19937* getRNG() const {
        return &myRNGs[myRNGIndex];
    }

    /// @brief return the number of RNGs
    static int getNumRNGs() {
        return (int)myRNGs.size();
    }

    /// @brief save random number generator states to the given output device
    static void saveRNGStates(OutputDevice& out);

    /// @brief load random number generator state for the given rng index
    static void loadRNGState(int index, const std::string& state);

    /// @name Additional initialisation
    /// @{

    /** @brief Delayed initialization
     *
     *  Not all lane-members are known at the time the lane is born, above all the pointers
     *   to other lanes, so we have to add them later.
     *
     * @param[in] link An outgoing link
     */
    void addLink(MSLink* link);

    /** @brief Adds a neighbor to this lane
     *
     * @param[in] id The lane's id
     */
    void addNeigh(const std::string& id);
    ///@}



    /// @name interaction with MSMoveReminder
    /// @{

    /** @brief Add a move-reminder to move-reminder container
     *
     * The move reminder will not be deleted by the lane.
     *
     * @param[in] rem The move reminder to add
     */
    virtual void addMoveReminder(MSMoveReminder* rem);


    /** @brief Return the list of this lane's move reminders
     * @return Previously added move reminder
     */
    inline const std::vector< MSMoveReminder* >& getMoveReminders() const {
        return myMoveReminders;
    }
    ///@}



    /// @name Vehicle insertion
    ///@{

    /** @brief Tries to insert the given vehicle
     *
     * The insertion position and speed are determined in dependence
     *  to the vehicle's departure definition, first.
     *
     * Then, the vehicle is tried to be inserted into the lane
     *  using these values by a call to "isInsertionSuccess". The result of
     *  "isInsertionSuccess" is returned.
     *
     * @param[in] v The vehicle to insert
     * @return Whether the vehicle could be inserted
     * @see isInsertionSuccess
     * @see MSVehicle::getDepartureDefinition
     * @see MSVehicle::DepartArrivalDefinition
     */
    bool insertVehicle(MSVehicle& v);


    /** @brief Tries to insert the given vehicle with the given state (speed and pos)
     *
     * Checks whether the vehicle can be inserted at the given position with the
     *  given speed so that no collisions with leader/follower occur and the speed
     *  does not cause unexpected behaviour on consecutive lanes. Returns false
     *  if the vehicle can not be inserted.
     *
     * If the insertion can take place, incorporateVehicle() is called and true is returned.
     *
     * @param[in] vehicle The vehicle to insert
     * @param[in] speed The speed with which it shall be inserted
     * @param[in] pos The position at which it shall be inserted
     * @param[in] posLat The lateral position at which it shall be inserted
     * @param[in] recheckNextLanes Forces patching the speed for not being too fast on next lanes
     * @param[in] notification The cause of insertion (i.e. departure, teleport, parking) defaults to departure
     * @return Whether the vehicle could be inserted
     * @see MSVehicle::enterLaneAtInsertion
     */
    bool isInsertionSuccess(MSVehicle* vehicle, double speed, double pos, double posLat,
                            bool recheckNextLanes,
                            MSMoveReminder::Notification notification);

    // XXX: Documentation?
    bool checkFailure(const MSVehicle* aVehicle, double& speed, double& dist, const double nspeed, const bool patchSpeed, const std::string errorMsg) const;

    /** @brief inserts vehicle as close as possible to the last vehicle on this
     * lane (or at the end of the lane if there is no leader)
     */
    bool lastInsertion(MSVehicle& veh, double mspeed, double posLat, bool patchSpeed);

    /** @brief Tries to insert the given vehicle on any place
     *
     * @param[in] veh The vehicle to insert
     * @param[in] speed The maximum insertion speed
     * @param[in] notification The cause of insertion (i.e. departure, teleport, parking) defaults to departure
     * @return Whether the vehicle could be inserted
     */
    bool freeInsertion(MSVehicle& veh, double speed, double posLat,
                       MSMoveReminder::Notification notification = MSMoveReminder::NOTIFICATION_DEPARTED);


    /** @brief Inserts the given vehicle at the given position
     *
     * No checks are done, vehicle insertion using this method may
     *  generate collisions (possibly delayed).
     * @param[in] veh The vehicle to insert
     * @param[in] pos The position at which the vehicle shall be inserted
     * @param[in] notification The cause of insertion (i.e. departure, teleport, parking) defaults to departure
     * @param[in] posLat The lateral position at which the vehicle shall be inserted
     */
    void forceVehicleInsertion(MSVehicle* veh, double pos, MSMoveReminder::Notification notification, double posLat = 0);
    /// @}



    /// @name Handling vehicles lapping into several lanes (-> partial occupation)
    ///       or which committed a maneuver that will lead them into another (sublane case -> maneuver reservations)
    /// @{
    /** @brief Sets the information about a vehicle lapping into this lane
     *
     * This vehicle is added to myVehicles and may be distinguished from regular
     * vehicles by the disparity between this lane and v->getLane()
     * @param[in] v The vehicle which laps into this lane
     * @return This lane's length
     */
    virtual double setPartialOccupation(MSVehicle* v);

    /** @brief Removes the information about a vehicle lapping into this lane
     * @param[in] v The vehicle which laps into this lane
     */
    virtual void resetPartialOccupation(MSVehicle* v);

    /** @brief Registers the lane change intentions (towards this lane) for the given vehicle
     */
    virtual void setManeuverReservation(MSVehicle* v);

    /** @brief Unregisters a vehicle, which previously registered for maneuvering into this lane
     * @param[in] v The vehicle
     */
    virtual void resetManeuverReservation(MSVehicle* v);

    /** @brief Returns the last vehicles on the lane
     *
     * The information about the last vehicles in this lanes in all sublanes
     * occupied by ego are
     * returned. Partial occupators are included
     * @param[in] ego The vehicle for which to restrict the returned leaderInfo
     * @param[in] minPos The minimum position from which to start search for leaders
     * @param[in] allowCached Whether the cached value may be used
     * @return Information about the last vehicles
     */
    const MSLeaderInfo getLastVehicleInformation(const MSVehicle* ego, double latOffset, double minPos = 0, bool allowCached = true) const;

    /// @brief analogue to getLastVehicleInformation but in the upstream direction
    const MSLeaderInfo getFirstVehicleInformation(const MSVehicle* ego, double latOffset, bool onlyFrontOnLane, double maxPos = std::numeric_limits<double>::max(), bool allowCached = true) const;

    /// @}

    /// @name Access to vehicles
    /// @{

    /** @brief Returns the number of vehicles on this lane (for which this lane
     * is responsible)
     * @return The number of vehicles with their front on this lane
     */
    int getVehicleNumber() const {
        return (int)myVehicles.size();
    }

    /** @brief Returns the number of vehicles on this lane (including partial
     * occupators)
     * @return The number of vehicles with intersecting this lane
     */
    int getVehicleNumberWithPartials() const {
        return (int)myVehicles.size() + (int)myPartialVehicles.size();
    }

    /** @brief Returns the number of vehicles partially on this lane (for which this lane
     * is not responsible)
     * @return The number of vehicles touching this lane but with their front on another lane
     */
    int getPartialVehicleNumber() const {
        return (int)myPartialVehicles.size();
    }


    /** @brief Returns the vehicles container; locks it for microsimulation
     *
     * Please note that it is necessary to release the vehicles container
     *  afterwards using "releaseVehicles".
     * @return The vehicles on this lane
     */
    virtual const VehCont& getVehiclesSecure() const {
        return myVehicles;
    }


    /// @brief begin iterator for iterating over all vehicles touching this lane in downstream direction
    AnyVehicleIterator anyVehiclesBegin() const {
        return AnyVehicleIterator(this, 0, 0, 0,
                                  (int)myVehicles.size(), (int)myPartialVehicles.size(), (int)myTmpVehicles.size(), true);
    }

    /// @brief end iterator for iterating over all vehicles touching this lane in downstream direction
    AnyVehicleIterator anyVehiclesEnd() const {
        return AnyVehicleIterator(this, (int)myVehicles.size(), (int)myPartialVehicles.size(), (int)myTmpVehicles.size(),
                                  (int)myVehicles.size(), (int)myPartialVehicles.size(), (int)myTmpVehicles.size(), true);
    }

    /// @brief begin iterator for iterating over all vehicles touching this lane in upstream direction
    AnyVehicleIterator anyVehiclesUpstreamBegin() const {
        return AnyVehicleIterator(this, (int)myVehicles.size() - 1, (int)myPartialVehicles.size() - 1, (int)myTmpVehicles.size() - 1,
                                  -1, -1, -1, false);
    }

    /// @brief end iterator for iterating over all vehicles touching this lane in upstream direction
    AnyVehicleIterator anyVehiclesUpstreamEnd() const {
        return AnyVehicleIterator(this, -1, -1, -1, -1, -1, -1, false);
    }

    /** @brief Allows to use the container for microsimulation again
     */
    virtual void releaseVehicles() const { }
    /// @}



    /// @name Atomar value getter
    /// @{


    /** @brief Returns this lane's numerical id
     * @return This lane's numerical id
     */
    inline int getNumericalID() const {
        return myNumericalID;
    }


    /** @brief Returns this lane's shape
     * @return This lane's shape
     */
    inline const PositionVector& getShape() const {
        return myShape;
    }

    /// @brief return shape.length() / myLength
    inline double getLengthGeometryFactor() const {
        return myLengthGeometryFactor;
    }

    /// @brief return whether this lane is an acceleration lane
    inline bool isAccelLane() const {
        return myIsRampAccel;
    }

    /// @brief return the type of this lane
    const std::string& getLaneType() const {
        return myLaneType;
    }

    /* @brief fit the given lane position to a visibly suitable geometry position
     * (lane length might differ from geometry length) */
    inline double interpolateLanePosToGeometryPos(double lanePos) const {
        return lanePos * myLengthGeometryFactor;
    }

    /* @brief fit the given lane position to a visibly suitable geometry position
     * and return the coordinates */
    inline const Position geometryPositionAtOffset(double offset, double lateralOffset = 0) const {
        return myShape.positionAtOffset(interpolateLanePosToGeometryPos(offset), lateralOffset);
    }

    /* @brief fit the given geometry position to a valid lane position
     * (lane length might differ from geometry length) */
    inline double interpolateGeometryPosToLanePos(double geometryPos) const {
        return geometryPos / myLengthGeometryFactor;
    }

    /** @brief Returns the lane's maximum speed, given a vehicle's speed limit adaptation
     * @param[in] The vehicle to return the adapted speed limit for
     * @return This lane's resulting max. speed
     */
    inline double getVehicleMaxSpeed(const SUMOTrafficObject* const veh) const {
        if (myRestrictions != nullptr) {
            std::map<SUMOVehicleClass, double>::const_iterator r = myRestrictions->find(veh->getVClass());
            if (r != myRestrictions->end()) {
                return MIN2(veh->getMaxSpeed(), r->second * veh->getChosenSpeedFactor());
            }
        }
        return MIN2(veh->getMaxSpeed(), myMaxSpeed * veh->getChosenSpeedFactor());
    }


    /** @brief Returns the lane's maximum allowed speed
     * @return This lane's maximum allowed speed
     */
    inline double getSpeedLimit() const {
        return myMaxSpeed;
    }


    /** @brief Returns the lane's length
     * @return This lane's length
     */
    inline double getLength() const {
        return myLength;
    }


    /** @brief Returns the vehicle class permissions for this lane
     * @return This lane's allowed vehicle classes
     */
    inline SVCPermissions getPermissions() const {
        return myPermissions;
    }


    /** @brief Returns the lane's width
     * @return This lane's width
     */
    double getWidth() const {
        return myWidth;
    }

    /** @brief Returns the lane's index
     * @return This lane's index
     */
    int getIndex() const {
        return myIndex;
    }
    /// @}

    /// @brief return the index of the link to the next crossing if this is walkingArea, else -1
    int getCrossingIndex() const;


    /// @name Vehicle movement (longitudinal)
    /// @{

    /** @brief Compute safe velocities for all vehicles based on positions and
     * speeds from the last time step. Also registers
     * ApproachingVehicleInformation for all links
     *
     * This method goes through all vehicles calling their "planMove" method.
     * @see MSVehicle::planMove
     */
    virtual void planMovements(const SUMOTime t);

    /** @brief Register junction approaches for all vehicles after velocities
     * have been planned.
     *
     * This method goes through all vehicles calling their * "setApproachingForAllLinks" method.
     */
    virtual void setJunctionApproaches(const SUMOTime t) const;

    /** @brief This updates the MSLeaderInfo argument with respect to the given MSVehicle.
     *         All leader-vehicles on the same edge, which are relevant for the vehicle
     *         (i.e. with position > vehicle's position) and not already integrated into
     *         the LeaderInfo, are integrated.
     *         The given iterators vehPart and vehRes give access to these vehicles which are
     *         either partial occupators or have issued a maneuver reservation for the lane
     *         (the latter occurs only for the sublane model).
     */
    void updateLeaderInfo(const MSVehicle* veh, VehCont::reverse_iterator& vehPart, VehCont::reverse_iterator& vehRes, MSLeaderInfo& ahead) const;

    /** @brief Executes planned vehicle movements with regards to right-of-way
     *
     * This method goes through all vehicles calling their executeMove method
     * which causes vehicles to update their positions and speeds.
     * Vehicles wich move to the next lane are stored in the targets lane buffer
     *
     * @return Returns true, if all vehicles left the lane.
     *
     * @see MSVehicle::executeMove
     */
    virtual void executeMovements(const SUMOTime t);

    /// Insert buffered vehicle into the real lane.
    virtual void integrateNewVehicles();

    /// @brief updated current vehicle length sum (delayed to avoid lane-order-dependency)
    void updateLengthSum();
    ///@}


    /// @brief short-circut collision check if nothing changed since the last check
    inline bool needsCollisionCheck() const {
        return myNeedsCollisionCheck;
    }

    /// @brief require another collision check due to relevant changes in the simulation
    inline void requireCollisionCheck() {
        myNeedsCollisionCheck = true;
    }

    /// Check if vehicles are too close.
    virtual void detectCollisions(SUMOTime timestep, const std::string& stage);


    /** Returns the information whether this lane may be used to continue
        the current route */
    virtual bool appropriate(const MSVehicle* veh);


    /// returns the container with all links !!!
    const MSLinkCont& getLinkCont() const;

    /// returns the link to the given lane or 0, if it is not connected
    MSLink* getLinkTo(const MSLane*) const;

    /// Returns the entry link if this is an internal lane, else 0
    MSLink* getEntryLink() const;


    /// Returns true if there is not a single vehicle on the lane.
    bool empty() const {
        assert(myVehBuffer.size() == 0);
        return myVehicles.empty();
    }

    /** @brief Sets a new maximum speed for the lane (used by TraCI and MSCalibrator)
     * @param[in] val the new speed in m/s
     */
    void setMaxSpeed(double val);

    /** @brief Sets a new length for the lane (used by TraCI only)
     * @param[in] val the new length in m
     */
    void setLength(double val);

    /** @brief Returns the lane's edge
     * @return This lane's edge
     */
    MSEdge& getEdge() const {
        return *myEdge;
    }


    /** @brief Returns the lane's follower if it is an internal lane, the edge of the lane otherwise
     * @return This lane's follower
     */
    const MSEdge* getNextNormal() const;


    /** @brief Returns 0 if the lane is not internal. Otherwise the first part of the
     *         connection (sequence of internal lanes along junction) corresponding to the lane
     *         is returned and the offset is set to the distance of the begin of this lane
     *         to the begin of the returned.
     */
    const MSLane* getFirstInternalInConnection(double& offset) const;


    /// @brief Static (sic!) container methods
    /// {

    /** @brief Inserts a MSLane into the static dictionary
     *
     * Returns true if the key id isn't already in the dictionary.
     *  Otherwise returns false.
     * @param[in] id The id of the lane
     * @param[in] lane The lane itself
     * @return Whether the lane was added
     * @todo make non-static
     * @todo why is the id given? The lane is named
     */
    static bool dictionary(const std::string& id, MSLane* lane);


    /** @brief Returns the MSLane associated to the key id
     *
     * The lane is returned if exists, otherwise 0 is returned.
     * @param[in] id The id of the lane
     * @return The lane
     */
    static MSLane* dictionary(const std::string& id);


    /** @brief Clears the dictionary */
    static void clear();


    /** @brief Returns the number of stored lanes
     * @return The number of stored lanes
     */
    static int dictSize() {
        return (int)myDict.size();
    }


    /** @brief Adds the ids of all stored lanes into the given vector
     * @param[in, filled] into The vector to add the IDs into
     */
    static void insertIDs(std::vector<std::string>& into);


    /** @brief Fills the given RTree with lane instances
     * @param[in, filled] into The RTree to fill
     * @see TraCILaneRTree
     */
    template<class RTREE>
    static void fill(RTREE& into);


    /// @brief initialize rngs
    static void initRNGs(const OptionsCont& oc);
    /// @}



    // XXX: succLink does not exist... Documentation?
    /** Same as succLink, but does not throw any assertions when
        the succeeding link could not be found;
        Returns the myLinks.end() instead; Further, the number of edges to
        look forward may be given */
    static MSLinkCont::const_iterator succLinkSec(const SUMOVehicle& veh,
            int nRouteSuccs,
            const MSLane& succLinkSource,
            const std::vector<MSLane*>& conts);


    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::const_iterator& i) const;

    /** Returns the information whether the given link shows at the end
        of the list of links (is not valid) */
    bool isLinkEnd(MSLinkCont::iterator& i);

    /** Returns the information whether the lane is has no vehicle and no
        partial occupation*/
    bool isEmpty() const;

    /** Returns whether the lane pertains to an internal edge*/
    bool isInternal() const;

    /// @brief returns the last vehicle for which this lane is responsible or 0
    MSVehicle* getLastFullVehicle() const;

    /// @brief returns the first vehicle for which this lane is responsible or 0
    MSVehicle* getFirstFullVehicle() const;

    /// @brief returns the last vehicle that is fully or partially on this lane
    MSVehicle* getLastAnyVehicle() const;

    /// @brief returns the first vehicle that is fully or partially on this lane
    MSVehicle* getFirstAnyVehicle() const;

    /* @brief remove the vehicle from this lane
     * @param[notify] whether moveReminders of the vehicle shall be triggered
     */
    virtual MSVehicle* removeVehicle(MSVehicle* remVehicle, MSMoveReminder::Notification notification, bool notify = true);

    void leftByLaneChange(MSVehicle* v);
    void enteredByLaneChange(MSVehicle* v);

    /** @brief Returns the lane with the given offset parallel to this one or 0 if it does not exist
     * @param[in] offset The offset of the result lane
     */
    MSLane* getParallelLane(int offset, bool includeOpposite = true) const;


    /** @brief Sets the permissions to the given value. If a transientID is given, the permissions are recored as temporary
     * @param[in] permissions The new permissions
     * @param[in] transientID The id of the permission-modification or the special value PERMANENT
     */
    void setPermissions(SVCPermissions permissions, long long transientID);
    void resetPermissions(long long transientID);


    inline bool allowsVehicleClass(SUMOVehicleClass vclass) const {
        return (myPermissions & vclass) == vclass;
    }

    void addIncomingLane(MSLane* lane, MSLink* viaLink);


    struct IncomingLaneInfo {
        MSLane* lane;
        double length;
        MSLink* viaLink;
    };

    const std::vector<IncomingLaneInfo>& getIncomingLanes() const {
        return myIncomingLanes;
    }


    void addApproachingLane(MSLane* lane, bool warnMultiCon);
    bool isApproachedFrom(MSEdge* const edge);
    bool isApproachedFrom(MSEdge* const edge, MSLane* const lane);

    /// @brief Returns vehicle class specific stopOffset for the vehicle
    double getStopOffset(const MSVehicle* veh) const;

    /// @brief Returns vehicle class specific stopOffsets
    const std::map<SVCPermissions, double>& getStopOffsets() const {
        return myStopOffsets;
    };

    /// @brief Set vehicle class specific stopOffsets
    void setStopOffsets(std::map<SVCPermissions, double> stopOffsets) {
        myStopOffsets = stopOffsets;
    };

    /// @brief return the sublane followers with the largest missing rear gap among all predecessor lanes (within dist)
    MSLeaderDistanceInfo getFollowersOnConsecutive(const MSVehicle* ego, double backOffset,
            bool allSublanes, double searchDist = -1, bool ignoreMinorLinks = false) const;

    /// @brief return by how much further the leader must be inserted to avoid rear end collisions
    double getMissingRearGap(const MSVehicle* leader, double backOffset, double leaderSpeed) const;

    /** @brief Returns the immediate leader of veh and the distance to veh
     * starting on this lane
     *
     * Iterates over the current lane to find a leader and then uses
     * getLeaderOnConsecutive()
     * @param[in] veh The vehicle for which the information shall be computed
     * @param[in] vehPos The vehicle position relative to this lane (may be negative)
     * @param[in] bestLaneConts The succeding lanes that shall be checked (if any)
     * @param[in] dist Optional distance to override default (ego stopDist)
     * @param[in] checkTmpVehicles Whether myTmpVehicles should be used instead of myVehicles
     * @return
     */
    std::pair<MSVehicle* const, double> getLeader(const MSVehicle* veh, const double vehPos, const std::vector<MSLane*>& bestLaneConts, double dist = -1, bool checkTmpVehicles = false) const;

    /** @brief Returns the immediate leader and the distance to him
     *
     * Goes along the vehicle's estimated used lanes (bestLaneConts). For each link,
     *  it is determined whether the vehicle will pass it. If so, the subsequent lane
     *  is investigated. If a vehicle (leader) is found, it is returned, together with the length
     *  of the investigated lanes until this vehicle's end, including the already seen
     *  place (seen).
     *
     * If no leading vehicle was found, <0, -1> is returned.
     *
     * Pretty slow, as it has to go along lanes.
     *
     * @todo: There are some oddities:
     * - what about crossing a link at red, or if a link is closed? Has a following vehicle to be regarded or not?
     *
     * @param[in] dist The distance to investigate
     * @param[in] seen The already seen place (normally the place in front on own lane)
     * @param[in] speed The speed of the vehicle used for determining whether a subsequent link will be opened at arrival time
     * @param[in] veh The vehicle for which the information shall be computed
     * @param[in] bestLaneConts The lanes the vehicle will use in future
     * @return
     */
    std::pair<MSVehicle* const, double> getLeaderOnConsecutive(double dist, double seen,
            double speed, const MSVehicle& veh, const std::vector<MSLane*>& bestLaneConts) const;

    /// @brief Returns the immediate leaders and the distance to them (as getLeaderOnConsecutive but for the sublane case)
    void getLeadersOnConsecutive(double dist, double seen, double speed, const MSVehicle* ego,
                                 const std::vector<MSLane*>& bestLaneConts, MSLeaderDistanceInfo& result) const;

    /** @brief Returns the most dangerous leader and the distance to him
     *
     * Goes along the vehicle's estimated used lanes (bestLaneConts). For each link,
     *  it is determined whether the ego vehicle will pass it. If so, the subsequent lane
     *  is investigated. Check all lanes up to the stopping distance of ego.
     *  Return the leader vehicle (and the gap) which puts the biggest speed constraint on ego.
     *
     * If no leading vehicle was found, <0, -1> is returned.
     *
     * Pretty slow, as it has to go along lanes.
     *
     * @param[in] dist The distance to investigate
     * @param[in] seen The already seen place (normally the place in front on own lane)
     * @param[in] speed The speed of the vehicle used for determining whether a subsequent link will be opened at arrival time
     * @param[in] veh The (ego) vehicle for which the information shall be computed
     * @return
     */
    std::pair<MSVehicle* const, double> getCriticalLeader(double dist, double seen, double speed, const MSVehicle& veh) const;

    /* @brief return the partial vehicle closest behind ego or 0
     * if no such vehicle exists */
    MSVehicle* getPartialBehind(const MSVehicle* ego) const;

    /// @brief get all vehicles that are inlapping from consecutive edges
    MSLeaderInfo getPartialBeyond() const;

    /// @brief Returns all vehicles closer than downstreamDist along the along the road network starting on the given
    ///        position. Predecessor lanes are searched upstream for the given upstreamDistance
    /// @note  Re-implementation of the corresponding method in MSDevice_SSM, which cannot be easily adapted, as it gathers
    ///        additional information for conflict lanes, etc.
    /// @param[in] lanes - sequence of lanes to search along
    /// @param[in] startPos - start position of the search on the first lane
    /// @param[in] downstreamDist - distance to search downstream
    /// @param[in] upstreamDist - distance to search upstream
    /// @param[in/out] checkedLanes - lanes, which were already scanned (current lane is added, if not present,
    ///                otherwise the scan is aborted; TODO: this may disregard unscanned parts of the lane in specific circular set ups.)
    /// @return    vehs - List of vehicles found
    std::set<MSVehicle*> getSurroundingVehicles(double startPos, double downstreamDist, double upstreamDist, std::shared_ptr<LaneCoverageInfo> checkedLanes) const;

    /// @brief Returns all vehicles on the lane overlapping with the interval [a,b]
    /// @note  Does not consider vehs with front on subsequent lanes
    std::set<MSVehicle*> getVehiclesInRange(const double a, const double b) const;


    /// @brief Returns all upcoming junctions within given range along the given (non-internal) continuation lanes measured from given position
    std::vector<const MSJunction*> getUpcomingJunctions(double pos, double range, const std::vector<MSLane*>& contLanes) const;
    /// @brief Returns all upcoming links within given range along the given (non-internal) continuation lanes measured from given position
    std::vector<const MSLink*> getUpcomingLinks(double pos, double range, const std::vector<MSLane*>& contLanes) const;

    /** @brief get the most likely precedecessor lane (sorted using by_connections_to_sorter).
     * The result is cached in myLogicalPredecessorLane
     */
    MSLane* getLogicalPredecessorLane() const;

    /** @brief get normal lane leading to this internal lane, for normal lanes,
     * the lane itself is returned
     */
    const MSLane* getNormalPredecessorLane() const;

    /** @brief return the (first) predecessor lane from the given edge
     */
    MSLane* getLogicalPredecessorLane(const MSEdge& fromEdge) const;


    /** Return the main predecessor lane for the current.
     * If there are several incoming lanes, the first attempt is to return the priorized.
     * If this does not yield an unambiguous lane, the one with the least angle difference
     * to the current is selected.
     */
    MSLane* getCanonicalPredecessorLane() const;


    /** Return the main successor lane for the current.
     * If there are several outgoing lanes, the first attempt is to return the priorized.
     * If this does not yield an unambiguous lane, the one with the least angle difference
     * to the current is selected.
     */
    MSLane* getCanonicalSuccessorLane() const;

    /// @brief get the state of the link from the logical predecessor to this lane
    LinkState getIncomingLinkState() const;

    /// @brief get the list of outgoing lanes
    const std::vector<std::pair<const MSLane*, const MSEdge*> > getOutgoingViaLanes() const;

    /// @brief get the list of all direct (disregarding internal predecessors) non-internal predecessor lanes of this lane
    std::vector<const MSLane*> getNormalIncomingLanes() const;

    /// @name Current state retrieval
    //@{

    /** @brief Returns the mean speed on this lane
     * @return The average speed of vehicles during the last step; default speed if no vehicle was on this lane
     */
    double getMeanSpeed() const;

    /// @brief get the mean speed of all bicycles on this lane
    double getMeanSpeedBike() const;

    /** @brief Returns the overall waiting time on this lane
    * @return The sum of the waiting time of all vehicles during the last step;
    */
    double getWaitingSeconds() const;


    /** @brief Returns the brutto (including minGaps) occupancy of this lane during the last step
     * @return The occupancy during the last step
     */
    double getBruttoOccupancy() const;


    /** @brief Returns the netto (excluding minGaps) occupancy of this lane during the last step (including minGaps)
     * @return The occupancy during the last step
     */
    double getNettoOccupancy() const;


    /** @brief Returns the sum of lengths of vehicles, including their minGaps, which were on the lane during the last step
     * @return The sum of vehicle lengths of vehicles in the last step
     */
    inline double getBruttoVehLenSum() const {
        return myBruttoVehicleLengthSum;
    }


    /** @brief Returns the sum of last step CO2 emissions
     * @return CO2 emissions of vehicles on this lane during the last step
     */
    double getCO2Emissions() const;


    /** @brief Returns the sum of last step CO emissions
     * @return CO emissions of vehicles on this lane during the last step
     */
    double getCOEmissions() const;


    /** @brief Returns the sum of last step PMx emissions
     * @return PMx emissions of vehicles on this lane during the last step
     */
    double getPMxEmissions() const;


    /** @brief Returns the sum of last step NOx emissions
     * @return NOx emissions of vehicles on this lane during the last step
     */
    double getNOxEmissions() const;


    /** @brief Returns the sum of last step HC emissions
     * @return HC emissions of vehicles on this lane during the last step
     */
    double getHCEmissions() const;


    /** @brief Returns the sum of last step fuel consumption
    * @return fuel consumption of vehicles on this lane during the last step
    */
    double getFuelConsumption() const;


    /** @brief Returns the sum of last step electricity consumption
    * @return electricity consumption of vehicles on this lane during the last step
    */
    double getElectricityConsumption() const;


    /** @brief Returns the sum of last step noise emissions
     * @return noise emissions of vehicles on this lane during the last step
     */
    double getHarmonoise_NoiseEmissions() const;
    /// @}

    void setRightSideOnEdge(double value, int rightmostSublane) {
        myRightSideOnEdge = value;
        myRightmostSublane = rightmostSublane;
    }

    /// @brief initialized vClass-specific speed limits
    void initRestrictions();

    void checkBufferType();

    double getRightSideOnEdge() const {
        return myRightSideOnEdge;
    }

    int getRightmostSublane() const {
        return myRightmostSublane;
    }

    double getCenterOnEdge() const {
        return myRightSideOnEdge + 0.5 * myWidth;
    }

    /// @brief sorts myPartialVehicles
    void sortPartialVehicles();

    /// @brief sorts myManeuverReservations
    void sortManeuverReservations();

    /// @brief return the opposite direction lane for lane changing or 0
    MSLane* getOpposite() const;

    /// @brief return the corresponding position on the opposite lane
    double getOppositePos(double pos) const;

    /* @brief find leader for a vehicle depending on the relative driving direction
     * @param[in] ego The ego vehicle
     * @param[in] dist The look-ahead distance when looking at consecutive lanes
     * @param[in] oppositeDir Whether the lane has the opposite driving direction of ego
     * @return the leader vehicle and it's gap to ego
     */
    std::pair<MSVehicle* const, double> getOppositeLeader(const MSVehicle* ego, double dist, bool oppositeDir) const;

    /* @brief find follower for a vehicle that is located on the opposite of this lane
     * @param[in] ego The ego vehicle
     * @return the follower vehicle and it's gap to ego
     */
    std::pair<MSVehicle* const, double> getOppositeFollower(const MSVehicle* ego) const;


    /** @brief Find follower vehicle for the given ego vehicle (which may be on the opposite direction lane)
     * @param[in] ego The ego vehicle
     * @param[in] egoPos The ego position mapped to the current lane
     * @param[in] dist The look-back distance when looking at consecutive lanes
     * @param[in] ignoreMinorLinks Whether backward search should stop at minor links
     * @return the follower vehicle and it's gap to ego
     */
    std::pair<MSVehicle* const, double> getFollower(const MSVehicle* ego, double egoPos, double dist, bool ignoreMinorLinks) const;


    ///@brief add parking vehicle. This should only used during state loading
    void addParking(MSVehicle* veh);

    ///@brief remove parking vehicle. This must be syncrhonized when running with GUI
    virtual void removeParking(MSVehicle* veh);

    /// @brief retrieve the parking vehicles (see GUIParkingArea)
    const std::set<const MSVehicle*>& getParkingVehicles() const {
        return myParkingVehicles;
    }

    /// @brief whether this lane is selected in the GUI
    virtual bool isSelected() const {
        return false;
    }

    /// @brief retrieve bidirectional lane or nullptr
    MSLane* getBidiLane() const;

    /// @brief whether this lane must check for junction collisions
    bool mustCheckJunctionCollisions() const;

#ifdef HAVE_FOX
    FXWorkerThread::Task* getPlanMoveTask(const SUMOTime time) {
        mySimulationTask.init(&MSLane::planMovements, time);
        return &mySimulationTask;
    }

    FXWorkerThread::Task* getExecuteMoveTask(const SUMOTime time) {
        mySimulationTask.init(&MSLane::executeMovements, time);
        return &mySimulationTask;
    }

    FXWorkerThread::Task* getLaneChangeTask(const SUMOTime time) {
        mySimulationTask.init(&MSLane::changeLanes, time);
        return &mySimulationTask;
    }
#endif

    void changeLanes(const SUMOTime time);

    /// @name State saving/loading
    /// @{

    /** @brief Saves the state of this lane into the given stream
     *
     * Basically, a list of vehicle ids
     *
     * @param[in, filled] out The (possibly binary) device to write the state into
     * @todo What about throwing an IOError?
     */
    void saveState(OutputDevice& out);

    /** @brief Remove all vehicles before quick-loading state */
    void clearState();

    /** @brief Loads the state of this segment with the given parameters
     *
     * This method is called for every internal que the segment has.
     *  Every vehicle is retrieved from the given MSVehicleControl and added to this
     *  lane.
     *
     * @param[in] vehIDs The vehicle ids for the current que
     * @param[in] vc The vehicle control to retrieve references vehicles from
     * @todo What about throwing an IOError?
     * @todo What about throwing an error if something else fails (a vehicle can not be referenced)?
     */
    void loadState(const std::vector<std::string>& vehIDs, MSVehicleControl& vc);
    /// @}


    /** @brief Callback for visiting the lane when traversing an RTree
     *
     * This is used in the TraCIServerAPI_Lane for context subscriptions.
     *
     * @param[in] cont The context doing all the work
     * @see libsumo::Helper::LaneStoringVisitor::add
     */
    void visit(const LaneStoringVisitor& cont) const {
        cont.add(this);
    }

    /// @brief whether the lane has pedestrians on it
    bool hasPedestrians() const;

    /// This is just a wrapper around MSPModel::nextBlocking. You should always check using hasPedestrians before calling this method.
    std::pair<const MSPerson*, double> nextBlocking(double minPos, double minRight, double maxLeft, double stopTime = 0) const;

    static void initCollisionOptions(const OptionsCont& oc);

    static bool teleportOnCollision() {
        return myCollisionAction == COLLISION_ACTION_TELEPORT;
    }

    static CollisionAction getCollisionAction() {
        return myCollisionAction;
    }

    static const long CHANGE_PERMISSIONS_PERMANENT = 0;
    static const long CHANGE_PERMISSIONS_GUI = 1;

protected:
    /// moves myTmpVehicles int myVehicles after a lane change procedure
    virtual void swapAfterLaneChange(SUMOTime t);

    /** @brief Inserts the vehicle into this lane, and informs it about entering the network
     *
     * Calls the vehicles enterLaneAtInsertion function,
     *  updates statistics and modifies the active state as needed
     * @param[in] veh The vehicle to be incorporated
     * @param[in] pos The position of the vehicle
     * @param[in] speed The speed of the vehicle
     * @param[in] posLat The lateral position of the vehicle
     * @param[in] at
     * @param[in] notification The cause of insertion (i.e. departure, teleport, parking) defaults to departure
     */
    virtual void incorporateVehicle(MSVehicle* veh, double pos, double speed, double posLat,
                                    const MSLane::VehCont::iterator& at,
                                    MSMoveReminder::Notification notification = MSMoveReminder::NOTIFICATION_DEPARTED);

    /// @brief detect whether a vehicle collids with pedestrians on the junction
    void detectPedestrianJunctionCollision(const MSVehicle* collider, const PositionVector& colliderBoundary, const MSLane* foeLane,
                                           SUMOTime timestep, const std::string& stage);

    /// @brief detect whether there is a collision between the two vehicles
    bool detectCollisionBetween(SUMOTime timestep, const std::string& stage, MSVehicle* collider, MSVehicle* victim,
                                std::set<const MSVehicle*, ComparatorNumericalIdLess>& toRemove,
                                std::set<const MSVehicle*, ComparatorNumericalIdLess>& toTeleport) const;

    /// @brief take action upon collision
    void handleCollisionBetween(SUMOTime timestep, const std::string& stage, MSVehicle* collider, MSVehicle* victim,
                                double gap, double latGap,
                                std::set<const MSVehicle*, ComparatorNumericalIdLess>& toRemove,
                                std::set<const MSVehicle*, ComparatorNumericalIdLess>& toTeleport) const;

    /// @brief compute maximum braking distance on this lane
    double getMaximumBrakeDist() const;

    /* @brief determine depart speed and whether it may be patched
     * @param[in] veh The departing vehicle
     * @param[out] whether the speed may be patched to account for safety
     * @return the depart speed
     */
    double getDepartSpeed(const MSVehicle& veh, bool& patchSpeed);

    /* @brief determine the lateral depart position
     * @param[in] veh The departing vehicle
     * @return the lateral depart position
     */
    double getDepartPosLat(const MSVehicle& veh);

    /** @brief return the maximum safe speed for insertion behind leaders
     * (a negative value indicates that safe insertion is impossible) */
    double safeInsertionSpeed(const MSVehicle* veh, double seen, const MSLeaderInfo& leaders, double speed);

    /// @brief check whether pedestrians on this lane interfere with vehicle insertion
    bool checkForPedestrians(const MSVehicle* aVehicle, double& speed, double& dist, double pos, bool patchSpeed) const;

    /// Unique numerical ID (set on reading by netload)
    int myNumericalID;

    /// The shape of the lane
    PositionVector myShape;

    /// The lane index
    int myIndex;

    /** @brief The lane's vehicles.
        This container holds all vehicles that have their front (longitudinally)
        and their center (laterally) on this lane.
        These are the vehicles that this lane is 'responsibly' for (i.e. when executing movements)

        The entering vehicles are inserted at the front
        of  this container and the leaving ones leave from the back, e.g. the
        vehicle in front of the junction (often called first) is
        myVehicles.back() (if it exists). And if it is an iterator at a
        vehicle, ++it points to the vehicle in front. This is the interaction
        vehicle. */
    VehCont myVehicles;

    /** @brief The lane's partial vehicles.
        This container holds all vehicles that are partially on this lane but which are
        in myVehicles of another lane.
        Reasons for partial occupancies include the following
        - the back is still on this lane during regular movement
        - the vehicle is performing a continuous lane-change maneuver
        - sub-lane simulation where vehicles can freely move laterally among the lanes of an edge

        The entering vehicles are inserted at the front
        of this container and the leaving ones leave from the back. */
    VehCont myPartialVehicles;

    /** @brief Container for lane-changing vehicles. After completion of lane-change-
        process, the containers will be swapped with myVehicles. */
    VehCont myTmpVehicles;

    /** @brief Buffer for vehicles that moved from their previous lane onto this one.
     * Integrated after all vehicles executed their moves*/
    FXSynchQue<MSVehicle*, std::vector<MSVehicle*> > myVehBuffer;

    /** @brief The vehicles which registered maneuvering into the lane within their current action step.
     *         This is currently only relevant for sublane simulation, since continuous lanechanging
     *         uses the partial vehicle mechanism.
     *
     *   The entering vehicles are inserted at the front
     *   of this container and the leaving ones leave from the back. */
    VehCont myManeuverReservations;

    /* @brief list of vehicles that are parking near this lane
     * (not necessarily on the road but having reached their stop on this lane)
     * */
    std::set<const MSVehicle*> myParkingVehicles;

    /// Lane length [m]
    double myLength;

    /// Lane width [m]
    const double myWidth;

    /// Lane's vClass specific stop offset [m]. The map is either of length 0, which means no
    /// special stopOffset was set, or of length 1, where the key is a bitset representing a subset
    /// of the SUMOVehicleClass Enum and the value is the offset in meters.
    std::map<SVCPermissions, double> myStopOffsets;

    /// The lane's edge, for routing only.
    MSEdge* const myEdge;

    /// Lane-wide speedlimit [m/s]
    double myMaxSpeed;

    /// The vClass permissions for this lane
    SVCPermissions myPermissions;

    /// The original vClass permissions for this lane (before temporary modifications)
    SVCPermissions myOriginalPermissions;

    /// The vClass speed restrictions for this lane
    const std::map<SUMOVehicleClass, double>* myRestrictions;

    /// All direct predecessor lanes
    std::vector<IncomingLaneInfo> myIncomingLanes;

    ///
    mutable MSLane* myLogicalPredecessorLane;

    /// Similar to LogicalPredecessorLane, @see getCanonicalPredecessorLane()
    mutable MSLane* myCanonicalPredecessorLane;

    /// Main successor lane, @see getCanonicalSuccessorLane()
    mutable MSLane* myCanonicalSuccessorLane;

    /// @brief The current length of all vehicles on this lane, including their minGaps
    double myBruttoVehicleLengthSum;

    /// @brief The current length of all vehicles on this lane, excluding their minGaps
    double myNettoVehicleLengthSum;

    /// @brief The length of all vehicles that have left this lane in the current step (this lane, including their minGaps)
    double myBruttoVehicleLengthSumToRemove;

    /// @brief The length of all vehicles that have left this lane in the current step (this lane, excluding their minGaps)
    double myNettoVehicleLengthSumToRemove;

    /** The lane's Links to it's succeeding lanes and the default
        right-of-way rule, i.e. blocked or not blocked. */
    MSLinkCont myLinks;

    /// All direct internal and direct (disregarding internal predecessors) non-internal predecessor lanes of this lane
    std::map<MSEdge*, std::vector<MSLane*> > myApproachingLanes;

    /// @brief leaders on all sublanes as seen by approaching vehicles (cached)
    mutable MSLeaderInfo myLeaderInfo;
    /// @brief followers on all sublanes as seen by vehicles on consecutive lanes (cached)
    mutable MSLeaderInfo myFollowerInfo;

    /// @brief time step for which myLeaderInfo was last updated
    mutable SUMOTime myLeaderInfoTime;
    /// @brief time step for which myFollowerInfo was last updated
    mutable SUMOTime myFollowerInfoTime;

    /// @brief precomputed myShape.length / myLength
    const double myLengthGeometryFactor;

    /// @brief whether this lane is an acceleration lane
    const bool myIsRampAccel;

    /// @brief the type of this lane
    const std::string myLaneType;

    /// @brief the combined width of all lanes with lower index on myEdge
    double myRightSideOnEdge;
    /// @brief the index of the rightmost sublane of this lane on myEdge
    int myRightmostSublane;

    /// @brief whether a collision check is currently needed
    bool myNeedsCollisionCheck;

    // @brief the ids of neighboring lanes
    std::vector<std::string> myNeighs;

    // @brief transient changes in permissions
    std::map<long long, SVCPermissions> myPermissionChanges;

    // @brief index of the associated thread-rng
    int myRNGIndex;

    /// definition of the static dictionary type
    typedef std::map< std::string, MSLane* > DictType;

    /// Static dictionary to associate string-ids with objects.
    static DictType myDict;

    static std::vector<std::mt19937> myRNGs;

private:
    /// @brief This lane's move reminder
    std::vector< MSMoveReminder* > myMoveReminders;

    /// @brief the action to take on collisions
    static CollisionAction myCollisionAction;
    static bool myCheckJunctionCollisions;
    static SUMOTime myCollisionStopTime;
    static double myCollisionMinGapFactor;
    static bool myExtrapolateSubstepDepart;

    /**
     * @class vehicle_position_sorter
     * @brief Sorts vehicles by their position (descending)
     */
    class vehicle_position_sorter {
    public:
        /// @brief Constructor
        explicit vehicle_position_sorter(const MSLane* lane) :
            myLane(lane) {
        }


        /** @brief Comparing operator
         * @param[in] v1 First vehicle to compare
         * @param[in] v2 Second vehicle to compare
         * @return Whether the first vehicle is further on the lane than the second
         */
        int operator()(MSVehicle* v1, MSVehicle* v2) const;

        const MSLane* myLane;

    };

    /**
     * @class vehicle_reverse_position_sorter
     * @brief Sorts vehicles by their position (ascending)
     */
    class vehicle_natural_position_sorter {
    public:
        /// @brief Constructor
        explicit vehicle_natural_position_sorter(const MSLane* lane) :
            myLane(lane) {
        }


        /** @brief Comparing operator
         * @param[in] v1 First vehicle to compare
         * @param[in] v2 Second vehicle to compare
         * @return Whether the first vehicle is further on the lane than the second
         */
        int operator()(MSVehicle* v1, MSVehicle* v2) const;

        const MSLane* myLane;

    };

    /** @class by_connections_to_sorter
     * @brief Sorts edges by their angle relative to the given edge (straight comes first)
     *
     */
    class by_connections_to_sorter {
    public:
        /// @brief constructor
        explicit by_connections_to_sorter(const MSEdge* const e);

        /// @brief comparing operator
        int operator()(const MSEdge* const e1, const MSEdge* const e2) const;

    private:
        by_connections_to_sorter& operator=(const by_connections_to_sorter&); // just to avoid a compiler warning
    private:
        const MSEdge* const myEdge;
        double myLaneDir;
    };



    /** @class incoming_lane_priority_sorter
     * @brief Sorts lanes (IncomingLaneInfos) by their priority or, if this doesn't apply,
     *         wrt. the angle difference magnitude relative to the target lane's angle (straight comes first)
     */
    class incoming_lane_priority_sorter {
    public:
        /// @brief constructor
        explicit incoming_lane_priority_sorter(const MSLane* targetLane);

        /// @brief comparing operator
        int operator()(const IncomingLaneInfo& lane1, const IncomingLaneInfo& lane2) const;

    private:
        incoming_lane_priority_sorter& operator=(const incoming_lane_priority_sorter&); // just to avoid a compiler warning
    private:
        const MSLane* const myLane;
        double myLaneDir;
    };


    /** @class outgoing_lane_priority_sorter
     * @brief Sorts lanes (their origin link) by the priority of their noninternal target edges or, if this doesn't yield an unambiguous result,
     *         wrt. the angle difference magnitude relative to the target lane's angle (straight comes first)
     */
    class outgoing_lane_priority_sorter {
    public:
        /// @brief constructor
        explicit outgoing_lane_priority_sorter(const MSLane* sourceLane);

        /// @brief comparing operator
        int operator()(const MSLink* link1, const MSLink* link2) const;

    private:
        outgoing_lane_priority_sorter& operator=(const outgoing_lane_priority_sorter&); // just to avoid a compiler warning
    private:
        const MSLane* const myLane;
        double myLaneDir;
    };

    /**
     * @class edge_finder
     */
    class edge_finder {
    public:
        edge_finder(MSEdge* e) : myEdge(e) {}
        bool operator()(const IncomingLaneInfo& ili) const {
            return &(ili.lane->getEdge()) == myEdge;
        }
    private:
        edge_finder& operator=(const edge_finder&); // just to avoid a compiler warning
    private:
        const MSEdge* const myEdge;
    };

#ifdef HAVE_FOX
    /// Type of the function that is called for the simulation stage (e.g. planMovements).
    typedef void(MSLane::*Operation)(const SUMOTime);

    /**
     * @class SimulationTask
     * @brief the routing task which mainly calls reroute of the vehicle
     */
    class SimulationTask : public FXWorkerThread::Task {
    public:
        SimulationTask(MSLane& l, const SUMOTime time)
            : myLane(l), myTime(time) {}
        void init(Operation operation, const SUMOTime time) {
            myOperation = operation;
            myTime = time;
        }
        void run(FXWorkerThread* /*context*/) {
            try {
                (myLane.*(myOperation))(myTime);
            } catch (ProcessError& e) {
                WRITE_ERROR(e.what());
            }
        }
    private:
        Operation myOperation = nullptr;
        MSLane& myLane;
        SUMOTime myTime;
    private:
        /// @brief Invalidated assignment operator.
        SimulationTask& operator=(const SimulationTask&) = delete;
    };

    SimulationTask mySimulationTask;
    /// @brief Mutex for access to the cached leader info value
    mutable FXMutex myLeaderInfoMutex;
    /// @brief Mutex for access to the cached follower info value
    mutable FXMutex myFollowerInfoMutex;
    /// @brief Mutex for access to the cached follower info value
    mutable FXMutex myPartialOccupatorMutex;
#endif
private:
    /// @brief invalidated copy constructor
    MSLane(const MSLane&);

    /// @brief invalidated assignment operator
    MSLane& operator=(const MSLane&);


};
