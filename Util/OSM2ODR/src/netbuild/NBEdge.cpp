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
/// @file    NBEdge.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Leonhard Luecken
/// @date    Tue, 20 Nov 2001
///
// Methods for the representation of a single edge
/****************************************************************************/
#include <config.h>

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StdDefs.h>
#include <utils/geom/GeomHelper.h>
#include <utils/options/OptionsCont.h>
#include "NBEdgeCont.h"
#include "NBNode.h"
#include "NBNodeCont.h"
#include "NBContHelper.h"
#include "NBHelpers.h"
#include "NBTrafficLightDefinition.h"
#include "NBTypeCont.h"
#include "NBEdge.h"

//#define ADDITIONAL_WARNINGS
//#define DEBUG_CONNECTION_GUESSING
//#define DEBUG_ANGLES
//#define DEBUG_NODE_BORDER
//#define DEBUG_REPLACECONNECTION
#define DEBUGID ""
#define DEBUGCOND (getID() == DEBUGID)
//#define DEBUGCOND (StringUtils::startsWith(getID(), DEBUGID))
//#define DEBUGCOND (getID() == "22762377#1" || getID() == "146511467")
#define DEBUGCOND2(obj) ((obj != 0 && (obj)->getID() == DEBUGID))
//#define DEBUGCOND (true)

// ===========================================================================
// static members
// ===========================================================================
const double NBEdge::UNSPECIFIED_WIDTH = -1;
const double NBEdge::UNSPECIFIED_OFFSET = 0;
const double NBEdge::UNSPECIFIED_SPEED = -1;
const double NBEdge::UNSPECIFIED_CONTPOS = -1;
const double NBEdge::UNSPECIFIED_VISIBILITY_DISTANCE = -1;

const double NBEdge::UNSPECIFIED_SIGNAL_OFFSET = -1;
const double NBEdge::UNSPECIFIED_LOADED_LENGTH = -1;
const double NBEdge::ANGLE_LOOKAHEAD = 10.0;
const int NBEdge::UNSPECIFIED_INTERNAL_LANE_INDEX = -1;
const bool NBEdge::UNSPECIFIED_CONNECTION_UNCONTROLLED = false;

double NBEdge::myDefaultConnectionLength = NBEdge::UNSPECIFIED_LOADED_LENGTH;

NBEdge NBEdge::DummyEdge;

ConstRouterEdgePairVector NBEdge::Connection::myViaSuccessors = ConstRouterEdgePairVector({ std::pair<NBRouterEdge*, NBRouterEdge*>(nullptr, nullptr) });

// ===========================================================================
// method definitions
// ===========================================================================
std::string
NBEdge::Connection::getInternalLaneID() const {
    return id + "_" + toString(internalLaneIndex);
}


std::string
NBEdge::Connection::getDescription(const NBEdge* parent) const {
    return Named::getIDSecure(parent) + "_" + toString(fromLane) + "->" + Named::getIDSecure(toEdge) + "_" + toString(toLane);
}


NBEdge::Connection::Connection(int fromLane_, NBEdge* toEdge_, int toLane_) :
    fromLane(fromLane_),
    toEdge(toEdge_),
    toLane(toLane_),
    tlLinkIndex(-1),
    tlLinkIndex2(-1),
    mayDefinitelyPass(false),
    keepClear(true),
    contPos(UNSPECIFIED_CONTPOS),
    visibility(UNSPECIFIED_VISIBILITY_DISTANCE),
    speed(UNSPECIFIED_SPEED),
    customLength(myDefaultConnectionLength),
    permissions(SVC_UNSPECIFIED),
    id(toEdge_ == nullptr ? "" : toEdge->getFromNode()->getID()),
    haveVia(false),
    internalLaneIndex(UNSPECIFIED_INTERNAL_LANE_INDEX),
    uncontrolled(false) {
}


NBEdge::Connection::Connection(int fromLane_, NBEdge* toEdge_, int toLane_, bool mayDefinitelyPass_, bool keepClear_, double contPos_,
                               double visibility_, double speed_, double length_, bool haveVia_, bool uncontrolled_, const PositionVector& customShape_, SVCPermissions permissions_) :
    fromLane(fromLane_),
    toEdge(toEdge_),
    toLane(toLane_),
    tlLinkIndex(-1),
    tlLinkIndex2(-1),
    mayDefinitelyPass(mayDefinitelyPass_),
    keepClear(keepClear_),
    contPos(contPos_),
    visibility(visibility_),
    speed(speed_),
    customLength(length_),
    customShape(customShape_),
    permissions(permissions_),
    id(toEdge_ == nullptr ? "" : toEdge->getFromNode()->getID()),
    vmax(UNSPECIFIED_SPEED),
    haveVia(haveVia_),
    internalLaneIndex(UNSPECIFIED_INTERNAL_LANE_INDEX),
    uncontrolled(uncontrolled_)
{ }


NBEdge::Lane::Lane(NBEdge* e, const std::string& origID_) :
    speed(e->getSpeed()),
    permissions(SVCAll),
    preferred(0),
    endOffset(e->getEndOffset()),
    stopOffsets(e->getStopOffsets()),
    width(e->getLaneWidth()),
    accelRamp(false),
    connectionsDone(false) {
    if (origID_ != "") {
        setParameter(SUMO_PARAM_ORIGID, origID_);
    }
}


/* -------------------------------------------------------------------------
 * NBEdge::ToEdgeConnectionsAdder-methods
 * ----------------------------------------------------------------------- */
void
NBEdge::ToEdgeConnectionsAdder::execute(const int lane, const int virtEdge) {
    // check
    assert((int)myTransitions.size() > virtEdge);
    // get the approached edge
    NBEdge* succEdge = myTransitions[virtEdge];
    std::vector<int> lanes;

    // check whether the currently regarded, approached edge has already
    //  a connection starting at the edge which is currently being build
    std::map<NBEdge*, std::vector<int> >::iterator i = myConnections.find(succEdge);
    if (i != myConnections.end()) {
        // if there were already lanes assigned, get them
        lanes = (*i).second;
    }

    // check whether the current lane was already used to connect the currently
    //  regarded approached edge
    std::vector<int>::iterator j = std::find(lanes.begin(), lanes.end(), lane);
    if (j == lanes.end()) {
        // if not, add it to the list
        lanes.push_back(lane);
    }
    // set information about connecting lanes
    myConnections[succEdge] = lanes;
}



/* -------------------------------------------------------------------------
 * NBEdge::MainDirections-methods
 * ----------------------------------------------------------------------- */
NBEdge::MainDirections::MainDirections(const EdgeVector& outgoing, NBEdge* parent, NBNode* to, const std::vector<int>& availableLanes) : myStraightest(-1) {
    NBContHelper::edge_similar_direction_sorter sorter(parent);
    const NBEdge* straight = nullptr;
    for (const NBEdge* const out : outgoing) {
        const int outPerms = out->getPermissions();
        for (const int l : availableLanes) {
            if ((parent->myLanes[l].permissions & outPerms) != 0) {
                if (straight == nullptr || sorter(out, straight)) {
                    straight = out;
                }
                break;
            }
        }
    }
    if (straight == nullptr) {
        return;
    }
    myStraightest = (int)std::distance(outgoing.begin(), std::find(outgoing.begin(), outgoing.end(), straight));

    // check whether the right turn has a higher priority
    assert(outgoing.size() > 0);
    const LinkDirection straightestDir = to->getDirection(parent, straight);
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND2(parent)) {
        std::cout << " MainDirections edge=" << parent->getID() << " straightest=" << straight->getID() << " dir=" << toString(straightestDir) << "\n";
    }
#endif
    if (NBNode::isTrafficLight(to->getType()) &&
            (straightestDir == LinkDirection::STRAIGHT || straightestDir == LinkDirection::PARTLEFT || straightestDir == LinkDirection::PARTRIGHT)) {
        myDirs.push_back(MainDirections::Direction::FORWARD);
        return;
    }
    if (outgoing[0]->getJunctionPriority(to) == 1) {
        myDirs.push_back(MainDirections::Direction::RIGHTMOST);
    }
    // check whether the left turn has a higher priority
    if (outgoing.back()->getJunctionPriority(to) == 1) {
        // ok, the left turn belongs to the higher priorised edges on the junction
        //  let's check, whether it has also a higher priority (lane number/speed)
        //  than the current
        if (outgoing.back()->getPriority() > straight->getPriority()) {
            myDirs.push_back(MainDirections::Direction::LEFTMOST);
        } else {
            if (outgoing.back()->getNumLanes() > straight->getNumLanes()) {
                myDirs.push_back(MainDirections::Direction::LEFTMOST);
            }
        }
    }
    // check whether the forward direction has a higher priority
    // check whether it has a higher priority and is going straight
    if (straight->getJunctionPriority(to) == 1 && to->getDirection(parent, straight) == LinkDirection::STRAIGHT) {
        myDirs.push_back(MainDirections::Direction::FORWARD);
    }
}


NBEdge::MainDirections::~MainDirections() {}


bool
NBEdge::MainDirections::empty() const {
    return myDirs.empty();
}


bool
NBEdge::MainDirections::includes(Direction d) const {
    return std::find(myDirs.begin(), myDirs.end(), d) != myDirs.end();
}


/* -------------------------------------------------------------------------
 * NBEdge::connections_relative_edgelane_sorter-methods
 * ----------------------------------------------------------------------- */
int
NBEdge::connections_relative_edgelane_sorter::operator()(const Connection& c1, const Connection& c2) const {
    if (c1.toEdge != c2.toEdge) {
        return NBContHelper::relative_outgoing_edge_sorter(myEdge)(c1.toEdge, c2.toEdge);
    }
    return c1.toLane < c2.toLane;
}


/* -------------------------------------------------------------------------
 * NBEdge-methods
 * ----------------------------------------------------------------------- */
NBEdge::NBEdge(const std::string& id, NBNode* from, NBNode* to,
               std::string type, double speed, int nolanes,
               int priority, double laneWidth, double endOffset,
               const std::string& streetName,
               LaneSpreadFunction spread) :
    Named(StringUtils::convertUmlaute(id)),
    myStep(EdgeBuildingStep::INIT),
    myType(StringUtils::convertUmlaute(type)),
    myFrom(from), myTo(to),
    myStartAngle(0), myEndAngle(0), myTotalAngle(0),
    myPriority(priority), mySpeed(speed),
    myDistance(0),
    myTurnDestination(nullptr),
    myPossibleTurnDestination(nullptr),
    myFromJunctionPriority(-1), myToJunctionPriority(-1),
    myLaneSpreadFunction(spread), myEndOffset(endOffset),
    myStopOffsets(),
    myLaneWidth(laneWidth),
    myLoadedLength(UNSPECIFIED_LOADED_LENGTH),
    myAmInTLS(false), myAmMacroscopicConnector(false),
    myStreetName(streetName),
    mySignalPosition(Position::INVALID),
    mySignalNode(nullptr),
    myIsOffRamp(false),
    myIndex(-1) {
    init(nolanes, false, "");
}


NBEdge::NBEdge(const std::string& id, NBNode* from, NBNode* to,
               std::string type, double speed, int nolanes,
               int priority, double laneWidth, double endOffset,
               PositionVector geom,
               const std::string& streetName,
               const std::string& origID,
               LaneSpreadFunction spread, bool tryIgnoreNodePositions) :
    Named(StringUtils::convertUmlaute(id)),
    myStep(EdgeBuildingStep::INIT),
    myType(StringUtils::convertUmlaute(type)),
    myFrom(from), myTo(to),
    myStartAngle(0), myEndAngle(0), myTotalAngle(0),
    myPriority(priority), mySpeed(speed),
    myDistance(0),
    myTurnDestination(nullptr),
    myPossibleTurnDestination(nullptr),
    myFromJunctionPriority(-1), myToJunctionPriority(-1),
    myGeom(geom), myLaneSpreadFunction(spread), myEndOffset(endOffset),
    myStopOffsets(),
    myLaneWidth(laneWidth),
    myLoadedLength(UNSPECIFIED_LOADED_LENGTH),
    myAmInTLS(false), myAmMacroscopicConnector(false),
    myStreetName(streetName),
    mySignalPosition(Position::INVALID),
    mySignalNode(nullptr),
    myIsOffRamp(false),
    myIndex(-1) {
    init(nolanes, tryIgnoreNodePositions, origID);
}


NBEdge::NBEdge(const std::string& id, NBNode* from, NBNode* to, const NBEdge* tpl, const PositionVector& geom, int numLanes) :
    Named(StringUtils::convertUmlaute(id)),
    myStep(EdgeBuildingStep::INIT),
    myType(tpl->getTypeID()),
    myFrom(from), myTo(to),
    myStartAngle(0), myEndAngle(0), myTotalAngle(0),
    myPriority(tpl->getPriority()), mySpeed(tpl->getSpeed()),
    myDistance(0),
    myTurnDestination(nullptr),
    myPossibleTurnDestination(nullptr),
    myFromJunctionPriority(-1), myToJunctionPriority(-1),
    myGeom(geom),
    myLaneSpreadFunction(tpl->getLaneSpreadFunction()),
    myEndOffset(tpl->getEndOffset()),
    myStopOffsets(tpl->getStopOffsets()),
    myLaneWidth(tpl->getLaneWidth()),
    myLoadedLength(UNSPECIFIED_LOADED_LENGTH),
    myAmInTLS(false),
    myAmMacroscopicConnector(false),
    myStreetName(tpl->getStreetName()),
    mySignalPosition(to == tpl->myTo ? tpl->mySignalPosition : Position::INVALID),
    mySignalNode(to == tpl->myTo ? tpl->mySignalNode : nullptr) {
    init(numLanes > 0 ? numLanes : tpl->getNumLanes(), myGeom.size() > 0, "");
    for (int i = 0; i < getNumLanes(); i++) {
        const int tplIndex = MIN2(i, tpl->getNumLanes() - 1);
        setSpeed(i, tpl->getLaneSpeed(tplIndex));
        setPermissions(tpl->getPermissions(tplIndex), i);
        setLaneWidth(i, tpl->myLanes[tplIndex].width);
        myLanes[i].updateParameters(tpl->myLanes[tplIndex].getParametersMap());
        if (to == tpl->myTo) {
            setEndOffset(i, tpl->myLanes[tplIndex].endOffset);
            setStopOffsets(i, tpl->myLanes[tplIndex].stopOffsets);
        }
    }
    if (tpl->myLoadedLength > 0 && to == tpl->getFromNode() && from == tpl->getToNode() && geom == tpl->getGeometry().reverse()) {
        myLoadedLength = tpl->myLoadedLength;
    }
    updateParameters(tpl->getParametersMap());
}


NBEdge::NBEdge() :
    Named("DUMMY") {
}


void
NBEdge::reinit(NBNode* from, NBNode* to, const std::string& type,
               double speed, int nolanes, int priority,
               PositionVector geom, double laneWidth, double endOffset,
               const std::string& streetName,
               LaneSpreadFunction spread,
               bool tryIgnoreNodePositions) {
    if (myFrom != from) {
        myFrom->removeEdge(this, false);
    }
    if (myTo != to) {
        myTo->removeEdge(this, false);
    }
    myType = StringUtils::convertUmlaute(type);
    myFrom = from;
    myTo = to;
    myPriority = priority;
    //?myTurnDestination(0),
    //?myFromJunctionPriority(-1), myToJunctionPriority(-1),
    myGeom = geom;
    myLaneSpreadFunction = spread;
    myLoadedLength = UNSPECIFIED_LOADED_LENGTH;
    myStreetName = streetName;
    //?, myAmTurningWithAngle(0), myAmTurningOf(0),
    //?myAmInTLS(false), myAmMacroscopicConnector(false)

    // preserve lane-specific settings (geometry must be recomputed)
    // if new lanes are added they copy the values from the leftmost lane (if specified)
    const std::vector<Lane> oldLanes = myLanes;
    init(nolanes, tryIgnoreNodePositions, oldLanes.empty() ? "" : oldLanes[0].getParameter(SUMO_PARAM_ORIGID));
    for (int i = 0; i < (int)nolanes; ++i) {
        PositionVector newShape = myLanes[i].shape;
        myLanes[i] = oldLanes[MIN2(i, (int)oldLanes.size() - 1)];
        myLanes[i].shape = newShape;
    }
    // however, if the new edge defaults are explicityly given, they override the old settings
    if (endOffset != UNSPECIFIED_OFFSET) {
        setEndOffset(-1, endOffset);
    }
    if (laneWidth != UNSPECIFIED_WIDTH) {
        setLaneWidth(-1, laneWidth);
    }
    if (speed != UNSPECIFIED_SPEED) {
        setSpeed(-1, speed);
    }
}


void
NBEdge::reinitNodes(NBNode* from, NBNode* to) {
    // connections may still be valid
    if (from == nullptr || to == nullptr) {
        throw ProcessError("At least one of edge's '" + myID + "' nodes is not known.");
    }
    if (myFrom != from) {
        myFrom->removeEdge(this, false);
    }
    if (myTo != to) {
        myTo->removeEdge(this, false);
    }
    // remove first from both nodes and then add to the new nodes
    // (otherwise reversing does not work)
    if (myFrom != from) {
        myFrom = from;
        myFrom->addOutgoingEdge(this);
    }
    if (myTo != to) {
        myTo = to;
        myTo->addIncomingEdge(this);
    }
    computeAngle();
}


void
NBEdge::init(int noLanes, bool tryIgnoreNodePositions, const std::string& origID) {
    if (noLanes == 0) {
        throw ProcessError("Edge '" + myID + "' needs at least one lane.");
    }
    if (myFrom == nullptr || myTo == nullptr) {
        throw ProcessError("At least one of edge's '" + myID + "' nodes is not known.");
    }
    if (!SUMOXMLDefinitions::isValidNetID(myID)) {
        throw ProcessError("Invalid edge id '" + myID + "'.");
    }
    // revisit geometry
    //  should have at least two points at the end...
    //  and in dome cases, the node positions must be added
    myGeom.removeDoublePoints(myGeom.size() > 2 ? POSITION_EPS : NUMERICAL_EPS);
    if (!tryIgnoreNodePositions || myGeom.size() < 2) {
        if (myGeom.size() == 0) {
            myGeom.push_back(myFrom->getPosition());
            myGeom.push_back(myTo->getPosition());
        } else {
            myGeom.push_back_noDoublePos(myTo->getPosition());
            myGeom.push_front_noDoublePos(myFrom->getPosition());
        }
    }
    if (myGeom.size() < 2) {
        myGeom.clear();
        myGeom.push_back(myFrom->getPosition());
        myGeom.push_back(myTo->getPosition());
    }
    if (myGeom.size() == 2 && myGeom[0] == myGeom[1]) {
        WRITE_WARNINGF("Edge's '%' from- and to-node are at the same position.", myID);
        int patchIndex = myFrom->getID() < myTo->getID() ? 1 : 0;
        myGeom[patchIndex].add(Position(POSITION_EPS, POSITION_EPS));
    }
    //
    myFrom->addOutgoingEdge(this);
    myTo->addIncomingEdge(this);
    // prepare container
    myLength = myFrom->getPosition().distanceTo(myTo->getPosition());
    assert(myGeom.size() >= 2);
    if ((int)myLanes.size() > noLanes) {
        // remove connections starting at the removed lanes
        for (int lane = noLanes; lane < (int)myLanes.size(); ++lane) {
            removeFromConnections(nullptr, lane, -1);
        }
        // remove connections targeting the removed lanes
        const EdgeVector& incoming = myFrom->getIncomingEdges();
        for (EdgeVector::const_iterator i = incoming.begin(); i != incoming.end(); i++) {
            for (int lane = noLanes; lane < (int)myLanes.size(); ++lane) {
                (*i)->removeFromConnections(this, -1, lane);
            }
        }
    }
    myLanes.clear();
    for (int i = 0; i < noLanes; i++) {
        myLanes.push_back(Lane(this, origID));
    }
    computeLaneShapes();
    computeAngle();

#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "init edge=" << getID() << "\n";
        for (Connection& c : myConnections) {
            std::cout << "  conn " << c.getDescription(this) << "\n";
        }
        for (Connection& c : myConnectionsToDelete) {
            std::cout << "  connToDelete " << c.getDescription(this) << "\n";
        }
    }
