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
/// @file    RONetHandler.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Christian Roessel
/// @author  Michael Behrisch
/// @author  Yun-Pang Floetteroed
/// @date    Sept 2002
///
// The handler for SUMO-Networks
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/geom/PositionVector.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/vehicle/SUMORouteHandler.h>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "ROEdge.h"
#include "ROLane.h"
#include "RONode.h"
#include "RONet.h"
#include "RONetHandler.h"
#include "ROAbstractEdgeBuilder.h"


// ===========================================================================
// method definitions
// ===========================================================================
RONetHandler::RONetHandler(RONet& net, ROAbstractEdgeBuilder& eb, const bool ignoreInternal, const double minorPenalty) :
    SUMOSAXHandler("sumo-network"),
    myNet(net),
    myNetworkVersion(0),
    myEdgeBuilder(eb), myIgnoreInternal(ignoreInternal),
    myCurrentName(), myCurrentEdge(nullptr), myCurrentStoppingPlace(nullptr),
    myMinorPenalty(minorPenalty)
{}


RONetHandler::~RONetHandler() {}


void
RONetHandler::myStartElement(int element,
                             const SUMOSAXAttributes& attrs) {
    switch (element) {
        case SUMO_TAG_LOCATION:
            setLocation(attrs);
            break;
        case SUMO_TAG_NET: {
            bool ok;
            myNetworkVersion = attrs.get<double>(SUMO_ATTR_VERSION, nullptr, ok, false);
            break;
        }
        case SUMO_TAG_EDGE:
            // in the first step, we do need the name to allocate the edge
            // in the second, we need it to know to which edge we have to add
            //  the following edges to
            parseEdge(attrs);
            break;
        case SUMO_TAG_LANE:
            parseLane(attrs);
            break;
        case SUMO_TAG_JUNCTION:
            parseJunction(attrs);
            break;
        case SUMO_TAG_CONNECTION:
            parseConnection(attrs);
            break;
        case SUMO_TAG_BUS_STOP:
        case SUMO_TAG_TRAIN_STOP:
        case SUMO_TAG_CONTAINER_STOP:
        case SUMO_TAG_PARKING_AREA:
            parseStoppingPlace(attrs, (SumoXMLTag)element);
            break;
        case SUMO_TAG_ACCESS:
            parseAccess(attrs);
            break;
        case SUMO_TAG_TAZ:
            parseDistrict(attrs);
            break;
        case SUMO_TAG_TAZSOURCE:
            parseDistrictEdge(attrs, true);
            break;
        case SUMO_TAG_TAZSINK:
            parseDistrictEdge(attrs, false);
            break;
        case SUMO_TAG_TYPE: {
            bool ok = true;
            myCurrentTypeID = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
            break;
        }
        case SUMO_TAG_RESTRICTION: {
            bool ok = true;
            const SUMOVehicleClass svc = getVehicleClassID(attrs.get<std::string>(SUMO_ATTR_VCLASS, myCurrentTypeID.c_str(), ok));
            const double speed = attrs.get<double>(SUMO_ATTR_SPEED, myCurrentTypeID.c_str(), ok);
            if (ok) {
                myNet.addRestriction(myCurrentTypeID, svc, speed);
            }
            break;
        }
        case SUMO_TAG_PARAM:
            addParam(attrs);
            break;
        default:
            break;
    }
}


void
RONetHandler::myEndElement(int element) {
    switch (element) {
        case SUMO_TAG_NET:
            // build junction graph
            for (std::set<std::string>::const_iterator it = myUnseenNodeIDs.begin(); it != myUnseenNodeIDs.end(); ++it) {
                WRITE_ERROR("Unknown node '" + *it + "'.");
            }
            break;
        default:
            break;
    }
}


void
RONetHandler::addParam(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    const std::string key = attrs.get<std::string>(SUMO_ATTR_KEY, nullptr, ok);
    // circumventing empty string test
    const std::string val = attrs.hasAttribute(SUMO_ATTR_VALUE) ? attrs.getString(SUMO_ATTR_VALUE) : "";
    // add parameter in current created element, or in myLoadedParameterised
    if (myCurrentEdge != nullptr) {
        myCurrentEdge->setParameter(key, val);
    }
}


