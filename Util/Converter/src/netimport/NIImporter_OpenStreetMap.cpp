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
/// @file    NIImporter_OpenStreetMap.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @author  Gregor Laemmel
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in OpenStreetMap format
/****************************************************************************/
#include <config.h>
#include <algorithm>
#include <set>
#include <functional>
#include <sstream>
#include <limits>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBOwnTLDef.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>
#include <netbuild/NBPTLine.h>
#include <netbuild/NBPTLineCont.h>
#include "NILoader.h"
#include "NIImporter_OpenStreetMap.h"

#define KM_PER_MILE 1.609344

//#define DEBUG_LAYER_ELEVATION

// ---------------------------------------------------------------------------
// static members
// ---------------------------------------------------------------------------
const double NIImporter_OpenStreetMap::MAXSPEED_UNGIVEN = -1;

const long long int NIImporter_OpenStreetMap::INVALID_ID = std::numeric_limits<long long int>::max();

// ===========================================================================
// Private classes
// ===========================================================================

/** @brief Functor which compares two Edges
 */
class NIImporter_OpenStreetMap::CompareEdges {
public:
    bool operator()(const Edge* e1, const Edge* e2) const {
        if (e1->myHighWayType != e2->myHighWayType) {
            return e1->myHighWayType > e2->myHighWayType;
        }
        if (e1->myNoLanes != e2->myNoLanes) {
            return e1->myNoLanes > e2->myNoLanes;
        }
        if (e1->myNoLanesForward != e2->myNoLanesForward) {
            return e1->myNoLanesForward > e2->myNoLanesForward;
        }
        if (e1->myMaxSpeed != e2->myMaxSpeed) {
            return e1->myMaxSpeed > e2->myMaxSpeed;
        }
        if (e1->myIsOneWay != e2->myIsOneWay) {
            return e1->myIsOneWay > e2->myIsOneWay;
        }
        return e1->myCurrentNodes > e2->myCurrentNodes;
    }
};

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
const std::string NIImporter_OpenStreetMap::compoundTypeSeparator("|"); //clang-tidy says: "compundTypeSeparator with
// static storage duration my throw an exception that cannot be caught

void
NIImporter_OpenStreetMap::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    NIImporter_OpenStreetMap importer;
    importer.load(oc, nb);
}

NIImporter_OpenStreetMap::NIImporter_OpenStreetMap() = default;

NIImporter_OpenStreetMap::~NIImporter_OpenStreetMap() {
    // delete nodes
    for (auto myUniqueNode : myUniqueNodes) {
        delete myUniqueNode;
    }
    // delete edges
    for (auto& myEdge : myEdges) {
        delete myEdge.second;
    }
    // delete platform shapes
    for (auto& myPlatformShape : myPlatformShapes) {
        delete myPlatformShape.second;
    }
}

void
NIImporter_OpenStreetMap::load(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isSet("osm-files")) {
        return;
    }
    /* Parse file(s)
     * Each file is parsed twice: first for nodes, second for edges. */
    std::vector<std::string> files = oc.getStringVector("osm-files");
    // load nodes, first
    NodesHandler nodesHandler(myOSMNodes, myUniqueNodes, oc);
    // for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
    //     // nodes
    //     if (!FileHelpers::isReadable(*file)) {
    //         WRITE_ERROR("Could not open osm-file '" + *file + "'.");
    //         return;
    //     }
    //     nodesHandler.setFileName(*file);
    //     PROGRESS_BEGIN_MESSAGE("Parsing nodes from osm-file '" + *file + "'");
    //     if (!XMLSubSys::runParser(nodesHandler, *file)) {
    //         return;
    //     }
    //     if (nodesHandler.getDuplicateNodes() > 0) {
    //         WRITE_MESSAGE("Found and substituted " + toString(nodesHandler.getDuplicateNodes()) + " osm nodes.");
    //     }
    //     PROGRESS_DONE_MESSAGE();
    // }
    nodesHandler.setFileName("osm_file");
    PROGRESS_BEGIN_MESSAGE("Parsing nodes from osm string");
    if (!XMLSubSys::runParserFromString(nodesHandler, oc.input_osm_file)) {
        return;
    }
    if (nodesHandler.getDuplicateNodes() > 0) {
        WRITE_MESSAGE("Found and substituted " + toString(nodesHandler.getDuplicateNodes()) + " osm nodes.");
    }
    PROGRESS_DONE_MESSAGE();

    // load edges, then
    EdgesHandler edgesHandler(myOSMNodes, myEdges, myPlatformShapes);
    // for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
    //     // edges
    //     edgesHandler.setFileName(*file);
    //     PROGRESS_BEGIN_MESSAGE("Parsing edges from osm-file '" + *file + "'");
    //     XMLSubSys::runParser(edgesHandler, *file);
    //     PROGRESS_DONE_MESSAGE();
    // }
    // edges
    edgesHandler.setFileName("osm_file");
    PROGRESS_BEGIN_MESSAGE("Parsing nodes from osm string");
    XMLSubSys::runParserFromString(edgesHandler, oc.input_osm_file);
    PROGRESS_DONE_MESSAGE();


    /* Remove duplicate edges with the same shape and attributes */
    if (!oc.getBool("osm.skip-duplicates-check")) {
        int numRemoved = 0;
        PROGRESS_BEGIN_MESSAGE("Removing duplicate edges");
        if (myEdges.size() > 1) {
            std::set<const Edge*, CompareEdges> dupsFinder;
            for (auto it = myEdges.begin(); it != myEdges.end();) {
                if (dupsFinder.count(it->second) > 0) {
                    numRemoved++;
                    delete it->second;
                    myEdges.erase(it++);
                } else {
                    dupsFinder.insert(it->second);
                    it++;
                }
            }
        }
        if (numRemoved > 0) {
            WRITE_MESSAGE("Removed " + toString(numRemoved) + " duplicate osm edges.");
        }
        PROGRESS_DONE_MESSAGE();
    }

    /* Mark which nodes are used (by edges or traffic lights).
     * This is necessary to detect which OpenStreetMap nodes are for
     * geometry only */
    std::map<long long int, int> nodeUsage;
    // Mark which nodes are used by edges (begin and end)
    for (std::map<long long int, Edge*>::const_iterator i = myEdges.begin(); i != myEdges.end(); ++i) {
        Edge* e = (*i).second;
        assert(e->myCurrentIsRoad);
        for (std::vector<long long int>::const_iterator j = e->myCurrentNodes.begin();
                j != e->myCurrentNodes.end();
                ++j) {
            if (nodeUsage.find(*j) == nodeUsage.end()) {
                nodeUsage[*j] = 0;
            }
            nodeUsage[*j] = nodeUsage[*j] + 1;
        }
    }
    // Mark which nodes are used by traffic lights
    for (std::map<long long int, NIOSMNode*>::const_iterator nodesIt = myOSMNodes.begin();
            nodesIt != myOSMNodes.end();
            ++nodesIt) {
        if (nodesIt->second->tlsControlled || nodesIt->second->railwaySignal /* || nodesIt->second->railwayCrossing*/) {
            // If the key is not found in the map, the value is automatically
            // initialized with 0.
            nodeUsage[nodesIt->first] += 1;
        }
    }

    /* Instantiate edges
     * Only those nodes in the middle of an edge which are used by more than
     * one edge are instantiated. Other nodes are considered as geometry nodes. */
    NBNodeCont& nc = nb.getNodeCont();
    NBTrafficLightLogicCont& tlsc = nb.getTLLogicCont();
    for (auto& myEdge : myEdges) {
        Edge* e = myEdge.second;
        assert(e->myCurrentIsRoad);
        if (e->myCurrentNodes.size() < 2) {
            WRITE_WARNINGF("Discarding way '%' because it has only % node(s)", e->id, e->myCurrentNodes.size());
            continue;
        }
        extendRailwayDistances(e, nb.getTypeCont());
        // build nodes;
        //  - the from- and to-nodes must be built in any case
        //  - the in-between nodes are only built if more than one edge references them
        NBNode* currentFrom = insertNodeChecking(*e->myCurrentNodes.begin(), nc, tlsc);
        NBNode* last = insertNodeChecking(*(e->myCurrentNodes.end() - 1), nc, tlsc);
        int running = 0;
        std::vector<long long int> passed;
        for (auto j = e->myCurrentNodes.begin(); j != e->myCurrentNodes.end(); ++j) {
            passed.push_back(*j);
            if (nodeUsage[*j] > 1 && j != e->myCurrentNodes.end() - 1 && j != e->myCurrentNodes.begin()) {
                NBNode* currentTo = insertNodeChecking(*j, nc, tlsc);
                running = insertEdge(e, running, currentFrom, currentTo, passed, nb);
                currentFrom = currentTo;
                passed.clear();
                passed.push_back(*j);
            }
        }
        if (running == 0) {
            running = -1;
        }
        insertEdge(e, running, currentFrom, last, passed, nb);
    }

    const double layerElevation = oc.getFloat("osm.layer-elevation");
    if (layerElevation > 0) {
        reconstructLayerElevation(layerElevation, nb);
    }

    //revise pt stops; remove stops on deleted edges
    if (OptionsCont::getOptions().isSet("ptstop-output")) {
        nb.getPTStopCont().cleanupDeleted(nb.getEdgeCont());
    }

    // load relations (after edges are built since we want to apply
    // turn-restrictions directly to NBEdges)
    RelationHandler relationHandler(myOSMNodes, myEdges, &(nb.getPTStopCont()), myPlatformShapes,
                                    &nb.getPTLineCont(), oc);
    // for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
    //     // relations
    //     relationHandler.setFileName(*file);
    //     PROGRESS_BEGIN_MESSAGE("Parsing relations from osm-file '" + *file + "'");
    //     XMLSubSys::runParser(relationHandler, *file);
    //     PROGRESS_DONE_MESSAGE();
    // }
    // relations
    relationHandler.setFileName("osm-file");
    PROGRESS_BEGIN_MESSAGE("Parsing nodes from osm string");
    XMLSubSys::runParserFromString(edgesHandler, oc.input_osm_file);
    PROGRESS_DONE_MESSAGE();

}