#endif
}


NBEdge::~NBEdge() {}


// -----------  Applying offset
void
NBEdge::reshiftPosition(double xoff, double yoff) {
    myGeom.add(xoff, yoff, 0);
    for (Lane& lane : myLanes) {
        lane.customShape.add(xoff, yoff, 0);
    }
    computeLaneShapes(); // old shapes are dubious if computed with large coordinates
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        (*i).customShape.add(xoff, yoff, 0);
    }
    if (mySignalPosition != Position::INVALID) {
        mySignalPosition.add(xoff, yoff);
    }
    myFromBorder.add(xoff, yoff, 0);
    myToBorder.add(xoff, yoff, 0);
    computeEdgeShape();
    computeAngle(); // update angles because they are numerically sensitive (especially where based on centroids)
}


void
NBEdge::mirrorX() {
    myGeom.mirrorX();
    for (int i = 0; i < (int)myLanes.size(); i++) {
        myLanes[i].shape.mirrorX();
        myLanes[i].customShape.mirrorX();
    }
    for (Connection& c : myConnections) {
        c.shape.mirrorX();
        c.viaShape.mirrorX();
        c.customShape.mirrorX();
    }
    computeAngle(); // update angles because they are numerically sensitive (especially where based on centroids)
}


// ----------- Edge geometry access and computation
const PositionVector
NBEdge::getInnerGeometry() const {
    return myGeom.getSubpartByIndex(1, (int)myGeom.size() - 2);
}


bool
NBEdge::hasDefaultGeometry() const {
    return myGeom.size() == 2 && hasDefaultGeometryEndpoints();
}


bool
NBEdge::hasDefaultGeometryEndpoints() const {
    return myGeom.front().almostSame(myFrom->getPosition(), 0.01)  &&
           myGeom.back().almostSame(myTo->getPosition(), 0.01);
}


bool
NBEdge::hasDefaultGeometryEndpointAtNode(const NBNode* node) const {
    // do not extend past the node position
    if (node == myFrom) {
        return myGeom.front() == node->getPosition();
    } else {
        assert(node == myTo);
        return myGeom.back() == node->getPosition();
    }
}

Position
NBEdge::getEndpointAtNode(const NBNode* node) const {
    return node == myFrom ? myGeom.front() : myGeom.back();
}

void
NBEdge::setGeometry(const PositionVector& s, bool inner) {
    Position begin = myGeom.front(); // may differ from node position
    Position end = myGeom.back(); // may differ from node position
    myGeom = s;
    if (inner) {
        myGeom.insert(myGeom.begin(), begin);
        myGeom.push_back(end);
    }
    computeLaneShapes();
    computeAngle();
}


void
NBEdge::extendGeometryAtNode(const NBNode* node, double maxExtent) {
    //std::cout << "extendGeometryAtNode edge=" << getID() << " node=" << node->getID() << " nodePos=" << node->getPosition() << " extent=" << maxExtent << " geom=" << myGeom;
    if (node == myFrom) {
        myGeom.extrapolate(maxExtent, true);
        double offset = myGeom.nearest_offset_to_point2D(node->getPosition());
        //std::cout << " geom2=" << myGeom << " offset=" << offset;
        if (offset != GeomHelper::INVALID_OFFSET) {
            myGeom = myGeom.getSubpart2D(MIN2(offset, myGeom.length2D() - 2 * POSITION_EPS), myGeom.length2D());
        }
    } else {
        assert(node == myTo);
        myGeom.extrapolate(maxExtent, false, true);
        double offset = myGeom.nearest_offset_to_point2D(node->getPosition());
        //std::cout << " geom2=" << myGeom << " offset=" << offset;
        if (offset != GeomHelper::INVALID_OFFSET) {
            myGeom = myGeom.getSubpart2D(0, MAX2(offset, 2 * POSITION_EPS));
        }
    }
    //std::cout << " geom3=" << myGeom << "\n";
}


void
NBEdge::shortenGeometryAtNode(const NBNode* node, double reduction) {
    //std::cout << "shortenGeometryAtNode edge=" << getID() << " node=" << node->getID() << " nodePos=" << node->getPosition() << " reduction=" << reduction << " geom=" << myGeom;
    reduction = MIN2(reduction, myGeom.length2D() - 2 * POSITION_EPS);
    if (node == myFrom) {
        myGeom = myGeom.getSubpart2D(reduction, myGeom.length2D());
    } else {
        myGeom = myGeom.getSubpart2D(0, myGeom.length2D() - reduction);
    }
    computeLaneShapes();
    //std::cout << " geom2=" << myGeom << "\n";
}


void
NBEdge::setNodeBorder(const NBNode* node, const Position& p, const Position& p2, bool rectangularCut) {
    PositionVector border;
    if (rectangularCut) {
        const double extend = 100;
        border = myGeom.getOrthogonal(p, extend, node == myTo);
    } else {
        border.push_back(p);
        border.push_back(p2);
    }
    if (border.size() == 2) {
        double edgeWidth = 0;
        for (int i = 0; i < (int)myLanes.size(); i++) {
            edgeWidth += getLaneWidth(i);
        }
        border.extrapolate2D(getTotalWidth());
        if (node == myFrom) {
            myFromBorder = border;
        } else {
            assert(node == myTo);
            myToBorder = border;
        }
    }
#ifdef DEBUG_NODE_BORDER
    gDebugFlag1 = DEBUGCOND;
    if (DEBUGCOND) std::cout << "setNodeBorder edge=" << getID() << " node=" << node->getID()
                                 << " rect=" << rectangularCut
                                 << " p=" << p << " p2=" << p2
                                 << " border=" << border
                                 << " myGeom=" << myGeom
                                 << "\n";

#endif
}


const PositionVector&
NBEdge::getNodeBorder(const NBNode* node) {
    if (node == myFrom) {
        return myFromBorder;
    } else {
        assert(node == myTo);
        return myToBorder;
    }
}


void
NBEdge::resetNodeBorder(const NBNode* node) {
    if (node == myFrom) {
        myFromBorder.clear();
    } else {
        assert(node == myTo);
        myToBorder.clear();
    }
}


bool
NBEdge::isBidiRail(bool ignoreSpread) const {
    return (isRailway(getPermissions())
            && (ignoreSpread || myLaneSpreadFunction == LaneSpreadFunction::CENTER)
            && myPossibleTurnDestination != nullptr
            && (ignoreSpread || myPossibleTurnDestination->getLaneSpreadFunction() == LaneSpreadFunction::CENTER)
            && isRailway(myPossibleTurnDestination->getPermissions())
            && myPossibleTurnDestination->getGeometry().reverse() == getGeometry());
}


bool
NBEdge::isRailDeadEnd() const {
    if (!isRailway(getPermissions())) {
        return false;
    }
    for (NBEdge* out : myTo->getOutgoingEdges()) {
        if (isRailway(out->getPermissions()) &&
                out != getTurnDestination(true)) {
            return true;
        }
    }
    return true;
}


PositionVector
NBEdge::cutAtIntersection(const PositionVector& old) const {
    PositionVector shape = old;
    shape = startShapeAt(shape, myFrom, myFromBorder);
    if (shape.size() < 2) {
        // only keep the last snippet
        const double oldLength = old.length();
        shape = old.getSubpart(oldLength - 2 * POSITION_EPS, oldLength);
    }
    shape = startShapeAt(shape.reverse(), myTo, myToBorder).reverse();
    // sanity checks
    if (shape.length() < POSITION_EPS) {
        if (old.length() < 2 * POSITION_EPS) {
            shape = old;
        } else {
            const double midpoint = old.length() / 2;
            // EPS*2 because otherwhise shape has only a single point
            shape = old.getSubpart(midpoint - POSITION_EPS, midpoint + POSITION_EPS);
            assert(shape.size() >= 2);
            assert(shape.length() > 0);
        }
    } else {
        // @note If the node shapes are overlapping we may get a shape which goes in the wrong direction
        // in this case the result shape should shortened
        if (DEG2RAD(135) < fabs(GeomHelper::angleDiff(shape.beginEndAngle(), old.beginEndAngle()))) {
            // eliminate intermediate points
            PositionVector tmp;
            tmp.push_back(shape[0]);
            tmp.push_back(shape[-1]);
            shape = tmp;
            if (tmp.length() < POSITION_EPS) {
                // fall back to original shape
                if (old.length() < 2 * POSITION_EPS) {
                    shape = old;
                } else {
                    const double midpoint = old.length() / 2;
                    // EPS*2 because otherwhise shape has only a single point
                    shape = old.getSubpart(midpoint - POSITION_EPS, midpoint + POSITION_EPS);
                    assert(shape.size() >= 2);
                    assert(shape.length() > 0);
                }
            } else {
                const double midpoint = shape.length() / 2;
                // cut to size and reverse
                shape = shape.getSubpart(midpoint - POSITION_EPS, midpoint + POSITION_EPS);
                if (shape.length() < POSITION_EPS) {
                    assert(false);
                    // the shape has a sharp turn near the midpoint
                }
                shape = shape.reverse();
            }
            // make short edge flat (length <= 2 * POSITION_EPS)
            const double z = (shape[0].z() + shape[1].z()) / 2;
            shape[0].setz(z);
            shape[1].setz(z);
        }
    }
    return shape;
}


void
NBEdge::computeEdgeShape(double smoothElevationThreshold) {
    if (smoothElevationThreshold > 0 && myGeom.hasElevation()) {
        PositionVector cut = cutAtIntersection(myGeom);
        // cutting and patching z-coordinate may cause steep grades which should be smoothed
        if (!myFrom->geometryLike()) {
            cut[0].setz(myFrom->getPosition().z());
            const double d = cut[0].distanceTo2D(cut[1]);
            const double dZ = fabs(cut[0].z() - cut[1].z());
            if (dZ / smoothElevationThreshold > d) {
                cut = cut.smoothedZFront(MIN2(cut.length2D() / 2, dZ / smoothElevationThreshold));
            }
        }
        if (!myTo->geometryLike()) {
            cut[-1].setz(myTo->getPosition().z());
            const double d = cut[-1].distanceTo2D(cut[-2]);
            const double dZ = fabs(cut[-1].z() - cut[-2].z());
            if (dZ / smoothElevationThreshold > d) {
                cut = cut.reverse().smoothedZFront(MIN2(cut.length2D() / 2, dZ / smoothElevationThreshold)).reverse();
            }
        }
        cut[0] = myGeom[0];
        cut[-1] = myGeom[-1];
        if (cut != myGeom) {
            myGeom = cut;
            computeLaneShapes();
        }
    }
    for (int i = 0; i < (int)myLanes.size(); i++) {
        myLanes[i].shape = cutAtIntersection(myLanes[i].shape);
    }
    // recompute edge's length as the average of lane lengths
    double avgLength = 0;
    for (int i = 0; i < (int)myLanes.size(); i++) {
        avgLength += myLanes[i].shape.length();
    }
    myLength = avgLength / (double) myLanes.size();
    computeAngle(); // update angles using the finalized node and lane shapes
}


PositionVector
NBEdge::startShapeAt(const PositionVector& laneShape, const NBNode* startNode, PositionVector nodeShape) {
    if (nodeShape.size() == 0) {
        nodeShape = startNode->getShape();
        nodeShape.closePolygon();
    }
    PositionVector lb = laneShape;
    lb.extrapolate2D(100.0);
    if (nodeShape.intersects(laneShape)) {
        // shape intersects directly
        std::vector<double> pbv = laneShape.intersectsAtLengths2D(nodeShape);
        assert(pbv.size() > 0);
        // ensure that the subpart has at least two points
        double pb = MIN2(laneShape.length2D() - POSITION_EPS - NUMERICAL_EPS, VectorHelper<double>::maxValue(pbv));
        if (pb < 0) {
            return laneShape;
        }
        PositionVector ns = laneShape.getSubpart2D(pb, laneShape.length2D());
        //PositionVector ns = pb < (laneShape.length() - POSITION_EPS) ? laneShape.getSubpart2D(pb, laneShape.length()) : laneShape;
        const double delta = ns[0].z() - laneShape[0].z();
        //std::cout << "a) startNode=" << startNode->getID() << " z=" << startNode->getPosition().z() << " oldZ=" << laneShape[0].z() << " cutZ=" << ns[0].z() << " delta=" << delta << "\n";
        if (fabs(delta) > 2 * POSITION_EPS && (!startNode->geometryLike() || pb < 1)) {
            // make "real" intersections and small intersections flat
            //std::cout << "a) startNode=" << startNode->getID() << " z=" << startNode->getPosition().z() << " oldZ=" << laneShape[0].z() << " cutZ=" << ns[0].z() << " delta=" << delta << "\n";
            ns[0].setz(startNode->getPosition().z());
        }
        assert(ns.size() >= 2);
        return ns;
    } else if (nodeShape.intersects(lb)) {
        // extension of first segment intersects
        std::vector<double> pbv = lb.intersectsAtLengths2D(nodeShape);
        assert(pbv.size() > 0);
        double pb = VectorHelper<double>::maxValue(pbv);
        assert(pb >= 0);
        PositionVector result = laneShape.getSubpartByIndex(1, (int)laneShape.size() - 1);
        Position np = lb.positionAtOffset2D(pb);
        const double delta = np.z() - laneShape[0].z();
        //std::cout << "b) startNode=" << startNode->getID() << " z=" << startNode->getPosition().z() << " oldZ=" << laneShape[0].z() << " cutZ=" << np.z() << " delta=" << delta << "\n";
        if (fabs(delta) > 2 * POSITION_EPS && !startNode->geometryLike()) {
            // avoid z-overshoot when extrapolating
            //std::cout << "b) startNode=" << startNode->getID() << " z=" << startNode->getPosition().z() << " oldZ=" << laneShape[0].z() << " cutZ=" << np.z() << " delta=" << delta << "\n";
            np.setz(startNode->getPosition().z());
        }
        result.push_front_noDoublePos(np);
        return result;
        //if (result.size() >= 2) {
        //    return result;
        //} else {
        //    WRITE_WARNING(error + " (resulting shape is too short)");
        //    return laneShape;
        //}
    } else {
        // could not find proper intersection. Probably the edge is very short
        // and lies within nodeShape
        // @todo enable warning WRITE_WARNING(error + " (laneShape lies within nodeShape)");
        return laneShape;
    }
}


const PositionVector&
NBEdge::getLaneShape(int i) const {
    return myLanes[i].shape;
}


void
NBEdge::setLaneSpreadFunction(LaneSpreadFunction spread) {
    myLaneSpreadFunction = spread;
}


void
NBEdge::addGeometryPoint(int index, const Position& p) {
    if (index >= 0) {
        myGeom.insert(myGeom.begin() + index, p);
    } else {
        myGeom.insert(myGeom.end() + index, p);
    }
}


void
NBEdge::reduceGeometry(const double minDist) {
    if (isBidiRail() && getID() < myPossibleTurnDestination->getID()) {
        // ensure symmetrical removal
        PositionVector reverse = myGeom.reverse();
        reverse.removeDoublePoints(minDist, true);
        myGeom = reverse.reverse();
    } else {
        myGeom.removeDoublePoints(minDist, true);
        for (Lane& lane : myLanes) {
            lane.customShape.removeDoublePoints(minDist, true);
        }
    }
}


void
NBEdge::checkGeometry(const double maxAngle, const double minRadius, bool fix, bool silent) {
    if (myGeom.size() < 3) {
        return;
    }
    //std::cout << "checking geometry of " << getID() << " geometry = " << toString(myGeom) << "\n";
    std::vector<double> angles; // absolute segment angles
    //std::cout << "  absolute angles:";
    for (int i = 0; i < (int)myGeom.size() - 1; ++i) {
        angles.push_back(myGeom.angleAt2D(i));
        //std::cout << " " << angles.back();
    }
    //std::cout << "\n  relative angles: ";
    for (int i = 0; i < (int)angles.size() - 1; ++i) {
        const double relAngle = fabs(GeomHelper::angleDiff(angles[i], angles[i + 1]));
        //std::cout << relAngle << " ";
        if (maxAngle > 0 && relAngle > maxAngle && !silent) {
            WRITE_WARNINGF("Found angle of % degrees at edge '%', segment %.", RAD2DEG(relAngle), getID(), i);
        }
        if (relAngle < DEG2RAD(1)) {
            continue;
        }
        if (i == 0 || i == (int)angles.size() - 2) {
            const bool start = i == 0;
            const double dist = (start ? myGeom[0].distanceTo2D(myGeom[1]) : myGeom[-2].distanceTo2D(myGeom[-1]));
            const double r = tan(0.5 * (M_PI - relAngle)) * dist;
            //std::cout << (start ? "  start" : "  end") << " length=" << dist << " radius=" << r << "  ";
            if (minRadius > 0 && r < minRadius) {
                if (fix) {
                    WRITE_MESSAGE("Removing sharp turn with radius " + toString(r) + " at the " +
                                  (start ? "start" : "end") + " of edge '" + getID() + "'.");
                    myGeom.erase(myGeom.begin() + (start ? 1 : i + 1));
                    checkGeometry(maxAngle, minRadius, fix, silent);
                    return;
                } else if (!silent) {
                    WRITE_WARNINGF("Found sharp turn with radius % at the " +
                                   toString(start ? "start" : "end") + " of edge '%'.", r, getID());
                }
            }
        }
    }
    //std::cout << "\n";
}


// ----------- Setting and getting connections
bool
NBEdge::addEdge2EdgeConnection(NBEdge* dest, bool overrideRemoval) {
    if (myStep == EdgeBuildingStep::INIT_REJECT_CONNECTIONS) {
        return true;
    }
    // check whether the node was merged and now a connection between
    //  not matching edges is tried to be added
    //  This happens f.e. within the ptv VISSIM-example "Beijing"
    if (dest != nullptr && myTo != dest->myFrom) {
        return false;
    }
    if (dest == nullptr) {
        invalidateConnections();
        myConnections.push_back(Connection(-1, dest, -1));
    } else if (find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(dest)) == myConnections.end()) {
        myConnections.push_back(Connection(-1, dest, -1));
    }
    if (overrideRemoval) {
        // override earlier delete decision
        for (std::vector<Connection>::iterator it = myConnectionsToDelete.begin(); it != myConnectionsToDelete.end();) {
            if (it->toEdge == dest) {
                it = myConnectionsToDelete.erase(it);
            } else {
                it++;
            }
        }
    }
    if (myStep < EdgeBuildingStep::EDGE2EDGES) {
        myStep = EdgeBuildingStep::EDGE2EDGES;
    }
    return true;
}