void
RONetHandler::parseEdge(const SUMOSAXAttributes& attrs) {
    // get the id, report an error if not given or empty...
    bool ok = true;
    myCurrentName = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        throw ProcessError();
    }
    const SumoXMLEdgeFunc func = attrs.getEdgeFunc(ok);
    if (!ok) {
        WRITE_ERROR("Edge '" + myCurrentName + "' has an unknown type.");
        return;
    }
    // get the edge
    std::string from;
    std::string to;
    int priority;
    myCurrentEdge = nullptr;
    if (func == SumoXMLEdgeFunc::INTERNAL || func == SumoXMLEdgeFunc::CROSSING || func == SumoXMLEdgeFunc::WALKINGAREA) {
        assert(myCurrentName[0] == ':');
        const std::string junctionID = myCurrentName.substr(1, myCurrentName.rfind('_') - 1);
        from = junctionID;
        to = junctionID;
        priority = 0;
    } else {
        from = attrs.get<std::string>(SUMO_ATTR_FROM, myCurrentName.c_str(), ok);
        to = attrs.get<std::string>(SUMO_ATTR_TO, myCurrentName.c_str(), ok);
        priority = attrs.get<int>(SUMO_ATTR_PRIORITY, myCurrentName.c_str(), ok);
        if (!ok) {
            return;
        }
    }
    RONode* fromNode = myNet.getNode(from);
    if (fromNode == nullptr) {
        myUnseenNodeIDs.insert(from);
        fromNode = new RONode(from);
        myNet.addNode(fromNode);
    }
    RONode* toNode = myNet.getNode(to);
    if (toNode == nullptr) {
        myUnseenNodeIDs.insert(to);
        toNode = new RONode(to);
        myNet.addNode(toNode);
    }
    // build the edge
    myCurrentEdge = myEdgeBuilder.buildEdge(myCurrentName, fromNode, toNode, priority);
    // set the type
    myCurrentEdge->setRestrictions(myNet.getRestrictions(attrs.getOpt<std::string>(SUMO_ATTR_TYPE, myCurrentName.c_str(), ok, "")));
    myCurrentEdge->setFunction(func);

    if (myNet.addEdge(myCurrentEdge)) {
        fromNode->addOutgoing(myCurrentEdge);
        toNode->addIncoming(myCurrentEdge);
        const std::string bidi = attrs.getOpt<std::string>(SUMO_ATTR_BIDI, myCurrentName.c_str(), ok, "");
        if (bidi != "") {
            myBidiEdges[myCurrentEdge] = bidi;
        }
    } else {
        myCurrentEdge = nullptr;
    }
}


void
RONetHandler::parseLane(const SUMOSAXAttributes& attrs) {
    if (myCurrentEdge == nullptr) {
        // was an internal edge to skip or an error occurred
        return;
    }
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    // get the speed
    double maxSpeed = attrs.get<double>(SUMO_ATTR_SPEED, id.c_str(), ok);
    double length = attrs.get<double>(SUMO_ATTR_LENGTH, id.c_str(), ok);
    std::string allow = attrs.getOpt<std::string>(SUMO_ATTR_ALLOW, id.c_str(), ok, "");
    std::string disallow = attrs.getOpt<std::string>(SUMO_ATTR_DISALLOW, id.c_str(), ok, "");
    const PositionVector shape = attrs.get<PositionVector>(SUMO_ATTR_SHAPE, id.c_str(), ok);
    if (!ok) {
        return;
    }
    if (shape.size() < 2) {
        WRITE_ERROR("Ignoring lane '" + id + "' with broken shape.");
        return;
    }
    // get the length
    // get the vehicle classes
    SVCPermissions permissions = parseVehicleClasses(allow, disallow, myNetworkVersion);
    if (permissions != SVCAll) {
        myNet.setPermissionsFound();
    }
    // add when both values are valid
    if (maxSpeed > 0 && length > 0 && id.length() > 0) {
        myCurrentEdge->addLane(new ROLane(id, myCurrentEdge, length, maxSpeed, permissions, shape));
    } else {
        WRITE_WARNING("Ignoring lane '" + id + "' with speed " + toString(maxSpeed) + " and length " + toString(length));
    }
}


