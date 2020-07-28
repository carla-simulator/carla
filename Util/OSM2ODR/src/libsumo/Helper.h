/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Helper.h
/// @author  Robert Hilbrich
/// @author  Leonhard Luecken
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <memory>
#include <libsumo/Subscription.h>
#include <microsim/MSNet.h>


// ===========================================================================
// class declarations
// ===========================================================================
class Position;
class PositionVector;
class RGBColor;
class MSEdge;
class SUMOTrafficObject;
class MSPerson;
class MSVehicle;
class MSBaseVehicle;
class MSVehicleType;


// ===========================================================================
// type definitions
// ===========================================================================
typedef std::map<const MSLane*, std::pair<double, double> >  LaneCoverageInfo; // also declared in MSLane.h!

// ===========================================================================
// class definitions
// ===========================================================================

class LaneStoringVisitor {
public:
    /// @brief Constructor
    LaneStoringVisitor(std::set<const Named*>& objects, const PositionVector& shape,
                       const double range, const int domain)
        : myObjects(objects), myShape(shape), myRange(range), myDomain(domain) {}

    /// @brief Destructor
    ~LaneStoringVisitor() {}

    /// @brief Adds the given object to the container
    void add(const MSLane* const l) const;

    /// @brief The container
    std::set<const Named*>& myObjects;
    const PositionVector& myShape;
    const double myRange;
    const int myDomain;

private:
    /// @brief invalidated copy constructor
    LaneStoringVisitor(const LaneStoringVisitor& src);

    /// @brief invalidated assignment operator
    LaneStoringVisitor& operator=(const LaneStoringVisitor& src);
};

#define LANE_RTREE_QUAL RTree<MSLane*, MSLane, float, 2, LaneStoringVisitor>
template<>
inline float LANE_RTREE_QUAL::RectSphericalVolume(Rect* a_rect) {
    ASSERT(a_rect);
    const float extent0 = a_rect->m_max[0] - a_rect->m_min[0];
    const float extent1 = a_rect->m_max[1] - a_rect->m_min[1];
    return .78539816f * (extent0 * extent0 + extent1 * extent1);
}

template<>
inline LANE_RTREE_QUAL::Rect LANE_RTREE_QUAL::CombineRect(Rect* a_rectA, Rect* a_rectB) {
    ASSERT(a_rectA && a_rectB);
    Rect newRect;
    newRect.m_min[0] = rtree_min(a_rectA->m_min[0], a_rectB->m_min[0]);
    newRect.m_max[0] = rtree_max(a_rectA->m_max[0], a_rectB->m_max[0]);
    newRect.m_min[1] = rtree_min(a_rectA->m_min[1], a_rectB->m_min[1]);
    newRect.m_max[1] = rtree_max(a_rectA->m_max[1], a_rectB->m_max[1]);
    return newRect;
}

namespace libsumo {

/**
* @class Helper
* @brief C++ TraCI client API implementation
*/
class Helper {
public:
    static void subscribe(const int commandId, const std::string& id, const std::vector<int>& variables,
                          const double beginTime, const double endTime, const int contextDomain = 0, const double range = 0.);

    static void addSubscriptionParam(double param);

    static void handleSubscriptions(const SUMOTime t);

    static bool needNewSubscription(libsumo::Subscription& s, std::vector<Subscription>& subscriptions, libsumo::Subscription*& modifiedSubscription);

    static void clearSubscriptions();

    static Subscription* addSubscriptionFilter(SubscriptionFilterType filter);

    /// @brief helper functions
    static TraCIPositionVector makeTraCIPositionVector(const PositionVector& positionVector);
    static TraCIPosition makeTraCIPosition(const Position& position, const bool includeZ = false);
    static Position makePosition(const TraCIPosition& position);

    static PositionVector makePositionVector(const TraCIPositionVector& vector);
    static TraCIColor makeTraCIColor(const RGBColor& color);
    static RGBColor makeRGBColor(const TraCIColor& color);

    static MSEdge* getEdge(const std::string& edgeID);
    static const MSLane* getLaneChecking(const std::string& edgeID, int laneIndex, double pos);
    static std::pair<MSLane*, double> convertCartesianToRoadMap(const Position& pos, const SUMOVehicleClass vClass);
    static double getDrivingDistance(std::pair<const MSLane*, double>& roadPos1, std::pair<const MSLane*, double>& roadPos2);

    static MSBaseVehicle* getVehicle(const std::string& id);
    static MSPerson* getPerson(const std::string& id);
    static SUMOTrafficObject* getTrafficObject(int domain, const std::string& id);
    static const MSVehicleType& getVehicleType(const std::string& vehicleID);

    static void findObjectShape(int domain, const std::string& id, PositionVector& shape);

    static void collectObjectsInRange(int domain, const PositionVector& shape, double range, std::set<const Named*>& into);
    static void collectObjectIDsInRange(int domain, const PositionVector& shape, double range, std::set<std::string>& into);

    /**
     * @brief Filter the given ID-Set (which was obtained from an R-Tree search)
     *        according to the filters set by the subscription or firstly build the object ID list if
     *        the filters rather demand searching along the road network than considering a geometric range.
     * @param[in] s Subscription which holds the filter specification to be applied
     * @param[in/out] objIDs Set of object IDs that is to be filtered. Result is stored in place.
     * @note Currently this assumes that the objects are vehicles.
     */
    static void applySubscriptionFilters(const Subscription& s, std::set<std::string>& objIDs);

    static void applySubscriptionFilterFieldOfVision(const Subscription& s, std::set<std::string>& objIDs);

    static void applySubscriptionFilterLateralDistanceSinglePass(const Subscription& s,
            std::set<std::string>& objIDs,
            std::set<const SUMOTrafficObject*>& vehs,
            const std::vector<const MSLane*>& lanes,
            double posOnLane, double posLat, bool isDownstream);