bool
NBEdge::addLane2LaneConnection(int from, NBEdge* dest,
                               int toLane, Lane2LaneInfoType type,
                               bool mayUseSameDestination,
                               bool mayDefinitelyPass,
                               bool keepClear,
                               double contPos,
                               double visibility,
                               double speed,
                               double length,
                               const PositionVector& customShape,
                               bool uncontrolled,
                               SVCPermissions permissions,
                               bool postProcess) {
    if (myStep == EdgeBuildingStep::INIT_REJECT_CONNECTIONS) {
        return true;
    }
    // check whether the node was merged and now a connection between
    //  not matching edges is tried to be added
    //  This happens f.e. within the ptv VISSIM-example "Beijing"
    if (myTo != dest->myFrom) {
        return false;
    }
    if (!addEdge2EdgeConnection(dest)) {
        return false;
    }
    return setConnection(from, dest, toLane, type, mayUseSameDestination, mayDefinitelyPass, keepClear, contPos, visibility, speed, length, customShape, uncontrolled, permissions, postProcess);
}


bool
NBEdge::addLane2LaneConnections(int fromLane,
                                NBEdge* dest, int toLane,
                                int no, Lane2LaneInfoType type,
                                bool invalidatePrevious,
                                bool mayDefinitelyPass) {
    if (invalidatePrevious) {
        invalidateConnections(true);
    }
    bool ok = true;
    for (int i = 0; i < no && ok; i++) {
        ok &= addLane2LaneConnection(fromLane + i, dest, toLane + i, type, false, mayDefinitelyPass);
    }
    return ok;
}


bool
NBEdge::setConnection(int lane, NBEdge* destEdge,
                      int destLane, Lane2LaneInfoType type,
                      bool mayUseSameDestination,
                      bool mayDefinitelyPass,
                      bool keepClear,
                      double contPos,
                      double visibility,
                      double speed,
                      double length,
                      const PositionVector& customShape,
                      bool uncontrolled,
                      SVCPermissions permissions,
                      bool postProcess) {
    if (myStep == EdgeBuildingStep::INIT_REJECT_CONNECTIONS) {
        return false;
    }
    // some kind of a misbehaviour which may occure when the junction's outgoing
    //  edge priorities were not properly computed, what may happen due to
    //  an incomplete or not proper input
    // what happens is that under some circumstances a single lane may set to
    //  be approached more than once by the one of our lanes.
    //  This must not be!
    // we test whether it is the case and do nothing if so - the connection
    //  will be refused
    //
    if (!mayUseSameDestination && hasConnectionTo(destEdge, destLane)) {
        return false;
    }
    if (find_if(myConnections.begin(), myConnections.end(), connections_finder(lane, destEdge, destLane)) != myConnections.end()) {
        return true;
    }
    if ((int)myLanes.size() <= lane || destEdge->getNumLanes() <= (int)destLane) {
        // problem might be corrigible in post-processing
        WRITE_WARNINGF("Could not set connection from '%' to '%'.", getLaneID(lane), destEdge->getLaneID(destLane));
        return false;
    }
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end();) {
        if ((*i).toEdge == destEdge && ((*i).fromLane == -1 || (*i).toLane == -1)) {
            i = myConnections.erase(i);
        } else {
            ++i;
        }
    }
    myConnections.push_back(Connection(lane, destEdge, destLane));
    if (mayDefinitelyPass) {
        myConnections.back().mayDefinitelyPass = true;
    }
    myConnections.back().keepClear = keepClear;
    myConnections.back().contPos = contPos;
    myConnections.back().visibility = visibility;
    myConnections.back().permissions = permissions;
    myConnections.back().speed = speed;
    myConnections.back().customLength = length;
    myConnections.back().customShape = customShape;
    myConnections.back().uncontrolled = uncontrolled;
    if (type == Lane2LaneInfoType::USER) {
        myStep = EdgeBuildingStep::LANES2LANES_USER;
    } else {
        // check whether we have to take another look at it later
        if (type == Lane2LaneInfoType::COMPUTED) {
            // yes, the connection was set using an algorithm which requires a recheck
            myStep = EdgeBuildingStep::LANES2LANES_RECHECK;
        } else {
            // ok, let's only not recheck it if we did no add something that has to be rechecked
            if (myStep != EdgeBuildingStep::LANES2LANES_RECHECK) {
                myStep = EdgeBuildingStep::LANES2LANES_DONE;
            }
        }
    }
    if (postProcess) {
        // override earlier delete decision
        for (std::vector<Connection>::iterator it = myConnectionsToDelete.begin(); it != myConnectionsToDelete.end();) {
            if ((it->fromLane < 0 || it->fromLane == lane)
                    && (it->toEdge == nullptr || it->toEdge == destEdge)
                    && (it->toLane < 0 || it->toLane == destLane)) {
                it = myConnectionsToDelete.erase(it);
            } else {
                it++;
            }
        }
    }
    return true;
}


std::vector<NBEdge::Connection>
NBEdge::getConnectionsFromLane(int lane, NBEdge* to, int toLane) const {
    std::vector<NBEdge::Connection> ret;
    for (const Connection& c : myConnections) {
        if ((lane < 0 || c.fromLane == lane)
                && (to == nullptr || to == c.toEdge)
                && (toLane < 0 || toLane == c.toLane)) {
            ret.push_back(c);
        }
    }
    return ret;
}


NBEdge::Connection
NBEdge::getConnection(int fromLane, const NBEdge* to, int toLane) const {
    for (std::vector<Connection>::const_iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if (
            (*i).fromLane == fromLane
            && (*i).toEdge == to
            && (*i).toLane == toLane) {
            return *i;
        }
    }
    throw ProcessError("Connection from " + getID() + "_" + toString(fromLane)
                       + " to " + to->getID() + "_" + toString(toLane) + " not found");
}

NBEdge::Connection&
NBEdge::getConnectionRef(int fromLane, const NBEdge* to, int toLane) {
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if (
            (*i).fromLane == fromLane
            && (*i).toEdge == to
            && (*i).toLane == toLane) {
            return *i;
        }
    }
    throw ProcessError("Connection from " + getID() + "_" + toString(fromLane)
                       + " to " + to->getID() + "_" + toString(toLane) + " not found");
}


bool
NBEdge::hasConnectionTo(NBEdge* destEdge, int destLane, int fromLane) const {
    return destEdge != nullptr && find_if(myConnections.begin(), myConnections.end(), connections_toedgelane_finder(destEdge, destLane, fromLane)) != myConnections.end();
}


bool
NBEdge::isConnectedTo(const NBEdge* e, const bool ignoreTurnaround) const {
    if (!ignoreTurnaround && (e == myTurnDestination)) {
        return true;
    }
    return
        find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(e))
        !=
        myConnections.end();

}


const EdgeVector*
NBEdge::getConnectedSorted() {
    // check whether connections exist and if not, use edges from the node
    EdgeVector outgoing;
    if (myConnections.size() == 0) {
        outgoing = myTo->getOutgoingEdges();
    } else {
        for (std::vector<Connection>::const_iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
            if (find(outgoing.begin(), outgoing.end(), (*i).toEdge) == outgoing.end()) {
                outgoing.push_back((*i).toEdge);
            }
        }
    }
    for (std::vector<Connection>::iterator it = myConnectionsToDelete.begin(); it != myConnectionsToDelete.end(); ++it) {
        if (it->fromLane < 0 && it->toLane < 0) {
            // found an edge that shall not be connected
            EdgeVector::iterator forbidden = std::find(outgoing.begin(), outgoing.end(), it->toEdge);
            if (forbidden != outgoing.end()) {
                outgoing.erase(forbidden);
            }
        }
    }
    // allocate the sorted container
    int size = (int) outgoing.size();
    EdgeVector* edges = new EdgeVector();
    edges->reserve(size);
    for (EdgeVector::const_iterator i = outgoing.begin(); i != outgoing.end(); i++) {
        NBEdge* outedge = *i;
        if (outedge != nullptr && outedge != myTurnDestination) {
            edges->push_back(outedge);
        }
    }
    std::sort(edges->begin(), edges->end(), NBContHelper::relative_outgoing_edge_sorter(this));
    return edges;
}


EdgeVector
NBEdge::getConnectedEdges() const {
    EdgeVector ret;
    for (std::vector<Connection>::const_iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if (find(ret.begin(), ret.end(), (*i).toEdge) == ret.end()) {
            ret.push_back((*i).toEdge);
        }
    }
    return ret;
}


EdgeVector
NBEdge::getIncomingEdges() const {
    EdgeVector ret;
    const EdgeVector& candidates = myFrom->getIncomingEdges();
    for (EdgeVector::const_iterator i = candidates.begin(); i != candidates.end(); i++) {
        if ((*i)->isConnectedTo(this)) {
            ret.push_back(*i);
        }
    }
    return ret;
}


std::vector<int>
NBEdge::getConnectionLanes(NBEdge* currentOutgoing, bool withBikes) const {
    std::vector<int> ret;
    if (currentOutgoing != myTurnDestination) {
        for (const Connection& c : myConnections) {
            if (c.toEdge == currentOutgoing && (withBikes || getPermissions(c.fromLane) != SVC_BICYCLE)) {
                ret.push_back(c.fromLane);
            }
        }
    }
    return ret;
}


void
NBEdge::sortOutgoingConnectionsByAngle() {
    sort(myConnections.begin(), myConnections.end(), connections_relative_edgelane_sorter(this));
}


void
NBEdge::sortOutgoingConnectionsByIndex() {
    sort(myConnections.begin(), myConnections.end(), connections_sorter);
}


void
NBEdge::remapConnections(const EdgeVector& incoming) {
    EdgeVector connected = getConnectedEdges();
    for (EdgeVector::const_iterator i = incoming.begin(); i != incoming.end(); i++) {
        NBEdge* inc = *i;
        // We have to do this
        inc->myStep = EdgeBuildingStep::EDGE2EDGES;
        // add all connections
        for (EdgeVector::iterator j = connected.begin(); j != connected.end(); j++) {
            inc->addEdge2EdgeConnection(*j);
        }
        inc->removeFromConnections(this);
    }
}


void
NBEdge::removeFromConnections(NBEdge* toEdge, int fromLane, int toLane, bool tryLater, const bool adaptToLaneRemoval,
                              const bool keepPossibleTurns) {
    // remove from "myConnections"
    const int fromLaneRemoved = adaptToLaneRemoval && fromLane >= 0 ? fromLane : -1;
    const int toLaneRemoved = adaptToLaneRemoval && toLane >= 0 ? toLane : -1;
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end();) {
        Connection& c = *i;
        if ((toEdge == nullptr || c.toEdge == toEdge)
                && (fromLane < 0 || c.fromLane == fromLane)
                && (toLane < 0 || c.toLane == toLane)) {
            if (myTo->isTLControlled()) {
                std::set<NBTrafficLightDefinition*> tldefs = myTo->getControllingTLS();
                for (std::set<NBTrafficLightDefinition*>::iterator it = tldefs.begin(); it != tldefs.end(); it++) {
                    (*it)->removeConnection(NBConnection(this, c.fromLane, c.toEdge, c.toLane));
                }
            }
            i = myConnections.erase(i);
            tryLater = false;
        } else {
            if (fromLaneRemoved >= 0 && c.fromLane > fromLaneRemoved) {
                if (myTo->isTLControlled()) {
                    std::set<NBTrafficLightDefinition*> tldefs = myTo->getControllingTLS();
                    for (std::set<NBTrafficLightDefinition*>::iterator it = tldefs.begin(); it != tldefs.end(); it++) {
                        for (NBConnectionVector::iterator tlcon = (*it)->getControlledLinks().begin(); tlcon != (*it)->getControlledLinks().end(); ++tlcon) {
                            NBConnection& tc = *tlcon;
                            if (tc.getTo() == c.toEdge && tc.getFromLane() == c.fromLane && tc.getToLane() == c.toLane) {
                                tc.shiftLaneIndex(this, -1);
                            }
                        }
                    }
                }
                //std::cout << getID() << " removeFromConnections fromLane=" << fromLane << " to=" << Named::getIDSecure(toEdge) << " toLane=" << toLane << " reduceFromLane=" << c.fromLane << " (to=" << c.toLane << ")\n";
                c.fromLane--;
            }
            if (toLaneRemoved >= 0 && c.toLane > toLaneRemoved && (toEdge == nullptr || c.toEdge == toEdge)) {
                //std::cout << getID() << " removeFromConnections fromLane=" << fromLane << " to=" << Named::getIDSecure(toEdge) << " toLane=" << toLane << " reduceToLane=" << c.toLane << " (from=" << c.fromLane << ")\n";
                c.toLane--;
            }
            ++i;
        }
    }
    // check whether it was the turn destination
    if (myTurnDestination == toEdge && fromLane < 0) {
        myTurnDestination = nullptr;
    }
    if (myPossibleTurnDestination == toEdge && fromLane < 0 && !keepPossibleTurns) {
        myPossibleTurnDestination = nullptr;
    }
    if (tryLater) {
        myConnectionsToDelete.push_back(Connection(fromLane, toEdge, toLane));
#ifdef DEBUG_CONNECTION_GUESSING
        if (DEBUGCOND) {
            std::cout << "removeFromConnections " << getID() << "_" << fromLane << "->" << toEdge->getID() << "_" << toLane << "\n";
            for (Connection& c : myConnections) {
                std::cout << "  conn " << c.getDescription(this) << "\n";
            }
            for (Connection& c : myConnectionsToDelete) {
                std::cout << "  connToDelete " << c.getDescription(this) << "\n";
            }
        }
#endif
    }
}


bool
NBEdge::removeFromConnections(const NBEdge::Connection& connectionToRemove) {
    // iterate over connections
    for (auto i = myConnections.begin(); i !=  myConnections.end(); i++) {
        if ((i->toEdge == connectionToRemove.toEdge) && (i->fromLane == connectionToRemove.fromLane) && (i->toLane == connectionToRemove.toLane)) {
            // remove connection
            myConnections.erase(i);
            return true;
        }
    }
    // assert(false);
    return false;
}


void
NBEdge::invalidateConnections(bool reallowSetting) {
    myTurnDestination = nullptr;
    myConnections.clear();
    if (reallowSetting) {
        myStep = EdgeBuildingStep::INIT;
    } else {
        myStep = EdgeBuildingStep::INIT_REJECT_CONNECTIONS;
    }
}


void
NBEdge::replaceInConnections(NBEdge* which, NBEdge* by, int laneOff) {
    // replace in "_connectedEdges"
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if ((*i).toEdge == which) {
            (*i).toEdge = by;
            (*i).toLane += laneOff;
        }
    }
    // check whether it was the turn destination
    if (myTurnDestination == which) {
        myTurnDestination = by;
    }
}

void
NBEdge::replaceInConnections(NBEdge* which, const std::vector<NBEdge::Connection>& origConns) {
    std::map<int, int> laneMap;
    int minLane = -1;
    int maxLane = -1;
    // get lanes used to approach the edge to remap
    bool wasConnected = false;
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if ((*i).toEdge != which) {
            continue;
        }
        wasConnected = true;
        if ((*i).fromLane != -1) {
            int fromLane = (*i).fromLane;
            laneMap[(*i).toLane] = fromLane;
            if (minLane == -1 || minLane > fromLane) {
                minLane = fromLane;
            }
            if (maxLane == -1 || maxLane < fromLane) {
                maxLane = fromLane;
            }
        }
    }
    if (!wasConnected) {
        return;
    }
    // add new connections
    std::vector<NBEdge::Connection> conns = origConns;
    EdgeVector origTargets = getSuccessors();
    for (std::vector<NBEdge::Connection>::iterator i = conns.begin(); i != conns.end(); ++i) {
        if ((*i).toEdge == which || (*i).toEdge == this
                // if we already have connections to the target edge, do not add new ones as they are probably from a circular replacement
                || std::find(origTargets.begin(), origTargets.end(), (*i).toEdge) != origTargets.end()) {
#ifdef DEBUG_REPLACECONNECTION
            if (DEBUGCOND) {
                std::cout << " replaceInConnections edge=" << getID() << " which=" << which->getID()
                          << " origTargets=" << toString(origTargets) << " newTarget=" << i->toEdge->getID() << " skipped\n";
            }
#endif
            continue;
        }
        if (which->getStep() == EdgeBuildingStep::EDGE2EDGES) {
            // do not set lane-level connections
            replaceInConnections(which, (*i).toEdge, 0);
            continue;
        }
        int fromLane = (*i).fromLane;
        int toUse = -1;
        if (laneMap.find(fromLane) == laneMap.end()) {
            if (fromLane >= 0 && fromLane <= minLane) {
                toUse = minLane;
                // patch laneMap to avoid crossed-over connections
                for (auto& item : laneMap) {
                    if (item.first < fromLane) {
                        item.second = MIN2(item.second, minLane);
                    }
                }
            }
            if (fromLane >= 0 && fromLane >= maxLane) {
                toUse = maxLane;
                // patch laneMap to avoid crossed-over connections
                for (auto& item : laneMap) {
                    if (item.first > fromLane) {
                        item.second = MAX2(item.second, maxLane);
                    }
                }
            }
        } else {
            toUse = laneMap[fromLane];
        }
        if (toUse == -1) {
            toUse = 0;
        }
#ifdef DEBUG_REPLACECONNECTION
        if (DEBUGCOND) {
            std::cout  << " replaceInConnections edge=" << getID() << " which=" << which->getID() << " origTargets=" << toString(origTargets)
                       << " origFrom=" << fromLane << " laneMap=" << joinToString(laneMap, ":", ",") << " minLane=" << minLane << " maxLane=" << maxLane
                       << " newTarget=" << i->toEdge->getID() << " fromLane=" << toUse << " toLane=" << i->toLane << "\n";
        }
#endif
        setConnection(toUse, i->toEdge, i->toLane, Lane2LaneInfoType::COMPUTED, false, i->mayDefinitelyPass, i->keepClear,
                      i->contPos, i->visibility, i->speed, i->customLength, i->customShape, i->uncontrolled);
    }
    // remove the remapped edge from connections
    removeFromConnections(which);
}


void
NBEdge::copyConnectionsFrom(NBEdge* src) {
    myStep = src->myStep;
    myConnections = src->myConnections;
}


bool
NBEdge::canMoveConnection(const Connection& con, int newFromLane) const {
    // only allow using newFromLane if at least 1 vClass is permitted to use
    // this connection. If the connection shall be moved to a sidewalk, only create the connection if there is no walking area
    const SVCPermissions common = (getPermissions(newFromLane) & con.toEdge->getPermissions(con.toLane));
    return (common > 0 && common != SVC_PEDESTRIAN);
}