void
RONetHandler::parseJunction(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    // get the id, report an error if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (attrs.getNodeType(ok) == SumoXMLNodeType::INTERNAL) {
        return;
    }
    myUnseenNodeIDs.erase(id);
    // get the position of the node
    const double x = attrs.get<double>(SUMO_ATTR_X, id.c_str(), ok);
    const double y = attrs.get<double>(SUMO_ATTR_Y, id.c_str(), ok);
    const double z = attrs.getOpt<double>(SUMO_ATTR_Z, id.c_str(), ok, 0.);
    if (!ok) {
        return;
    }
    RONode* n = myNet.getNode(id);
    if (n == nullptr) {
        WRITE_WARNING("Skipping isolated junction '" + id + "'.");
    } else {
        n->setPosition(Position(x, y, z));
    }
}


void
RONetHandler::parseConnection(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    std::string fromID = attrs.get<std::string>(SUMO_ATTR_FROM, nullptr, ok);
    std::string toID = attrs.get<std::string>(SUMO_ATTR_TO, nullptr, ok);
    const int fromLane = attrs.get<int>(SUMO_ATTR_FROM_LANE, nullptr, ok);
    const int toLane = attrs.get<int>(SUMO_ATTR_TO_LANE, nullptr, ok);
    std::string dir = attrs.get<std::string>(SUMO_ATTR_DIR, nullptr, ok);
    std::string viaID = attrs.getOpt<std::string>(SUMO_ATTR_VIA, nullptr, ok, "");
    ROEdge* from = myNet.getEdge(fromID);
    ROEdge* to = myNet.getEdge(toID);
    if (from == nullptr) {
        throw ProcessError("unknown from-edge '" + fromID + "' in connection");
    }
    if (to == nullptr) {
        throw ProcessError("unknown to-edge '" + toID + "' in connection");
    }
    if ((int)from->getLanes().size() <= fromLane) {
        throw ProcessError("invalid fromLane '" + toString(fromLane) + "' in connection from '" + fromID + "'.");
    }
    if ((int)to->getLanes().size() <= toLane) {
        throw ProcessError("invalid toLane '" + toString(toLane) + "' in connection to '" + toID + "'.");
    }
    if (myIgnoreInternal || viaID == "") {
        from->getLanes()[fromLane]->addOutgoingLane(to->getLanes()[toLane]);
        from->addSuccessor(to, nullptr, dir);
    }  else {
        ROEdge* const via = myNet.getEdge(viaID.substr(0, viaID.rfind('_')));
        if (via == nullptr) {
            throw ProcessError("unknown via-edge '" + viaID + "' in connection");
        }
        from->getLanes()[fromLane]->addOutgoingLane(to->getLanes()[toLane], via);
        from->addSuccessor(to, via, dir);
        via->addSuccessor(to, nullptr, dir);
        LinkState state = SUMOXMLDefinitions::LinkStates.get(attrs.get<std::string>(SUMO_ATTR_STATE, nullptr, ok));
        if (state == LINKSTATE_MINOR || state == LINKSTATE_EQUAL || state == LINKSTATE_STOP || state == LINKSTATE_ALLWAY_STOP) {
            via->setTimePenalty(myMinorPenalty);
        }
    }
}


void
RONetHandler::parseStoppingPlace(const SUMOSAXAttributes& attrs, const SumoXMLTag element) {
    bool ok = true;
    myCurrentStoppingPlace = new SUMOVehicleParameter::Stop();
    // get the id, throw if not given or empty...
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, toString(element).c_str(), ok);
    // get the lane
    myCurrentStoppingPlace->lane = attrs.get<std::string>(SUMO_ATTR_LANE, toString(element).c_str(), ok);
    if (!ok) {
        throw ProcessError();
    }
    const ROEdge* edge = myNet.getEdgeForLaneID(myCurrentStoppingPlace->lane);
    if (edge == nullptr) {
        throw InvalidArgument("Unknown lane '" + myCurrentStoppingPlace->lane + "' for " + toString(element) + " '" + id + "'.");
    }
    // get the positions
    myCurrentStoppingPlace->startPos = attrs.getOpt<double>(SUMO_ATTR_STARTPOS, id.c_str(), ok, 0.);
    myCurrentStoppingPlace->endPos = attrs.getOpt<double>(SUMO_ATTR_ENDPOS, id.c_str(), ok, edge->getLength());
    const bool friendlyPos = attrs.getOpt<bool>(SUMO_ATTR_FRIENDLY_POS, id.c_str(), ok, false);
    if (!ok || (SUMORouteHandler::checkStopPos(myCurrentStoppingPlace->startPos, myCurrentStoppingPlace->endPos, edge->getLength(), POSITION_EPS, friendlyPos) != SUMORouteHandler::StopPos::STOPPOS_VALID)) {
        throw InvalidArgument("Invalid position for " + toString(element) + " '" + id + "'.");
    }
    // this is a hack: the busstop attribute is meant to hold the id within the simulation context but this is not used within the router context
    myCurrentStoppingPlace->busstop = attrs.getOpt<std::string>(SUMO_ATTR_NAME, id.c_str(), ok, "");
    myNet.addStoppingPlace(id, element, myCurrentStoppingPlace);
}


