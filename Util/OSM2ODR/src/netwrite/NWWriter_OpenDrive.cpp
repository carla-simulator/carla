/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
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
/// @file    NWWriter_OpenDrive.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @date    Tue, 04.05.2011
///
// Exporter writing networks using the openDRIVE format
/****************************************************************************/
#include <config.h>

#include <ctime>
#include "NWWriter_OpenDrive.h"
#include <utils/iodevices/OutputDevice_String.h>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StdDefs.h>
#include <utils/common/StringUtils.h>
#include <utils/common/StringTokenizer.h>
#include <utils/geom/GeoConvHelper.h>

#define INVALID_ID -1

//#define DEBUG_SMOOTH_GEOM
#define DEBUGCOND true

#define MIN_TURN_DIAMETER 2.0


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NWWriter_OpenDrive::writeNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether an opendrive-file shall be generated
    if (!oc.isSet("opendrive-output")) {
        return;
    }
    const NBNodeCont& nc = nb.getNodeCont();
    const NBEdgeCont& ec = nb.getEdgeCont();
    const bool origNames = oc.getBool("output.original-names");
    const bool lefthand = oc.getBool("lefthand");
    const double straightThresh = DEG2RAD(oc.getFloat("opendrive-output.straight-threshold"));
    // some internal mapping containers
    int nodeID = 1;
    int edgeID = nc.size() * 10; // distinct from node ids
    StringBijection<int> edgeMap;
    StringBijection<int> nodeMap;
    //
    OutputDevice_String device;
    device << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    device.openTag("OpenDRIVE");
    time_t now = time(nullptr);
    std::string dstr(ctime(&now));
    const Boundary& b = GeoConvHelper::getFinal().getConvBoundary();
    // write header
    device.openTag("header");
    device.writeAttr("revMajor", "1");
    device.writeAttr("revMinor", "4");
    device.writeAttr("name", "");
    device.writeAttr("version", "1.00");
    device.writeAttr("date", dstr.substr(0, dstr.length() - 1));
    device.writeAttr("north", b.ymax());
    device.writeAttr("south", b.ymin());
    device.writeAttr("east", b.xmax());
    device.writeAttr("west", b.xmin());
    /* @note obsolete in 1.4
    device.writeAttr("maxRoad", ec.size());
    device.writeAttr("maxJunc", nc.size());
    device.writeAttr("maxPrg", 0);
    */
    device.closeTag();
    // write optional geo reference
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    if (gch.usingGeoProjection()) {
        if (gch.getOffsetBase() == Position(0, 0)) {
            device.openTag("geoReference");
            device.writePreformattedTag(" <![CDATA[\n "
                                        + gch.getProjString()
                                        + "\n]]>\n");
            device.closeTag();
        } else {
            WRITE_WARNING("Could not write OpenDRIVE geoReference. Only unshifted Coordinate systems are supported (offset=" + toString(gch.getOffsetBase()) + ")");
        }
    }

    // write normal edges (road)
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        const NBEdge* e = (*i).second;
        const int fromNodeID = e->getIncomingEdges().size() > 0 ? getID(e->getFromNode()->getID(), nodeMap, nodeID) : INVALID_ID;
        const int toNodeID = e->getConnections().size() > 0 ? getID(e->getToNode()->getID(), nodeMap, nodeID) : INVALID_ID;
        writeNormalEdge(device, e,
                        getID(e->getID(), edgeMap, edgeID),
                        fromNodeID, toNodeID,
                        origNames, straightThresh,
                        nb.getShapeCont());
    }
    device.lf();

    // write junction-internal edges (road). In OpenDRIVE these are called 'paths' or 'connecting roads'
    OutputDevice_String junctionOSS(3);
    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        NBNode* n = (*i).second;
        int connectionID = 0; // unique within a junction
        const int nID = getID(n->getID(), nodeMap, nodeID);
        if (n->numNormalConnections() > 0) {
            junctionOSS << "    <junction name=\"" << n->getID() << "\" id=\"" << nID << "\">\n";
        }
        std::vector<NBEdge*> incoming = (*i).second->getIncomingEdges();
        if (lefthand) {
            std::reverse(incoming.begin(), incoming.end());
        }
        for (NBEdge* inEdge : incoming) {
            std::string centerMark = "none";
            const int inEdgeID = getID(inEdge->getID(), edgeMap, edgeID);
            // group parallel edges
            const NBEdge* outEdge = nullptr;
            bool isOuterEdge = true; // determine where a solid outer border should be drawn
            int lastFromLane = -1;
            std::vector<NBEdge::Connection> parallel;
            std::vector<NBEdge::Connection> connections = inEdge->getConnections();
            if (lefthand) {
                std::reverse(connections.begin(), connections.end());
            }
            for (const NBEdge::Connection& c : connections) {
                assert(c.toEdge != 0);
                if (outEdge != c.toEdge || c.fromLane == lastFromLane) {
                    if (outEdge != nullptr) {
                        if (isOuterEdge) {
                            addPedestrianConnection(inEdge, outEdge, parallel);
                        }
                        connectionID = writeInternalEdge(device, junctionOSS, inEdge, nID,
                                                         getID(parallel.back().getInternalLaneID(), edgeMap, edgeID),
                                                         inEdgeID,
                                                         getID(outEdge->getID(), edgeMap, edgeID),
                                                         connectionID,
                                                         parallel, isOuterEdge, straightThresh, centerMark);
                        parallel.clear();
                        isOuterEdge = false;
                    }
                    outEdge = c.toEdge;
                }
                lastFromLane = c.fromLane;
                parallel.push_back(c);
            }
            if (isOuterEdge) {
                addPedestrianConnection(inEdge, outEdge, parallel);
            }
            if (!parallel.empty()) {
                if (!lefthand && (n->geometryLike() || inEdge->isTurningDirectionAt(outEdge))) {
                    centerMark = "solid";
                }
                connectionID = writeInternalEdge(device, junctionOSS, inEdge, nID,
                                                 getID(parallel.back().getInternalLaneID(), edgeMap, edgeID),
                                                 inEdgeID,
                                                 getID(outEdge->getID(), edgeMap, edgeID),
                                                 connectionID,
                                                 parallel, isOuterEdge, straightThresh, centerMark);
                parallel.clear();
            }
        }
        if (n->numNormalConnections() > 0) {
            junctionOSS << "    </junction>\n";
        }
    }
    device.lf();
    // write junctions (junction)
    device << junctionOSS.getString();

    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        NBNode* n = (*i).second;
        const std::vector<NBEdge*>& incoming = n->getIncomingEdges();
        // check if any connections must be written
        int numConnections = 0;
        for (std::vector<NBEdge*>::const_iterator j = incoming.begin(); j != incoming.end(); ++j) {
            numConnections += (int)((*j)->getConnections().size());
        }
        if (numConnections == 0) {
            continue;
        }
        for (std::vector<NBEdge*>::const_iterator j = incoming.begin(); j != incoming.end(); ++j) {
            const NBEdge* inEdge = *j;
            const std::vector<NBEdge::Connection>& elv = inEdge->getConnections();
            for (std::vector<NBEdge::Connection>::const_iterator k = elv.begin(); k != elv.end(); ++k) {
                const NBEdge::Connection& c = *k;
                const NBEdge* outEdge = c.toEdge;
                if (outEdge == nullptr) {
                    continue;
                }
            }
        }
    }

    device.closeTag();

    OptionsCont::getOptions().output_xodr_file = device.getString();

    // device.close();
}