NBNode*
NIImporter_OpenStreetMap::insertNodeChecking(long long int id, NBNodeCont& nc, NBTrafficLightLogicCont& tlsc) {
    NBNode* node = nc.retrieve(toString(id));
    if (node == nullptr) {
        NIOSMNode* n = myOSMNodes.find(id)->second;
        Position pos(n->lon, n->lat, n->ele);
        if (!NBNetBuilder::transformCoordinate(pos, true)) {
            WRITE_ERROR("Unable to project coordinates for junction '" + toString(id) + "'.");
            return nullptr;
        }
        node = new NBNode(toString(id), pos);
        if (!nc.insert(node)) {
            WRITE_ERROR("Could not insert junction '" + toString(id) + "'.");
            delete node;
            return nullptr;
        }
        n->node = node;
        if (n->railwayCrossing) {
            node->reinit(pos, SumoXMLNodeType::RAIL_CROSSING);
        } else if (n->railwaySignal) {
            node->reinit(pos, SumoXMLNodeType::RAIL_SIGNAL);
        } else if (n->tlsControlled) {
            // ok, this node is a traffic light node where no other nodes
            //  participate
            // @note: The OSM-community has not settled on a schema for differentiating between fixed and actuated lights
            TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(
                                        OptionsCont::getOptions().getString("tls.default-type"));
            NBOwnTLDef* tlDef = new NBOwnTLDef(toString(id), node, 0, type);
            if (!tlsc.insert(tlDef)) {
                // actually, nothing should fail here
                delete tlDef;
                throw ProcessError("Could not allocate tls '" + toString(id) + "'.");
            }
        }
        if (n->railwayBufferStop) {
            node->setParameter("buffer_stop", "true");
        }
    }
    return node;
}

int
NIImporter_OpenStreetMap::insertEdge(Edge* e, int index, NBNode* from, NBNode* to,
                                     const std::vector<long long int>& passed, NBNetBuilder& nb) {
    NBNodeCont& nc = nb.getNodeCont();
    NBEdgeCont& ec = nb.getEdgeCont();
    NBTypeCont& tc = nb.getTypeCont();
    NBPTStopCont& sc = nb.getPTStopCont();

    NBTrafficLightLogicCont& tlsc = nb.getTLLogicCont();
    // patch the id
    std::string id = toString(e->id);
    if (from == nullptr || to == nullptr) {
        WRITE_ERROR("Discarding edge '" + id + "' because the nodes could not be built.");
        return index;
    }
    if (index >= 0) {
        id = id + "#" + toString(index);
    } else {
        index = 0;
    }
    if (from == to) {
        assert(passed.size() >= 2);
        if (passed.size() == 2) {
            WRITE_WARNINGF("Discarding edge '%' which connects two identical nodes without geometry.", id);
            return index;
        }
        // in the special case of a looped way split again using passed
        int intermediateIndex = (int) passed.size() / 2;
        NBNode* intermediate = insertNodeChecking(passed[intermediateIndex], nc, tlsc);
        std::vector<long long int> part1(passed.begin(), passed.begin() + intermediateIndex + 1);
        std::vector<long long int> part2(passed.begin() + intermediateIndex, passed.end());
        index = insertEdge(e, index, from, intermediate, part1, nb);
        return insertEdge(e, index, intermediate, to, part2, nb);
    }
    const int newIndex = index + 1;

    // convert the shape
    PositionVector shape;
    double distanceStart = myOSMNodes[passed.front()]->positionMeters;
    double distanceEnd = myOSMNodes[passed.back()]->positionMeters;
    const bool useDistance = distanceStart != std::numeric_limits<double>::max() && distanceEnd != std::numeric_limits<double>::max();
    if (useDistance) {
        // negative sign denotes counting in the other direction
        if (distanceStart < distanceEnd) {
            distanceStart *= -1;
        } else {
            distanceEnd *= -1;
        }
    } else {
        distanceStart = 0;
        distanceEnd = 0;
    }
    for (long long i : passed) {
        NIOSMNode* n = myOSMNodes.find(i)->second;
        if (n->ptStopPosition) {
            NBPTStop* existingPtStop = sc.get(toString(n->id));
            if (existingPtStop != nullptr) {
                existingPtStop->registerAdditionalEdge(toString(e->id), id);
            } else {
                Position ptPos(n->lon, n->lat, n->ele);
                if (!NBNetBuilder::transformCoordinate(ptPos)) {
                    WRITE_ERROR("Unable to project coordinates for node '" + toString(n->id) + "'.");
                }
                NBPTStop* ptStop = new NBPTStop(toString(n->id), ptPos, id, toString(e->id), n->ptStopLength, n->name,
                                                n->permissions);

                sc.insert(ptStop);
            }
        }
        Position pos(n->lon, n->lat, n->ele);
        shape.push_back(pos);
    }
    if (!NBNetBuilder::transformCoordinates(shape)) {
        WRITE_ERROR("Unable to project coordinates for edge '" + id + "'.");
    }
    std::string type = usableType(e->myHighWayType, id, tc);
    if (type == "") {
        return newIndex;
    }

    // otherwise it is not an edge and will be ignored
    bool ok = true;
    int numLanesForward = tc.getNumLanes(type);
    int numLanesBackward = tc.getNumLanes(type);
    double speed = tc.getSpeed(type);
    bool defaultsToOneWay = tc.getIsOneWay(type);
    SVCPermissions permissions = tc.getPermissions(type);
    if (e->myCurrentIsElectrified && (permissions & SVC_RAIL) != 0) {
        permissions |= (SVC_RAIL_ELECTRIC | SVC_RAIL_FAST);
    }
    SVCPermissions forwardPermissions = permissions;
    SVCPermissions backwardPermissions = permissions;
    const std::string streetName = isRailway(forwardPermissions) && e->ref != "" ? e->ref : e->streetName;
    if (streetName == e->ref) {
        e->unsetParameter("ref"); // avoid superfluous param for railways
    }
    double forwardWidth = tc.getWidth(type);
    double backwardWidth = tc.getWidth(type);
    const bool addSidewalk = (tc.getSidewalkWidth(type) != NBEdge::UNSPECIFIED_WIDTH);
    const bool addBikeLane = (tc.getBikeLaneWidth(type) != NBEdge::UNSPECIFIED_WIDTH);
    // check directions
    bool addForward = true;
    bool addBackward = true;
    if ((e->myIsOneWay == "true" || e->myIsOneWay == "yes" || e->myIsOneWay == "1"
            || (defaultsToOneWay && e->myIsOneWay != "no" && e->myIsOneWay != "false" && e->myIsOneWay != "0"))
            && e->myRailDirection != WAY_BOTH) {
        addBackward = false;
    }
    if (e->myIsOneWay == "-1" || e->myIsOneWay == "reverse" || e->myRailDirection == WAY_BACKWARD) {
        // one-way in reversed direction of way
        addForward = false;
        addBackward = true;
    }
    if (!e->myIsOneWay.empty() && e->myIsOneWay != "false" && e->myIsOneWay != "no" && e->myIsOneWay != "true"
            && e->myIsOneWay != "yes" && e->myIsOneWay != "-1" && e->myIsOneWay != "1" && e->myIsOneWay != "reverse") {
        WRITE_WARNINGF("New value for oneway found: %", e->myIsOneWay);
    }
    // if we had been able to extract the number of lanes, override the highway type default
    if (e->myNoLanes > 0) {
        if (addForward && !addBackward) {
            numLanesForward = e->myNoLanes;
        } else if (!addForward && addBackward) {
            numLanesBackward = e->myNoLanes;
        } else {
            if (e->myNoLanesForward > 0) {
                numLanesForward = e->myNoLanesForward;
            } else if (e->myNoLanesForward < 0) {
                numLanesForward = e->myNoLanes + e->myNoLanesForward;
            } else {
                numLanesForward = (int) std::ceil(e->myNoLanes / 2.0);
            }
            numLanesBackward = e->myNoLanes - numLanesForward;
            // sometimes ways are tagged according to their physical width of a single
            // lane but they are intended for traffic in both directions
            numLanesForward = MAX2(1, numLanesForward);
            numLanesBackward = MAX2(1, numLanesBackward);
        }
    } else if (e->myNoLanes == 0) {
        WRITE_WARNINGF("Skipping edge '%' because it has zero lanes.", id);
        ok = false;
    }
    // if we had been able to extract the maximum speed, override the type's default
    if (e->myMaxSpeed != MAXSPEED_UNGIVEN) {
        speed = e->myMaxSpeed / 3.6;
    }
    double speedBackward = speed;
    if (e->myMaxSpeedBackward != MAXSPEED_UNGIVEN) {
        speedBackward = e->myMaxSpeedBackward / 3.6;
    }
    if (speed <= 0 || speedBackward <= 0) {
        WRITE_WARNINGF("Skipping edge '%' because it has speed %.", id, speed);
        ok = false;
    }
    // deal with cycleways that run in the opposite direction of a one-way street
    WayType cyclewayType = e->myCyclewayType; // make a copy because we do some temporary modifications
    if (addBikeLane) {
        if (!addForward && (cyclewayType & WAY_FORWARD) != 0) {
            addForward = true;
            forwardPermissions = SVC_BICYCLE;
            forwardWidth = tc.getBikeLaneWidth(type);
            numLanesForward = 1;
            // do not add an additional cycle lane
            cyclewayType = (WayType)(cyclewayType & ~WAY_FORWARD);  //clang tidy thinks "!WAY_FORWARD" is always false
        }
        if (!addBackward && (cyclewayType & WAY_BACKWARD) != 0) {
            addBackward = true;
            backwardPermissions = SVC_BICYCLE;
            backwardWidth = tc.getBikeLaneWidth(type);
            numLanesBackward = 1;
            // do not add an additional cycle lane
            cyclewayType = (WayType)(cyclewayType & ~WAY_BACKWARD); //clang tidy thinks "!WAY_BACKWARD" is always false
        }
    }
    // deal with sidewalks that run in the opposite direction of a one-way street
    WayType sidewalkType = e->mySidewalkType; // make a copy because we do some temporary modifications
    if (addSidewalk) {
        if (!addForward && (sidewalkType & WAY_FORWARD) != 0) {
            addForward = true;
            forwardPermissions = SVC_PEDESTRIAN;
            forwardWidth = tc.getSidewalkWidth(type);
            numLanesForward = 1;
            // do not add an additional sidewalk
            sidewalkType = (WayType)(sidewalkType & ~WAY_FORWARD);  //clang tidy thinks "!WAY_FORWARD" is always false
        }
        if (!addBackward && (sidewalkType & WAY_BACKWARD) != 0) {
            addBackward = true;
            backwardPermissions = SVC_PEDESTRIAN;
            backwardWidth = tc.getSidewalkWidth(type);
            numLanesBackward = 1;
            // do not add an additional cycle lane
            sidewalkType = (WayType)(sidewalkType & ~WAY_BACKWARD); //clang tidy thinks "!WAY_BACKWARD" is always false
        }
    }
    // deal with busways that run in the opposite direction of a one-way street
    if (!addForward && (e->myBuswayType & WAY_FORWARD) != 0) {
        addForward = true;
        forwardPermissions = SVC_BUS;
        numLanesForward = 1;
    }
    if (!addBackward && (e->myBuswayType & WAY_BACKWARD) != 0) {
        addBackward = true;
        backwardPermissions = SVC_BUS;
        numLanesBackward = 1;
    }

    const std::string origID = OptionsCont::getOptions().getBool("output.original-names") ? toString(e->id) : "";
    if (ok) {
        const int offsetFactor = OptionsCont::getOptions().getBool("lefthand") ? -1 : 1;
        LaneSpreadFunction lsf = (addBackward || OptionsCont::getOptions().getBool("osm.oneway-spread-right")) &&
                                 e->myRailDirection == WAY_UNKNOWN ? LaneSpreadFunction::RIGHT : LaneSpreadFunction::CENTER;

        id = StringUtils::escapeXML(id);
        const std::string reverseID = "-" + id;

        if (addForward) {
            assert(numLanesForward > 0);
            NBEdge* nbe = new NBEdge(id, from, to, type, speed, numLanesForward, tc.getPriority(type),
                                     forwardWidth, NBEdge::UNSPECIFIED_OFFSET, shape,
                                     StringUtils::escapeXML(streetName), origID, lsf, true);
            nbe->setPermissions(forwardPermissions);
            if ((e->myBuswayType & WAY_FORWARD) != 0) {
                nbe->setPermissions(SVC_BUS, 0);
            }
            if (addBikeLane && (cyclewayType == WAY_UNKNOWN || (cyclewayType & WAY_FORWARD) != 0)) {
                nbe->addBikeLane(tc.getBikeLaneWidth(type) * offsetFactor);
            } else if (nbe->getPermissions(0) == SVC_BUS) {
                // bikes drive on buslanes if no separate cycle lane is available
                nbe->setPermissions(SVC_BUS | SVC_BICYCLE, 0);
            }
            if (addSidewalk && (sidewalkType == WAY_UNKNOWN || (sidewalkType & WAY_FORWARD) != 0)) {
                nbe->addSidewalk(tc.getSidewalkWidth(type) * offsetFactor);
            }
            nbe->updateParameters(e->getParametersMap());
            nbe->setDistance(distanceStart);
            if (!ec.insert(nbe)) {
                delete nbe;
                throw ProcessError("Could not add edge '" + id + "'.");
            }
        }
        if (addBackward) {
            assert(numLanesBackward > 0);
            NBEdge* nbe = new NBEdge(reverseID, to, from, type, speedBackward, numLanesBackward, tc.getPriority(type),
                                     backwardWidth, NBEdge::UNSPECIFIED_OFFSET, shape.reverse(),
                                     StringUtils::escapeXML(streetName), origID, lsf, true);
            nbe->setPermissions(backwardPermissions);
            if ((e->myBuswayType & WAY_BACKWARD) != 0) {
                nbe->setPermissions(SVC_BUS, 0);
            }
            if (addBikeLane && (cyclewayType == WAY_UNKNOWN || (cyclewayType & WAY_BACKWARD) != 0)) {
                nbe->addBikeLane(tc.getBikeLaneWidth(type) * offsetFactor);
            } else if (nbe->getPermissions(0) == SVC_BUS) {
                // bikes drive on buslanes if no separate cycle lane is available
                nbe->setPermissions(SVC_BUS | SVC_BICYCLE, 0);
            }
            if (addSidewalk && (sidewalkType == WAY_UNKNOWN || (sidewalkType & WAY_BACKWARD) != 0)) {
                nbe->addSidewalk(tc.getSidewalkWidth(type) * offsetFactor);
            }
            nbe->updateParameters(e->getParametersMap());
            nbe->setDistance(distanceEnd);
            if (!ec.insert(nbe)) {
                delete nbe;
                throw ProcessError("Could not add edge '-" + id + "'.");
            }
        }
        if ((e->myParkingType & PARKING_BOTH) != 0 && OptionsCont::getOptions().isSet("parking-output")) {
            if ((e->myParkingType & PARKING_RIGHT) != 0) {
                if (addForward) {
                    nb.getParkingCont().push_back(NBParking(id, id));
                } else {
                    /// XXX parking area should be added on the left side of a reverse one-way street
                    if ((e->myParkingType & PARKING_LEFT) == 0 && !addBackward) {
                        /// put it on the wrong side (better than nothing)
                        nb.getParkingCont().push_back(NBParking(reverseID, reverseID));
                    }
                }
            }
            if ((e->myParkingType & PARKING_LEFT) != 0) {
                if (addBackward) {
                    nb.getParkingCont().push_back(NBParking(reverseID, reverseID));
                } else {
                    /// XXX parking area should be added on the left side of an one-way street
                    if ((e->myParkingType & PARKING_RIGHT) == 0 && !addForward) {
                        /// put it on the wrong side (better than nothing)
                        nb.getParkingCont().push_back(NBParking(id, id));
                    }
                }
            }
        }
    }
    return newIndex;
}

