/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel_Striping.h
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model (prototype)
/****************************************************************************/
#pragma once
#include <config.h>

#include <string>
#include <limits>
#include <utils/common/SUMOTime.h>
#include <utils/common/Command.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSLane.h>
#include "MSPerson.h"
#include "MSPModel.h"

// ===========================================================================
// class declarations
// ===========================================================================
class MSNet;
class MSLink;
class MSJunction;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSPModel_Striping
 * @brief The pedestrian following model
 *
 */
class MSPModel_Striping : public MSPModel {

    friend class GUIPerson; // for debugging

public:

    /// @brief Constructor (it should not be necessary to construct more than one instance)
    MSPModel_Striping(const OptionsCont& oc, MSNet* net);

    ~MSPModel_Striping();

    /// @brief register the given person as a pedestrian
    MSTransportableStateAdapter* add(MSTransportable* transportable, MSStageMoving* stage, SUMOTime now);

    /// @brief remove the specified person from the pedestrian simulation
    void remove(MSTransportableStateAdapter* state);

    /** @brief whether a pedestrian is blocking the crossing of lane for the given vehicle bondaries
     * @param[in] lane The crossing to check
     * @param[in] vehside The offset to the vehicle side near the start of the crossing
     * @param[in] vehWidth The width of the vehicle
     * @param[in] oncomingGap The distance which the vehicle wants to keep from oncoming pedestrians
     * @param[in] collectBlockers The list of persons blocking the crossing
     * @return Whether the vehicle must wait
     */
    bool blockedAtDist(const MSLane* lane, double vehSide, double vehWidth,
                       double oncomingGap, std::vector<const MSPerson*>* collectBlockers);

    /// @brief whether the given lane has pedestrians on it
    bool hasPedestrians(const MSLane* lane);

    /// @brief whether movements on intersections are modelled
    //// @note function declared as member for sake of inheritance (delegates to static function)
    bool usingInternalLanes();

    /// @brief returns the next pedestrian beyond minPos that is laterally between minRight and maxLeft or 0
    PersonDist nextBlocking(const MSLane* lane, double minPos, double minRight, double maxLeft, double stopTime = 0);

    /// @brief model parameters
    ///@{

    // @brief the width of a pedstrian stripe
    static double stripeWidth;

    // @brief the factor for random slow-down
    static double dawdling;

    // @brief the time threshold before becoming jammed
    static SUMOTime jamTime;
    static SUMOTime jamTimeCrossing;

    // @brief the distance (in seconds) to look ahead for changing stripes
    static const double LOOKAHEAD_SAMEDIR;
    // @brief the distance (in seconds) to look ahead for changing stripes (regarding oncoming pedestrians)
    static const double LOOKAHEAD_ONCOMING;
    // @brief the distance (in m) to look around for vehicles
    static const double LOOKAROUND_VEHICLES;

    // @brief the utility penalty for moving sideways (corresponds to meters)
    static const double LATERAL_PENALTY;

    // @brief the utility penalty for obstructed (physically blocking me) stripes (corresponds to meters)
    static const double OBSTRUCTED_PENALTY;

    // @brief the utility penalty for inappropriate (reserved for oncoming traffic or may violate my min gap) stripes (corresponds to meters)
    static const double INAPPROPRIATE_PENALTY;

    // @brief the utility penalty for oncoming conflicts on stripes (corresponds to meters)
    static const double ONCOMING_CONFLICT_PENALTY;

    // @brief the minimum utility that indicates obstruction
    static const double OBSTRUCTION_THRESHOLD;

    // @brief the factor by which pedestrian width is reduced when sqeezing past each other
    static const double SQUEEZE;

    // @brief fraction of the leftmost lanes to reserve for oncoming traffic
    static double RESERVE_FOR_ONCOMING_FACTOR;
    static double RESERVE_FOR_ONCOMING_FACTOR_JUNCTIONS;

    // @brief the time pedestrians take to reach maximum impatience
    static const double MAX_WAIT_TOLERANCE;

    // @brief the fraction of forward speed to be used for lateral movemenk
    static const double LATERAL_SPEED_FACTOR;

    // @brief the minimum distance to the next obstacle in order to start walking after stopped
    static const double MIN_STARTUP_DIST;

    ///@}


protected:
    static const double DIST_FAR_AWAY;
    static const double DIST_BEHIND;
    static const double DIST_OVERLAP;

    class lane_by_numid_sorter {
    public:
        /// comparing operation
        bool operator()(const MSLane* l1, const MSLane* l2) const {
            return l1->getNumericalID() < l2->getNumericalID();
        }
    };