void
NWWriter_OpenDrive::writeNormalEdge(OutputDevice& device, const NBEdge* e,
                                    int edgeID, int fromNodeID, int toNodeID,
                                    const bool origNames,
                                    const double straightThresh,
                                    const ShapeContainer& shc) {
    // buffer output because some fields are computed out of order
    OutputDevice_String elevationOSS(3);
    elevationOSS.setPrecision(8);
    OutputDevice_String planViewOSS(2);
    planViewOSS.setPrecision(8);
    double length = 0;

    planViewOSS.openTag("planView");
    // for the shape we need to use the leftmost border of the leftmost lane
    const std::vector<NBEdge::Lane>& lanes = e->getLanes();
    PositionVector ls = getLeftLaneBorder(e);
#ifdef DEBUG_SMOOTH_GEOM
    if (DEBUGCOND) {
        std::cout << "write planview for edge " << e->getID() << "\n";
    }
#endif

    if (ls.size() == 2 || e->getPermissions() == SVC_PEDESTRIAN) {
        // foot paths may contain sharp angles
        length = writeGeomLines(ls, planViewOSS, elevationOSS);
    } else {
        bool ok = writeGeomSmooth(ls, e->getSpeed(), planViewOSS, elevationOSS, straightThresh, length);
        if (!ok) {
            WRITE_WARNING("Could not compute smooth shape for edge '" + e->getID() + "'.");
        }
    }
    planViewOSS.closeTag();

    device.openTag("road");
    device.writeAttr("name", StringUtils::escapeXML(e->getStreetName()));
    device.setPrecision(8); // length requires higher precision
    device.writeAttr("length", MAX2(POSITION_EPS, length));
    device.setPrecision(gPrecision);
    device.writeAttr("id", edgeID);
    device.writeAttr("junction", -1);
    if (fromNodeID != INVALID_ID || toNodeID != INVALID_ID) {
        device.openTag("link");
        if (fromNodeID != INVALID_ID) {
            device.openTag("predecessor");
            device.writeAttr("elementType", "junction");
            device.writeAttr("elementId", fromNodeID);
            device.closeTag();
        }
        if (toNodeID != INVALID_ID) {
            device.openTag("successor");
            device.writeAttr("elementType", "junction");
            device.writeAttr("elementId", toNodeID);
            device.closeTag();
        }
        device.closeTag();
    }
    device.openTag("type").writeAttr("s", 0).writeAttr("type", "town").closeTag();
    device << planViewOSS.getString();
    writeElevationProfile(ls, device, elevationOSS);
    device << "        <lateralProfile/>\n";
    device << "        <lanes>\n";
    device << "            <laneSection s=\"0\">\n";
    const std::string centerMark = e->getPermissions(e->getNumLanes() - 1) == 0 ? "none" : "solid";
    writeEmptyCenterLane(device, centerMark, 0.13);
    device << "                <right>\n";
    for (int j = e->getNumLanes(); --j >= 0;) {
        device << "                    <lane id=\"-" << e->getNumLanes() - j << "\" type=\"" << getLaneType(e->getPermissions(j)) << "\" level=\"true\">\n";
        device << "                        <link/>\n";
        // this could be used for geometry-link junctions without u-turn,
        // predecessor and sucessors would be lane indices,
        // road predecessor / succesfors would be of type 'road' rather than
        // 'junction'
        //device << "                            <predecessor id=\"-1\"/>\n";
        //device << "                            <successor id=\"-1\"/>\n";
        //device << "                        </link>\n";
        device << "                        <width sOffset=\"0\" a=\"" << e->getLaneWidth(j) << "\" b=\"0\" c=\"0\" d=\"0\"/>\n";
        std::string markType = "broken";
        if (j == 0) {
            markType = "solid";
        } else if (j > 0
                   && (e->getPermissions(j - 1) & ~(SVC_PEDESTRIAN | SVC_BICYCLE)) == 0) {
            // solid road mark to the left of sidewalk or bicycle lane
            markType = "solid";
        } else if (e->getPermissions(j) == 0) {
            // solid road mark to the right of a forbidden lane
            markType = "solid";
        }
        device << "                        <roadMark sOffset=\"0\" type=\"" << markType << "\" weight=\"standard\" color=\"standard\" width=\"0.13\"/>\n";
        device << "                        <speed sOffset=\"0\" max=\"" << lanes[j].speed << "\"/>\n";
        device << "                    </lane>\n";
    }
    device << "                 </right>\n";
    device << "            </laneSection>\n";
    device << "        </lanes>\n";
    writeRoadObjects(device, e, shc);
    device << "        <signals/>\n";
    if (origNames) {
        device << "        <userData code=\"sumoId\" value=\"" << e->getID() << "\"/>\n";
    }
    device.closeTag();
    checkLaneGeometries(e);
}