// ---------------------------------------------------------------------------
// definitions of NIImporter_OpenStreetMap::NodesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_OpenStreetMap::NodesHandler::NodesHandler(std::map<long long int, NIOSMNode*>& toFill,
        std::set<NIOSMNode*, CompareNodes>& uniqueNodes,
        const OptionsCont& oc)

    :
    SUMOSAXHandler("osm - file"),
    myToFill(toFill),
    myLastNodeID(-1),
    myIsInValidNodeTag(false),
    myHierarchyLevel(0),
    myUniqueNodes(uniqueNodes),
    myImportElevation(oc.getBool("osm.elevation")),
    myDuplicateNodes(0),
    myOptionsCont(oc) {
}

NIImporter_OpenStreetMap::NodesHandler::~NodesHandler() = default;

void
NIImporter_OpenStreetMap::NodesHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    ++myHierarchyLevel;
    if (element == SUMO_TAG_NODE) {
        bool ok = true;
        if (myHierarchyLevel != 2) {
            WRITE_ERROR("Node element on wrong XML hierarchy level (id='" + toString(attrs.get<long
                        long
                        int>(SUMO_ATTR_ID,
                             nullptr, ok))
                        + "', level='" + toString(myHierarchyLevel) + "').");
            return;
        }
        const long long int id = attrs.get<long long int>(SUMO_ATTR_ID, nullptr, ok);
        const std::string action = attrs.hasAttribute("action") ? attrs.getStringSecure("action", "") : "";
        if (action == "delete" || !ok) {
            return;
        }
        myLastNodeID = -1;
        if (myToFill.find(id) == myToFill.end()) {
            myLastNodeID = id;
            // assume we are loading multiple files...
            //  ... so we won't report duplicate nodes
            bool ok2 = true;
            double tlat, tlon;
            std::istringstream lon(attrs.get<std::string>(SUMO_ATTR_LON, toString(id).c_str(), ok2));
            if (!ok2) {
                return;
            }
            lon >> tlon;
            if (lon.fail()) {
                WRITE_ERROR("Node's '" + toString(id) + "' lon information is not numeric.");
                return;
            }
            std::istringstream lat(attrs.get<std::string>(SUMO_ATTR_LAT, toString(id).c_str(), ok2));
            if (!ok2) {
                return;
            }
            lat >> tlat;
            if (lat.fail()) {
                WRITE_ERROR("Node's '" + toString(id) + "' lat information is not numeric.");
                return;
            }
            auto* toAdd = new NIOSMNode(id, tlon, tlat);
            myIsInValidNodeTag = true;

            auto similarNode = myUniqueNodes.find(toAdd);
            if (similarNode == myUniqueNodes.end()) {
                myUniqueNodes.insert(toAdd);
            } else {
                delete toAdd;
                toAdd = *similarNode;
                myDuplicateNodes++;
            }
            myToFill[id] = toAdd;
        }
    }
    if (element == SUMO_TAG_TAG && myIsInValidNodeTag) {
        if (myHierarchyLevel != 3) {
            WRITE_ERROR("Tag element on wrong XML hierarchy level.");
            return;
        }
        bool ok = true;
        std::string key = attrs.get<std::string>(SUMO_ATTR_K, toString(myLastNodeID).c_str(), ok, false);
        // we check whether the key is relevant (and we really need to transcode the value) to avoid hitting #1636
        if (key == "highway" || key == "ele" || key == "crossing" || key == "railway" || key == "public_transport"
                || key == "name" || key == "train" || key == "bus" || key == "tram" || key == "light_rail" || key == "subway" || key == "station" || key == "noexit"
                || StringUtils::startsWith(key, "railway:signal")
                || StringUtils::startsWith(key, "railway:position")
           ) {
            std::string value = attrs.get<std::string>(SUMO_ATTR_V, toString(myLastNodeID).c_str(), ok, false);
            if (key == "highway" && value.find("traffic_signal") != std::string::npos) {
                myToFill[myLastNodeID]->tlsControlled = true;
            } else if (key == "crossing" && value.find("traffic_signals") != std::string::npos) {
                myToFill[myLastNodeID]->tlsControlled = true;
            } else if ((key == "noexit" && value == "yes")
                       || (key == "railway" && value == "buffer_stop")) {
                myToFill[myLastNodeID]->railwayBufferStop = true;
            } else if (key == "railway" && value.find("crossing") != std::string::npos) {
                myToFill[myLastNodeID]->railwayCrossing = true;
            } else if (StringUtils::startsWith(key, "railway:signal") && (
                           value == "block" || value == "entry"  || value == "exit" || value == "intermediate")) {
                myToFill[myLastNodeID]->railwaySignal = true;
            } else if (StringUtils::startsWith(key, "railway:position") && value.size() > myToFill[myLastNodeID]->position.size()) {
                // use the entry with the highest precision (more digits)
                myToFill[myLastNodeID]->position = value;
            } else if ((key == "public_transport" && value == "stop_position") ||
                       (key == "highway" && value == "bus_stop")) {
                myToFill[myLastNodeID]->ptStopPosition = true;
                if (myToFill[myLastNodeID]->ptStopLength == 0) {
                    // default length
                    myToFill[myLastNodeID]->ptStopLength = myOptionsCont.getFloat("osm.stop-output.length");
                }
            } else if (key == "name") {
                myToFill[myLastNodeID]->name = value;
            } else if (myImportElevation && key == "ele") {
                try {
                    const double elevation = StringUtils::toDouble(value);
                    if (ISNAN(elevation)) {
                        WRITE_WARNINGF("Value of key '%' is invalid ('%') in node '%'.", key, value, toString(myLastNodeID));
                    } else {
                        myToFill[myLastNodeID]->ele = elevation;
                    }
                } catch (...) {
                    WRITE_WARNINGF("Value of key '%' is not numeric ('%') in node '%'.", key, value, toString(myLastNodeID));
                }
            } else if (key == "station") {
                interpretTransportType(value, myToFill[myLastNodeID]);
            } else {
                // v="yes"
                interpretTransportType(key, myToFill[myLastNodeID]);
            }
        }
    }
}