    struct Obstacle;
    struct WalkingAreaPath;
    class PState;
    typedef std::vector<PState*> Pedestrians;
    typedef std::map<const MSLane*, Pedestrians, lane_by_numid_sorter> ActiveLanes;
    typedef std::vector<Obstacle> Obstacles;
    typedef std::map<const MSLane*, Obstacles, lane_by_numid_sorter> NextLanesObstacles;
    typedef std::map<std::pair<const MSLane*, const MSLane*>, const WalkingAreaPath> WalkingAreaPaths;
    typedef std::map<const MSLane*, double> MinNextLengths;

    struct NextLaneInfo {
        NextLaneInfo(const MSLane* _lane, const MSLink* _link, int _dir) :
            lane(_lane),
            link(_link),
            dir(_dir) {
        }

        NextLaneInfo() :
            lane(0),
            link(0),
            dir(UNDEFINED_DIRECTION) {
        }

        // @brief the next lane to be used
        const MSLane* lane;
        // @brief the link from the current lane to the next lane
        const MSLink* link;
        // @brief the direction on the next lane
        int dir;
    };

    enum ObstacleType {
        OBSTACLE_NONE = 0,
        OBSTACLE_PED = 1,
        OBSTACLE_VEHICLE = 3,
        OBSTACLE_END = 4,
        OBSTACLE_NEXTEND = 5,
        OBSTACLE_LINKCLOSED = 6,
        OBSTACLE_ARRIVALPOS = 7
    };

    /// @brief information regarding surround Pedestrians (and potentially other things)
    struct Obstacle {
        /// @brief create No-Obstacle
        Obstacle(int dir, double dist = DIST_FAR_AWAY);
        /// @brief create an obstacle from ped for ego moving in dir
        Obstacle(const PState& ped);
        /// @brief create an obstacle from explict values
        Obstacle(double _x, double _speed, ObstacleType _type, const std::string& _description, const double width = 0.)
            : xFwd(_x + width / 2.), xBack(_x - width / 2.), speed(_speed), type(_type), description(_description) {};

        /// @brief maximal position on the current lane in forward direction
        double xFwd;
        /// @brief maximal position on the current lane in backward direction
        double xBack;
        /// @brief speed relative to lane direction (positive means in the same direction)
        double speed;
        /// @brief whether this obstacle denotes a border or a pedestrian
        ObstacleType type;
        /// @brief the id / description of the obstacle
        std::string description;
    };

    struct WalkingAreaPath {
        WalkingAreaPath(const MSLane* _from, const MSLane* _walkingArea, const MSLane* _to, const PositionVector& _shape, int _dir) :
            from(_from),
            to(_to),
            lane(_walkingArea),
            shape(_shape),
            dir(_dir),
            length(_shape.length()) {
        }

        const MSLane* const from;
        const MSLane* const to;
        const MSLane* const lane; // the walkingArea;
        const PositionVector shape;
        const int dir; // the direction when entering this path
        const double length;

    private:
        /// @brief Invalidated assignment operator
        WalkingAreaPath& operator=(const WalkingAreaPath& s) = delete;

    };

    class walkingarea_path_sorter {
    public:
        /// comparing operation
        bool operator()(const WalkingAreaPath* p1, const WalkingAreaPath* p2) const {
            if (p1->from->getNumericalID() < p2->from->getNumericalID()) {
                return true;
            }
            if (p1->from->getNumericalID() == p2->from->getNumericalID()) {
                if (p1->to->getNumericalID() < p2->to->getNumericalID()) {
                    return true;
                }
            }
            return false;
        }
    };


    /**
     * @class PState
     * @brief Container for pedestrian state and individual position update function
     */
    class PState : public MSTransportableStateAdapter {
    public:

        /// @brief abstract methods inherited from PedestrianState
        /// @{
        double getEdgePos(const MSStageMoving& stage, SUMOTime now) const;
        Position getPosition(const MSStageMoving& stage, SUMOTime now) const;
        double getAngle(const MSStageMoving& stage, SUMOTime now) const;
        SUMOTime getWaitingTime(const MSStageMoving& stage, SUMOTime now) const;
        double getSpeed(const MSStageMoving& stage) const;
        const MSEdge* getNextEdge(const MSStageMoving& stage) const;
        void moveToXY(MSPerson* p, Position pos, MSLane* lane, double lanePos,
                      double lanePosLat, double angle, int routeOffset,
                      const ConstMSEdgeVector& edges, SUMOTime t);
        /// @brief whether the transportable is jammed
        bool isJammed() const;
        const MSLane* getLane() const;
        /// @}