void
NBEdge::moveConnectionToLeft(int lane) {
    int index = 0;
    for (int i = 0; i < (int)myConnections.size(); ++i) {
        if (myConnections[i].fromLane == (int)(lane) && canMoveConnection(myConnections[i], lane + 1)) {
            index = i;
        }
    }
    std::vector<Connection>::iterator i = myConnections.begin() + index;
    Connection c = *i;
    myConnections.erase(i);
    setConnection(lane + 1, c.toEdge, c.toLane, Lane2LaneInfoType::VALIDATED, false);
}


void
NBEdge::moveConnectionToRight(int lane) {
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if ((*i).fromLane == (int)lane && canMoveConnection(*i, lane - 1)) {
            Connection c = *i;
            i = myConnections.erase(i);
            setConnection(lane - 1, c.toEdge, c.toLane, Lane2LaneInfoType::VALIDATED, false);
            return;
        }
    }
}


void
NBEdge::buildInnerEdges(const NBNode& n, int noInternalNoSplits, int& linkIndex, int& splitIndex) {
    const int numPoints = OptionsCont::getOptions().getInt("junctions.internal-link-detail");
    const bool joinTurns = OptionsCont::getOptions().getBool("junctions.join-turns");
    const double limitTurnSpeed = OptionsCont::getOptions().getFloat("junctions.limit-turn-speed");
    const double limitTurnSpeedMinAngle = DEG2RAD(OptionsCont::getOptions().getFloat("junctions.limit-turn-speed.min-angle"));
    const double limitTurnSpeedMinAngleRail = DEG2RAD(OptionsCont::getOptions().getFloat("junctions.limit-turn-speed.min-angle.railway"));
    const double limitTurnSpeedWarnStraight = OptionsCont::getOptions().getFloat("junctions.limit-turn-speed.warn.straight");
    const double limitTurnSpeedWarnTurn = OptionsCont::getOptions().getFloat("junctions.limit-turn-speed.warn.turn");
    const bool fromRail = isRailway(getPermissions());
    std::string innerID = ":" + n.getID();
    NBEdge* toEdge = nullptr;
    int edgeIndex = linkIndex;
    int internalLaneIndex = 0;
    int numLanes = 0; // number of lanes that share the same edge
    double lengthSum = 0; // total shape length of all lanes that share the same edge
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        Connection& con = *i;
        con.haveVia = false; // reset first since this may be called multiple times
        if (con.toEdge == nullptr) {
            continue;
        }
        LinkDirection dir = n.getDirection(this, con.toEdge);
        const bool isRightTurn = (dir == LinkDirection::RIGHT || dir == LinkDirection::PARTRIGHT);
        const bool isTurn = (isRightTurn || dir == LinkDirection::LEFT || dir == LinkDirection::PARTLEFT);

        // put turning internal lanes on separate edges
        if (con.toEdge != toEdge || (isTurn && !joinTurns)) {
            // skip indices to keep some correspondence between edge ids and link indices:
            // internalEdgeIndex + internalLaneIndex = linkIndex
            edgeIndex = linkIndex;
            toEdge = (*i).toEdge;
            internalLaneIndex = 0;
            assignInternalLaneLength(i, numLanes, lengthSum);
            numLanes = 0;
            lengthSum = 0;
        }
        SVCPermissions conPermissions = getPermissions(con.fromLane) & con.toEdge->getPermissions(con.toLane);
        int shapeFlag = (conPermissions & ~SVC_PEDESTRIAN) != 0 ? 0 : NBNode::SCURVE_IGNORE;
        PositionVector shape = n.computeInternalLaneShape(this, con, numPoints, myTo, shapeFlag);
        std::vector<int> foeInternalLinks;

        if (dir != LinkDirection::STRAIGHT && shape.length() < POSITION_EPS && !(isBidiRail() && getTurnDestination(true) == con.toEdge)) {
            WRITE_WARNINGF("Connection '%_%->%_%' is only %m short.", getID(), con.fromLane, con.toEdge->getID(), con.toLane, shape.length());
        }

        // crossingPosition, list of foe link indices
        std::pair<double, std::vector<int> > crossingPositions(-1, std::vector<int>());
        std::set<std::string> tmpFoeIncomingLanes;
        switch (dir) {
            case LinkDirection::RIGHT:
            case LinkDirection::PARTRIGHT:
            case LinkDirection::LEFT:
            case LinkDirection::PARTLEFT:
            case LinkDirection::TURN: {
                int index = 0;
                const std::vector<NBEdge*>& incoming = n.getIncomingEdges();
                for (EdgeVector::const_iterator i2 = incoming.begin(); i2 != incoming.end(); ++i2) {
                    const std::vector<Connection>& elv = (*i2)->getConnections();
                    for (std::vector<NBEdge::Connection>::const_iterator k2 = elv.begin(); k2 != elv.end(); k2++) {
                        if ((*k2).toEdge == nullptr) {
                            continue;
                        }
                        // vehicles are typically less wide than the lane
                        // they drive on but but bicycle lanes should be kept clear for their whole width
                        double width2 = (*k2).toEdge->getLaneWidth((*k2).toLane);
                        if ((*k2).toEdge->getPermissions((*k2).toLane) != SVC_BICYCLE) {
                            width2 *= 0.5;
                        }
                        const bool foes = n.foes(this, con.toEdge, *i2, (*k2).toEdge);
                        bool needsCont = !isRailway(conPermissions) && n.needsCont(this, *i2, con, *k2);
                        bool oppositeLeftIntersect = !foes && bothLeftIntersect(n, shape, dir, *i2, *k2, numPoints, width2);
                        int shapeFlag = 0;
                        // do not warn if only bicycles pedestrians or delivery vehicles are involved as this is a typical occurence
                        SVCPermissions warn = SVCAll & ~(SVC_PEDESTRIAN | SVC_BICYCLE | SVC_DELIVERY | SVC_RAIL_CLASSES);
                        if (oppositeLeftIntersect
                                && (((*i2)->getPermissions((*k2).fromLane) & warn) != 0
                                    && ((*k2).toEdge->getPermissions((*k2).toLane) & warn) != 0)) {
                            // recompute with different curve parameters (unless
                            // the other connection is "unimportant"
                            shapeFlag = NBNode::AVOID_INTERSECTING_LEFT_TURNS;
                            shape = n.computeInternalLaneShape(this, con, numPoints, myTo, shapeFlag);
                            oppositeLeftIntersect = bothLeftIntersect(n, shape, dir, *i2, *k2, numPoints, width2, shapeFlag);
                        }
                        const bool bothPrio = getJunctionPriority(&n) > 0 && (*i2)->getJunctionPriority(&n) > 0;
                        //std::cout << "n=" << n.getID() << " e1=" << getID() << " prio=" << getJunctionPriority(&n) << " e2=" << (*i2)->getID() << " prio2=" << (*i2)->getJunctionPriority(&n) << " both=" << bothPrio << " bothLeftIntersect=" << bothLeftIntersect(n, shape, dir, *i2, *k2, numPoints, width2) << " needsCont=" << needsCont << "\n";
                        // compute the crossing point
                        if (needsCont || (bothPrio && oppositeLeftIntersect)) {
                            crossingPositions.second.push_back(index);
                            const PositionVector otherShape = n.computeInternalLaneShape(*i2, *k2, numPoints, 0, shapeFlag);
                            const double minDV = firstIntersection(shape, otherShape, width2,
                                                                   "Could not compute intersection of conflicting internal lanes at node '" + myTo->getID() + "'");
                            if (minDV < shape.length() - POSITION_EPS && minDV > POSITION_EPS) { // !!!?
                                assert(minDV >= 0);
                                if (crossingPositions.first < 0 || crossingPositions.first > minDV) {
                                    crossingPositions.first = minDV;
                                }
                            }
                        }
                        const bool rightTurnConflict = NBNode::rightTurnConflict(
                                                           this, con.toEdge, con.fromLane, (*i2), (*k2).toEdge, (*k2).fromLane);
                        // compute foe internal lanes
                        if (foes || rightTurnConflict || oppositeLeftIntersect) {
                            foeInternalLinks.push_back(index);
                        }
                        // only warn once per pair of intersecting turns
                        if (oppositeLeftIntersect && getID() > (*i2)->getID()
                                && (getPermissions(con.fromLane) & warn) != 0
                                && (con.toEdge->getPermissions(con.toLane) & warn) != 0
                                && ((*i2)->getPermissions((*k2).fromLane) & warn) != 0
                                && ((*k2).toEdge->getPermissions((*k2).toLane) & warn) != 0
                                // do not warn for unregulated nodes
                                && n.getType() != SumoXMLNodeType::NOJUNCTION
                           ) {
                            WRITE_WARNINGF("Intersecting left turns at junction '%' from lane '%' and lane '%' (increase junction radius to avoid this).",
                                           n.getID(), getLaneID(con.fromLane), (*i2)->getLaneID((*k2).fromLane));
                        }
                        // compute foe incoming lanes
                        const bool signalised = hasSignalisedConnectionTo(con.toEdge);
                        if ((n.forbids(*i2, (*k2).toEdge, this, con.toEdge, signalised) || rightTurnConflict) && (needsCont || dir == LinkDirection::TURN)) {
                            tmpFoeIncomingLanes.insert((*i2)->getID() + "_" + toString((*k2).fromLane));
                        }
                        if (bothPrio && oppositeLeftIntersect && getID() < (*i2)->getID()) {
                            //std::cout << " c1=" << con.getDescription(this) << " c2=" << (*k2).getDescription(*i2) << " bothPrio=" << bothPrio << " oppositeLeftIntersect=" << oppositeLeftIntersect << "\n";
                            // break symmetry using edge id
                            tmpFoeIncomingLanes.insert(innerID + "_" + toString(index) + "_0");
                        }
                        index++;
                    }
                }
                // foe pedestrian crossings
                std::vector<NBNode::Crossing*> crossings = n.getCrossings();
                for (auto c : crossings) {
                    const NBNode::Crossing& crossing = *c;
                    for (EdgeVector::const_iterator it_e = crossing.edges.begin(); it_e != crossing.edges.end(); ++it_e) {
                        const NBEdge* edge = *it_e;
                        // compute foe internal lanes
                        if (this == edge || con.toEdge == edge) {
                            foeInternalLinks.push_back(index);
                            if (con.toEdge == edge &&
                                    ((isRightTurn && getJunctionPriority(&n) > 0) || (isTurn && con.tlID != ""))) {
                                // build internal junctions (not for left turns at uncontrolled intersections)
                                PositionVector crossingShape = crossing.shape;
                                crossingShape.extrapolate(5.0); // sometimes shapes miss each other by a small margin
                                const double minDV = firstIntersection(shape, crossingShape, crossing.width / 2);
                                if (minDV < shape.length() - POSITION_EPS && minDV > POSITION_EPS) {
                                    assert(minDV >= 0);
                                    if (crossingPositions.first < 0 || crossingPositions.first > minDV) {
                                        crossingPositions.first = minDV;
                                    }
                                }
                            }
                        }
                    }
                    index++;
                }

                if (dir == LinkDirection::TURN && crossingPositions.first < 0 && crossingPositions.second.size() != 0 && shape.length() > 2. * POSITION_EPS) {
                    // let turnarounds wait in the middle if no other crossing point was found and it has a sensible length
                    // (if endOffset is used, the crossing point is in the middle of the part within the junction shape)
                    crossingPositions.first = (double)(shape.length() + getEndOffset(con.fromLane)) / 2.;
                }
            }
            break;
            default:
                break;
        }
        if (con.contPos != UNSPECIFIED_CONTPOS) {
            // apply custom internal junction position
            if (con.contPos <= 0 || con.contPos >= shape.length()) {
                // disable internal junction
                crossingPositions.first = -1;
            } else {
                // set custom position
                crossingPositions.first = con.contPos;
            }
        }

        // @todo compute the maximum speed allowed based on angular velocity
        //  see !!! for an explanation (with a_lat_mean ~0.3)
        /*
        double vmax = (double) 0.3 * (double) 9.80778 *
                        getLaneShape(con.fromLane).back().distanceTo(
                            con.toEdge->getLaneShape(con.toLane).front())
                        / (double) 2.0 / (double) M_PI;
        vmax = MIN2(vmax, ((getSpeed() + con.toEdge->getSpeed()) / (double) 2.0));
        */
        if (con.speed == UNSPECIFIED_SPEED) {
            con.vmax = (myLanes[con.fromLane].speed + con.toEdge->getLanes()[con.toLane].speed) / (double) 2.0;
            if (limitTurnSpeed > 0) {
                // see [Odhams and Cole, Models of Driver Speed Choice in Curves, 2004]
                const double angleRaw = fabs(GeomHelper::angleDiff(
                                                 getLaneShape(con.fromLane).angleAt2D(-2),
                                                 con.toEdge->getLaneShape(con.toLane).angleAt2D(0)));
                const double angle = MAX2(0.0, angleRaw - (fromRail ? limitTurnSpeedMinAngleRail : limitTurnSpeedMinAngle));
                const double length = shape.length2D();
                // do not trust the radius of tiny junctions
                // formula adapted from [Odhams, Andre and Cole, David, Models of Driver Speed Choice in Curves, 2004]
                if (angle > 0 && length > 1) {
                    // permit higher turning speed on wide lanes
                    const double radius = length / angle + getLaneWidth(con.fromLane) / 4;
                    const double limit = sqrt(limitTurnSpeed * radius);
                    const double reduction = con.vmax - limit;
                    // always treat connctions at roundabout as turns when warning
                    const bool atRoundabout = getJunctionPriority(myTo) == JunctionPriority::ROUNDABOUT || con.toEdge->getJunctionPriority(myFrom) == JunctionPriority::ROUNDABOUT;
                    const LinkDirection dir2 = atRoundabout ? LinkDirection::LEFT : dir;
                    if ((dir2 == LinkDirection::STRAIGHT && reduction > limitTurnSpeedWarnStraight)
                            || (dir2 != LinkDirection::TURN && reduction > limitTurnSpeedWarnTurn)) {
                        std::string dirType = std::string(dir == LinkDirection::STRAIGHT ? "straight" : "turning");
                        if (atRoundabout) {
                            dirType = "roundabout";
                        }
                        WRITE_WARNINGF("Speed of % connection '%' reduced by % due to turning radius of % (length=%, angle=%).",
                                       dirType, con.getDescription(this), reduction, radius, length, RAD2DEG(angleRaw));
                    }
                    con.vmax = MIN2(con.vmax, limit);
                    // value is saved in <net> attribute. Must be set again when importing from .con.xml
                    // con.speed = con.vmax;
                }
                assert(con.vmax > 0);
                //if (getID() == "-1017000.0.00") {
                //    std::cout << con.getDescription(this) << " angleRaw=" << angleRaw << " angle=" << RAD2DEG(angle) << " length=" << length << " radius=" << length / angle
                //        << " vmaxTurn=" << sqrt(limitTurnSpeed * length / angle) << " vmax=" << con.vmax << "\n";
                //}
            } else if (fromRail && dir == LinkDirection::TURN) {
                con.vmax = 0.01;
            }
        } else {
            con.vmax = con.speed;
        }
        //
        assert(shape.size() >= 2);
        // get internal splits if any
        con.id = innerID + "_" + toString(edgeIndex);
        if (crossingPositions.first >= 0 && crossingPositions.first < shape.length()) {
            std::pair<PositionVector, PositionVector> split = shape.splitAt(crossingPositions.first);
            con.shape = split.first;
            con.foeIncomingLanes = std::vector<std::string>(tmpFoeIncomingLanes.begin(), tmpFoeIncomingLanes.end());
            con.foeInternalLinks = foeInternalLinks; // resolve link indices to lane ids later
            con.viaID = innerID + "_" + toString(splitIndex + noInternalNoSplits);
            ++splitIndex;
            con.viaShape = split.second;
            con.haveVia = true;
        } else {
            con.shape = shape;
        }
        con.internalLaneIndex = internalLaneIndex;
        ++internalLaneIndex;
        ++linkIndex;
        ++numLanes;
        if (con.customLength != UNSPECIFIED_LOADED_LENGTH) {
            lengthSum += con.customLength;
        } else {
            lengthSum += con.shape.length();
        }
    }
    assignInternalLaneLength(myConnections.end(), numLanes, lengthSum);
}


void
NBEdge::assignInternalLaneLength(std::vector<Connection>::iterator i, int numLanes, double lengthSum) {
    // assign average length to all lanes of the same internal edge
    // @note the actual length should be used once sumo supports lanes of
    // varying length within the same edge
    assert(i - myConnections.begin() >= numLanes);
    for (int prevIndex = 1; prevIndex <= numLanes; prevIndex++) {
        //std::cout << " con=" << (*(i - prevIndex)).getDescription(this) << " numLanes=" << numLanes << " avgLength=" << lengthSum / numLanes << "\n";
        Connection& c = (*(i - prevIndex));
        const double minLength = c.customLength != UNSPECIFIED_LOADED_LENGTH ? pow(10, -gPrecision) : POSITION_EPS;
        c.length = MAX2(minLength, lengthSum / numLanes);
        if (c.haveVia) {
            c.viaLength = c.viaShape.length();
            if (c.customLength != UNSPECIFIED_LOADED_LENGTH) {
                // split length proportionally
                const double firstLength = c.shape.length();
                const double a = firstLength / (firstLength + c.viaLength);
                c.length = MAX2(minLength, a * c.customLength);
                c.viaLength = MAX2(minLength, c.customLength - c.length);
            }
        }
    }
}

double
NBEdge::firstIntersection(const PositionVector& v1, const PositionVector& v2, double width2, const std::string& error) {
    double intersect = std::numeric_limits<double>::max();
    if (v2.length() < POSITION_EPS) {
        return intersect;
    }
    try {
        PositionVector v2Right = v2;
        v2Right.move2side(width2);

        PositionVector v2Left = v2;
        v2Left.move2side(-width2);

        // intersect center line of v1 with left and right border of v2
        for (double cand : v1.intersectsAtLengths2D(v2Right)) {
            intersect = MIN2(intersect, cand);
        }
        for (double cand : v1.intersectsAtLengths2D(v2Left)) {
            intersect = MIN2(intersect, cand);
        }
    } catch (InvalidArgument&) {
        if (error != "") {
            WRITE_WARNING(error);
        }
    }
    //std::cout << " v1=" << v1 << " v2Right=" << v2Right << " v2Left=" << v2Left << "\n";
    //std::cout << "  intersectsRight=" << toString(v1.intersectsAtLengths2D(v2Right)) << "\n";
    //std::cout << "  intersectsLeft=" << toString(v1.intersectsAtLengths2D(v2Left)) << "\n";
    return intersect;
}