void
NIImporter_OpenStreetMap::NodesHandler::myEndElement(int element) {
    if (element == SUMO_TAG_NODE && myHierarchyLevel == 2) {
        myLastNodeID = -1;
        myIsInValidNodeTag = false;
    }
    --myHierarchyLevel;
}

// ---------------------------------------------------------------------------
// definitions of NIImporter_OpenStreetMap::EdgesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_OpenStreetMap::EdgesHandler::EdgesHandler(
    const std::map<long long int, NIOSMNode*>& osmNodes,
    std::map<long long int, Edge*>& toFill, std::map<long long int, Edge*>& platformShapes):
    SUMOSAXHandler("osm - file"),
    myOSMNodes(osmNodes),
    myEdgeMap(toFill),
    myPlatformShapesMap(platformShapes) {
    mySpeedMap["sign"] = MAXSPEED_UNGIVEN;
    mySpeedMap["signals"] = MAXSPEED_UNGIVEN;
    mySpeedMap["none"] = 142.; // Auswirkungen eines allgemeinen Tempolimits auf Autobahnen im Land Brandeburg (2007)
    mySpeedMap["no"] = 142.;
    mySpeedMap["walk"] = 5.;
    // https://wiki.openstreetmap.org/wiki/Key:source:maxspeed#Commonly_used_values
    mySpeedMap["AT:urban"] = 50;
    mySpeedMap["AT:rural"] = 100;
    mySpeedMap["AT:trunk"] = 100;
    mySpeedMap["AT:motorway"] = 130;
    mySpeedMap["AU:urban"] = 50;
    mySpeedMap["BE:urban"] = 50;
    mySpeedMap["BE:zone"] = 30;
    mySpeedMap["BE:motorway"] = 120;
    mySpeedMap["BE:zone30"] = 30;
    mySpeedMap["BE-VLG:rural"] = 70;
    mySpeedMap["BE-WAL:rural"] = 90;
    mySpeedMap["BE:school"] = 30;
    mySpeedMap["CZ:motorway"] = 130;
    mySpeedMap["CZ:trunk"] = 110;
    mySpeedMap["CZ:rural"] = 90;
    mySpeedMap["CZ:urban_motorway"] = 80;
    mySpeedMap["CZ:urban_trunk"] = 80;
    mySpeedMap["CZ:urban"] = 50;
    mySpeedMap["DE:motorway"] = mySpeedMap["none"];
    mySpeedMap["DE:rural"] = 100;
    mySpeedMap["DE:urban"] = 50;
    mySpeedMap["DE:bicycle_road"] = 30;
    mySpeedMap["DK:motorway"] = 130;
    mySpeedMap["DK:rural"] = 80;
    mySpeedMap["DK:urban"] = 50;
    mySpeedMap["EE:urban"] = 50;
    mySpeedMap["EE:rural"] = 90;
    mySpeedMap["ES:urban"] = 50;
    mySpeedMap["ES:zone30"] = 30;
    mySpeedMap["FR:motorway"] = 130; // 110 (raining)
    mySpeedMap["FR:rural"] = 80;
    mySpeedMap["FR:urban"] = 50;
    mySpeedMap["FR:zone30"] = 30;
    mySpeedMap["HU:living_street"] = 20;
    mySpeedMap["HU:motorway"] = 130;
    mySpeedMap["HU:rural"] = 90;
    mySpeedMap["HU:trunk"] = 110;
    mySpeedMap["HU:urban"] = 50;
    mySpeedMap["IT:rural"] = 90;
    mySpeedMap["IT:motorway"] = 130;
    mySpeedMap["IT:urban"] = 50;
    mySpeedMap["JP:nsl"] = 60;
    mySpeedMap["JP:express"] = 100;
    mySpeedMap["LT:rural"] = 90;
    mySpeedMap["LT:urban"] = 50;
    mySpeedMap["NO:rural"] = 80;
    mySpeedMap["NO:urban"] = 50;
    mySpeedMap["ON:urban"] = 50;
    mySpeedMap["ON:rural"] = 80;
    mySpeedMap["PT:motorway"] = 120;
    mySpeedMap["PT:rural"] = 90;
    mySpeedMap["PT:trunk"] = 100;
    mySpeedMap["PT:urban"] = 50;
    mySpeedMap["RO:motorway"] = 130;
    mySpeedMap["RO:rural"] = 90;
    mySpeedMap["RO:trunk"] = 100;
    mySpeedMap["RO:urban"] = 50;
    mySpeedMap["RS:living_street"] = 30;
    mySpeedMap["RS:motorway"] = 130;
    mySpeedMap["RS:rural"] = 80;
    mySpeedMap["RS:trunk"] = 100;
    mySpeedMap["RS:urban"] = 50;
    mySpeedMap["RU:living_street"] = 20;
    mySpeedMap["RU:urban"] = 60;
    mySpeedMap["RU:rural"] = 90;
    mySpeedMap["RU:motorway"] = 110;
    mySpeedMap["GB:motorway"] = 70 * KM_PER_MILE;
    mySpeedMap["GB:nsl_dual"] = 70 * KM_PER_MILE;
    mySpeedMap["GB:nsl_single"] = 60 * KM_PER_MILE;
    mySpeedMap["UK:motorway"] = 70 * KM_PER_MILE;
    mySpeedMap["UK:nsl_dual"] = 70 * KM_PER_MILE;
    mySpeedMap["UK:nsl_single"] = 60 * KM_PER_MILE;
    mySpeedMap["UZ:living_street"] = 30;
    mySpeedMap["UZ:urban"] = 70;
    mySpeedMap["UZ:rural"] = 100;
    mySpeedMap["UZ:motorway"] = 110;
    myAllAttributes = OptionsCont::getOptions().getBool("osm.all-attributes");
}

NIImporter_OpenStreetMap::EdgesHandler::~EdgesHandler() = default;