void
NWWriter_OpenDrive::addPedestrianConnection(const NBEdge* inEdge, const NBEdge* outEdge, std::vector<NBEdge::Connection>& parallel) {
    // by default there are no internal lanes for pedestrians. Determine if
    // one is feasible and does not exist yet.
    if (outEdge != nullptr
            && inEdge->getPermissions(0) == SVC_PEDESTRIAN
            && outEdge->getPermissions(0) == SVC_PEDESTRIAN
            && (parallel.empty()
                || parallel.front().fromLane != 0
                || parallel.front().toLane != 0)) {
        parallel.insert(parallel.begin(), NBEdge::Connection(0, const_cast<NBEdge*>(outEdge), 0, false));
        parallel.front().vmax = (inEdge->getLanes()[0].speed + outEdge->getLanes()[0].speed) / (double) 2.0;
    }
}


int
NWWriter_OpenDrive::writeInternalEdge(OutputDevice& device, OutputDevice& junctionDevice, const NBEdge* inEdge, int nodeID,
                                      int edgeID, int inEdgeID, int outEdgeID,
                                      int connectionID,
                                      const std::vector<NBEdge::Connection>& parallel,
                                      const bool isOuterEdge,
                                      const double straightThresh,
                                      const std::string& centerMark) {
    assert(parallel.size() != 0);
    const NBEdge::Connection& cLeft = parallel.back();
    const NBEdge* outEdge = cLeft.toEdge;
    PositionVector begShape = getLeftLaneBorder(inEdge, cLeft.fromLane);
    PositionVector endShape = getLeftLaneBorder(outEdge, cLeft.toLane);
    //std::cout << "computing reference line for internal lane " << cLeft.getInternalLaneID() << " begLane=" << inEdge->getLaneShape(cLeft.fromLane) << " endLane=" << outEdge->getLaneShape(cLeft.toLane) << "\n";

    double length;
    double laneOffset = 0;
    PositionVector fallBackShape;
    fallBackShape.push_back(begShape.back());
    fallBackShape.push_back(endShape.front());
    const bool turnaround = inEdge->isTurningDirectionAt(outEdge);
    bool ok = true;
    PositionVector init = NBNode::bezierControlPoints(begShape, endShape, turnaround, 25, 25, ok, nullptr, straightThresh);
    if (init.size() == 0) {
        length = fallBackShape.length2D();
        // problem with turnarounds is known, method currently returns 'ok' (#2539)
        if (!ok) {
            WRITE_WARNING("Could not compute smooth shape from lane '" + inEdge->getLaneID(cLeft.fromLane) + "' to lane '" + outEdge->getLaneID(cLeft.toLane) + "'. Use option 'junctions.scurve-stretch' or increase radius of junction '" + inEdge->getToNode()->getID() + "' to fix this.");
        } else if (length <= NUMERICAL_EPS) {
            // left-curving geometry-like edges must use the right
            // side as reference line and shift
            begShape = getRightLaneBorder(inEdge, cLeft.fromLane);
            endShape = getRightLaneBorder(outEdge, cLeft.toLane);
            init = NBNode::bezierControlPoints(begShape, endShape, turnaround, 25, 25, ok, nullptr, straightThresh);
            if (init.size() != 0) {
                length = init.bezier(12).length2D();
                laneOffset = outEdge->getLaneWidth(cLeft.toLane);
                //std::cout << " internalLane=" << cLeft.getInternalLaneID() << " length=" << length << "\n";
            }
        }
    } else {
        length = init.bezier(12).length2D();
    }

    junctionDevice << "        <connection id=\"" << connectionID << "\" incomingRoad=\"" << inEdgeID << "\" connectingRoad=\"" << edgeID << "\" contactPoint=\"start\">\n";
    device.openTag("road");
    device.writeAttr("name", cLeft.id);
    device.setPrecision(8); // length requires higher precision
    device.writeAttr("length", MAX2(POSITION_EPS, length));
    device.setPrecision(gPrecision);
    device.writeAttr("id", edgeID);
    device.writeAttr("junction", nodeID);
    device.openTag("link");
    device.openTag("predecessor");
    device.writeAttr("elementType", "road");
    device.writeAttr("elementId", inEdgeID);
    device.writeAttr("contactPoint", "end");
    device.closeTag();
    device.openTag("successor");
    device.writeAttr("elementType", "road");
    device.writeAttr("elementId", outEdgeID);
    device.writeAttr("contactPoint", "start");
    device.closeTag();
    device.closeTag();
    device.openTag("type").writeAttr("s", 0).writeAttr("type", "town").closeTag();
    device.openTag("planView");
    device.setPrecision(8); // geometry hdg requires higher precision
    OutputDevice_String elevationOSS(3);
    elevationOSS.setPrecision(8);
#ifdef DEBUG_SMOOTH_GEOM
    if (DEBUGCOND) {
        std::cout << "write planview for internal edge " << cLeft.id << " init=" << init << " fallback=" << fallBackShape
                  << " begShape=" << begShape << " endShape=" << endShape
                  << "\n";
    }
#endif
    if (init.size() == 0) {
        writeGeomLines(fallBackShape, device, elevationOSS);
    } else {
        writeGeomPP3(device, elevationOSS, init, length);
    }
    device.setPrecision(gPrecision);
    device.closeTag();
    writeElevationProfile(fallBackShape, device, elevationOSS);
    device << "        <lateralProfile/>\n";
    device << "        <lanes>\n";
    if (laneOffset != 0) {
        device << "            <laneOffset s=\"0\" a=\"" << laneOffset << "\" b=\"0\" c=\"0\" d=\"0\"/>\n";
    }
    device << "            <laneSection s=\"0\">\n";
    writeEmptyCenterLane(device, centerMark, 0);
    device << "                <right>\n";
    for (int j = (int)parallel.size(); --j >= 0;) {
        const NBEdge::Connection& c = parallel[j];
        const int fromIndex = c.fromLane - inEdge->getNumLanes();
        const int toIndex = c.toLane - outEdge->getNumLanes();
        device << "                    <lane id=\"-" << parallel.size() - j << "\" type=\"" << getLaneType(outEdge->getPermissions(c.toLane)) << "\" level=\"true\">\n";
        device << "                        <link>\n";
        device << "                            <predecessor id=\"" << fromIndex << "\"/>\n";
        device << "                            <successor id=\"" << toIndex << "\"/>\n";
        device << "                        </link>\n";
        device << "                        <width sOffset=\"0\" a=\"" << outEdge->getLaneWidth(c.toLane) << "\" b=\"0\" c=\"0\" d=\"0\"/>\n";
        std::string markType = "broken";
        if (inEdge->isTurningDirectionAt(outEdge)) {
            markType = "none";
        } else if (c.fromLane == 0 && c.toLane == 0 && isOuterEdge) {
            // solid road mark at the outer border
            markType = "solid";
        } else if (isOuterEdge && j > 0
                   && (outEdge->getPermissions(parallel[j - 1].toLane) & ~(SVC_PEDESTRIAN | SVC_BICYCLE)) == 0) {
            // solid road mark to the left of sidewalk or bicycle lane
            markType = "solid";
        } else if (!inEdge->getToNode()->geometryLike()) {
            // draw shorter road marks to indicate turning paths
            LinkDirection dir = inEdge->getToNode()->getDirection(inEdge, outEdge, OptionsCont::getOptions().getBool("lefthand"));
            if (dir == LinkDirection::LEFT || dir == LinkDirection::RIGHT || dir == LinkDirection::PARTLEFT || dir == LinkDirection::PARTRIGHT) {
                // XXX <type><line/><type> is not rendered by odrViewer so cannot be validated
                // device << "                             <type name=\"broken\" width=\"0.13\">\n";
                // device << "                                  <line length=\"0.5\" space=\"0.5\" tOffset=\"0\" sOffset=\"0\" rule=\"none\"/>\n";
                // device << "                             </type>\n";
                markType = "none";
            }
        }
        device << "                        <roadMark sOffset=\"0\" type=\"" << markType << "\" weight=\"standard\" color=\"standard\" width=\"0.13\"/>\n";
        device << "                        <speed sOffset=\"0\" max=\"" << c.vmax << "\"/>\n";
        device << "                    </lane>\n";

        const int toLane = -(parallel.size() - j);
        junctionDevice << "            <laneLink from=\"" << fromIndex << "\" to=\"" << toLane << "\"/>\n";
        connectionID++;
    }
    device << "                 </right>\n";
    device << "            </laneSection>\n";
    device << "        </lanes>\n";
    device << "        <objects/>\n";
    device << "        <signals/>\n";
    device.closeTag();
    junctionDevice << "        </connection>\n";

    return connectionID;
}