bool
NBEdge::bothLeftIntersect(const NBNode& n, const PositionVector& shape, LinkDirection dir, NBEdge* otherFrom, const NBEdge::Connection& otherCon, int numPoints, double width2, int shapeFlag) const {
    if (otherFrom == this) {
        // not an opposite pair
        return false;
    }
    LinkDirection dir2 = n.getDirection(otherFrom, otherCon.toEdge);
    const bool bothLeft = (dir == LinkDirection::LEFT || dir == LinkDirection::PARTLEFT) && (dir2 == LinkDirection::LEFT || dir2 == LinkDirection::PARTLEFT);
    if (bothLeft) {
        const PositionVector otherShape = n.computeInternalLaneShape(otherFrom, otherCon, numPoints, 0, shapeFlag);
        const double minDV = firstIntersection(shape, otherShape, width2);
        if (minDV < shape.length() - POSITION_EPS && minDV > POSITION_EPS) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


// -----------
int
NBEdge::getJunctionPriority(const NBNode* const node) const {
    if (node == myFrom) {
        return myFromJunctionPriority;
    } else {
        return myToJunctionPriority;
    }
}


void
NBEdge::setJunctionPriority(const NBNode* const node, int prio) {
    if (node == myFrom) {
        myFromJunctionPriority = prio;
    } else {
        myToJunctionPriority = prio;
    }
}


double
NBEdge::getAngleAtNode(const NBNode* const atNode) const {
    // myStartAngle, myEndAngle are in [0,360] and this returns results in [-180,180]
    if (atNode == myFrom) {
        return GeomHelper::legacyDegree(myGeom.angleAt2D(0));
    } else {
        assert(atNode == myTo);
        return GeomHelper::legacyDegree(myGeom.angleAt2D(-2));
    }
}


double
NBEdge::getAngleAtNodeToCenter(const NBNode* const atNode) const {
    if (atNode == myFrom) {
        double res = myStartAngle - 180;
        if (res < 0) {
            res += 360;
        }
        return res;
    } else {
        assert(atNode == myTo);
        return myEndAngle;
    }
}


void
NBEdge::setTurningDestination(NBEdge* e, bool onlyPossible) {
    if (!onlyPossible) {
        myTurnDestination = e;
    }
    myPossibleTurnDestination = e;
}


double
NBEdge::getLaneSpeed(int lane) const {
    return myLanes[lane].speed;
}

void
NBEdge::resetLaneShapes() {
    computeLaneShapes();
}

void
NBEdge::computeLaneShapes() {
    // vissim needs this
    if (myFrom == myTo) {
        return;
    }
    // compute lane offset, first
    std::vector<double> offsets(myLanes.size(), 0.);
    double offset = 0;
    for (int i = (int)myLanes.size() - 2; i >= 0; --i) {
        offset += (getLaneWidth(i) + getLaneWidth(i + 1)) / 2. + SUMO_const_laneOffset;
        offsets[i] = offset;
    }
    if (myLaneSpreadFunction == LaneSpreadFunction::CENTER) {
        double width = 0;
        for (int i = 0; i < (int)myLanes.size(); ++i) {
            width += getLaneWidth(i);
        }
        width += SUMO_const_laneOffset * double(myLanes.size() - 1);
        offset = -width / 2. + getLaneWidth((int)myLanes.size() - 1) / 2.;
    } else {
        double laneWidth = myLanes.back().width != UNSPECIFIED_WIDTH ? myLanes.back().width : SUMO_const_laneWidth;
        offset = (laneWidth + SUMO_const_laneOffset) / 2.; // @note: offset for half of the center-line marking of the road
    }
    if (myLaneSpreadFunction == LaneSpreadFunction::ROADCENTER) {
        for (NBEdge* e : myTo->getOutgoingEdges()) {
            if (e->getToNode() == myFrom && getInnerGeometry().reverse() == e->getInnerGeometry()) {
                offset += (e->getTotalWidth() - getTotalWidth()) / 2;
                break;
            }
        }
    }

    for (int i = 0; i < (int)myLanes.size(); ++i) {
        offsets[i] += offset;
    }

    // build the shape of each lane
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        if (myLanes[i].customShape.size() != 0) {
            myLanes[i].shape = myLanes[i].customShape;
            continue;
        }
        try {
            myLanes[i].shape = computeLaneShape(i, offsets[i]);
        } catch (InvalidArgument& e) {
            WRITE_WARNINGF("In lane '%': lane shape could not be determined (%).", getLaneID(i), e.what());
            myLanes[i].shape = myGeom;
        }
    }
}


PositionVector
NBEdge::computeLaneShape(int lane, double offset) const {
    PositionVector shape = myGeom;
    try {
        shape.move2side(offset);
    } catch (InvalidArgument& e) {
        WRITE_WARNINGF("In lane '%': Could not build shape (%).", getLaneID(lane), e.what());
    }
    return shape;
}


void
NBEdge::computeAngle() {
    // taking the angle at the first might be unstable, thus we take the angle
    // at a certain distance. (To compare two edges, additional geometry
    // segments are considered to resolve ambiguities)
    const bool hasFromShape = myFrom->getShape().size() > 0;
    const bool hasToShape = myTo->getShape().size() > 0;
    Position fromCenter = (hasFromShape ? myFrom->getShape().getCentroid() : myFrom->getPosition());
    Position toCenter = (hasToShape ? myTo->getShape().getCentroid() : myTo->getPosition());
    PositionVector shape = myGeom;
    if ((hasFromShape || hasToShape) && getNumLanes() > 0) {
        if (myLaneSpreadFunction == LaneSpreadFunction::RIGHT) {
            shape = myLanes[getNumLanes() - 1].shape ;
        } else {
            shape = myLanes[getNumLanes() / 2].shape;
            if (getNumLanes() % 2 == 0) {
                // there is no center lane. shift to get the center
                shape.move2side(getLaneWidth(getNumLanes() / 2) * 0.5);
            }
        }
    }

    // if the junction shape is suspicious we cannot trust the angle to the centroid
    const bool suspiciousFromShape = hasFromShape && (myFrom->getShape().distance2D(shape[0]) > 2 * POSITION_EPS
                                     || myFrom->getShape().around(shape[-1])
                                     || !(myFrom->getShape().around(fromCenter)));
    const bool suspiciousToShape = hasToShape && (myTo->getShape().distance2D(shape[-1]) > 2 * POSITION_EPS
                                   || myTo->getShape().around(shape[0])
                                   || !(myTo->getShape().around(toCenter)));

    const double angleLookahead = MIN2(shape.length2D() / 2, ANGLE_LOOKAHEAD);
    const Position referencePosStart = shape.positionAtOffset2D(angleLookahead);
    const Position referencePosEnd = shape.positionAtOffset2D(shape.length() - angleLookahead);

    myStartAngle = GeomHelper::legacyDegree(fromCenter.angleTo2D(referencePosStart), true);
    const double myStartAngle2 = GeomHelper::legacyDegree(myFrom->getPosition().angleTo2D(referencePosStart), true);
    const double myStartAngle3 = getAngleAtNode(myFrom);
    myEndAngle = GeomHelper::legacyDegree(referencePosEnd.angleTo2D(toCenter), true);
    const double myEndAngle2 = GeomHelper::legacyDegree(referencePosEnd.angleTo2D(myTo->getPosition()), true);
    const double myEndAngle3 = getAngleAtNode(myTo);

#ifdef DEBUG_ANGLES
    if (DEBUGCOND) {
        if (suspiciousFromShape) {
            std::cout << "  len=" << shape.length() << " startA=" << myStartAngle << " startA2=" << myStartAngle2 << " startA3=" << myStartAngle3
                      << " rel=" << NBHelpers::normRelAngle(myStartAngle, myStartAngle2)
                      << " fromCenter=" << fromCenter
                      << " fromPos=" << myFrom->getPosition()
                      << " refStart=" << referencePosStart
                      << "\n";
        }
        if (suspiciousToShape) {
            std::cout << " len=" << shape.length() << "  endA=" << myEndAngle << " endA2=" << myEndAngle2 << " endA3=" << myEndAngle3
                      << " rel=" << NBHelpers::normRelAngle(myEndAngle, myEndAngle2)
                      << " toCenter=" << toCenter
                      << " toPos=" << myTo->getPosition()
                      << " refEnd=" << referencePosEnd
                      << "\n";
        }
    }
#endif

    if (suspiciousFromShape && shape.length() > 1) {
        myStartAngle = myStartAngle2;
    } else if (suspiciousToShape && fabs(NBHelpers::normRelAngle(myStartAngle, myStartAngle3)) > 90
               // don't trust footpath angles
               && (getPermissions() & ~SVC_PEDESTRIAN) != 0) {
        myStartAngle = myStartAngle3;
        if (myStartAngle < 0) {
            myStartAngle += 360;
        }
    }

    if (suspiciousToShape && shape.length() > 1) {
        myEndAngle = myEndAngle2;
    } else if (suspiciousToShape && fabs(NBHelpers::normRelAngle(myEndAngle, myEndAngle3)) > 90
               // don't trust footpath angles
               && (getPermissions() & ~SVC_PEDESTRIAN) != 0) {
        myEndAngle = myEndAngle3;
        if (myEndAngle < 0) {
            myEndAngle += 360;
        }
    }

    myTotalAngle = GeomHelper::legacyDegree(myFrom->getPosition().angleTo2D(myTo->getPosition()), true);
#ifdef DEBUG_ANGLES
    if (DEBUGCOND) std::cout << "computeAngle edge=" << getID()
                                 << " fromCenter=" << fromCenter << " toCenter=" << toCenter
                                 << " refStart=" << referencePosStart << " refEnd=" << referencePosEnd << " shape=" << shape
                                 << " hasFromShape=" << hasFromShape
                                 << " hasToShape=" << hasToShape
                                 << " numLanes=" << getNumLanes()
                                 << " shapeLane=" << getNumLanes() / 2
                                 << " startA=" << myStartAngle << " endA=" << myEndAngle << " totA=" << myTotalAngle << "\n";
#endif
}


double
NBEdge::getShapeStartAngle() const {
    const double angleLookahead = MIN2(myGeom.length2D() / 2, ANGLE_LOOKAHEAD);
    const Position referencePosStart = myGeom.positionAtOffset2D(angleLookahead);
    return GeomHelper::legacyDegree(myGeom.front().angleTo2D(referencePosStart), true);
}


double
NBEdge::getShapeEndAngle() const {
    const double angleLookahead = MIN2(myGeom.length2D() / 2, ANGLE_LOOKAHEAD);
    const Position referencePosEnd = myGeom.positionAtOffset2D(myGeom.length() - angleLookahead);
    return GeomHelper::legacyDegree(referencePosEnd.angleTo2D(myGeom.back()), true);
}


bool
NBEdge::hasPermissions() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if ((*i).permissions != SVCAll) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificPermissions() const {
    std::vector<Lane>::const_iterator i = myLanes.begin();
    SVCPermissions firstLanePermissions = i->permissions;
    i++;
    for (; i != myLanes.end(); ++i) {
        if (i->permissions != firstLanePermissions) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificSpeed() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->speed != getSpeed()) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificWidth() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->width != myLanes.begin()->width) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificType() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->type != myLanes.begin()->type) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificEndOffset() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->endOffset != myLanes.begin()->endOffset) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneSpecificStopOffsets() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (!i->stopOffsets.empty()) {
            const std::pair<const int, double>& offsets = *(i->stopOffsets.begin());
            if (myStopOffsets.empty() || offsets != *(myStopOffsets.begin())) {
                return true;
            }
        }
    }
    return false;
}


bool
NBEdge::hasAccelLane() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->accelRamp) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasCustomLaneShape() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->customShape.size() > 0) {
            return true;
        }
    }
    return false;
}


bool
NBEdge::hasLaneParams() const {
    for (std::vector<Lane>::const_iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        if (i->getParametersMap().size() > 0) {
            return true;
        }
    }
    return false;
}

bool
NBEdge::needsLaneSpecificOutput() const {
    return (hasLaneSpecificPermissions()
            || hasLaneSpecificSpeed()
            || hasLaneSpecificWidth()
            || hasLaneSpecificType()
            || hasLaneSpecificEndOffset()
            || hasLaneSpecificStopOffsets()
            || hasAccelLane()
            || hasCustomLaneShape()
            || hasLaneParams()
            || (!myLanes.empty() && myLanes.back().oppositeID != ""));
}



bool
NBEdge::computeEdge2Edges(bool noLeftMovers) {
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "computeEdge2Edges  edge=" << getID() << " step=" << (int)myStep << "\n";
        for (Connection& c : myConnections) {
            std::cout << "  conn " << c.getDescription(this) << "\n";
        }
        for (Connection& c : myConnectionsToDelete) {
            std::cout << "  connToDelete " << c.getDescription(this) << "\n";
        }
    }
#endif
    // return if this relationship has been build in previous steps or
    //  during the import
    if (myStep >= EdgeBuildingStep::EDGE2EDGES) {
        return true;
    }
    const EdgeVector& o = myTo->getOutgoingEdges();
    const bool fromRail = isRailway(getPermissions());
    for (EdgeVector::const_iterator i = o.begin(); i != o.end(); ++i) {
        if (noLeftMovers && myTo->isLeftMover(this, *i)) {
            continue;
        }
        // avoid sharp railway turns
        if (fromRail && isRailway((*i)->getPermissions()) &&
                fabs(NBHelpers::normRelAngle(getAngleAtNode(myTo), (*i)->getAngleAtNode(myTo))) > 90) {
            continue;
        }
        myConnections.push_back(Connection(-1, *i, -1));
    }
    myStep = EdgeBuildingStep::EDGE2EDGES;
    return true;
}


bool
NBEdge::computeLanes2Edges() {
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "computeLanes2Edges edge=" << getID() << " step=" << (int)myStep << "\n";
        for (Connection& c : myConnections) {
            std::cout << "  conn " << c.getDescription(this) << "\n";
        }
        for (Connection& c : myConnectionsToDelete) {
            std::cout << "  connToDelete " << c.getDescription(this) << "\n";
        }
    }
#endif
    // return if this relationship has been build in previous steps or
    //  during the import
    if (myStep >= EdgeBuildingStep::LANES2EDGES) {
        return true;
    }
    assert(myStep == EdgeBuildingStep::EDGE2EDGES);
    // get list of possible outgoing edges sorted by direction clockwise
    //  the edge in the backward direction (turnaround) is not in the list
    const EdgeVector* edges = getConnectedSorted();
    if (myConnections.size() != 0 && edges->size() == 0) {
        // dead end per definition!?
        myConnections.clear();
    } else {
        // divide the lanes on reachable edges
        divideOnEdges(edges);
    }
    delete edges;
    myStep = EdgeBuildingStep::LANES2EDGES;
    return true;
}


bool
NBEdge::recheckLanes() {
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "recheckLanes (initial) edge=" << getID() << "\n";
        for (Connection& c : myConnections) {
            std::cout << "  conn " << c.getDescription(this) << "\n";
        }
        for (Connection& c : myConnectionsToDelete) {
            std::cout << "  connToDelete " << c.getDescription(this) << "\n";
        }
    }
#endif
    // check delayed removals
    for (std::vector<Connection>::iterator it = myConnectionsToDelete.begin(); it != myConnectionsToDelete.end(); ++it) {
        removeFromConnections(it->toEdge, it->fromLane, it->toLane, false, false, true);
    }
    std::vector<int> connNumbersPerLane(myLanes.size(), 0);
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end();) {
        if ((*i).toEdge == nullptr || (*i).fromLane < 0 || (*i).toLane < 0) {
            i = myConnections.erase(i);
        } else {
            if ((*i).fromLane >= 0) {
                ++connNumbersPerLane[(*i).fromLane];
            }
            ++i;
        }
    }
    if (myStep != EdgeBuildingStep::LANES2LANES_DONE && myStep != EdgeBuildingStep::LANES2LANES_USER) {
        // check #1:
        // If there is a lane with no connections and any neighbour lane has
        //  more than one connections, try to move one of them.
        // This check is only done for edges which connections were assigned
        //  using the standard algorithm.
        for (int i = 0; i < (int)myLanes.size(); i++) {
            if (connNumbersPerLane[i] == 0 && !isForbidden(getPermissions((int)i))) {
                // dead-end lane found
                bool hasDeadEnd = true;
                // find lane with two connections or more to the right of the current lane
                for (int i2 = i - 1; hasDeadEnd && i2 >= 0; i2--) {
                    if (getPermissions(i) != getPermissions(i2)) {
                        break;
                    }
                    if (connNumbersPerLane[i2] > 1) {
                        connNumbersPerLane[i2]--;
                        for (int i3 = i2; i3 != i; i3++) {
                            moveConnectionToLeft(i3);
                            sortOutgoingConnectionsByAngle();
                            sortOutgoingConnectionsByIndex();
                        }
                        hasDeadEnd = false;
                    }
                }
                if (hasDeadEnd) {
                    // find lane with two connections or more to the left of the current lane
                    for (int i2 = i + 1; hasDeadEnd && i2 < getNumLanes(); i2++) {
                        if (getPermissions(i) != getPermissions(i2)) {
                            break;
                        }
                        if (connNumbersPerLane[i2] > 1) {
                            connNumbersPerLane[i2]--;
                            for (int i3 = i2; i3 != i; i3--) {
                                moveConnectionToRight(i3);
                                sortOutgoingConnectionsByAngle();
                                sortOutgoingConnectionsByIndex();
                            }
                            hasDeadEnd = false;
                        }
                    }
                }
            }
        }
        // check restrictions
        for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end();) {
            Connection& c = *i;
            const SVCPermissions common = getPermissions(c.fromLane) & c.toEdge->getPermissions(c.toLane);
            if (common == SVC_PEDESTRIAN || getPermissions(c.fromLane) == SVC_PEDESTRIAN) {
                // these are computed in NBNode::buildWalkingAreas
                i = myConnections.erase(i);
            } else if (common == 0) {
                // no common permissions.
                // try to find a suitable target lane to the right
                const int origToLane = c.toLane;
                c.toLane = -1; // ignore this connection when calling hasConnectionTo
                int toLane = origToLane;
                while (toLane > 0
                        && (getPermissions(c.fromLane) & c.toEdge->getPermissions(toLane)) == 0
                        && !hasConnectionTo(c.toEdge, toLane)
                      ) {
                    toLane--;
                }
                if ((getPermissions(c.fromLane) & c.toEdge->getPermissions(toLane)) != 0
                        && !hasConnectionTo(c.toEdge, toLane)) {
                    c.toLane = toLane;
                    ++i;
                } else {
                    // try to find a suitable target lane to the left
                    int toLane = origToLane;
                    while (toLane < (int)c.toEdge->getNumLanes() - 1
                            && (getPermissions(c.fromLane) & c.toEdge->getPermissions(toLane)) == 0
                            && !hasConnectionTo(c.toEdge, toLane)
                          ) {
                        toLane++;
                    }
                    if ((getPermissions(c.fromLane) & c.toEdge->getPermissions(toLane)) != 0
                            && !hasConnectionTo(c.toEdge, toLane)) {
                        c.toLane = toLane;
                        ++i;
                    } else {
                        // no alternative target found
                        i = myConnections.erase(i);
                    }
                }
            } else if (isRailway(getPermissions(c.fromLane)) && isRailway(c.toEdge->getPermissions(c.toLane))
                       && isTurningDirectionAt(c.toEdge))  {
                // do not allow sharp rail turns
                i = myConnections.erase(i);
            } else {
                ++i;
            }
        }
    }
    // check involuntary dead end at "real" junctions
    if (getPermissions() != SVC_PEDESTRIAN) {
        if (myConnections.empty() && myTo->getOutgoingEdges().size() > 1 && (getPermissions() & ~SVC_PEDESTRIAN) != 0) {
            WRITE_WARNINGF("Edge '%' is not connected to outgoing edges at junction '%'.", getID(), myTo->getID());
        }
        const EdgeVector& incoming = myFrom->getIncomingEdges();
        if (incoming.size() > 1) {
            for (int i = 0; i < (int)myLanes.size(); i++) {
                if (getPermissions(i) != 0 && getPermissions(i) != SVC_PEDESTRIAN) {
                    bool connected = false;
                    for (std::vector<NBEdge*>::const_iterator in = incoming.begin(); in != incoming.end(); ++in) {
                        if ((*in)->hasConnectionTo(this, i)) {
                            connected = true;
                            break;
                        }
                    }
                    if (!connected) {
                        WRITE_WARNINGF("Lane '%' is not connected from any incoming edge at junction '%'.", getLaneID(i), myFrom->getID());
                    }
                }
            }
        }
    }