void
NIImporter_OpenStreetMap::EdgesHandler::myStartElement(int element,
        const SUMOSAXAttributes& attrs) {
    myParentElements.push_back(element);
    // parse "way" elements
    if (element == SUMO_TAG_WAY) {
        bool ok = true;
        const long long int id = attrs.get<long long int>(SUMO_ATTR_ID, nullptr, ok);
        std::string action = attrs.hasAttribute("action") ? attrs.getStringSecure("action", "") : "";
        if (action == "delete" || !ok) {
            myCurrentEdge = nullptr;
            return;
        }
        myCurrentEdge = new Edge(id);
    }
    // parse "nd" (node) elements
    if (element == SUMO_TAG_ND && myCurrentEdge != nullptr) {
        bool ok = true;
        long long int ref = attrs.get<long long int>(SUMO_ATTR_REF, nullptr, ok);
        if (ok) {
            auto node = myOSMNodes.find(ref);
            if (node == myOSMNodes.end()) {
                WRITE_WARNINGF("The referenced geometry information (ref='%') is not known", toString(ref));
                return;
            }

            ref = node->second->id; // node may have been substituted
            if (myCurrentEdge->myCurrentNodes.empty() ||
                    myCurrentEdge->myCurrentNodes.back() != ref) { // avoid consecutive duplicates
                myCurrentEdge->myCurrentNodes.push_back(ref);
            }

        }
    }
    // parse values
    if (element == SUMO_TAG_TAG && myParentElements.size() > 2
            && myParentElements[myParentElements.size() - 2] == SUMO_TAG_WAY) {
        if (myCurrentEdge == nullptr) {
            return;
        }
        bool ok = true;
        std::string key = attrs.get<std::string>(SUMO_ATTR_K, toString(myCurrentEdge->id).c_str(), ok, false);
        if (key.size() > 8 && StringUtils::startsWith(key, "cycleway:")) {
            // handle special cycleway keys
            const std::string cyclewaySpec = key.substr(9);
            key = "cycleway";
            if (cyclewaySpec == "right") {
                myCurrentEdge->myCyclewayType = (WayType)(myCurrentEdge->myCyclewayType | WAY_FORWARD);
            } else if (cyclewaySpec == "left") {
                myCurrentEdge->myCyclewayType = (WayType)(myCurrentEdge->myCyclewayType | WAY_BACKWARD);
            } else if (cyclewaySpec == "both") {
                myCurrentEdge->myCyclewayType = (WayType)(myCurrentEdge->myCyclewayType | WAY_BOTH);
            } else {
                key = "ignore";
            }
            if ((myCurrentEdge->myCyclewayType & WAY_BOTH) != 0) {
                // now we have some info on directionality
                myCurrentEdge->myCyclewayType = (WayType)(myCurrentEdge->myCyclewayType & ~WAY_UNKNOWN);
            }
        } else if (key.size() > 6 && StringUtils::startsWith(key, "busway:")) {
            // handle special busway keys
            const std::string buswaySpec = key.substr(7);
            key = "busway";
            if (buswaySpec == "right") {
                myCurrentEdge->myBuswayType = (WayType)(myCurrentEdge->myBuswayType | WAY_FORWARD);
            } else if (buswaySpec == "left") {
                myCurrentEdge->myBuswayType = (WayType)(myCurrentEdge->myBuswayType | WAY_BACKWARD);
            } else if (buswaySpec == "both") {
                myCurrentEdge->myBuswayType = (WayType)(myCurrentEdge->myBuswayType | WAY_BOTH);
            } else {
                key = "ignore";
            }
        }
        if (myAllAttributes && (key == "bridge" || key == "tunnel")) {
            myCurrentEdge->setParameter(key, "true"); // could be differentiated further if necessary
        }
        // we check whether the key is relevant (and we really need to transcode the value) to avoid hitting #1636
        if (!StringUtils::endsWith(key, "way") && !StringUtils::startsWith(key, "lanes")
                && key != "maxspeed" && key != "maxspeed:type"
                && key != "maxspeed:forward" && key != "maxspeed:backward"
                && key != "junction" && key != "name" && key != "tracks" && key != "layer"
                && key != "route"
                && key != "sidewalk"
                && key != "ref"
                && key != "highspeed"
                && !StringUtils::startsWith(key, "parking")
                && key != "postal_code"
                && key != "railway:preferred_direction"
                && key != "railway:bidirectional"
                && key != "railway:track_ref"
                && key != "usage"
                && key != "electrified"
                && key != "public_transport") {
            return;
        }
        std::string value = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentEdge->id).c_str(), ok, false);

        if ((key == "highway" && value != "platform") || key == "railway" || key == "waterway" || key == "cycleway"
                || key == "busway" || key == "route" || key == "sidewalk" || key == "highspeed"
                || key == "usage") {
            // build type id
            std::string singleTypeID = key + "." + value;
            myCurrentEdge->myCurrentIsRoad = true;
            // special cycleway stuff
            if (key == "cycleway") {
                if (value == "no") {
                    return;
                }
                if (value == "opposite_track") {
                    myCurrentEdge->myCyclewayType = WAY_BACKWARD;
                } else if (value == "opposite_lane") {
                    myCurrentEdge->myCyclewayType = WAY_BACKWARD;
                }
            }
            // special sidewalk stuff
            if (key == "sidewalk") {
                if (value == "no" || value == "none") {
                    myCurrentEdge->mySidewalkType = WAY_NONE;
                } else if (value == "both") {
                    myCurrentEdge->mySidewalkType = WAY_BOTH;
                } else if (value == "right") {
                    myCurrentEdge->mySidewalkType = WAY_FORWARD;
                } else if (value == "left") {
                    myCurrentEdge->mySidewalkType = WAY_BACKWARD;
                }
                // no need to extend the type id
                return;
            }
            // special busway stuff
            if (key == "busway") {
                if (value == "no") {
                    return;
                }
                if (value == "opposite_track") {
                    myCurrentEdge->myBuswayType = WAY_BACKWARD;
                } else if (value == "opposite_lane") {
                    myCurrentEdge->myBuswayType = WAY_BACKWARD;
                }
                // no need to extend the type id
                return;
            }
            if (key == "highspeed") {
                if (value == "no") {
                    return;
                }
                singleTypeID = "railway.highspeed";
            }
            // special case: never build compound type for highspeed rail
            if (!myCurrentEdge->myHighWayType.empty() && singleTypeID != "railway.highspeed") {
                if (myCurrentEdge->myHighWayType == "railway.highspeed") {
                    return;
                }
                // osm-ways may be used by more than one mode (eg railway.tram + highway.residential. this is relevant for multimodal traffic)
                // we create a new type for this kind of situation which must then be resolved in insertEdge()
                std::vector<std::string> types = StringTokenizer(myCurrentEdge->myHighWayType,
                                                 compoundTypeSeparator).getVector();
                types.push_back(singleTypeID);
                myCurrentEdge->myHighWayType = joinToStringSorting(types, compoundTypeSeparator);
            } else {
                myCurrentEdge->myHighWayType = singleTypeID;
            }
        } else if (key == "lanes") {
            try {
                myCurrentEdge->myNoLanes = StringUtils::toInt(value);
            } catch (NumberFormatException&) {
                // might be a list of values
                StringTokenizer st(value, ";", true);
                std::vector<std::string> list = st.getVector();
                if (list.size() >= 2) {
                    int minLanes = std::numeric_limits<int>::max();
                    try {
                        for (auto& i : list) {
                            int numLanes = StringUtils::toInt(StringUtils::prune(i));
                            minLanes = MIN2(minLanes, numLanes);
                        }
                        myCurrentEdge->myNoLanes = minLanes;
                        WRITE_WARNINGF("Using minimum lane number from list (%) for edge '%'.", value, toString(myCurrentEdge->id));
                    } catch (NumberFormatException&) {
                        WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                                      toString(myCurrentEdge->id) + "'.");
                    }
                }
            } catch (EmptyData&) {
                WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                              toString(myCurrentEdge->id) + "'.");
            }
        } else if (key == "lanes:forward") {
            try {
                myCurrentEdge->myNoLanesForward = StringUtils::toInt(value);
            } catch (...) {
                WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                              toString(myCurrentEdge->id) + "'.");
            }
        } else if (key == "lanes:backward") {
            try {
                // denote backwards count with a negative sign
                myCurrentEdge->myNoLanesForward = -StringUtils::toInt(value);
            } catch (...) {
                WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                              toString(myCurrentEdge->id) + "'.");
            }
        } else if (myCurrentEdge->myMaxSpeed == MAXSPEED_UNGIVEN &&
                   (key == "maxspeed" || key == "maxspeed:type" || key == "maxspeed:forward")) {
            // both 'maxspeed' and 'maxspeed:type' may be given so we must take care not to overwrite an already seen value
            myCurrentEdge->myMaxSpeed = interpretSpeed(key, value);
        } else if (key == "maxspeed:backward" && myCurrentEdge->myMaxSpeedBackward == MAXSPEED_UNGIVEN) {
            myCurrentEdge->myMaxSpeedBackward = interpretSpeed(key, value);
        } else if (key == "junction") {
            if ((value == "roundabout") && (myCurrentEdge->myIsOneWay.empty())) {
                myCurrentEdge->myIsOneWay = "yes";
            }
        } else if (key == "oneway") {
            myCurrentEdge->myIsOneWay = value;
        } else if (key == "name") {
            myCurrentEdge->streetName = value;
        } else if (key == "ref") {
            myCurrentEdge->ref = value;
            myCurrentEdge->setParameter("ref", value);
        } else if (key == "layer") {
            if (myAllAttributes) {
                myCurrentEdge->setParameter(key, value);
            }
            try {
                myCurrentEdge->myLayer = StringUtils::toInt(value);
            } catch (...) {
                WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                              toString(myCurrentEdge->id) + "'.");
            }
        } else if (key == "tracks") {
            try {
                if (StringUtils::toInt(value) == 1) {
                    myCurrentEdge->myIsOneWay = "true";
                } else {
                    WRITE_WARNING("Ignoring track count " + value + " for edge '" + toString(myCurrentEdge->id) + "'.");
                }
            } catch (...) {
                WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                              toString(myCurrentEdge->id) + "'.");
            }
        } else if (myAllAttributes && key == "postal_code") {
            myCurrentEdge->setParameter(key, value);
        } else if (key == "railway:preferred_direction") {
            if (value == "both") {
                myCurrentEdge->myRailDirection = WAY_BOTH;
            } else if (value == "backward") {
                myCurrentEdge->myRailDirection = WAY_BACKWARD;
            }
        } else if (key == "railway:bidirectional") {
            if (value == "regular") {
                myCurrentEdge->myRailDirection = WAY_BOTH;
            }
        } else if (key == "electrified") {
            if (value != "no") {
                myCurrentEdge->myCurrentIsElectrified = true;
            }
        } else if (key == "railway:track_ref") {
            myCurrentEdge->setParameter(key, value);
        } else if (key == "public_transport" && value == "platform") {
            myCurrentEdge->myCurrentIsPlatform = true;
        } else if (key == "parking:lane:both" && !StringUtils::startsWith(value, "no")) {
            myCurrentEdge->myParkingType |= PARKING_BOTH;
        } else if (key == "parking:lane:left" && !StringUtils::startsWith(value, "no")) {
            myCurrentEdge->myParkingType |= PARKING_LEFT;
        } else if (key == "parking:lane:right" && !StringUtils::startsWith(value, "no")) {
            myCurrentEdge->myParkingType |= PARKING_RIGHT;
        }
    }
}


double
NIImporter_OpenStreetMap::EdgesHandler::interpretSpeed(const std::string& key, std::string value) {
    if (mySpeedMap.find(value) != mySpeedMap.end()) {
        return mySpeedMap[value];
    } else {
        double conversion = 1; // OSM default is km/h
        if (StringUtils::to_lower_case(value).find("km/h") != std::string::npos) {
            value = StringUtils::prune(value.substr(0, value.find_first_not_of("0123456789")));
        } else if (StringUtils::to_lower_case(value).find("mph") != std::string::npos) {
            value = StringUtils::prune(value.substr(0, value.find_first_not_of("0123456789")));
            conversion = KM_PER_MILE;
        }
        try {
            return StringUtils::toDouble(value) * conversion;
        } catch (...) {
            WRITE_WARNING("Value of key '" + key + "' is not numeric ('" + value + "') in edge '" +
                          toString(myCurrentEdge->id) + "'.");
            return MAXSPEED_UNGIVEN;
        }
    }
}


void
NIImporter_OpenStreetMap::EdgesHandler::myEndElement(int element) {
    myParentElements.pop_back();
    if (element == SUMO_TAG_WAY && myCurrentEdge != nullptr) {
        if (myCurrentEdge->myCurrentIsRoad) {
            myEdgeMap[myCurrentEdge->id] = myCurrentEdge;
        } else if (myCurrentEdge->myCurrentIsPlatform) {
            myPlatformShapesMap[myCurrentEdge->id] = myCurrentEdge;
        } else {
            delete myCurrentEdge;
        }
        myCurrentEdge = nullptr;
    }
}

// ---------------------------------------------------------------------------
// definitions of NIImporter_OpenStreetMap::RelationHandler-methods
// ---------------------------------------------------------------------------
NIImporter_OpenStreetMap::RelationHandler::RelationHandler(
    const std::map<long long int, NIOSMNode*>& osmNodes,
    const std::map<long long int, Edge*>& osmEdges, NBPTStopCont* nbptStopCont,
    const std::map<long long int, Edge*>& platformShapes,
    NBPTLineCont* nbptLineCont,
    const OptionsCont& oc) :
    SUMOSAXHandler("osm - file"),
    myOSMNodes(osmNodes),
    myOSMEdges(osmEdges),
    myPlatformShapes(platformShapes),
    myNBPTStopCont(nbptStopCont),
    myNBPTLineCont(nbptLineCont),
    myOptionsCont(oc) {
    resetValues();
}

NIImporter_OpenStreetMap::RelationHandler::~RelationHandler() = default;

void
NIImporter_OpenStreetMap::RelationHandler::resetValues() {
    myCurrentRelation = INVALID_ID;
    myIsRestriction = false;
    myFromWay = INVALID_ID;
    myToWay = INVALID_ID;
    myViaNode = INVALID_ID;
    myViaWay = INVALID_ID;
    myRestrictionType = RESTRICTION_UNKNOWN;
    myPlatforms.clear();
    myStops.clear();
    myWays.clear();
    myIsStopArea = false;
    myIsRoute = false;
    myPTRouteType = "";
}