        PState(MSPerson* person, MSStageMoving* stage, const MSLane* lane);


        ~PState() {};
        MSPerson* myPerson;
        MSStageMoving* myStage;
        /// @brief the current lane of this pedestrian
        const MSLane* myLane;
        /// @brief the advancement along the current lane
        double myRelX;
        /// @brief the orthogonal shift on the current lane
        double myRelY;
        /// @brief the walking direction on the current lane (1 forward, -1 backward)
        int myDir;
        /// @brief the current walking speed
        double mySpeed;
        /// @brief whether the pedestrian is waiting to start its walk
        bool myWaitingToEnter;
        /// @brief the consecutive time spent at speed 0
        SUMOTime myWaitingTime;
        /// @brief information about the upcoming lane
        NextLaneInfo myNLI;
        /// @brief the current walkingAreaPath or 0
        const WalkingAreaPath* myWalkingAreaPath;
        /// @brief whether the person is jammed
        bool myAmJammed;
        /// @brief remote-controlled position
        Position myRemoteXYPos;
        /// @brief cached angle
        mutable double myAngle;

        /// @brief return the minimum position on the lane
        virtual double getMinX(const bool includeMinGap = true) const;

        /// @brief return the maximum position on the lane
        virtual double getMaxX(const bool includeMinGap = true) const;

        /// @brief return the length of the pedestrian
        double getLength() const;

        /// @brief return the minimum gap of the pedestrian
        double getMinGap() const;

        /// @brief the absolute distance to the end of the lane in walking direction (or to the arrivalPos)
        double distToLaneEnd() const;

        /// @brief return whether this pedestrian has passed the end of the current lane and update myRelX if so
        bool moveToNextLane(SUMOTime currentTime);

        /// @brief perform position update
        void walk(const Obstacles& obs, SUMOTime currentTime);

        /// @brief returns the impatience
        double getImpatience(SUMOTime now) const;

        int stripe() const;
        int otherStripe() const;

        static int stripe(const double relY);
        int otherStripe(const double relY) const;

        /* @brief calculate distance to the given obstacle,
         * - non-negative values signify an obstacle in front of ego
         * the special values DIST_OVERLAP and DIST_BEHIND are used to signify
         * obstacles that overlap and obstacles behind ego respectively
         * the result is the same regardless of walking direction
         */
        double distanceTo(const Obstacle& obs, const bool includeMinGap = true) const;

        /// @brief replace obstacles in the first vector with obstacles from the second if they are closer to me
        void mergeObstacles(Obstacles& into, const Obstacles& obs2);

        /// @brief replace obstacles in the first vector with obstacles from the second if they are closer in the given direction
        static void mergeObstacles(Obstacles& into, const Obstacles& obs2, int dir, int offset);

        /// @brief whether the pedestrian may ignore a red light
        bool ignoreRed(const MSLink* link) const;

        /// @brief return the person id
        virtual const std::string& getID() const;

        /// @brief return the person width
        virtual double getWidth() const;

        /// @brief whether the person is currently being controlled via TraCI
        bool isRemoteControlled() const;

    protected:
        /// @brief constructor for PStateVehicle
        PState();
    };

    class PStateVehicle : public PState {
    public:
        PStateVehicle(const MSVehicle* veh, const MSLane* walkingarea, double relX, double relY);
        const std::string& getID() const;
        double getMinX(const bool includeMinGap = true) const;
        double getMaxX(const bool includeMinGap = true) const;
        double getWidth() const;
    private:
        const MSVehicle* myVehicle;
    };


    class MovePedestrians : public Command {
    public:
        MovePedestrians(MSPModel_Striping* model) : myModel(model) {};
        ~MovePedestrians() {};
        SUMOTime execute(SUMOTime currentTime);
    private:
        MSPModel_Striping* const myModel;
    private:
        /// @brief Invalidated assignment operator.
        MovePedestrians& operator=(const MovePedestrians&);
    };

    /// @brief sorts the persons by position on the lane. If dir is forward, higher x positions come first.
    class by_xpos_sorter {
    public:
        /// constructor
        by_xpos_sorter(int dir): myDir(dir) {}

    public:
        /// comparing operation
        bool operator()(const PState* p1, const PState* p2) const {
            if (p1->myRelX != p2->myRelX) {
                return myDir * p1->myRelX > myDir * p2->myRelX;
            }
            return p1->getID() < p2->getID();
        }