double
NWWriter_OpenDrive::writeGeomLines(const PositionVector& shape, OutputDevice& device, OutputDevice& elevationDevice, double offset) {
    for (int j = 0; j < (int)shape.size() - 1; ++j) {
        const Position& p = shape[j];
        const Position& p2 = shape[j + 1];
        const double hdg = shape.angleAt2D(j);
        const double length = p.distanceTo2D(p2);
        device.openTag("geometry");
        device.writeAttr("s", offset);
        device.writeAttr("x", p.x());
        device.writeAttr("y", p.y());
        device.writeAttr("hdg", hdg);
        device.writeAttr("length", length);
        device.openTag("line").closeTag();
        device.closeTag();
        elevationDevice << "            <elevation s=\"" << offset << "\" a=\"" << p.z() << "\" b=\"" << (p2.z() - p.z()) / MAX2(POSITION_EPS, length) << "\" c=\"0\" d=\"0\"/>\n";
        offset += length;
    }
    return offset;
}


void
NWWriter_OpenDrive::writeEmptyCenterLane(OutputDevice& device, const std::string& mark, double markWidth) {
    device << "                <center>\n";
    device << "                    <lane id=\"0\" type=\"none\" level=\"true\">\n";
    device << "                        <link/>\n";
    device << "                        <roadMark sOffset=\"0\" type=\"" << mark << "\" weight=\"standard\" color=\"standard\" width=\"" << markWidth << "\"/>\n";
    device << "                    </lane>\n";
    device << "                </center>\n";
}