void
NIImporter_OpenStreetMap::RelationHandler::myStartElement(int element,
        const SUMOSAXAttributes& attrs) {
    myParentElements.push_back(element);
    // parse "way" elements
    if (element == SUMO_TAG_RELATION) {
        bool ok = true;
        myCurrentRelation = attrs.get<long long int>(SUMO_ATTR_ID, nullptr, ok);
        const std::string action = attrs.hasAttribute("action") ? attrs.getStringSecure("action", "") : "";
        if (action == "delete" || !ok) {
            myCurrentRelation = INVALID_ID;
        }
        myName = "";
        myRef = "";
        myInterval = -1;
        myNightService = "";
        return;
    }
    if (myCurrentRelation == INVALID_ID) {
        return;
    }
    // parse member elements
    if (element == SUMO_TAG_MEMBER) {
        bool ok = true;
        std::string role = attrs.hasAttribute("role") ? attrs.getStringSecure("role", "") : "";
        auto ref = attrs.get<long
                   long
                   int>(SUMO_ATTR_REF, nullptr, ok);
        if (role == "via") {
            // u-turns for divided ways may be given with 2 via-nodes or 1 via-way
            std::string memberType = attrs.get<std::string>(SUMO_ATTR_TYPE, nullptr, ok);
            if (memberType == "way" && checkEdgeRef(ref)) {
                myViaWay = ref;
            } else if (memberType == "node") {
                if (myOSMNodes.find(ref) != myOSMNodes.end()) {
                    myViaNode = ref;
                } else {
                    WRITE_WARNINGF("No node found for reference '%' in relation '%'.", toString(ref), toString(myCurrentRelation));
                }
            }
        } else if (role == "from" && checkEdgeRef(ref)) {
            myFromWay = ref;
        } else if (role == "to" && checkEdgeRef(ref)) {
            myToWay = ref;
        } else if (role == "stop") {
            myStops.push_back(ref);
        } else if (role == "platform") {
            std::string memberType = attrs.get<std::string>(SUMO_ATTR_TYPE, nullptr, ok);
            if (memberType == "way") {
                const std::map<long long int,
                      NIImporter_OpenStreetMap::Edge*>::const_iterator& wayIt = myPlatformShapes.find(ref);
                if (wayIt != myPlatformShapes.end()) {

                    NIIPTPlatform platform;
                    platform.isWay = true;
                    platform.ref = ref;
                    myPlatforms.push_back(platform);
                }
            } else if (memberType == "node") {
                if (!myIsStopArea) {
                    // for routes, a mix of stop and platform members is permitted
                    myStops.push_back(ref);
                }
                NIIPTPlatform platform;
                platform.isWay = false;
                platform.ref = ref;
                myPlatforms.push_back(platform);
            }

        } else if (role.empty()) {
            std::string memberType = attrs.get<std::string>(SUMO_ATTR_TYPE, nullptr, ok);
            if (memberType == "way") {
                myWays.push_back(ref);
            } else if (memberType == "node") {
                myStops.push_back(ref);
            }
        }
        return;
    }
    // parse values
    if (element == SUMO_TAG_TAG) {
        bool ok = true;
        std::string key = attrs.get<std::string>(SUMO_ATTR_K, toString(myCurrentRelation).c_str(), ok, false);
        // we check whether the key is relevant (and we really need to transcode the value) to avoid hitting #1636
        if (key == "type" || key == "restriction") {
            std::string value = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
            if (key == "type" && value == "restriction") {
                myIsRestriction = true;
                return;
            }
            if (key == "type" && value == "route") {
                myIsRoute = true;
                return;
            }
            if (key == "restriction") {
                // @note: the 'right/left/straight' part is ignored since the information is
                // redundantly encoded in the 'from', 'to' and 'via' members
                if (value.substr(0, 5) == "only_") {
                    myRestrictionType = RESTRICTION_ONLY;
                } else if (value.substr(0, 3) == "no_") {
                    myRestrictionType = RESTRICTION_NO;
                } else {
                    WRITE_WARNINGF("Found unknown restriction type '%' in relation '%'", value, toString(myCurrentRelation));
                }
                return;
            }
        } else if (key == "public_transport") {
            std::string value = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
            if (value == "stop_area") {
                myIsStopArea = true;
            }
        } else if (key == "route") {
            std::string value = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
            if (value == "train" || value == "subway" || value == "light_rail" || value == "monorail" || value == "tram" || value == "bus"
                    || value == "trolleybus" || value == "arialway" || value == "ferry") {
                myPTRouteType = value;
            }

        } else if (key == "name") {
            myName = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
        } else if (key == "ref") {
            myRef = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
        } else if (key == "interval" || key == "headway") {
            myInterval = attrs.get<int>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
        } else if (key == "by_night") {
            myNightService = attrs.get<std::string>(SUMO_ATTR_V, toString(myCurrentRelation).c_str(), ok, false);
        }
    }
}

bool
NIImporter_OpenStreetMap::RelationHandler::checkEdgeRef(long long int ref) const {
    if (myOSMEdges.find(ref) != myOSMEdges.end()) {
        return true;
    }

    WRITE_WARNINGF("No way found for reference '%' in relation '%'", toString(ref), toString(myCurrentRelation));
    return false;

}

void
NIImporter_OpenStreetMap::RelationHandler::myEndElement(int element) {
    myParentElements.pop_back();
    if (element == SUMO_TAG_RELATION) {
        if (myIsRestriction) {
            assert(myCurrentRelation != INVALID_ID);
            bool ok = true;
            if (myRestrictionType == RESTRICTION_UNKNOWN) {
                WRITE_WARNINGF("Ignoring restriction relation '%' with unknown type.", toString(myCurrentRelation));
                ok = false;
            }
            if (myFromWay == INVALID_ID) {
                WRITE_WARNINGF("Ignoring restriction relation '%' with unknown from-way.", toString(myCurrentRelation));
                ok = false;
            }
            if (myToWay == INVALID_ID) {
                WRITE_WARNINGF("Ignoring restriction relation '%' with unknown to-way.", toString(myCurrentRelation));
                ok = false;
            }
            if (myViaNode == INVALID_ID && myViaWay == INVALID_ID) {
                WRITE_WARNINGF("Ignoring restriction relation '%' with unknown via.", toString(myCurrentRelation));
                ok = false;
            }
            if (ok && !applyRestriction()) {
                WRITE_WARNINGF("Ignoring restriction relation '%'.", toString(myCurrentRelation));
            }
        } else if (myIsStopArea && OptionsCont::getOptions().isSet("ptstop-output")) {
            for (long long ref : myStops) {
                if (myOSMNodes.find(ref) == myOSMNodes.end()) {
                    //WRITE_WARNING(
                    //    "Referenced node: '" + toString(ref) + "' in relation: '" + toString(myCurrentRelation)
                    //    + "' does not exist. Probably OSM file is incomplete.");
                    continue;
                }

                NIOSMNode* n = myOSMNodes.find(ref)->second;
                NBPTStop* ptStop = myNBPTStopCont->get(toString(n->id));
                if (ptStop == nullptr) {
                    //WRITE_WARNING(
                    //    "Relation '" + toString(myCurrentRelation) + "' refers to a non existing pt stop at node: '"
                    //    + toString(n->id) + "'. Probably OSM file is incomplete.");
                    continue;
                }
                for (NIIPTPlatform& myPlatform : myPlatforms) {
                    if (myPlatform.isWay) {
                        assert(myPlatformShapes.find(myPlatform.ref) != myPlatformShapes.end()); //already tested earlier
                        Edge* edge = (*myPlatformShapes.find(myPlatform.ref)).second;
                        if (edge->myCurrentNodes[0] == *(edge->myCurrentNodes.end() - 1)) {
                            WRITE_WARNINGF("Platform '%' in relation: '%' is given as polygon, which currently is not supported.", myPlatform.ref, myCurrentRelation);
                            continue;

                        }
                        PositionVector p;
                        for (auto nodeRef : edge->myCurrentNodes) {
                            if (myOSMNodes.find(nodeRef) == myOSMNodes.end()) {
                                //WRITE_WARNING(
                                //    "Referenced node: '" + toString(ref) + "' in relation: '" + toString(myCurrentRelation)
                                //    + "' does not exist. Probably OSM file is incomplete.");
                                continue;
                            }
                            NIOSMNode* pNode = myOSMNodes.find(nodeRef)->second;
                            Position pNodePos(pNode->lon, pNode->lat, pNode->ele);
                            if (!NBNetBuilder::transformCoordinate(pNodePos)) {
                                WRITE_ERROR("Unable to project coordinates for node '" + toString(pNode->id) + "'.");
                                continue;
                            }
                            p.push_back(pNodePos);
                        }
                        if (p.size() == 0) {
                            WRITE_WARNINGF("Referenced platform: '%' in relation: '%' is corrupt. Probably OSM file is incomplete.",
                                           toString(myPlatform.ref), toString(myCurrentRelation));
                            continue;
                        }
                        NBPTPlatform platform(p[(int)p.size() / 2], p.length());
                        ptStop->addPlatformCand(platform);
                    } else {
                        if (myOSMNodes.find(myPlatform.ref) == myOSMNodes.end()) {
                            //WRITE_WARNING(
                            //    "Referenced node: '" + toString(ref) + "' in relation: '" + toString(myCurrentRelation)
                            //    + "' does not exist. Probably OSM file is incomplete.");
                            continue;
                        }
                        NIOSMNode* pNode = myOSMNodes.find(myPlatform.ref)->second;
                        Position platformPos(pNode->lon, pNode->lat, pNode->ele);
                        if (!NBNetBuilder::transformCoordinate(platformPos)) {
                            WRITE_ERROR("Unable to project coordinates for node '" + toString(pNode->id) + "'.");
                        }
                        NBPTPlatform platform(platformPos, myOptionsCont.getFloat(
                                                  "osm.stop-output.length"));
                        ptStop->addPlatformCand(platform);

                    }
                }
                ptStop->setIsMultipleStopPositions(myStops.size() > 1);
            }
        } else if (myPTRouteType != "" && myIsRoute && OptionsCont::getOptions().isSet("ptline-output")) {
            NBPTLine* ptLine = new NBPTLine(toString(myCurrentRelation), myName, myPTRouteType, myRef, myInterval, myNightService, interpretTransportType(myPTRouteType));
            ptLine->setMyNumOfStops((int)myStops.size());
            for (long long ref : myStops) {
                if (myOSMNodes.find(ref) == myOSMNodes.end()) {
                    //WRITE_WARNING(
                    //    "Referenced node: '" + toString(ref) + "' in relation: '" + toString(myCurrentRelation)
                    //    + "' does not exist. Probably OSM file is incomplete.");
//                    resetValues();
//                    return;
                    if (!ptLine->getStops().empty()) {
                        WRITE_WARNINGF("Done reading first coherent chunk of pt stops. Further stops in relation % are ignored", myCurrentRelation);
                        break;
                    }
                    continue;
                }

                NIOSMNode* n = myOSMNodes.find(ref)->second;
                NBPTStop* ptStop = myNBPTStopCont->get(toString(n->id));
                if (ptStop == nullptr) {
                    // loose stop, which must later be mapped onto a line way
                    Position ptPos(n->lon, n->lat, n->ele);
                    if (!NBNetBuilder::transformCoordinate(ptPos)) {
                        WRITE_ERROR("Unable to project coordinates for node '" + toString(n->id) + "'.");
                    }
                    ptStop = new NBPTStop(toString(n->id), ptPos, "", "", n->ptStopLength, n->name, n->permissions);
                    myNBPTStopCont->insert(ptStop);
                }
                ptLine->addPTStop(ptStop);
            }
            for (long long& myWay : myWays) {
                auto entr = myOSMEdges.find(myWay);
                if (entr != myOSMEdges.end()) {
                    Edge* edge = entr->second;
                    for (long long& myCurrentNode : edge->myCurrentNodes) {
                        ptLine->addWayNode(myWay, myCurrentNode);
                    }
                }
            }
            if (ptLine->getStops().empty()) {
                WRITE_WARNINGF("PT line in relation % with no stops ignored. Probably OSM file is incomplete.", myCurrentRelation);
                resetValues();
                return;
            }
            if (myNBPTLineCont->getLines().count(ptLine->getLineID()) == 0) {
                myNBPTLineCont->insert(ptLine);
            } else {
                WRITE_WARNINGF("Ignoring duplicate PT line '%'.", toString(myCurrentRelation));
                delete ptLine;
            }
        }
        // other relations might use similar subelements so reset in any case
        resetValues();
    }
}