void
RONetHandler::parseAccess(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    const std::string lane = attrs.get<std::string>(SUMO_ATTR_LANE, "access", ok);
    const ROEdge* edge = myNet.getEdgeForLaneID(lane);
    if (edge == nullptr) {
        throw InvalidArgument("Unknown lane '" + lane + "' for access.");
    }
    if ((edge->getPermissions() & SVC_PEDESTRIAN) == 0) {
        WRITE_WARNING("Ignoring invalid access from non-pedestrian edge '" + edge->getID() + "'.");
        return;
    }
    double pos = attrs.getOpt<double>(SUMO_ATTR_POSITION, "access", ok, 0.);
    const double length = attrs.getOpt<double>(SUMO_ATTR_LENGTH, "access", ok, -1);
    const bool friendlyPos = attrs.getOpt<bool>(SUMO_ATTR_FRIENDLY_POS, "access", ok, false);
    if (!ok || (SUMORouteHandler::checkStopPos(pos, pos, edge->getLength(), 0., friendlyPos) != SUMORouteHandler::StopPos::STOPPOS_VALID)) {
        throw InvalidArgument("Invalid position " + toString(pos) + " for access on lane '" + lane + "'.");
    }
    if (!ok) {
        throw ProcessError();
    }
    myCurrentStoppingPlace->accessPos.push_back(std::make_tuple(lane, pos, length));
}


void
RONetHandler::parseDistrict(const SUMOSAXAttributes& attrs) {
    myCurrentEdge = nullptr;
    bool ok = true;
    myCurrentName = attrs.get<std::string>(SUMO_ATTR_ID, nullptr, ok);
    if (!ok) {
        return;
    }
    myNet.addDistrict(myCurrentName, myEdgeBuilder.buildEdge(myCurrentName + "-source", nullptr, nullptr, 0), myEdgeBuilder.buildEdge(myCurrentName + "-sink", nullptr, nullptr, 0));
    if (attrs.hasAttribute(SUMO_ATTR_EDGES)) {
        std::vector<std::string> desc = attrs.getStringVector(SUMO_ATTR_EDGES);
        for (std::vector<std::string>::const_iterator i = desc.begin(); i != desc.end(); ++i) {
            myNet.addDistrictEdge(myCurrentName, *i, true);
            myNet.addDistrictEdge(myCurrentName, *i, false);
        }
    }
}


void
RONetHandler::parseDistrictEdge(const SUMOSAXAttributes& attrs, bool isSource) {
    bool ok = true;
    std::string id = attrs.get<std::string>(SUMO_ATTR_ID, myCurrentName.c_str(), ok);
    myNet.addDistrictEdge(myCurrentName, id, isSource);
}

void
RONetHandler::setLocation(const SUMOSAXAttributes& attrs) {
    bool ok = true;
    PositionVector s = attrs.get<PositionVector>(SUMO_ATTR_NET_OFFSET, nullptr, ok);
    Boundary convBoundary = attrs.get<Boundary>(SUMO_ATTR_CONV_BOUNDARY, nullptr, ok);
    Boundary origBoundary = attrs.get<Boundary>(SUMO_ATTR_ORIG_BOUNDARY, nullptr, ok);
    std::string proj = attrs.get<std::string>(SUMO_ATTR_ORIG_PROJ, nullptr, ok);
    if (ok) {
        Position networkOffset = s[0];
        GeoConvHelper::init(proj, networkOffset, origBoundary, convBoundary);
    }
}


/****************************************************************************/