int
NWWriter_OpenDrive::getID(const std::string& origID, StringBijection<int>& map, int& lastID) {
    if (map.hasString(origID)) {
        return map.get(origID);
    }
    map.insert(origID, lastID++);
    return lastID - 1;
}


std::string
NWWriter_OpenDrive::getLaneType(SVCPermissions permissions) {
    switch (permissions) {
        case SVC_PEDESTRIAN:
            return "sidewalk";
        //case (SVC_BICYCLE | SVC_PEDESTRIAN):
        //    WRITE_WARNING("Ambiguous lane type (biking+driving) for road '" + roadID + "'");
        //    return "sidewalk";
        case SVC_BICYCLE:
            return "biking";
        case 0:
            // ambiguous
            return "none";
        case SVC_RAIL:
        case SVC_RAIL_URBAN:
        case SVC_RAIL_ELECTRIC:
        case SVC_RAIL_FAST:
            return "rail";
        case SVC_TRAM:
            return "tram";
        default: {
            // complex permissions
            if (permissions == SVCAll) {
                return "driving";
            } else if (isRailway(permissions)) {
                return "rail";
            } else if ((permissions & SVC_PASSENGER) != 0) {
                return "driving";
            } else {
                return "restricted";
            }
        }
    }
}


PositionVector
NWWriter_OpenDrive::getLeftLaneBorder(const NBEdge* edge, int laneIndex, double widthOffset) {
    const bool lefthand = OptionsCont::getOptions().getBool("lefthand");
    if (laneIndex == -1) {
        // leftmost lane
        laneIndex = lefthand ? 0 : (int)edge->getNumLanes() - 1;
    }
    /// it would be tempting to use
    // PositionVector result = edge->getLaneShape(laneIndex);
    // (and the moveo2side)
    // However, the lanes in SUMO have a small lateral gap (SUMO_const_laneOffset) to account for markings
    // In OpenDRIVE this gap does not exists so we have to do all lateral
    // computations based on the reference line
    // This assumes that the 'stop line' for all lanes is colinear!
    const int leftmost = lefthand ? 0 : (int)edge->getNumLanes() - 1;
    widthOffset -= (edge->getLaneWidth(leftmost) / 2);
    // collect lane widths from left border of edge to left border of lane to connect to
    if (lefthand) {
        for (int i = leftmost; i < laneIndex; i++) {
            widthOffset += edge->getLaneWidth(i);
        }
    } else {
        for (int i = leftmost; i > laneIndex; i--) {
            widthOffset += edge->getLaneWidth(i);
        }
    }
    PositionVector result = edge->getLaneShape(leftmost);
    try {
        result.move2side(widthOffset);
    } catch (InvalidArgument&) { }
    return result;
}