bool
NIImporter_OpenStreetMap::RelationHandler::applyRestriction() const {
    // since OSM ways are bidirectional we need the via to figure out which direction was meant
    if (myViaNode != INVALID_ID) {
        NBNode* viaNode = myOSMNodes.find(myViaNode)->second->node;
        if (viaNode == nullptr) {
            WRITE_WARNINGF("Via-node '%' was not instantiated", toString(myViaNode));
            return false;
        }
        NBEdge* from = findEdgeRef(myFromWay, viaNode->getIncomingEdges());
        NBEdge* to = findEdgeRef(myToWay, viaNode->getOutgoingEdges());
        if (from == nullptr) {
            WRITE_WARNINGF("from-edge '%' of restriction relation could not be determined", toString(myFromWay));
            return false;
        }
        if (to == nullptr) {
            WRITE_WARNINGF("to-edge '%' of restriction relation could not be determined", toString(myToWay));
            return false;
        }
        if (myRestrictionType == RESTRICTION_ONLY) {
            from->addEdge2EdgeConnection(to, true);
            // make sure that these connections remain disabled even if network
            // modifications (ramps.guess) reset existing connections
            for (NBEdge* cand : from->getToNode()->getOutgoingEdges()) {
                if (!from->isConnectedTo(cand)) {
                    from->removeFromConnections(cand, -1, -1, true);
                }
            }
        } else {
            from->removeFromConnections(to, -1, -1, true);
        }
    } else {
        // XXX interpreting via-ways or via-node lists not yet implemented
        WRITE_WARNINGF("direction of restriction relation could not be determined%", "");
        return false;
    }
    return true;
}

NBEdge*
NIImporter_OpenStreetMap::RelationHandler::findEdgeRef(long long int wayRef,
        const std::vector<NBEdge*>& candidates) const {
    const std::string prefix = toString(wayRef);
    const std::string backPrefix = "-" + prefix;
    NBEdge* result = nullptr;
    int found = 0;
    for (auto candidate : candidates) {
        if ((candidate->getID().substr(0, prefix.size()) == prefix) ||
                (candidate->getID().substr(0, backPrefix.size()) == backPrefix)) {
            result = candidate;
            found++;
        }
    }
    if (found > 1) {
        WRITE_WARNINGF("Ambigous way reference '%' in restriction relation", prefix);
        result = nullptr;
    }
    return result;
}


void
NIImporter_OpenStreetMap::reconstructLayerElevation(const double layerElevation, NBNetBuilder& nb) {
    NBNodeCont& nc = nb.getNodeCont();
    NBEdgeCont& ec = nb.getEdgeCont();
    // reconstruct elevation from layer info
    // build a map of raising and lowering forces (attractor and distance)
    // for all nodes unknownElevation
    std::map<NBNode*, std::vector<std::pair<double, double> > > layerForces;

    // collect all nodes that belong to a way with layer information
    std::set<NBNode*> knownElevation;
    for (auto& myEdge : myEdges) {
        Edge* e = myEdge.second;
        if (e->myLayer != 0) {
            for (auto j = e->myCurrentNodes.begin(); j != e->myCurrentNodes.end(); ++j) {
                NBNode* node = nc.retrieve(toString(*j));
                if (node != nullptr) {
                    knownElevation.insert(node);
                    layerForces[node].emplace_back(e->myLayer * layerElevation, POSITION_EPS);
                }
            }
        }
    }
#ifdef DEBUG_LAYER_ELEVATION
    std::cout << "known elevations:\n";
    for (std::set<NBNode*>::iterator it = knownElevation.begin(); it != knownElevation.end(); ++it) {
        const std::vector<std::pair<double, double> >& primaryLayers = layerForces[*it];
        std::cout << "  node=" << (*it)->getID() << " ele=";
        for (std::vector<std::pair<double, double> >::const_iterator it_ele = primaryLayers.begin(); it_ele != primaryLayers.end(); ++it_ele) {
            std::cout << it_ele->first << " ";
        }
        std::cout << "\n";
    }
#endif
    // layer data only provides a lower bound on elevation since it is used to
    // resolve the relation among overlapping ways.
    // Perform a sanity check for steep inclines and raise the knownElevation if necessary
    std::map<NBNode*, double> knownEleMax;
    for (auto it : knownElevation) {
        double eleMax = -std::numeric_limits<double>::max();
        const std::vector<std::pair<double, double> >& primaryLayers = layerForces[it];
        for (const auto& primaryLayer : primaryLayers) {
            eleMax = MAX2(eleMax, primaryLayer.first);
        }
        knownEleMax[it] = eleMax;
    }
    const double gradeThreshold = OptionsCont::getOptions().getFloat("osm.layer-elevation.max-grade") / 100;
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = knownElevation.begin(); it != knownElevation.end(); ++it) {
            std::map<NBNode*, std::pair<double, double> > neighbors = getNeighboringNodes(*it,
                    knownEleMax[*it]
                    / gradeThreshold * 3,
                    knownElevation);
            for (auto& neighbor : neighbors) {
                if (knownElevation.count(neighbor.first) != 0) {
                    const double grade = fabs(knownEleMax[*it] - knownEleMax[neighbor.first])
                                         / MAX2(POSITION_EPS, neighbor.second.first);
#ifdef DEBUG_LAYER_ELEVATION
                    std::cout << "   grade at node=" << (*it)->getID() << " ele=" << knownEleMax[*it] << " neigh=" << it_neigh->first->getID() << " neighEle=" << knownEleMax[it_neigh->first] << " grade=" << grade << " dist=" << it_neigh->second.first << " speed=" << it_neigh->second.second << "\n";
#endif
                    if (grade > gradeThreshold * 50 / 3.6 / neighbor.second.second) {
                        // raise the lower node to the higher level
                        const double eleMax = MAX2(knownEleMax[*it], knownEleMax[neighbor.first]);
                        if (knownEleMax[*it] < eleMax) {
                            knownEleMax[*it] = eleMax;
                        } else {
                            knownEleMax[neighbor.first] = eleMax;
                        }
                        changed = true;
                    }
                }
            }
        }
    }

    // collect all nodes within a grade-dependent range around knownElevation-nodes and apply knowElevation forces
    std::set<NBNode*> unknownElevation;
    for (auto it = knownElevation.begin(); it != knownElevation.end(); ++it) {
        const double eleMax = knownEleMax[*it];
        const double maxDist = fabs(eleMax) * 100 / layerElevation;
        std::map<NBNode*, std::pair<double, double> > neighbors = getNeighboringNodes(*it, maxDist, knownElevation);
        for (auto& neighbor : neighbors) {
            if (knownElevation.count(neighbor.first) == 0) {
                unknownElevation.insert(neighbor.first);
                layerForces[neighbor.first].emplace_back(eleMax, neighbor.second.first);
            }
        }
    }

    // apply forces to ground-level nodes (neither in knownElevation nor unknownElevation)
    for (auto it = unknownElevation.begin(); it != unknownElevation.end(); ++it) {
        double eleMax = -std::numeric_limits<double>::max();
        const std::vector<std::pair<double, double> >& primaryLayers = layerForces[*it];
        for (const auto& primaryLayer : primaryLayers) {
            eleMax = MAX2(eleMax, primaryLayer.first);
        }
        const double maxDist = fabs(eleMax) * 100 / layerElevation;
        std::map<NBNode*, std::pair<double, double> > neighbors = getNeighboringNodes(*it, maxDist, knownElevation);
        for (auto& neighbor : neighbors) {
            if (knownElevation.count(neighbor.first) == 0 && unknownElevation.count(neighbor.first) == 0) {
                layerForces[*it].emplace_back(0, neighbor.second.first);
            }
        }
    }
    // compute the elevation for each node as the weighted average of all forces
#ifdef DEBUG_LAYER_ELEVATION
    std::cout << "summation of forces\n";
#endif
    std::map<NBNode*, double> nodeElevation;
    for (auto& layerForce : layerForces) {
        const std::vector<std::pair<double, double> >& forces = layerForce.second;
        if (knownElevation.count(layerForce.first) != 0) {
            // use the maximum value
            /*
            double eleMax = -std::numeric_limits<double>::max();
            for (std::vector<std::pair<double, double> >::const_iterator it_force = forces.begin(); it_force != forces.end(); ++it_force) {
                eleMax = MAX2(eleMax, it_force->first);
            }
            */
#ifdef DEBUG_LAYER_ELEVATION
            std::cout << "   node=" << it->first->getID() << " knownElevation=" << knownEleMax[it->first] << "\n";
#endif
            nodeElevation[layerForce.first] = knownEleMax[layerForce.first];
        } else if (forces.size() == 1) {
            nodeElevation[layerForce.first] = forces.front().first;
        } else {
            // use the weighted sum
            double distSum = 0;
            for (const auto& force : forces) {
                distSum += force.second;
            }
            double weightSum = 0;
            double elevation = 0;
#ifdef DEBUG_LAYER_ELEVATION
            std::cout << "   node=" << it->first->getID() << "  distSum=" << distSum << "\n";
#endif
            for (const auto& force : forces) {
                const double weight = (distSum - force.second) / distSum;
                weightSum += weight;
                elevation += force.first * weight;

#ifdef DEBUG_LAYER_ELEVATION
                std::cout << "       force=" << it_force->first << " dist=" << it_force->second << "  weight=" << weight << " ele=" << elevation << "\n";
#endif
            }
            nodeElevation[layerForce.first] = elevation / weightSum;
        }
    }
#ifdef DEBUG_LAYER_ELEVATION
    std::cout << "final elevations:\n";
    for (std::map<NBNode*, double>::iterator it = nodeElevation.begin(); it != nodeElevation.end(); ++it) {
        std::cout << "  node=" << (it->first)->getID() << " ele=" << it->second << "\n";
    }