#ifdef ADDITIONAL_WARNINGS
    // check for connections with bad access permissions
    for (const Connection& c : myConnections) {
        SVCPermissions fromP = getPermissions(c.fromLane);
        SVCPermissions toP = c.toEdge->getPermissions(c.toLane);
        if ((fromP & SVC_PASSENGER) != 0
                && toP == SVC_BICYCLE) {
            bool hasAlternative = false;
            for (const Connection& c2 : myConnections) {
                if (c.fromLane == c2.fromLane && c.toEdge == c2.toEdge
                        && (c.toEdge->getPermissions(c2.toLane) & SVC_PASSENGER) != 0) {
                    hasAlternative = true;
                }
            }
            if (!hasAlternative) {
                WRITE_WARNING("Road lane ends on bikeLane for connection " + c.getDescription(this));
            }
        }
    }
    // check for dead-end passenger lanes when there are still unconnected outgoing edges
    int passengerLanes = 0;
    int passengerTargetLanes = 0;
    for (const Lane& lane : myLanes) {
        if ((lane.permissions & SVC_PASSENGER) != 0) {
            passengerLanes++;
        }
    }
    for (const NBEdge* out : myTo->getOutgoingEdges()) {
        if (!isTurningDirectionAt(out)) {
            for (const Lane& lane : out->getLanes()) {
                if ((lane.permissions & SVC_PASSENGER) != 0) {
                    passengerTargetLanes++;
                }
            }
        }
    }
    if (passengerLanes <= passengerTargetLanes) {
        // no need for dead-ends
        connNumbersPerLane = std::vector<int>(myLanes.size(), 0);
        for (const Connection& c : myConnections) {
            connNumbersPerLane[c.fromLane]++;
        }
        for (int i = 0; i < (int)myLanes.size(); i++) {
            if (connNumbersPerLane[i] == 0 && !isForbidden(getPermissions(i))) {
                // dead-end lane found
                WRITE_WARNING("Found dead-end lane " + getLaneID(i));
            }
        }
    }

#endif
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "recheckLanes (final) edge=" << getID() << "\n";
        for (Connection& c : myConnections) {
            std::cout << "  conn " << c.getDescription(this) << "\n";
        }
    }
#endif
    return true;
}


void
NBEdge::divideOnEdges(const EdgeVector* outgoing) {
    if (outgoing->size() == 0) {
        // we have to do this, because the turnaround may have been added before
        myConnections.clear();
        return;
    }

#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << " divideOnEdges " << getID() << " outgoing=" << toString(*outgoing) << "\n";
    }
#endif

    // build connections for miv lanes
    std::vector<int> availableLanes;
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        if ((getPermissions(i) & SVC_PASSENGER) != 0) {
            availableLanes.push_back(i);
        }
    }
    if (availableLanes.size() > 0) {
        divideSelectedLanesOnEdges(outgoing, availableLanes);
    }
    // build connections for miscellaneous further modes (more than bike,peds,bus and without passenger)
    availableLanes.clear();
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        const SVCPermissions perms = getPermissions(i);
        if ((perms & ~(SVC_PEDESTRIAN | SVC_BICYCLE | SVC_BUS)) == 0 || (perms & SVC_PASSENGER) != 0 || isForbidden(perms)) {
            continue;
        }
        availableLanes.push_back(i);
    }
    if (availableLanes.size() > 0) {
        divideSelectedLanesOnEdges(outgoing, availableLanes);
    }
    // build connections for busses (possibly combined with bicycles)
    availableLanes.clear();
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        const SVCPermissions perms = getPermissions(i);
        if (perms != SVC_BUS && perms != (SVC_BUS | SVC_BICYCLE)) {
            continue;
        }
        availableLanes.push_back(i);
    }
    if (availableLanes.size() > 0) {
        divideSelectedLanesOnEdges(outgoing, availableLanes);
    }
    // build connections for bicycles (possibly combined with pedestrians)
    availableLanes.clear();
    for (int i = 0; i < (int)myLanes.size(); ++i) {
        const SVCPermissions perms = getPermissions(i);
        if (perms != SVC_BICYCLE && perms != (SVC_BICYCLE | SVC_PEDESTRIAN)) {
            continue;
        }
        availableLanes.push_back(i);
    }
    if (availableLanes.size() > 0) {
        divideSelectedLanesOnEdges(outgoing, availableLanes);
    }
    // clean up unassigned fromLanes
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end();) {
        if ((*i).fromLane == -1) {
            i = myConnections.erase(i);
        } else {
            ++i;
        }
    }
    sortOutgoingConnectionsByIndex();
}


void
NBEdge::divideSelectedLanesOnEdges(const EdgeVector* outgoing, const std::vector<int>& availableLanes) {
    const std::vector<int>& priorities = prepareEdgePriorities(outgoing, availableLanes);
    if (priorities.empty()) {
        return;
    }
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "divideSelectedLanesOnEdges " << getID() << " out=" << toString(*outgoing) << " prios=" << toString(priorities) << " avail=" << toString(availableLanes) << "\n";
    }
#endif
    // compute the resulting number of lanes that should be used to reach the following edge
    const int numOutgoing = (int)outgoing->size();
    std::vector<int> resultingLanesFactor;
    resultingLanesFactor.reserve(numOutgoing);
    int minResulting = std::numeric_limits<int>::max();
    for (int i = 0; i < numOutgoing; i++) {
        // res / minResulting will be the number of lanes which are meant to reach the current outgoing edge
        const int res = priorities[i] * (int)availableLanes.size();
        resultingLanesFactor.push_back(res);
        if (minResulting > res && res > 0) {
            // prevent minResulting from becoming 0
            minResulting = res;
        }
    }
    // compute the number of virtual edges
    //  a virtual edge is used as a replacement for a real edge from now on
    //  it shall allow to divide the existing lanes on this structure without
    //  regarding the structure of outgoing edges
    int numVirtual = 0;
    // compute the transition from virtual to real edges
    EdgeVector transition;
    transition.reserve(numOutgoing);
    for (int i = 0; i < numOutgoing; i++) {
        // tmpNum will be the number of connections from this edge to the next edge
        assert(i < (int)resultingLanesFactor.size());
        const int tmpNum = (resultingLanesFactor[i] + minResulting - 1) / minResulting; // integer division rounding up
        numVirtual += tmpNum;
        for (int j = 0; j < tmpNum; j++) {
            transition.push_back((*outgoing)[i]);
        }
    }
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "   minResulting=" << minResulting << " numVirtual=" << numVirtual << " availLanes=" << toString(availableLanes) << " resLanes=" << toString(resultingLanesFactor) << " transition=" << toString(transition) << "\n";
    }
#endif

    // assign lanes to edges
    //  (conversion from virtual to real edges is done)
    ToEdgeConnectionsAdder adder(transition);
    Bresenham::compute(&adder, static_cast<int>(availableLanes.size()), numVirtual);
    const std::map<NBEdge*, std::vector<int> >& l2eConns = adder.getBuiltConnections();
    for (NBEdge* const target : *outgoing) {
        assert(l2eConns.find(target) != l2eConns.end());
        for (const int j : l2eConns.find(target)->second) {
            const int fromIndex = availableLanes[j];
            if ((getPermissions(fromIndex) & target->getPermissions()) == 0) {
                // exclude connection if fromLane and toEdge have no common permissions
                continue;
            }
            if ((getPermissions(fromIndex) & target->getPermissions()) == SVC_PEDESTRIAN) {
                // exclude connection if the only commonly permitted class are pedestrians
                // these connections are later built in NBNode::buildWalkingAreas
                continue;
            }
            // avoid building more connections than the edge has viable lanes (earlier
            // ones have precedence). This is necessary when running divideSelectedLanesOnEdges more than once.
            //    @todo To decide which target lanes are still available we need to do a
            // preliminary lane-to-lane assignment in regard to permissions (rather than to ordering)
            const int numConsToTarget = (int)count_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(target, true));
            int targetLanes = target->getNumLanes();
            if (target->getPermissions(0) == SVC_PEDESTRIAN) {
                --targetLanes;
            }
            if (numConsToTarget >= targetLanes) {
                // let bicycles move onto the road to allow continuation
                // the speed limit is taken from rural roads (which allow cycles)
                // (pending implementation of #1859)
                if (getPermissions(fromIndex) == SVC_BICYCLE && getSpeed() <= (101 / 3.6)) {
                    for (NBEdge::Lane& lane : myLanes) {
                        if (lane.permissions != SVC_PEDESTRIAN) {
                            lane.permissions |= SVC_BICYCLE;
                        }
                    }
                }
                continue;
            }
            if (myLanes[fromIndex].connectionsDone) {
                // we already have complete information about connections from
                // this lane. do not add anything else
#ifdef DEBUG_CONNECTION_GUESSING
                if (DEBUGCOND) {
                    std::cout << "     connectionsDone from " << getID() << "_" << fromIndex << ": ";
                    for (const Connection& c : getConnectionsFromLane(fromIndex)) {
                        std::cout << c.getDescription(this) << ", ";
                    }
                    std::cout << "\n";
                }
#endif
                continue;
            }
            myConnections.push_back(Connection(fromIndex, target, -1));
#ifdef DEBUG_CONNECTION_GUESSING
            if (DEBUGCOND) {
                std::cout << "     request connection from " << getID() << "_" << fromIndex << " to " << target->getID() << "\n";
            }
#endif
        }
    }

    addStraightConnections(outgoing, availableLanes, priorities);
}


void
NBEdge::addStraightConnections(const EdgeVector* outgoing, const std::vector<int>& availableLanes, const std::vector<int>& priorities) {
    // ensure sufficient straight connections for the (highest-priority) straight target
    const int numOutgoing = (int) outgoing->size();
    NBEdge* target = nullptr;
    NBEdge* rightOfTarget = nullptr;
    NBEdge* leftOfTarget = nullptr;
    int maxPrio = 0;
    for (int i = 0; i < numOutgoing; i++) {
        if (maxPrio < priorities[i]) {
            const LinkDirection dir = myTo->getDirection(this, (*outgoing)[i]);
            if (dir == LinkDirection::STRAIGHT) {
                maxPrio = priorities[i];
                target = (*outgoing)[i];
                rightOfTarget = i == 0 ? outgoing->back() : (*outgoing)[i - 1];
                leftOfTarget = i + 1 == numOutgoing ? outgoing->front() : (*outgoing)[i + 1];
            }
        }
    }
    if (target == nullptr) {
        return;
    }
    int numConsToTarget = (int)count_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(target, true));
    int targetLanes = (int)target->getNumLanes();
    if (target->getPermissions(0) == SVC_PEDESTRIAN) {
        --targetLanes;
    }
    const int numDesiredConsToTarget = MIN2(targetLanes, (int)availableLanes.size());
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) {
        std::cout << "  checking extra lanes for target=" << target->getID() << " cons=" << numConsToTarget << " desired=" << numDesiredConsToTarget << "\n";
    }
#endif
    std::vector<int>::const_iterator it_avail = availableLanes.begin();
    while (numConsToTarget < numDesiredConsToTarget && it_avail != availableLanes.end()) {
        const int fromIndex = *it_avail;
        if (
            // not yet connected
            (count_if(myConnections.begin(), myConnections.end(), connections_finder(fromIndex, target, -1)) == 0)
            // matching permissions
            && ((getPermissions(fromIndex) & target->getPermissions()) != 0)
            // more than pedestrians
            && ((getPermissions(fromIndex) & target->getPermissions()) != SVC_PEDESTRIAN)
            // lane not yet fully defined
            && !myLanes[fromIndex].connectionsDone
        ) {
#ifdef DEBUG_CONNECTION_GUESSING
            if (DEBUGCOND) {
                std::cout << "    candidate from " << getID() << "_" << fromIndex << " to " << target->getID() << "\n";
            }
#endif
            // prevent same-edge conflicts
            if (
                // no outgoing connections to the right from further left
                ((it_avail + 1) == availableLanes.end() || count_if(myConnections.begin(), myConnections.end(), connections_conflict_finder(fromIndex, rightOfTarget, false)) == 0)
                // no outgoing connections to the left from further right
                && (it_avail == availableLanes.begin() || count_if(myConnections.begin(), myConnections.end(), connections_conflict_finder(fromIndex, leftOfTarget, true)) == 0)) {
#ifdef DEBUG_CONNECTION_GUESSING
                if (DEBUGCOND) {
                    std::cout << "     request additional connection from " << getID() << "_" << fromIndex << " to " << target->getID() << "\n";
                }
#endif
                myConnections.push_back(Connection(fromIndex, target, -1));
                numConsToTarget++;
            } else {
#ifdef DEBUG_CONNECTION_GUESSING
                if (DEBUGCOND) std::cout
                            << "     fail check1="
                            << ((it_avail + 1) == availableLanes.end() || count_if(myConnections.begin(), myConnections.end(), connections_conflict_finder(fromIndex, rightOfTarget, false)) == 0)
                            << " check2=" << (it_avail == availableLanes.begin() || count_if(myConnections.begin(), myConnections.end(), connections_conflict_finder(fromIndex, leftOfTarget, true)) == 0)
                            << " rightOfTarget=" << rightOfTarget->getID()
                            << " leftOfTarget=" << leftOfTarget->getID()
                            << "\n";
#endif

            }
        }
        ++it_avail;
    }
}


const std::vector<int>
NBEdge::prepareEdgePriorities(const EdgeVector* outgoing, const std::vector<int>& availableLanes) {
    std::vector<int> priorities;
    MainDirections mainDirections(*outgoing, this, myTo, availableLanes);
    const int dist = mainDirections.getStraightest();
    if (dist == -1) {
        return priorities;
    }
    // copy the priorities first
    priorities.reserve(outgoing->size());
    for (const NBEdge* const out : *outgoing) {
        int prio = NBNode::isTrafficLight(myTo->getType()) ? 0 : out->getJunctionPriority(myTo);
        assert((prio + 1) * 2 > 0);
        prio = (prio + 1) * 2;
        priorities.push_back(prio);
    }
    // when the right turning direction has not a higher priority, divide
    //  the importance by 2 due to the possibility to leave the junction
    //  faster from this lane
#ifdef DEBUG_CONNECTION_GUESSING
    if (DEBUGCOND) std::cout << "  prepareEdgePriorities " << getID()
                                 << " outgoing=" << toString(*outgoing)
                                 << " priorities1=" << toString(priorities)
                                 << " dist=" << dist
                                 << "\n";
#endif
    if (dist != 0 && !mainDirections.includes(MainDirections::Direction::RIGHTMOST)) {
        assert(priorities.size() > 0);
        priorities[0] /= 2;
#ifdef DEBUG_CONNECTION_GUESSING
        if (DEBUGCOND) {
            std::cout << "   priorities2=" << toString(priorities) << "\n";
        }
#endif
    }
    // HEURISTIC:
    // when no higher priority exists, let the forward direction be
    //  the main direction
    if (mainDirections.empty()) {
        assert(dist < (int)priorities.size());
        priorities[dist] *= 2;
#ifdef DEBUG_CONNECTION_GUESSING
        if (DEBUGCOND) {
            std::cout << "   priorities3=" << toString(priorities) << "\n";
        }
#endif
    }
    if (NBNode::isTrafficLight(myTo->getType())) {
        priorities[dist] += 1;
    } else {
        // try to ensure separation of left turns
        if (mainDirections.includes(MainDirections::Direction::RIGHTMOST) && mainDirections.includes(MainDirections::Direction::LEFTMOST)) {
            priorities[0] /= 4;
            priorities[(int)priorities.size() - 1] /= 2;
#ifdef DEBUG_CONNECTION_GUESSING
            if (DEBUGCOND) {
                std::cout << "   priorities6=" << toString(priorities) << "\n";
            }
#endif
        }
    }
    if (mainDirections.includes(MainDirections::Direction::FORWARD)) {
        if (myLanes.size() > 2) {
            priorities[dist] *= 2;
#ifdef DEBUG_CONNECTION_GUESSING
            if (DEBUGCOND) {
                std::cout << "   priorities4=" << toString(priorities) << "\n";
            }
#endif
        } else {
            priorities[dist] *= 3;
#ifdef DEBUG_CONNECTION_GUESSING
            if (DEBUGCOND) {
                std::cout << "   priorities5=" << toString(priorities) << "\n";
            }
#endif
        }
    }
    return priorities;
}