PositionVector
NWWriter_OpenDrive::getRightLaneBorder(const NBEdge* edge, int laneIndex) {
    return getLeftLaneBorder(edge, laneIndex, edge->getLaneWidth(laneIndex));
}


double
NWWriter_OpenDrive::writeGeomPP3(
    OutputDevice& device,
    OutputDevice& elevationDevice,
    PositionVector init,
    double length,
    double offset) {
    assert(init.size() == 3 || init.size() == 4);

    // avoid division by 0
    length = MAX2(POSITION_EPS, length);

    const Position p = init.front();
    const double hdg = init.angleAt2D(0);

    // backup elevation values
    const PositionVector initZ = init;
    // translate to u,v coordinates
    init.add(-p.x(), -p.y(), -p.z());
    init.rotate2D(-hdg);

    // parametric coefficients
    double aU, bU, cU, dU;
    double aV, bV, cV, dV;
    double aZ, bZ, cZ, dZ;

    // unfactor the Bernstein polynomials of degree 2 (or 3) and collect the coefficients
    if (init.size() == 3) {
        //f(x, a, b ,c) = a + (2*b - 2*a)*x + (a - 2*b + c)*x*x
        aU = init[0].x();
        bU = 2 * init[1].x() - 2 * init[0].x();
        cU = init[0].x() - 2 * init[1].x() + init[2].x();
        dU = 0;

        aV = init[0].y();
        bV = 2 * init[1].y() - 2 * init[0].y();
        cV = init[0].y() - 2 * init[1].y() + init[2].y();
        dV = 0;

        // elevation is not parameteric on [0:1] but on [0:length]
        aZ = initZ[0].z();
        bZ = (2 * initZ[1].z() - 2 * initZ[0].z()) / length;
        cZ = (initZ[0].z() - 2 * initZ[1].z() + initZ[2].z()) / (length * length);
        dZ = 0;

    } else {
        // f(x, a, b, c, d) = a + (x*((3*b) - (3*a))) + ((x*x)*((3*a) + (3*c) - (6*b))) + ((x*x*x)*((3*b) - (3*c) - a + d))
        aU = init[0].x();
        bU = 3 * init[1].x() - 3 * init[0].x();
        cU = 3 * init[0].x() - 6 * init[1].x() + 3 * init[2].x();
        dU = -init[0].x() + 3 * init[1].x() - 3 * init[2].x() + init[3].x();

        aV = init[0].y();
        bV = 3 * init[1].y() - 3 * init[0].y();
        cV = 3 * init[0].y() - 6 * init[1].y() + 3 * init[2].y();
        dV = -init[0].y() + 3 * init[1].y() - 3 * init[2].y() + init[3].y();

        // elevation is not parameteric on [0:1] but on [0:length]
        aZ = initZ[0].z();
        bZ = (3 * initZ[1].z() - 3 * initZ[0].z()) / length;
        cZ = (3 * initZ[0].z() - 6 * initZ[1].z() + 3 * initZ[2].z()) / (length * length);
        dZ = (-initZ[0].z() + 3 * initZ[1].z() - 3 * initZ[2].z() + initZ[3].z()) / (length * length * length);
    }

    device.openTag("geometry");
    device.writeAttr("s", offset);
    device.writeAttr("x", p.x());
    device.writeAttr("y", p.y());
    device.writeAttr("hdg", hdg);
    device.writeAttr("length", length);

    device.openTag("paramPoly3");
    device.writeAttr("aU", aU);
    device.writeAttr("bU", bU);
    device.writeAttr("cU", cU);
    device.writeAttr("dU", dU);
    device.writeAttr("aV", aV);
    device.writeAttr("bV", bV);
    device.writeAttr("cV", cV);
    device.writeAttr("dV", dV);
    device.closeTag();
    device.closeTag();

    // write elevation
    elevationDevice.openTag("elevation");
    elevationDevice.writeAttr("s", offset);
    elevationDevice.writeAttr("a", aZ);
    elevationDevice.writeAttr("b", bZ);
    elevationDevice.writeAttr("c", cZ);
    elevationDevice.writeAttr("d", dZ);
    elevationDevice.closeTag();

    return offset + length;
}