#endif
    // apply node elevations
    for (auto& it : nodeElevation) {
        NBNode* n = it.first;
        Position pos = n->getPosition();
        n->reinit(n->getPosition() + Position(0, 0, it.second), n->getType());
    }

    // apply way elevation to all edges that had layer information
    for (const auto& it : ec) {
        NBEdge* edge = it.second;
        const PositionVector& geom = edge->getGeometry();
        const double length = geom.length2D();
        const double zFrom = nodeElevation[edge->getFromNode()];
        const double zTo = nodeElevation[edge->getToNode()];
        // XXX if the from- or to-node was part of multiple ways with
        // different layers, reconstruct the layer value from origID
        double dist = 0;
        PositionVector newGeom;
        for (auto it_pos = geom.begin(); it_pos != geom.end(); ++it_pos) {
            if (it_pos != geom.begin()) {
                dist += (*it_pos).distanceTo2D(*(it_pos - 1));
            }
            newGeom.push_back((*it_pos) + Position(0, 0, zFrom + (zTo - zFrom) * dist / length));
        }
        edge->setGeometry(newGeom);
    }
}

std::map<NBNode*, std::pair<double, double> >
NIImporter_OpenStreetMap::getNeighboringNodes(NBNode* node, double maxDist, const std::set<NBNode*>& knownElevation) {
    std::map<NBNode*, std::pair<double, double> > result;
    std::set<NBNode*> visited;
    std::vector<NBNode*> open;
    open.push_back(node);
    result[node] = std::make_pair(0, 0);
    while (!open.empty()) {
        NBNode* n = open.back();
        open.pop_back();
        if (visited.count(n) != 0) {
            continue;
        }
        visited.insert(n);
        const EdgeVector& edges = n->getEdges();
        for (auto e : edges) {
            NBNode* s = nullptr;
            if (n->hasIncoming(e)) {
                s = e->getFromNode();
            } else {
                s = e->getToNode();
            }
            const double dist = result[n].first + e->getGeometry().length2D();
            const double speed = MAX2(e->getSpeed(), result[n].second);
            if (result.count(s) == 0) {
                result[s] = std::make_pair(dist, speed);
            } else {
                result[s] = std::make_pair(MIN2(dist, result[s].first), MAX2(speed, result[s].second));
            }
            if (dist < maxDist && knownElevation.count(s) == 0) {
                open.push_back(s);
            }
        }
    }
    result.erase(node);
    return result;
}


std::string
NIImporter_OpenStreetMap::usableType(const std::string& type, const std::string& id, NBTypeCont& tc) {
    if (tc.knows(type)) {
        return type;
    }
    if (myUnusableTypes.count(type) > 0) {
        return "";
    }
    if (myKnownCompoundTypes.count(type) > 0) {
        return myKnownCompoundTypes[type];
    }
    // this edge has a type which does not yet exist in the TypeContainer
    StringTokenizer tok = StringTokenizer(type, compoundTypeSeparator);
    std::vector<std::string> types;
    while (tok.hasNext()) {
        std::string t = tok.next();
        if (tc.knows(t)) {
            if (std::find(types.begin(), types.end(), t) == types.end()) {
                types.push_back(t);
            }
        } else if (tok.size() > 1) {
            WRITE_WARNINGF("Discarding unknown compound '%' in type '%' (first occurence for edge '%').", t, type, id);
        }
    }
    if (types.empty()) {
        WRITE_WARNINGF("Discarding unusable type '%' (first occurence for edge '%').", type, id);
        myUnusableTypes.insert(type);
        return "";
    }
    const std::string newType = joinToString(types, "|");
    if (tc.knows(newType)) {
        myKnownCompoundTypes[type] = newType;
        return newType;
    } else if (myKnownCompoundTypes.count(newType) > 0) {
        return myKnownCompoundTypes[newType];
    } else {
        // build a new type by merging all values
        int numLanes = 0;
        double maxSpeed = 0;
        int prio = 0;
        double width = NBEdge::UNSPECIFIED_WIDTH;
        double sidewalkWidth = NBEdge::UNSPECIFIED_WIDTH;
        double bikelaneWidth = NBEdge::UNSPECIFIED_WIDTH;
        bool defaultIsOneWay = true;
        SVCPermissions permissions = 0;
        bool discard = true;
        for (auto& type2 : types) {
            if (!tc.getShallBeDiscarded(type2)) {
                numLanes = MAX2(numLanes, tc.getNumLanes(type2));
                maxSpeed = MAX2(maxSpeed, tc.getSpeed(type2));
                prio = MAX2(prio, tc.getPriority(type2));
                defaultIsOneWay &= tc.getIsOneWay(type2);
                //std::cout << "merging component " << type2 << " into type " << newType << " allows=" << getVehicleClassNames(tc.getPermissions(type2)) << " oneway=" << defaultIsOneWay << "\n";
                permissions |= tc.getPermissions(type2);
                width = MAX2(width, tc.getWidth(type2));
                sidewalkWidth = MAX2(sidewalkWidth, tc.getSidewalkWidth(type2));
                bikelaneWidth = MAX2(bikelaneWidth, tc.getBikeLaneWidth(type2));
                discard = false;
            }
        }
        if (width != NBEdge::UNSPECIFIED_WIDTH) {
            width = MAX2(width, SUMO_const_laneWidth);
        }
        // ensure pedestrians don't run into trains
        if (sidewalkWidth == NBEdge::UNSPECIFIED_WIDTH
                && (permissions & SVC_PEDESTRIAN) != 0
                && (permissions & SVC_RAIL_CLASSES) != 0) {
            //std::cout << "patching sidewalk for type '" << newType << "' which allows=" << getVehicleClassNames(permissions) << "\n";
            sidewalkWidth = OptionsCont::getOptions().getFloat("default.sidewalk-width");
        }

        if (discard) {
            WRITE_WARNINGF("Discarding compound type '%' (first occurence for edge '%').", newType, id);
            myUnusableTypes.insert(newType);
            return "";
        }

        WRITE_MESSAGE("Adding new type '" + type + "' (first occurence for edge '" + id + "').");
        tc.insert(newType, numLanes, maxSpeed, prio, permissions, width, defaultIsOneWay,
                  sidewalkWidth, bikelaneWidth, 0, 0, 0);
        for (auto& type3 : types) {
            if (!tc.getShallBeDiscarded(type3)) {
                tc.copyRestrictionsAndAttrs(type3, newType);
            }
        }
        myKnownCompoundTypes[type] = newType;
        return newType;
    }
}

void
NIImporter_OpenStreetMap::extendRailwayDistances(Edge* e, NBTypeCont& tc) {
    const std::string id = toString(e->id);
    std::string type = usableType(e->myHighWayType, id, tc);
    if (type != "" && isRailway(tc.getPermissions(type))) {
        std::vector<NIOSMNode*> nodes;
        std::vector<double> usablePositions;
        std::vector<int> usableIndex;
        for (long long int n : e->myCurrentNodes) {
            NIOSMNode* node = myOSMNodes[n];
            node->positionMeters = interpretDistance(node);
            if (node->positionMeters != std::numeric_limits<double>::max()) {
                usablePositions.push_back(node->positionMeters);
                usableIndex.push_back((int)nodes.size());
            }
            nodes.push_back(node);
        }
        if (usablePositions.size() == 0) {
            return;
        } else {
            bool forward = true;
            if (usablePositions.size() == 1) {
                WRITE_WARNINGF("Ambiguous railway kilometrage direction for way '%' (assuming forward)", id);
            } else {
                forward = usablePositions.front() < usablePositions.back();
            }
            // check for consistency
            for (int i = 1; i < (int)usablePositions.size(); i++) {
                if ((usablePositions[i - 1] < usablePositions[i]) != forward) {
                    WRITE_WARNINGF("Inconsistent railway kilometrage direction for way '%': %s (skipping)", id, toString(usablePositions));
                    return;
                }
            }
            if (nodes.size() > usablePositions.size()) {
                // complete missing values
                PositionVector shape;
                for (NIOSMNode* node : nodes) {
                    shape.push_back(Position(node->lon, node->lat, 0));
                }
                if (!NBNetBuilder::transformCoordinates(shape)) {
                    return; // error will be given later
                }
                double sign = forward ? 1 : -1;
                // extend backward before first usable value
                for (int i = usableIndex.front() - 1; i >= 0; i--) {
                    nodes[i]->positionMeters = nodes[i + 1]->positionMeters - sign * shape[i].distanceTo2D(shape[i + 1]);
                }
                // extend forward
                for (int i = usableIndex.front() + 1; i < (int)nodes.size(); i++) {
                    if (nodes[i]->positionMeters == std::numeric_limits<double>::max()) {
                        nodes[i]->positionMeters = nodes[i - 1]->positionMeters + sign * shape[i].distanceTo2D(shape[i - 1]);
                    }
                }
                //std::cout << " way=" << id << " usable=" << toString(usablePositions) << "\n indices=" << toString(usableIndex)
                //    << " final:\n";
                //for (auto n : nodes) {
                //    std::cout << "    " << n->id << " " << n->positionMeters << " " << n->position<< "\n";
                //}
            }
        }
    }
}


double
NIImporter_OpenStreetMap::interpretDistance(NIOSMNode* node) {
    if (node->position.size() > 0) {
        try {
            if (StringUtils::startsWith(node->position, "mi:")) {
                return StringUtils::toDouble(node->position.substr(3)) * 1609.344; // meters per mile
            } else {
                return StringUtils::toDouble(node->position) * 1000;
            }
        } catch (...) {
            WRITE_WARNINGF("Value of railway:position is not numeric ('%') in node '%'.", node->position, toString(node->id));
        }
    }
    return std::numeric_limits<double>::max();
}

SUMOVehicleClass
NIImporter_OpenStreetMap::interpretTransportType(const std::string& type, NIOSMNode* toSet) {
    SUMOVehicleClass result = SVC_IGNORING;
    if (type == "train") {
        result = SVC_RAIL;
    } else if (type == "subway" || type == "light_rail") {
        result = SVC_RAIL_URBAN;
    } else if (SumoVehicleClassStrings.hasString(type)) {
        result = SumoVehicleClassStrings.get(type);
    }
    std::string stop = "";
    if (result == SVC_TRAM) {
        stop = ".tram";
    } else if (result == SVC_BUS) {
        stop = ".bus";
    } else if (isRailway(result)) {
        stop = ".train";
    }
    if (toSet != nullptr && result != SVC_IGNORING) {
        toSet->permissions |= result;
        toSet->ptStopLength = OptionsCont::getOptions().getFloat("osm.stop-output.length" + stop);
    }
    return result;
}


/****************************************************************************/