void
NBEdge::appendTurnaround(bool noTLSControlled, bool noFringe, bool onlyDeadends, bool onlyTurnlane, bool noGeometryLike, bool checkPermissions) {
    // do nothing if no turnaround is known
    if (myTurnDestination == nullptr || myTo->getType() == SumoXMLNodeType::RAIL_CROSSING) {
        return;
    }
    // do nothing if the destination node is controlled by a tls and no turnarounds
    //  shall be appended for such junctions
    if (noTLSControlled && myTo->isTLControlled()) {
        return;
    }
    if (noFringe && myTo->getFringeType() == FringeType::OUTER) {
        return;
    }
    bool isDeadEnd = true;
    for (const Connection& c : myConnections) {
        if ((c.toEdge->getPermissions(c.toLane)
                & getPermissions(c.fromLane)
                & SVC_PASSENGER) != 0
                || (c.toEdge->getPermissions() & getPermissions()) == getPermissions()) {
            isDeadEnd = false;
            break;
        }
    }
    if (onlyDeadends && !isDeadEnd) {
        return;
    }
    const int fromLane = (int)myLanes.size() - 1;
    if (onlyTurnlane) {
        for (const Connection& c : getConnectionsFromLane(fromLane)) {
            LinkDirection dir = myTo->getDirection(this, c.toEdge);
            if (dir != LinkDirection::LEFT && dir != LinkDirection::PARTLEFT) {
                return;
            }
        }
    }
    const int toLane = (int)myTurnDestination->getNumLanes() - 1;
    if (checkPermissions) {
        if ((getPermissions(fromLane) & myTurnDestination->getPermissions(toLane)) == 0) {
            // exclude connection if fromLane and toEdge have no common permissions
            return;
        }
        if ((getPermissions(fromLane) & myTurnDestination->getPermissions(toLane)) == SVC_PEDESTRIAN) {
            // exclude connection if the only commonly permitted class are pedestrians
            // these connections are later built in NBNode::buildWalkingAreas
            return;
        }
    }
    // avoid railway turn-arounds
    if (isRailway(getPermissions()) && isRailway(myTurnDestination->getPermissions())
            && fabs(NBHelpers::normRelAngle(getAngleAtNode(myTo), myTurnDestination->getAngleAtNode(myTo))) > 90) {
        // except at dead-ends on bidi-edges where they model a reversal in train direction
        // @todo #4382: once the network fringe is tagged, it also should not receive turn-arounds)
        if (isBidiRail() && isRailDeadEnd()) {
            // add a slow connection because direction-reversal implies stopping
            setConnection(fromLane, myTurnDestination, toLane, Lane2LaneInfoType::VALIDATED, false, false, true, UNSPECIFIED_CONTPOS, UNSPECIFIED_VISIBILITY_DISTANCE, SUMO_const_haltingSpeed);
            return;
        } else {
            return;
        }
    };
    if (noGeometryLike && myTo->geometryLike() && !isDeadEnd) {
        // make sure the turnDestination has other incoming edges
        EdgeVector turnIncoming = myTurnDestination->getIncomingEdges();
        if (turnIncoming.size() > 1) {
            // this edge is always part of incoming
            return;
        }
    }
    setConnection(fromLane, myTurnDestination, toLane, Lane2LaneInfoType::VALIDATED);
}


bool
NBEdge::isTurningDirectionAt(const NBEdge* const edge) const {
    // maybe it was already set as the turning direction
    if (edge == myTurnDestination) {
        return true;
    } else if (myTurnDestination != nullptr) {
        // otherwise - it's not if a turning direction exists
        return false;
    }
    return edge == myPossibleTurnDestination;
}


NBNode*
NBEdge::tryGetNodeAtPosition(double pos, double tolerance) const {
    // return the from-node when the position is at the begin of the edge
    if (pos < tolerance) {
        return myFrom;
    }
    // return the to-node when the position is at the end of the edge
    if (pos > myLength - tolerance) {
        return myTo;
    }
    return nullptr;
}


void
NBEdge::moveOutgoingConnectionsFrom(NBEdge* e, int laneOff) {
    int lanes = e->getNumLanes();
    for (int i = 0; i < lanes; i++) {
        std::vector<NBEdge::Connection> elv = e->getConnectionsFromLane(i);
        for (std::vector<NBEdge::Connection>::iterator j = elv.begin(); j != elv.end(); j++) {
            NBEdge::Connection el = *j;
            assert(el.tlID == "");
            addLane2LaneConnection(i + laneOff, el.toEdge, el.toLane, Lane2LaneInfoType::COMPUTED);
        }
    }
}


bool
NBEdge::lanesWereAssigned() const {
    return myStep == EdgeBuildingStep::LANES2LANES_DONE || myStep == EdgeBuildingStep::LANES2LANES_USER;
}


double
NBEdge::getMaxLaneOffset() {
    return (double) SUMO_const_laneWidthAndOffset * myLanes.size();
}


bool
NBEdge::mayBeTLSControlled(int fromLane, NBEdge* toEdge, int toLane) const {
    for (const Connection& c : myConnections) {
        if (c.fromLane == fromLane && c.toEdge == toEdge && c.toLane == toLane && c.uncontrolled) {
            return false;
        }
    }
    return true;
}


bool
NBEdge::setControllingTLInformation(const NBConnection& c, const std::string& tlID) {
    const int fromLane = c.getFromLane();
    NBEdge* toEdge = c.getTo();
    const int toLane = c.getToLane();
    const int tlIndex = c.getTLIndex();
    const int tlIndex2 = c.getTLIndex2();
    // check whether the connection was not set as not to be controled previously
    if (!mayBeTLSControlled(fromLane, toEdge, toLane)) {
        return false;
    }

    assert(fromLane < 0 || fromLane < (int) myLanes.size());
    // try to use information about the connections if given
    if (fromLane >= 0 && toLane >= 0) {
        // find the specified connection
        std::vector<Connection>::iterator i =
            find_if(myConnections.begin(), myConnections.end(), connections_finder(fromLane, toEdge, toLane));
        // ok, we have to test this as on the removal of self-loop edges some connections
        //  will be reassigned
        if (i != myConnections.end()) {
            // get the connection
            Connection& connection = *i;
            // set the information about the tl
            connection.tlID = tlID;
            connection.tlLinkIndex = tlIndex;
            connection.tlLinkIndex2 = tlIndex2;
            return true;
        }
    }
    // if the original connection was not found, set the information for all
    //  connections
    int no = 0;
    bool hadError = false;
    for (std::vector<Connection>::iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if ((*i).toEdge != toEdge) {
            continue;
        }
        if (fromLane >= 0 && fromLane != (*i).fromLane) {
            continue;
        }
        if (toLane >= 0 && toLane != (*i).toLane) {
            continue;
        }
        if ((*i).tlID == "") {
            (*i).tlID = tlID;
            (*i).tlLinkIndex = tlIndex;
            (*i).tlLinkIndex2 = tlIndex2;
            no++;
        } else {
            if ((*i).tlID != tlID && (*i).tlLinkIndex == tlIndex) {
                WRITE_WARNINGF("The lane '%' on edge '%' already had a traffic light signal.", i->fromLane, getID());
                hadError = true;
            }
        }
    }
    if (hadError && no == 0) {
        WRITE_WARNINGF("Could not set any signal of the tlLogic '%' (unknown group).", tlID);
    }
    return true;
}


void
NBEdge::clearControllingTLInformation() {
    for (std::vector<Connection>::iterator it = myConnections.begin(); it != myConnections.end(); it++) {
        it->tlID = "";
    }
}


PositionVector
NBEdge::getCWBoundaryLine(const NBNode& n) const {
    PositionVector ret;
    double width;
    int lane;
    if (myFrom == (&n)) {
        // outgoing
        lane = getFirstAllowedLaneIndex(NBNode::FORWARD);
        ret = myLanes[lane].shape;
    } else {
        // incoming
        lane = getFirstAllowedLaneIndex(NBNode::BACKWARD);
        ret = myLanes[lane].shape.reverse();
    }
    width = getLaneWidth(lane);
    ret.move2side(width * 0.5);
    return ret;
}


PositionVector
NBEdge::getCCWBoundaryLine(const NBNode& n) const {
    PositionVector ret;
    double width;
    int lane;
    if (myFrom == (&n)) {
        // outgoing
        lane = getFirstAllowedLaneIndex(NBNode::BACKWARD);
        ret = myLanes[lane].shape;
    } else {
        // incoming
        lane = getFirstAllowedLaneIndex(NBNode::FORWARD);
        ret = myLanes[lane].shape.reverse();
    }
    width = getLaneWidth(lane);
    ret.move2side(-width * 0.5);
    return ret;
}


bool
NBEdge::expandableBy(NBEdge* possContinuation, std::string& reason) const {
    // ok, the number of lanes must match
    if (myLanes.size() != possContinuation->myLanes.size()) {
        reason = "laneNumber";
        return false;
    }
    // do not create self loops
    if (myFrom == possContinuation->myTo) {
        reason = "loop";
        return false;
    }
    // conserve bidi-rails
    if (isBidiRail() != possContinuation->isBidiRail()) {
        reason = "bidi-rail";
        return false;
    }
    // also, check whether the connections - if any exit do allow to join
    //  both edges
    // This edge must have a one-to-one connection to the following lanes
    switch (myStep) {
        case EdgeBuildingStep::INIT_REJECT_CONNECTIONS:
            break;
        case EdgeBuildingStep::INIT:
            break;
        case EdgeBuildingStep::EDGE2EDGES: {
            // the following edge must be connected
            const EdgeVector& conn = getConnectedEdges();
            if (find(conn.begin(), conn.end(), possContinuation) == conn.end()) {
                reason = "disconnected";
                return false;
            }
        }
        break;
        case EdgeBuildingStep::LANES2EDGES:
        case EdgeBuildingStep::LANES2LANES_RECHECK:
        case EdgeBuildingStep::LANES2LANES_DONE:
        case EdgeBuildingStep::LANES2LANES_USER: {
            // the possible continuation must be connected
            if (find_if(myConnections.begin(), myConnections.end(), connections_toedge_finder(possContinuation)) == myConnections.end()) {
                reason = "disconnected";
                return false;
            }
            // all lanes must go to the possible continuation
            std::vector<int> conns = getConnectionLanes(possContinuation);
            const int offset = MAX2(0, getFirstNonPedestrianLaneIndex(NBNode::FORWARD, true));
            if (conns.size() < myLanes.size() - offset) {
                reason = "some lanes disconnected";
                return false;
            }
        }
        break;
        default:
            break;
    }
    const double minLength = OptionsCont::getOptions().getFloat("geometry.remove.min-length");
    if (minLength > 0 && (possContinuation->getLoadedLength() < minLength || getLoadedLength() < minLength)) {
        return true;
    }
    // the priority, too (?)
    if (getPriority() != possContinuation->getPriority()) {
        reason = "priority";
        return false;
    }
    // the speed allowed
    if (mySpeed != possContinuation->mySpeed) {
        reason = "speed";
        return false;
    }
    // spreadtype should match or it will look ugly
    if (myLaneSpreadFunction != possContinuation->myLaneSpreadFunction) {
        reason = "spreadType";
        return false;
    }
    // matching lanes must have identical properties
    for (int i = 0; i < (int)myLanes.size(); i++) {
        if (myLanes[i].speed != possContinuation->myLanes[i].speed) {
            reason = "lane " + toString(i) + " speed";
            return false;
        } else if (myLanes[i].permissions != possContinuation->myLanes[i].permissions) {
            reason = "lane " + toString(i) + " permissions";
            return false;
        } else if (myLanes[i].width != possContinuation->myLanes[i].width &&
                   fabs(myLanes[i].width - possContinuation->myLanes[i].width) > OptionsCont::getOptions().getFloat("geometry.remove.width-tolerance")) {
            reason = "lane " + toString(i) + " width";
            return false;
        }
    }

    return true;
}


void
NBEdge::append(NBEdge* e) {
    // append geometry
    myGeom.append(e->myGeom);
    for (int i = 0; i < (int)myLanes.size(); i++) {
        myLanes[i].customShape.append(e->myLanes[i].customShape);
        if (myLanes[i].knowsParameter(SUMO_PARAM_ORIGID) || e->myLanes[i].knowsParameter(SUMO_PARAM_ORIGID)
                || OptionsCont::getOptions().getBool("output.original-names")) {
            const std::string origID = myLanes[i].getParameter(SUMO_PARAM_ORIGID, getID());
            const std::string origID2 = e->myLanes[i].getParameter(SUMO_PARAM_ORIGID, e->getID());
            if (origID != origID2) {
                myLanes[i].setParameter(SUMO_PARAM_ORIGID, origID + " " + origID2);
            }
        }
        myLanes[i].connectionsDone = e->myLanes[i].connectionsDone;
    }
    if (e->getLength() > myLength) {
        // possibly some lane attributes differ (when using option geometry.remove.min-length)
        // make sure to use the attributes from the longer edge
        for (int i = 0; i < (int)myLanes.size(); i++) {
            myLanes[i].width = e->myLanes[i].width;
        }
    }
    // recompute length
    myLength += e->myLength;
    if (myLoadedLength > 0 || e->myLoadedLength > 0) {
        myLoadedLength = getFinalLength() + e->getFinalLength();
    }
    // copy the connections and the building step if given
    myStep = e->myStep;
    myConnections = e->myConnections;
    myTurnDestination = e->myTurnDestination;
    myPossibleTurnDestination = e->myPossibleTurnDestination;
    myConnectionsToDelete = e->myConnectionsToDelete;
    // set the node
    myTo = e->myTo;
    myToBorder = e->myToBorder;
    if (e->knowsParameter("origTo")) {
        setParameter("origTo", e->getParameter("origTo"));
    }
    if (e->mySignalPosition != Position::INVALID) {
        mySignalPosition = e->mySignalPosition;
    }
    computeAngle(); // myEndAngle may be different now
}


bool
NBEdge::hasSignalisedConnectionTo(const NBEdge* const e) const {
    for (std::vector<Connection>::const_iterator i = myConnections.begin(); i != myConnections.end(); ++i) {
        if ((*i).toEdge == e && (*i).tlID != "") {
            return true;
        }
    }
    return false;
}


NBEdge*
NBEdge::getTurnDestination(bool possibleDestination) const {
    if (myTurnDestination == nullptr && possibleDestination) {
        return myPossibleTurnDestination;
    }
    return myTurnDestination;
}


std::string
NBEdge::getLaneID(int lane) const {
    return myID + "_" + toString(lane);
}


bool
NBEdge::isNearEnough2BeJoined2(NBEdge* e, double threshold) const {
    std::vector<double> distances = myGeom.distances(e->getGeometry());
    assert(distances.size() > 0);
    return VectorHelper<double>::maxValue(distances) < threshold;
}


void
NBEdge::addLane(int index, bool recomputeShape, bool recomputeConnections, bool shiftIndices) {
    assert(index <= (int)myLanes.size());
    myLanes.insert(myLanes.begin() + index, Lane(this, ""));
    // copy attributes
    if (myLanes.size() > 1) {
        int templateIndex = index > 0 ? index - 1 : index + 1;
        myLanes[index].speed = myLanes[templateIndex].speed;
        myLanes[index].permissions = myLanes[templateIndex].permissions;
        myLanes[index].preferred = myLanes[templateIndex].preferred;
        myLanes[index].endOffset = myLanes[templateIndex].endOffset;
        myLanes[index].width = myLanes[templateIndex].width;
        myLanes[index].updateParameters(myLanes[templateIndex].getParametersMap());
    }
    const EdgeVector& incs = myFrom->getIncomingEdges();
    if (recomputeShape) {
        computeLaneShapes();
    }
    if (recomputeConnections) {
        for (EdgeVector::const_iterator i = incs.begin(); i != incs.end(); ++i) {
            (*i)->invalidateConnections(true);
        }
        invalidateConnections(true);
    } else if (shiftIndices) {
        // shift outgoing connections above the added lane to the left
        for (Connection& c : myConnections) {
            if (c.fromLane >= index) {
                c.fromLane += 1;
            }
        }
        // shift incoming connections above the added lane to the left
        for (NBEdge* inc : myFrom->getIncomingEdges()) {
            for (Connection& c : inc->myConnections) {
                if (c.toEdge == this && c.toLane >= index) {
                    c.toLane += 1;
                }
            }
        }
        myFrom->shiftTLConnectionLaneIndex(this, +1, index - 1);
        myTo->shiftTLConnectionLaneIndex(this, +1, index - 1);
    }
}

void
NBEdge::incLaneNo(int by) {
    int newLaneNo = (int)myLanes.size() + by;
    while ((int)myLanes.size() < newLaneNo) {
        // recompute shapes on last addition
        const bool recompute = ((int)myLanes.size() == newLaneNo - 1) && myStep < EdgeBuildingStep::LANES2LANES_USER;
        addLane((int)myLanes.size(), recompute, recompute, false);
    }
}


void
NBEdge::deleteLane(int index, bool recompute, bool shiftIndices) {
    assert(index < (int)myLanes.size());
    myLanes.erase(myLanes.begin() + index);
    if (recompute) {
        computeLaneShapes();
        const EdgeVector& incs = myFrom->getIncomingEdges();
        for (EdgeVector::const_iterator i = incs.begin(); i != incs.end(); ++i) {
            (*i)->invalidateConnections(true);
        }
        invalidateConnections(true);
    } else if (shiftIndices) {
        removeFromConnections(nullptr, index, -1, false, true);
        for (NBEdge* inc : myFrom->getIncomingEdges()) {
            inc->removeFromConnections(this, -1, index, false, true);
        }
    }
}


void
NBEdge::decLaneNo(int by) {
    int newLaneNo = (int) myLanes.size() - by;
    assert(newLaneNo > 0);
    while ((int)myLanes.size() > newLaneNo) {
        // recompute shapes on last removal
        const bool recompute = (int)myLanes.size() == newLaneNo + 1 && myStep < EdgeBuildingStep::LANES2LANES_USER;
        deleteLane((int)myLanes.size() - 1, recompute, false);
    }
}


void
NBEdge::markAsInLane2LaneState() {
    assert(myTo->getOutgoingEdges().size() == 0);
    myStep = EdgeBuildingStep::LANES2LANES_DONE;
}


void
NBEdge::allowVehicleClass(int lane, SUMOVehicleClass vclass) {
    if (lane < 0) { // all lanes are meant...
        for (int i = 0; i < (int)myLanes.size(); i++) {
            allowVehicleClass(i, vclass);
        }
    } else {
        assert(lane < (int)myLanes.size());
        myLanes[lane].permissions |= vclass;
    }
}


void
NBEdge::disallowVehicleClass(int lane, SUMOVehicleClass vclass) {
    if (lane < 0) { // all lanes are meant...
        for (int i = 0; i < (int)myLanes.size(); i++) {
            disallowVehicleClass((int) i, vclass);
        }
    } else {
        assert(lane < (int)myLanes.size());
        myLanes[lane].permissions &= ~vclass;
    }
}


void
NBEdge::preferVehicleClass(int lane, SUMOVehicleClass vclass) {
    if (lane < 0) { // all lanes are meant...
        for (int i = 0; i < (int)myLanes.size(); i++) {
            allowVehicleClass(i, vclass);
        }
    } else {
        assert(lane < (int)myLanes.size());
        myLanes[lane].preferred |= vclass;
    }
}


void
NBEdge::setLaneWidth(int lane, double width) {
    if (lane < 0) {
        // all lanes are meant...
        myLaneWidth = width;
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setLaneWidth(i, width);
        }
        return;
    }
    assert(lane < (int)myLanes.size());
    myLanes[lane].width = width;
}

void
NBEdge::setLaneType(int lane, const std::string& type) {
    if (lane < 0) {
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setLaneType(i, type);
        }
        return;
    }
    assert(lane < (int)myLanes.size());
    myLanes[lane].type = type;
}


double
NBEdge::getLaneWidth(int lane) const {
    return myLanes[lane].width != UNSPECIFIED_WIDTH
           ? myLanes[lane].width
           : getLaneWidth() != UNSPECIFIED_WIDTH ? getLaneWidth() : SUMO_const_laneWidth;
}


double
NBEdge::getTotalWidth() const {
    double result = 0;
    for (int i = 0; i < (int)myLanes.size(); i++) {
        result += getLaneWidth(i);
    }
    return result;
}

double
NBEdge::getEndOffset(int lane) const {
    return myLanes[lane].endOffset != UNSPECIFIED_OFFSET ? myLanes[lane].endOffset : getEndOffset();
}