    static void setRemoteControlled(MSVehicle* v, Position xyPos, MSLane* l, double pos, double posLat, double angle,
                                    int edgeOffset, ConstMSEdgeVector route, SUMOTime t);

    static void setRemoteControlled(MSPerson* p, Position xyPos, MSLane* l, double pos, double posLat, double angle,
                                    int edgeOffset, ConstMSEdgeVector route, SUMOTime t);

    static void postProcessRemoteControl();

    static void cleanup();

    static void registerVehicleStateListener();

    static const std::vector<std::string>& getVehicleStateChanges(const MSNet::VehicleState state);

    static void clearVehicleStates();

    /// @name functions for moveToXY
    /// @{
    static bool moveToXYMap(const Position& pos, double maxRouteDistance, bool mayLeaveNetwork, const std::string& origID,
                            const double angle, double speed, const ConstMSEdgeVector& currentRoute, const int routePosition,
                            MSLane* currentLane, double currentLanePos, bool onRoad, SUMOVehicleClass vClass, bool setLateralPos,
                            double& bestDistance, MSLane** lane, double& lanePos, int& routeOffset, ConstMSEdgeVector& edges);

    static bool moveToXYMap_matchingRoutePosition(const Position& pos, const std::string& origID,
            const ConstMSEdgeVector& currentRoute, int routeIndex,
            SUMOVehicleClass vClass, bool setLateralPos,
            double& bestDistance, MSLane** lane, double& lanePos, int& routeOffset);

    static bool findCloserLane(const MSEdge* edge, const Position& pos, SUMOVehicleClass vClass, double& bestDistance, MSLane** lane);

    class LaneUtility {
    public:
        LaneUtility(double dist_, double perpendicularDist_, double lanePos_, double angleDiff_, bool ID_,
                    bool onRoute_, bool sameEdge_, const MSEdge* prevEdge_, const MSEdge* nextEdge_) :
            dist(dist_), perpendicularDist(perpendicularDist_), lanePos(lanePos_), angleDiff(angleDiff_), ID(ID_),
            onRoute(onRoute_), sameEdge(sameEdge_), prevEdge(prevEdge_), nextEdge(nextEdge_) {}
        ~LaneUtility() {}

        double dist;
        double perpendicularDist;
        double lanePos;
        double angleDiff;
        bool ID;
        bool onRoute;
        bool sameEdge;
        const MSEdge* prevEdge;
        const MSEdge* nextEdge;
    };
    /// @}

    class SubscriptionWrapper final : public VariableWrapper {
    public:
        SubscriptionWrapper(VariableWrapper::SubscriptionHandler handler, SubscriptionResults& into, ContextSubscriptionResults& context);
        void setContext(const std::string& refID);
        void setParams(const std::vector<unsigned char>* params);
        const std::vector<unsigned char>* getParams() const {
            return myParams;
        }
        void clear();
        bool wrapDouble(const std::string& objID, const int variable, const double value);
        bool wrapInt(const std::string& objID, const int variable, const int value);
        bool wrapString(const std::string& objID, const int variable, const std::string& value);
        bool wrapStringList(const std::string& objID, const int variable, const std::vector<std::string>& value);
        bool wrapPosition(const std::string& objID, const int variable, const TraCIPosition& value);
        bool wrapColor(const std::string& objID, const int variable, const TraCIColor& value);
        bool wrapRoadPosition(const std::string& objID, const int variable, const TraCIRoadPosition& value);
    private:
        SubscriptionResults& myResults;
        ContextSubscriptionResults& myContextResults;
        SubscriptionResults* myActiveResults;
        const std::vector<unsigned char>* myParams = nullptr;
    private:
        /// @brief Invalidated assignment operator
        SubscriptionWrapper& operator=(const SubscriptionWrapper& s) = delete;
    };

private:
    static void handleSingleSubscription(const Subscription& s);

    /// @brief Adds lane coverage information from newLaneCoverage into aggregatedLaneCoverage
    /// @param[in/out] aggregatedLaneCoverage - aggregated lane coverage info, to which the new will be added
    /// @param[in] newLaneCoverage - new lane coverage to be added
    /// @todo Disjunct ranges are not handled (LaneCoverageInfo definition would need to allow several intervals per lane) but
    ///       the intermediate range is simply assimilated.
    static void fuseLaneCoverage(std::shared_ptr<LaneCoverageInfo> aggregatedLaneCoverage, const std::shared_ptr<LaneCoverageInfo> newLaneCoverage);

    static void debugPrint(const SUMOTrafficObject* veh);

private:
    class VehicleStateListener : public MSNet::VehicleStateListener {
    public:
        void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info = "");
        /// @brief Changes in the states of simulated vehicles
        std::map<MSNet::VehicleState, std::vector<std::string> > myVehicleStateChanges;
    };

    /// @brief The list of known, still valid subscriptions
    static std::vector<Subscription> mySubscriptions;

    /// @brief The last context subscription
    static Subscription* myLastContextSubscription;

    /// @brief Map of commandIds -> their executors; applicable if the executor applies to the method footprint
    static std::map<int, std::shared_ptr<VariableWrapper> > myWrapper;

    /// @brief Changes in the states of simulated vehicles
    static VehicleStateListener myVehicleStateListener;

    /// @brief A storage of lanes
    static LANE_RTREE_QUAL* myLaneTree;

    static std::map<std::string, MSVehicle*> myRemoteControlledVehicles;
    static std::map<std::string, MSPerson*> myRemoteControlledPersons;

    /// @brief invalidated standard constructor
    Helper() = delete;
};

}