bool
NWWriter_OpenDrive::writeGeomSmooth(const PositionVector& shape, double speed, OutputDevice& device, OutputDevice& elevationDevice, double straightThresh, double& length) {
#ifdef DEBUG_SMOOTH_GEOM
    if (DEBUGCOND) {
        std::cout << "writeGeomSmooth\n  n=" << shape.size() << " shape=" << toString(shape) << "\n";
    }
#endif
    bool ok = true;
    const double longThresh = speed; //  16.0; // make user-configurable (should match the sampling rate of the source data)
    const double curveCutout = longThresh / 2; // 8.0; // make user-configurable (related to the maximum turning rate)
    // the length of the segment that is added for cutting a corner can be bounded by 2*curveCutout (prevent the segment to be classified as 'long')
    assert(longThresh >= 2 * curveCutout);
    assert(shape.size() > 2);
    // add intermediate points wherever there is a strong angular change between long segments
    // assume the geometry is simplified so as not to contain consecutive colinear points
    PositionVector shape2 = shape;
    double maxAngleDiff = 0;
    double offset = 0;
    for (int j = 1; j < (int)shape.size() - 1; ++j) {
        //const double hdg = shape.angleAt2D(j);
        const Position& p0 = shape[j - 1];
        const Position& p1 = shape[j];
        const Position& p2 = shape[j + 1];
        const double dAngle = fabs(GeomHelper::angleDiff(p0.angleTo2D(p1), p1.angleTo2D(p2)));
        const double length1 = p0.distanceTo2D(p1);
        const double length2 = p1.distanceTo2D(p2);
        maxAngleDiff = MAX2(maxAngleDiff, dAngle);
#ifdef DEBUG_SMOOTH_GEOM
        if (DEBUGCOND) {
            std::cout << "   j=" << j << " dAngle=" << RAD2DEG(dAngle) << " length1=" << length1 << " length2=" << length2 << "\n";
        }
#endif
        if (dAngle > straightThresh
                && (length1 > longThresh || j == 1)
                && (length2 > longThresh || j == (int)shape.size() - 2)) {
            shape2.insertAtClosest(shape.positionAtOffset2D(offset + length1 - MIN2(length1 - POSITION_EPS, curveCutout)), false);
            shape2.insertAtClosest(shape.positionAtOffset2D(offset + length1 + MIN2(length2 - POSITION_EPS, curveCutout)), false);
            shape2.removeClosest(p1);
        }
        offset += length1;
    }
    const int numPoints = (int)shape2.size();
#ifdef DEBUG_SMOOTH_GEOM
    if (DEBUGCOND) {
        std::cout << " n=" << numPoints << " shape2=" << toString(shape2) << "\n";
    }
#endif

    if (maxAngleDiff < straightThresh) {
        length = writeGeomLines(shape2, device, elevationDevice, 0);
#ifdef DEBUG_SMOOTH_GEOM
        if (DEBUGCOND) {
            std::cout << "   special case: all lines. maxAngleDiff=" << maxAngleDiff << "\n";
        }
#endif
        return ok;
    }

    // write the long segments as lines, short segments as curves
    offset = 0;
    for (int j = 0; j < numPoints - 1; ++j) {
        const Position& p0 = shape2[j];
        const Position& p1 = shape2[j + 1];
        PositionVector line;
        line.push_back(p0);
        line.push_back(p1);
        const double lineLength = line.length2D();
        if (lineLength >= longThresh) {
            offset = writeGeomLines(line, device, elevationDevice, offset);
#ifdef DEBUG_SMOOTH_GEOM
            if (DEBUGCOND) {
                std::cout << "      writeLine=" << toString(line) << "\n";
            }
#endif
        } else {
            // find control points
            PositionVector begShape;
            PositionVector endShape;
            if (j == 0 || j == numPoints - 2) {
                // keep the angle of the first/last segment but end at the front of the shape
                begShape = line;
                begShape.add(p0 - begShape.back());
            } else if (j == 1 || p0.distanceTo2D(shape2[j - 1]) > longThresh) {
                // use the previous segment if it is long or the first one
                begShape.push_back(shape2[j - 1]);
                begShape.push_back(p0);
            } else {
                // end at p0 with mean angle of the previous and current segment
                begShape.push_back(shape2[j - 1]);
                begShape.push_back(p1);
                begShape.add(p0 - begShape.back());
            }

            if (j == 0 || j == numPoints - 2) {
                // keep the angle of the first/last segment but start at the end of the shape
                endShape = line;
                endShape.add(p1 - endShape.front());
            } else if (j == numPoints - 3 || p1.distanceTo2D(shape2[j + 2]) > longThresh) {
                // use the next segment if it is long or the final one
                endShape.push_back(p1);
                endShape.push_back(shape2[j + 2]);
            } else {
                // start at p1 with mean angle of the current and next segment
                endShape.push_back(p0);
                endShape.push_back(shape2[j + 2]);
                endShape.add(p1 - endShape.front());
            }
            const double extrapolateLength = MIN2((double)25, lineLength / 4);
            PositionVector init = NBNode::bezierControlPoints(begShape, endShape, false, extrapolateLength, extrapolateLength, ok, nullptr, straightThresh);
            if (init.size() == 0) {
                // could not compute control points, write line
                offset = writeGeomLines(line, device, elevationDevice, offset);
#ifdef DEBUG_SMOOTH_GEOM
                if (DEBUGCOND) {
                    std::cout << "      writeLine lineLength=" << lineLength << " begShape" << j << "=" << toString(begShape) << " endShape" << j << "=" << toString(endShape) << " init" << j << "=" << toString(init) << "\n";
                }
#endif
            } else {
                // write bezier
                const double curveLength = init.bezier(12).length2D();
                offset = writeGeomPP3(device, elevationDevice, init, curveLength, offset);
#ifdef DEBUG_SMOOTH_GEOM
                if (DEBUGCOND) {
                    std::cout << "      writeCurve lineLength=" << lineLength << " curveLength=" << curveLength << " begShape" << j << "=" << toString(begShape) << " endShape" << j << "=" << toString(endShape) << " init" << j << "=" << toString(init) << "\n";
                }
#endif
            }
        }
    }
    length = offset;
    return ok;
}