const std::map<SVCPermissions, double>&
NBEdge::getStopOffsets(int lane) const {
    if (lane == -1) {
        return myStopOffsets;
    } else {
        return myLanes[lane].stopOffsets;
    }
}

void
NBEdge::setEndOffset(int lane, double offset) {
    if (lane < 0) {
        // all lanes are meant...
        myEndOffset = offset;
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setEndOffset(i, offset);
        }
        return;
    }
    assert(lane < (int)myLanes.size());
    myLanes[lane].endOffset = offset;
}


bool
NBEdge::setStopOffsets(int lane, std::map<int, double> offsets, bool overwrite) {
    if (lane < 0) {
        if (!overwrite && myStopOffsets.size() != 0) {
            return false;
        }
        // all lanes are meant...
        if (offsets.size() != 0 && 0 > offsets.begin()->second) {
            // Edge length unknown at parsing time, thus check here.
            WRITE_WARNINGF("Ignoring invalid stopOffset for edge '%' (negative offset).", getID());
            return false;
        } else {
            myStopOffsets = offsets;
        }
    } else {
        assert(lane < (int)myLanes.size());
        if (myLanes[lane].stopOffsets.size() == 0 || overwrite) {
            if (offsets.size() != 0 && 0 > offsets.begin()->second) {
                // Edge length unknown at parsing time, thus check here.
                WRITE_WARNINGF("Ignoring invalid stopOffset for lane '%' (negative offset).", getLaneID(lane));
            } else {
                myLanes[lane].stopOffsets = offsets;
            }
        }
    }
    return true;
}


void
NBEdge::setSpeed(int lane, double speed) {
    if (lane < 0) {
        // all lanes are meant...
        mySpeed = speed;
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setSpeed(i, speed);
        }
        return;
    }
    assert(lane < (int)myLanes.size());
    myLanes[lane].speed = speed;
}

void
NBEdge::setAcceleration(int lane, bool accelRamp) {
    assert(lane >= 0);
    assert(lane < (int)myLanes.size());
    myLanes[lane].accelRamp = accelRamp;
}

void
NBEdge::setLaneShape(int lane, const PositionVector& shape) {
    assert(lane >= 0);
    assert(lane < (int)myLanes.size());
    myLanes[lane].customShape = shape;
}


void
NBEdge::setPermissions(SVCPermissions permissions, int lane) {
    if (lane < 0) {
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setPermissions(permissions, i);
        }
    } else {
        assert(lane < (int)myLanes.size());
        myLanes[lane].permissions = permissions;
    }
}


void
NBEdge::setPreferredVehicleClass(SVCPermissions permissions, int lane) {
    if (lane < 0) {
        for (int i = 0; i < (int)myLanes.size(); i++) {
            // ... do it for each lane
            setPreferredVehicleClass(permissions, i);
        }
    } else {
        assert(lane < (int)myLanes.size());
        myLanes[lane].preferred = permissions;
    }
}


SVCPermissions
NBEdge::getPermissions(int lane) const {
    if (lane < 0) {
        SVCPermissions result = 0;
        for (int i = 0; i < (int)myLanes.size(); i++) {
            result |= getPermissions(i);
        }
        return result;
    } else {
        assert(lane < (int)myLanes.size());
        return myLanes[lane].permissions;
    }
}


void
NBEdge::setLoadedLength(double val) {
    myLoadedLength = val;
}

void
NBEdge::setAverageLengthWithOpposite(double val) {
    myLength = val;
}


void
NBEdge::dismissVehicleClassInformation() {
    for (std::vector<Lane>::iterator i = myLanes.begin(); i != myLanes.end(); ++i) {
        (*i).permissions = SVCAll;
        (*i).preferred = 0;
    }
}


bool
NBEdge::connections_sorter(const Connection& c1, const Connection& c2) {
    if (c1.fromLane != c2.fromLane) {
        return c1.fromLane < c2.fromLane;
    }
    if (c1.toEdge != c2.toEdge) {
        return false; // do not change ordering among toEdges as this is determined by angle in an earlier step
    }
    return c1.toLane < c2.toLane;
}

double
NBEdge::getSignalOffset() const {
    if (mySignalPosition == Position::INVALID) {
        return UNSPECIFIED_SIGNAL_OFFSET;
    } else {
        Position laneEnd = myLaneSpreadFunction == LaneSpreadFunction::RIGHT ?
                           myLanes.back().shape.back() : myLanes[getNumLanes() / 2].shape.back();
        //std::cout << getID() << " signalPos=" << mySignalPosition << " laneEnd=" << laneEnd << " toShape=" << myTo->getShape() << " toBorder=" << myToBorder << "\n";
        return mySignalPosition.distanceTo2D(laneEnd);
    }
}

int
NBEdge::getFirstNonPedestrianLaneIndex(int direction, bool exclusive) const {
    assert(direction == NBNode::FORWARD || direction == NBNode::BACKWARD);
    const int start = (direction == NBNode::FORWARD ? 0 : (int)myLanes.size() - 1);
    const int end = (direction == NBNode::FORWARD ? (int)myLanes.size() : - 1);
    for (int i = start; i != end; i += direction) {
        // SVCAll, does not count as a sidewalk, green verges (permissions = 0) do not count as road
        // in the exclusive case, lanes that allow pedestrians along with any other class also count as road
        if ((exclusive && myLanes[i].permissions != SVC_PEDESTRIAN && myLanes[i].permissions != 0)
                || (myLanes[i].permissions == SVCAll || ((myLanes[i].permissions & SVC_PEDESTRIAN) == 0 && myLanes[i].permissions != 0))) {
            return i;
        }
    }
    return -1;
}

int
NBEdge::getSpecialLane(SVCPermissions permissions) const {
    for (int i = 0; i < (int)myLanes.size(); i++) {
        if (myLanes[i].permissions == permissions) {
            return i;
        }
    }
    return -1;
}

int
NBEdge::getFirstAllowedLaneIndex(int direction) const {
    assert(direction == NBNode::FORWARD || direction == NBNode::BACKWARD);
    const int start = (direction == NBNode::FORWARD ? 0 : (int)myLanes.size() - 1);
    const int end = (direction == NBNode::FORWARD ? (int)myLanes.size() : - 1);
    for (int i = start; i != end; i += direction) {
        if (myLanes[i].permissions != 0) {
            return i;
        }
    }
    return end - direction;
}


std::set<SVCPermissions>
NBEdge::getPermissionVariants(int iStart, int iEnd) const {
    std::set<SVCPermissions> result;
    if (iStart < 0 || iStart >= getNumLanes() || iEnd > getNumLanes()) {
        throw ProcessError("invalid indices iStart " + toString(iStart) + " iEnd " + toString(iEnd) + " for edge with " + toString(getNumLanes()) + " lanes.");
    }
    for (int i = iStart; i < iEnd; ++i) {
        result.insert(getPermissions(i));
    }
    return result;
}

int
NBEdge::getNumLanesThatAllow(SVCPermissions permissions) const {
    int result = 0;
    for (const Lane& lane : myLanes) {
        if ((lane.permissions & permissions) == permissions) {
            result++;
        }
    }
    return result;
}

double
NBEdge::getCrossingAngle(NBNode* node) {
    double angle = getAngleAtNode(node) + (getFromNode() == node ? 180.0 : 0.0);
    if (angle < 0) {
        angle += 360.0;
    }
    if (angle >= 360) {
        angle -= 360.0;
    }
    if (gDebugFlag1) {
        std::cout << getID() << " angle=" << getAngleAtNode(node) << " convAngle=" << angle << "\n";
    }
    return angle;
}


NBEdge::Lane
NBEdge::getFirstNonPedestrianLane(int direction) const {
    int index = getFirstNonPedestrianLaneIndex(direction);
    if (index < 0) {
        throw ProcessError("Edge " + getID() + " allows pedestrians on all lanes");
    }
    return myLanes[index];
}

std::string
NBEdge::getSidewalkID() {
    // see IntermodalEdge::getSidewalk()
    for (int i = 0; i < (int)myLanes.size(); i++) {
        if (myLanes[i].permissions == SVC_PEDESTRIAN) {
            return getLaneID(i);
        }
    }
    for (int i = 0; i < (int)myLanes.size(); i++) {
        if ((myLanes[i].permissions & SVC_PEDESTRIAN) != 0) {
            return getLaneID(i);
        }
    }
    return getLaneID(0);
}

void
NBEdge::addSidewalk(double width) {
    addRestrictedLane(width, SVC_PEDESTRIAN);
}


void
NBEdge::restoreSidewalk(std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections) {
    restoreRestrictedLane(SVC_PEDESTRIAN, oldLanes, oldGeometry, oldConnections);
}


void
NBEdge::addBikeLane(double width) {
    addRestrictedLane(width, SVC_BICYCLE);
}


void
NBEdge::restoreBikelane(std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections) {
    restoreRestrictedLane(SVC_BICYCLE, oldLanes, oldGeometry, oldConnections);
}

bool
NBEdge::hasRestrictedLane(SUMOVehicleClass vclass) const {
    for (const Lane& lane : myLanes) {
        if (lane.permissions == vclass) {
            return true;
        }
    }
    return false;
}


void
NBEdge::addRestrictedLane(double width, SUMOVehicleClass vclass) {
    if (hasRestrictedLane(vclass)) {
        WRITE_WARNINGF("Edge '%' already has a dedicated lane for %s. Not adding another one.", getID(), toString(vclass));
        return;
    }
    if (myLaneSpreadFunction == LaneSpreadFunction::CENTER) {
        myGeom.move2side(width / 2);
    }
    // disallow pedestrians on all lanes to ensure that sidewalks are used and
    // crossings can be guessed
    disallowVehicleClass(-1, vclass);
    // add new lane
    myLanes.insert(myLanes.begin(), Lane(this, myLanes[0].getParameter(SUMO_PARAM_ORIGID)));
    myLanes[0].permissions = vclass;
    myLanes[0].width = fabs(width);
    // shift outgoing connections to the left
    for (std::vector<Connection>::iterator it = myConnections.begin(); it != myConnections.end(); ++it) {
        Connection& c = *it;
        if (c.fromLane >= 0) {
            c.fromLane += 1;
        }
    }
    // shift incoming connections to the left
    const EdgeVector& incoming = myFrom->getIncomingEdges();
    for (EdgeVector::const_iterator it = incoming.begin(); it != incoming.end(); ++it) {
        (*it)->shiftToLanesToEdge(this, 1);
    }
    myFrom->shiftTLConnectionLaneIndex(this, 1);
    myTo->shiftTLConnectionLaneIndex(this, 1);
    computeLaneShapes();
}


void
NBEdge::restoreRestrictedLane(SUMOVehicleClass vclass, std::vector<NBEdge::Lane> oldLanes, PositionVector oldGeometry, std::vector<NBEdge::Connection> oldConnections) {
    // check that previously lane was transformed
    if (myLanes[0].permissions != vclass) {
        WRITE_WARNINGF("Edge '%' doesn't have a dedicated lane for %s. Cannot be restored.", getID(), toString(vclass));
        return;
    }
    // restore old values
    myGeom = oldGeometry;
    myLanes = oldLanes;
    myConnections = oldConnections;
    // shift incoming connections to the right
    const EdgeVector& incoming = myFrom->getIncomingEdges();
    for (EdgeVector::const_iterator it = incoming.begin(); it != incoming.end(); ++it) {
        (*it)->shiftToLanesToEdge(this, 0);
    }
    // Shift TL conections
    myFrom->shiftTLConnectionLaneIndex(this, 0);
    myTo->shiftTLConnectionLaneIndex(this, 0);
    computeLaneShapes();
}


void
NBEdge::shiftToLanesToEdge(NBEdge* to, int laneOff) {
    /// XXX could we repurpose the function replaceInConnections ?
    for (std::vector<Connection>::iterator it = myConnections.begin(); it != myConnections.end(); ++it) {
        if ((*it).toEdge == to && (*it).toLane >= 0) {
            (*it).toLane += laneOff;
        }
    }
}


void
NBEdge::shiftPositionAtNode(NBNode* node, NBEdge* other) {
    if (myLaneSpreadFunction == LaneSpreadFunction::CENTER && !isRailway(getPermissions())) {
        const int i = (node == myTo ? -1 : 0);
        const int i2 = (node == myTo ? 0 : -1);
        const double dist = myGeom[i].distanceTo2D(node->getPosition());
        const double neededOffset = (getTotalWidth() + getNumLanes() * SUMO_const_laneOffset) / 2;
        const double dist2 = MIN2(myGeom.distance2D(other->getGeometry()[i2]),
                                  other->getGeometry().distance2D(myGeom[i]));
        const double neededOffset2 = neededOffset + (other->getTotalWidth() + other->getNumLanes() * SUMO_const_laneOffset) / 2;
        if (dist < neededOffset && dist2 < neededOffset2) {
            PositionVector tmp = myGeom;
            // @note this doesn't work well for vissim networks
            //tmp.move2side(MIN2(neededOffset - dist, neededOffset2 - dist2));
            try {
                tmp.move2side(neededOffset - dist);
                myGeom[i] = tmp[i];
            } catch (InvalidArgument&) {
                WRITE_WARNINGF("Could not avoid overlapping shape at node '%' for edge '%'.", node->getID(), getID());
            }
        }
    }
}


Position
NBEdge::geometryPositionAtOffset(double offset) const {
    if (myLoadedLength > 0) {
        return myGeom.positionAtOffset(offset * myLength / myLoadedLength);
    } else {
        return myGeom.positionAtOffset(offset);
    }
}


double
NBEdge::getFinalLength() const {
    double result = getLoadedLength();
    if (OptionsCont::getOptions().getBool("no-internal-links") && !hasLoadedLength()) {
        // use length to junction center even if a modified geometry was given
        PositionVector geom = cutAtIntersection(myGeom);
        geom.push_back_noDoublePos(getToNode()->getCenter());
        geom.push_front_noDoublePos(getFromNode()->getCenter());
        result = geom.length();
    }
    double avgEndOffset = 0;
    for (const Lane& lane : myLanes) {
        avgEndOffset += lane.endOffset;
    }
    if (isBidiRail()) {
        avgEndOffset += myPossibleTurnDestination->getEndOffset();
    }
    avgEndOffset /= myLanes.size();
    return MAX2(result - avgEndOffset, POSITION_EPS);
}

void
NBEdge::setOrigID(const std::string origID) {
    if (origID != "") {
        for (int i = 0; i < (int)myLanes.size(); i++) {
            myLanes[i].setParameter(SUMO_PARAM_ORIGID, origID);
        }
    } else {
        // do not record empty origID parameter
        for (int i = 0; i < (int)myLanes.size(); i++) {
            myLanes[i].unsetParameter(SUMO_PARAM_ORIGID);
        }
    }
}


const EdgeVector&
NBEdge::getSuccessors(SUMOVehicleClass vClass) const {
    // @todo cache successors instead of recomputing them every time
    mySuccessors.clear();
    //std::cout << "getSuccessors edge=" << getID() << " svc=" << toString(vClass) << " cons=" << myConnections.size() << "\n";
    for (const Connection& con : myConnections) {
        if (con.fromLane >= 0 && con.toLane >= 0 && con.toEdge != nullptr &&
                (vClass == SVC_IGNORING || (getPermissions(con.fromLane)
                                            & con.toEdge->getPermissions(con.toLane) & vClass) != 0)
                && std::find(mySuccessors.begin(), mySuccessors.end(), con.toEdge) == mySuccessors.end()) {
            mySuccessors.push_back(con.toEdge);
            //std::cout << "   succ=" << con.toEdge->getID() << "\n";
        }
    }
    return mySuccessors;
}


const ConstRouterEdgePairVector&
NBEdge::getViaSuccessors(SUMOVehicleClass vClass) const {
    // @todo cache successors instead of recomputing them every time
    myViaSuccessors.clear();
    for (const Connection& con : myConnections) {
        std::pair<const NBEdge*, const Connection*> pair(con.toEdge, nullptr);
        // special case for Persons in Netedit
        if (vClass == SVC_PEDESTRIAN) {         //
            myViaSuccessors.push_back(pair);    //
        } else if (con.fromLane >= 0 && con.toLane >= 0 &&
                   con.toEdge != nullptr &&
                   (getPermissions(con.fromLane) & con.toEdge->getPermissions(con.toLane) & vClass) != 0) {
            // ignore duplicates
            if (con.getLength() > 0) {
                pair.second = &con;
            }
            myViaSuccessors.push_back(pair);
        }
    }
    return myViaSuccessors;
}


void
NBEdge::debugPrintConnections(bool outgoing, bool incoming) const {
    if (outgoing) {
        for (const Connection& c : myConnections) {
            std::cout << " " << getID() << "_" << c.fromLane << "->" << c.toEdge->getID() << "_" << c.toLane << "\n";
        }
    }
    if (incoming) {
        for (NBEdge* inc : myFrom->getIncomingEdges()) {
            for (Connection& c : inc->myConnections) {
                if (c.toEdge == this) {
                    std::cout << " " << inc->getID() << "_" << c.fromLane << "->" << c.toEdge->getID() << "_" << c.toLane << "\n";
                }
            }
        }
    }
}


int
NBEdge::getLaneIndexFromLaneID(const std::string laneID) {
    return StringUtils::toInt(laneID.substr(laneID.rfind("_") + 1));
}

bool
NBEdge::joinLanes(SVCPermissions perms) {
    bool haveJoined = false;
    int i = 0;
    while (i < getNumLanes() - 1) {
        if ((getPermissions(i) == perms) && (getPermissions(i + 1) == perms)) {
            const double newWidth = getLaneWidth(i) + getLaneWidth(i + 1);
            const std::string newType = myLanes[i].type + "|" + myLanes[i + 1].type;
            deleteLane(i, false, true);
            setLaneWidth(i, newWidth);
            setLaneType(i, newType);
            haveJoined = true;
        } else {
            i++;
        }
    }
    return haveJoined;
}


EdgeVector
NBEdge::filterByPermissions(const EdgeVector& edges, SVCPermissions permissions) {
    EdgeVector result;
    for (NBEdge* edge : edges) {
        if ((edge->getPermissions() & permissions) != 0) {
            result.push_back(edge);
        }
    }
    return result;
}

NBEdge*
NBEdge::getStraightContinuation(SVCPermissions permissions) const {
    EdgeVector cands = filterByPermissions(myTo->getOutgoingEdges(), permissions);
    if (cands.size() == 0) {
        return nullptr;
    }
    sort(cands.begin(), cands.end(), NBContHelper::edge_similar_direction_sorter(this));
    NBEdge* best = cands.front();
    if (isTurningDirectionAt(best)) {
        return nullptr;
    } else {
        return best;
    }
}

NBEdge*
NBEdge::getStraightPredecessor(SVCPermissions permissions) const {
    EdgeVector cands = filterByPermissions(myFrom->getIncomingEdges(), permissions);
    if (cands.size() == 0) {
        return nullptr;
    }
    sort(cands.begin(), cands.end(), NBContHelper::edge_similar_direction_sorter(this, false));
    NBEdge* best = cands.front();
    if (best->isTurningDirectionAt(this)) {
        return nullptr;
    } else {
        return best;
    }
}


/****************************************************************************/