    private:
        const int myDir;

    private:
        /// @brief Invalidated assignment operator.
        by_xpos_sorter& operator=(const by_xpos_sorter&);
    };


    /// @brief move all pedestrians forward and advance to the next lane if applicable
    void moveInDirection(SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir);

    /// @brief move pedestrians forward on one lane
    void moveInDirectionOnLane(Pedestrians& pedestrians, const MSLane* lane, SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir);

    /// @brief handle arrivals and lane advancement
    void arriveAndAdvance(Pedestrians& pedestrians, SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir);

    const ActiveLanes& getActiveLanes() {
        return myActiveLanes;
    }

    /// @brief return the number of active objects
    int getActiveNumber() {
        return myNumActivePedestrians;
    }

    void registerActive() {
        myNumActivePedestrians++;
    }

private:
    static void DEBUG_PRINT(const Obstacles& obs);

    /// @brief returns the direction in which these lanes are connectioned or 0 if they are not
    static int connectedDirection(const MSLane* from, const MSLane* to);

    /** @brief computes the successor lane for the given pedestrian and sets the
     * link as well as the direction to use on the succesor lane
     * @param[in] currentLane The lane the pedestrian is currently on
     * @param[in] ped The pedestrian for which to compute the next lane
     */
    static NextLaneInfo getNextLane(const PState& ped, const MSLane* currentLane, const MSLane* prevLane);

    /// @brief return the next walkingArea in the given direction
    static const MSLane* getNextWalkingArea(const MSLane* currentLane, const int dir, MSLink*& link);

    static void initWalkingAreaPaths(const MSNet* net);

    static const WalkingAreaPath* getWalkingAreaPath(const MSEdge* walkingArea, const MSLane* before, const MSLane* after);

    /// @brief return an arbitrary path across the given walkingArea
    static const WalkingAreaPath* getArbitraryPath(const MSEdge* walkingArea);

    static const WalkingAreaPath* guessPath(const MSEdge* walkingArea, const MSEdge* before, const MSEdge* after);

    /// @brief return the maximum number of pedestrians walking side by side
    static int numStripes(const MSLane* lane);

    static Obstacles getNeighboringObstacles(const Pedestrians& pedestrians, int egoIndex, int stripes);

    const Obstacles& getNextLaneObstacles(NextLanesObstacles& nextLanesObs, const MSLane* lane, const MSLane* nextLane, int stripes,
                                          int nextDir, double currentLength, int currentDir);

    static void transformToCurrentLanePositions(Obstacles& o, int currentDir, int nextDir, double currentLength, double nextLength);

    static void addCloserObstacle(Obstacles& obs, double x, int stripe, int numStripes, const std::string& id, double width, int dir, ObstacleType type);

    /// @brief retrieves the pedestian vector for the given lane (may be empty)
    Pedestrians& getPedestrians(const MSLane* lane);

    /* @brief compute stripe-offset to transform relY values from a lane with origStripes into a lane wit destStrips
     * @note this is called once for transforming nextLane peds to into the current system as obstacles and another time
     * (in reverse) to transform the pedestrian coordinates into the nextLane-coordinates when changing lanes
     */
    static int getStripeOffset(int origStripes, int destStripes, bool addRemainder);

    ///@brief add vehicles driving across
    static bool addCrossingVehs(const MSLane* crossing, int stripes, double lateral_offset, int dir, Obstacles& crossingVehs, bool prio);

    ///@brief retrieve vehicle obstacles on the given lane
    static Obstacles getVehicleObstacles(const MSLane* lane, int dir, PState* ped = 0);

    static bool usingInternalLanesStatic();

    static bool addVehicleFoe(const MSVehicle* veh, const MSLane* walkingarea, const Position& relPos, double lateral_offset,
                              double minY, double maxY, Pedestrians& toDelete, Pedestrians& transformedPeds);

private:
    /// @brief the total number of active pedestrians
    int myNumActivePedestrians;

    /// @brief store of all lanes which have pedestrians on them
    ActiveLanes myActiveLanes;

    /// @brief whether an event for pedestrian processing was added
    bool myAmActive;

    /// @brief store for walkinArea elements
    static WalkingAreaPaths myWalkingAreaPaths;
    static std::map<const MSEdge*, std::vector<const MSLane*> > myWalkingAreaFoes;
    static MinNextLengths myMinNextLengths;

    /// @brief empty pedestrian vector
    static Pedestrians noPedestrians;

};