void
NWWriter_OpenDrive::writeElevationProfile(const PositionVector& shape, OutputDevice& device, const OutputDevice_String& elevationDevice) {
    // check if the shape is flat
    bool flat = true;
    double z = shape.size() == 0 ? 0 : shape[0].z();
    for (int i = 1; i < (int)shape.size(); ++i) {
        if (fabs(shape[i].z() - z) > NUMERICAL_EPS) {
            flat = false;
            break;
        }
    }
    device << "        <elevationProfile>\n";
    if (flat) {
        device << "            <elevation s=\"0\" a=\"" << z << "\" b=\"0\" c=\"0\" d=\"0\"/>\n";
    } else {
        device << elevationDevice.getString();
    }
    device << "        </elevationProfile>\n";

}


void
NWWriter_OpenDrive::checkLaneGeometries(const NBEdge* e) {
    if (e->getNumLanes() > 1) {
        // compute 'stop line' of rightmost lane
        const PositionVector shape0 = e->getLaneShape(0);
        assert(shape0.size() >= 2);
        const Position& from = shape0[-2];
        const Position& to = shape0[-1];
        PositionVector stopLine;
        stopLine.push_back(to);
        stopLine.push_back(to - PositionVector::sideOffset(from, to, -1000.0));
        // endpoints of all other lanes should be on the stop line
        for (int lane = 1; lane < e->getNumLanes(); ++lane) {
            const double dist = stopLine.distance2D(e->getLaneShape(lane)[-1]);
            if (dist > NUMERICAL_EPS) {
                WRITE_WARNING("Uneven stop line at lane '" + e->getLaneID(lane) + "' (dist=" + toString(dist) + ") cannot be represented in OpenDRIVE.");
            }
        }
    }
}

void
NWWriter_OpenDrive::writeRoadObjects(OutputDevice& device, const NBEdge* e, const ShapeContainer& shc) {
    if (e->knowsParameter("roadObjects")) {
        device.openTag("objects");
        device.setPrecision(8); // geometry hdg requires higher precision
        PositionVector road = getLeftLaneBorder(e);
        for (std::string id : StringTokenizer(e->getParameter("roadObjects", "")).getVector()) {
            SUMOPolygon* p = shc.getPolygons().get(id);
            if (p == nullptr) {
                WRITE_WARNING("Road object polygon '" + id + "' not found for edge '" + e->getID() + "'");
            } else if (p->getShape().size() != 4) {
                WRITE_WARNING("Cannot convert road object polygon '" + id + "' with " + toString(p->getShape().size()) + " points for edge '" + e->getID() + "'");
            } else {
                const PositionVector& shape = p->getShape();
                device.openTag("object");
                Position center = shape.getPolygonCenter();
                PositionVector sideline = shape.getSubpartByIndex(0, 2);
                PositionVector ortholine = shape.getSubpartByIndex(1, 2);
                const double absAngle = sideline.angleAt2D(0);
                const double length = sideline.length2D();
                const double width = ortholine.length2D();
                const double edgeOffset = road.nearest_offset_to_point2D(center);
                if (edgeOffset == GeomHelper::INVALID_OFFSET) {
                    WRITE_WARNING("Cannot map road object polygon '" + id + "' with center " + toString(center) + " onto edge '" + e->getID() + "'");
                    continue;
                }
                Position edgePos = road.positionAtOffset2D(edgeOffset);
                const double edgeAngle = road.rotationAtOffset(edgeOffset);
                const double relAngle = absAngle - edgeAngle;
                double sideOffset = center.distanceTo2D(edgePos);
                // determine sign of sideOffset
                PositionVector tmp = road.getSubpart2D(MAX2(0.0, edgeOffset - 1), MIN2(road.length2D(), edgeOffset + 1));
                tmp.move2side(sideOffset);
                if (tmp.distance2D(center) < sideOffset) {
                    sideOffset *= -1;
                }
                //std::cout << " id=" << id
                //    << " shape=" << shape
                //    << " center=" << center
                //    << " edgeOffset=" << edgeOffset
                //    << "\n";
                device.writeAttr("id", id);
                device.writeAttr("type", p->getShapeType());
                device.writeAttr("name", p->getParameter("name", ""));
                device.writeAttr("s", edgeOffset);
                device.writeAttr("t", sideOffset);
                device.writeAttr("width", width);
                device.writeAttr("length", length);
                device.writeAttr("hdg", relAngle);
                device.closeTag();
            }
        }
        device.setPrecision(gPrecision);
        device.closeTag();
    } else {
        device << "        <objects/>\n";
    }
}


/****************************************************************************/
