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
/// @file    NWWriter_XML.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Leonhard Luecken
/// @date    Tue, 11.05.2011
///
// Exporter writing networks using XML (native input) format
/****************************************************************************/
#include <config.h>
#include <algorithm>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBPTLineCont.h>
#include <netbuild/NBParking.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice.h>
#include <utils/geom/GeoConvHelper.h>
#include "NWFrame.h"
#include "NWWriter_SUMO.h"
#include "NWWriter_XML.h"



// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NWWriter_XML::writeNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether plain-output files shall be generated
    if (oc.isSet("plain-output-prefix")) {
        writeNodes(oc, nb.getNodeCont());
        if (nb.getTypeCont().size() > 0) {
            writeTypes(oc, nb.getTypeCont());
        }
        writeEdgesAndConnections(oc, nb.getNodeCont(), nb.getEdgeCont());
        writeTrafficLights(oc, nb.getTLLogicCont(), nb.getEdgeCont());
    }
    if (oc.isSet("junctions.join-output")) {
        writeJoinedJunctions(oc, nb.getNodeCont());
    }
    if (oc.isSet("street-sign-output")) {
        writeStreetSigns(oc, nb.getEdgeCont());
    }
    if (oc.exists("ptstop-output") && oc.isSet("ptstop-output")) {
        writePTStops(oc, nb.getPTStopCont());
    }
    if (oc.exists("ptline-output") && oc.isSet("ptline-output")) {
        writePTLines(oc, nb.getPTLineCont(), nb.getEdgeCont());
    }

    if (oc.exists("parking-output") && oc.isSet("parking-output")) {
        writeParkingAreas(oc, nb.getParkingCont(), nb.getEdgeCont());
    }
    if (oc.exists("taz-output") && oc.isSet("taz-output")) {
        writeDistricts(oc, nb.getDistrictCont());
    }
}


void
NWWriter_XML::writeNodes(const OptionsCont& oc, NBNodeCont& nc) {
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    bool useGeo = oc.exists("proj.plain-geo") && oc.getBool("proj.plain-geo");
    if (useGeo && !gch.usingGeoProjection()) {
        WRITE_WARNING("Ignoring option \"proj.plain-geo\" because no geo-conversion has been defined");
        useGeo = false;
    }
    const bool geoAccuracy = useGeo || gch.usingInverseGeoProjection();

    OutputDevice& device = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".nod.xml");
    std::map<SumoXMLAttr, std::string> attrs;
    attrs[SUMO_ATTR_VERSION] = toString(NETWORK_VERSION, 1);
    device.writeXMLHeader("nodes", "nodes_file.xsd", attrs);

    // write network offsets and projection to allow reconstruction of original coordinates
    if (!useGeo) {
        GeoConvHelper::writeLocation(device);
    }

    // write nodes
    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        NBNode* n = (*i).second;
        device.openTag(SUMO_TAG_NODE);
        device.writeAttr(SUMO_ATTR_ID, n->getID());
        // write position
        Position pos = n->getPosition();
        if (useGeo) {
            gch.cartesian2geo(pos);
        }
        if (geoAccuracy) {
            device.setPrecision(gPrecisionGeo);
        }
        NWFrame::writePositionLong(pos, device);
        if (geoAccuracy) {
            device.setPrecision();
        }

        device.writeAttr(SUMO_ATTR_TYPE, toString(n->getType()));
        if (n->isTLControlled()) {
            const std::set<NBTrafficLightDefinition*>& tlss = n->getControllingTLS();
            // set may contain multiple programs for the same id.
            // make sure ids are unique and sorted
            std::set<std::string> tlsIDs;
            std::set<std::string> controlledInnerEdges;
            for (std::set<NBTrafficLightDefinition*>::const_iterator it_tl = tlss.begin(); it_tl != tlss.end(); it_tl++) {
                tlsIDs.insert((*it_tl)->getID());
                std::vector<std::string> cie = (*it_tl)->getControlledInnerEdges();
                controlledInnerEdges.insert(cie.begin(), cie.end());
            }
            std::vector<std::string> sortedIDs(tlsIDs.begin(), tlsIDs.end());
            sort(sortedIDs.begin(), sortedIDs.end());
            device.writeAttr(SUMO_ATTR_TLID, sortedIDs);
            if (controlledInnerEdges.size() > 0) {
                std::vector<std::string> sortedCIEs(controlledInnerEdges.begin(), controlledInnerEdges.end());
                sort(sortedCIEs.begin(), sortedCIEs.end());
                device.writeAttr(SUMO_ATTR_CONTROLLED_INNER, joinToString(sortedCIEs, " "));
            }
        }
        if (n->hasCustomShape()) {
            writeShape(device, gch, n->getShape(), SUMO_ATTR_SHAPE, useGeo, geoAccuracy);
        }
        if (n->getRadius() != NBNode::UNSPECIFIED_RADIUS) {
            device.writeAttr(SUMO_ATTR_RADIUS, n->getRadius());
        }
        if (n->getKeepClear() == false) {
            device.writeAttr<bool>(SUMO_ATTR_KEEP_CLEAR, n->getKeepClear());
        }
        if (n->getRightOfWay() != RightOfWay::DEFAULT) {
            device.writeAttr<std::string>(SUMO_ATTR_RIGHT_OF_WAY, toString(n->getRightOfWay()));
        }
        if (n->getFringeType() != FringeType::DEFAULT) {
            device.writeAttr<std::string>(SUMO_ATTR_FRINGE, toString(n->getFringeType()));
        }
        if (n->getName() != "") {
            device.writeAttr<std::string>(SUMO_ATTR_NAME, n->getName());
        }
        n->writeParams(device);
        device.closeTag();
    }
    device.close();
}


void
NWWriter_XML::writeTypes(const OptionsCont& oc, NBTypeCont& tc) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".typ.xml");
    std::map<SumoXMLAttr, std::string> attrs;
    attrs[SUMO_ATTR_VERSION] = toString(NETWORK_VERSION, 1);
    device.writeXMLHeader("types", "types_file.xsd", attrs);
    tc.writeTypes(device);
    device.close();
}


void
NWWriter_XML::writeEdgesAndConnections(const OptionsCont& oc, NBNodeCont& nc, NBEdgeCont& ec) {
    const GeoConvHelper& gch = GeoConvHelper::getFinal();
    bool useGeo = oc.exists("proj.plain-geo") && oc.getBool("proj.plain-geo");
    const bool geoAccuracy = useGeo || gch.usingInverseGeoProjection();

    std::map<SumoXMLAttr, std::string> attrs;
    attrs[SUMO_ATTR_VERSION] = toString(NETWORK_VERSION, 1);
    OutputDevice& edevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".edg.xml");
    edevice.writeXMLHeader("edges", "edges_file.xsd", attrs);
    OutputDevice& cdevice = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".con.xml");
    cdevice.writeXMLHeader("connections", "connections_file.xsd", attrs);
    const bool writeNames = oc.getBool("output.street-names");
    LaneSpreadFunction defaultSpread = SUMOXMLDefinitions::LaneSpreadFunctions.get(oc.getString("default.spreadtype"));
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        // write the edge itself to the edges-files
        NBEdge* e = (*i).second;
        edevice.openTag(SUMO_TAG_EDGE);
        edevice.writeAttr(SUMO_ATTR_ID, e->getID());
        edevice.writeAttr(SUMO_ATTR_FROM, e->getFromNode()->getID());
        edevice.writeAttr(SUMO_ATTR_TO, e->getToNode()->getID());
        if (writeNames && e->getStreetName() != "") {
            edevice.writeAttr(SUMO_ATTR_NAME, StringUtils::escapeXML(e->getStreetName()));
        }
        edevice.writeAttr(SUMO_ATTR_PRIORITY, e->getPriority());
        // write the type if given
        if (e->getTypeID() != "") {
            edevice.writeAttr(SUMO_ATTR_TYPE, e->getTypeID());
        }
        edevice.writeAttr(SUMO_ATTR_NUMLANES, e->getNumLanes());
        if (!e->hasLaneSpecificSpeed()) {
            edevice.writeAttr(SUMO_ATTR_SPEED, e->getSpeed());
        }
        // write non-default geometry
        if (!e->hasDefaultGeometry()) {
            writeShape(edevice, gch, e->getGeometry(), SUMO_ATTR_SHAPE, useGeo, geoAccuracy);
        }
        // write the spread type if not default ("right")
        if (e->getLaneSpreadFunction() != defaultSpread) {
            edevice.writeAttr(SUMO_ATTR_SPREADTYPE, toString(e->getLaneSpreadFunction()));
        }
        // write the length if it was specified
        if (e->hasLoadedLength()) {
            edevice.writeAttr(SUMO_ATTR_LENGTH, e->getLoadedLength());
        }
        // some attributes can be set by edge default or per lane. Write as default if possible (efficiency)
        if (e->getLaneWidth() != NBEdge::UNSPECIFIED_WIDTH && !e->hasLaneSpecificWidth()) {
            edevice.writeAttr(SUMO_ATTR_WIDTH, e->getLaneWidth());
        }
        if (e->getEndOffset() != NBEdge::UNSPECIFIED_OFFSET && !e->hasLaneSpecificEndOffset()) {
            edevice.writeAttr(SUMO_ATTR_ENDOFFSET, e->getEndOffset());
        }
        if (!e->hasLaneSpecificPermissions()) {
            writePermissions(edevice, e->getPermissions(0));
        }
        if (!e->hasLaneSpecificStopOffsets() && e->getStopOffsets().size() != 0) {
            NWWriter_SUMO::writeStopOffsets(edevice, e->getStopOffsets());
        }
        if (e->getDistance() != 0) {
            edevice.writeAttr(SUMO_ATTR_DISTANCE, e->getDistance());
        }
        if (e->needsLaneSpecificOutput()) {
            for (int i = 0; i < (int)e->getLanes().size(); ++i) {
                const NBEdge::Lane& lane = e->getLanes()[i];
                edevice.openTag(SUMO_TAG_LANE);
                edevice.writeAttr(SUMO_ATTR_INDEX, i);
                // write allowed lanes
                if (e->hasLaneSpecificPermissions()) {
                    writePermissions(edevice, lane.permissions);
                }
                writePreferences(edevice, lane.preferred);
                // write other attributes
                if (lane.width != NBEdge::UNSPECIFIED_WIDTH && e->hasLaneSpecificWidth()) {
                    edevice.writeAttr(SUMO_ATTR_WIDTH, lane.width);
                }
                if (lane.endOffset != NBEdge::UNSPECIFIED_OFFSET && e->hasLaneSpecificEndOffset()) {
                    edevice.writeAttr(SUMO_ATTR_ENDOFFSET, lane.endOffset);
                }
                if (e->hasLaneSpecificSpeed()) {
                    edevice.writeAttr(SUMO_ATTR_SPEED, lane.speed);
                }
                if (lane.accelRamp) {
                    edevice.writeAttr(SUMO_ATTR_ACCELERATION, lane.accelRamp);
                }
                if (lane.customShape.size() > 0) {
                    writeShape(edevice, gch, lane.customShape, SUMO_ATTR_SHAPE, useGeo, geoAccuracy);
                }
                if (lane.type != "") {
                    edevice.writeAttr(SUMO_ATTR_TYPE, lane.type);
                }
                if (lane.oppositeID != "") {
                    edevice.openTag(SUMO_TAG_NEIGH);
                    edevice.writeAttr(SUMO_ATTR_LANE, lane.oppositeID);
                    edevice.closeTag();
                }
                lane.writeParams(edevice);
                NWWriter_SUMO::writeStopOffsets(edevice, lane.stopOffsets);
                edevice.closeTag();
            }
        }
        e->writeParams(edevice);
        edevice.closeTag();
        // write this edge's connections to the connections-files
        const std::vector<NBEdge::Connection> connections = e->getConnections();
        if (connections.empty()) {
            // if there are no connections and this appears to be customized, preserve the information
            const int numOutgoing = (int)e->getToNode()->getOutgoingEdges().size();
            if (numOutgoing > 0) {
                const SVCPermissions inPerm = e->getPermissions();
                SVCPermissions outPerm = 0;
                for (auto out : e->getToNode()->getOutgoingEdges()) {
                    outPerm |= out->getPermissions();
                }
                if ((inPerm & outPerm) != 0 && (inPerm & outPerm) != SVC_PEDESTRIAN) {
                    cdevice.openTag(SUMO_TAG_CONNECTION);
                    cdevice.writeAttr(SUMO_ATTR_FROM, e->getID());
                    cdevice.closeTag();
                    cdevice << "\n";
                }
            }
        } else {
            for (NBEdge::Connection c : connections) {
                if (useGeo) {
                    for (int i = 0; i < (int) c.customShape.size(); i++) {
                        gch.cartesian2geo(c.customShape[i]);
                    }
                }
                NWWriter_SUMO::writeConnection(cdevice, *e, c, false, NWWriter_SUMO::PLAIN, geoAccuracy);
            }
            cdevice << "\n";
        }
    }
    // write roundabout information to the edges-files
    if (ec.getRoundabouts().size() > 0) {
        edevice.lf();
        NWWriter_SUMO::writeRoundabouts(edevice, ec.getRoundabouts(), ec);
    }

    // write loaded prohibitions to the connections-file
    for (std::map<std::string, NBNode*>::const_iterator i = nc.begin(); i != nc.end(); ++i) {
        NWWriter_SUMO::writeProhibitions(cdevice, i->second->getProhibitions());
    }
    // write pedestrian crossings to the connections-file
    for (std::map<std::string, NBNode*>::const_iterator it_node = nc.begin(); it_node != nc.end(); ++it_node) {
        const std::vector<NBNode::Crossing*>& crossings = (*it_node).second->getCrossings();
        for (auto c : crossings) {
            cdevice.openTag(SUMO_TAG_CROSSING);
            cdevice.writeAttr(SUMO_ATTR_NODE, (*it_node).second->getID());
            cdevice.writeAttr(SUMO_ATTR_EDGES, c->edges);
            cdevice.writeAttr(SUMO_ATTR_PRIORITY, c->priority);
            if (c->customWidth != NBEdge::UNSPECIFIED_WIDTH) {
                cdevice.writeAttr(SUMO_ATTR_WIDTH, c->customWidth);
            }
            if (c->customTLIndex != -1) {
                cdevice.writeAttr(SUMO_ATTR_TLLINKINDEX, c->customTLIndex);
            }
            if (c->customTLIndex2 != -1) {
                cdevice.writeAttr(SUMO_ATTR_TLLINKINDEX2, c->customTLIndex2);
            }
            if (c->customShape.size() != 0) {
                writeShape(cdevice, gch, c->customShape, SUMO_ATTR_SHAPE, useGeo, geoAccuracy);
            }
            cdevice.closeTag();
        }
    }
    // write custom walkingarea shapes to the connections file
    for (std::map<std::string, NBNode*>::const_iterator it_node = nc.begin(); it_node != nc.end(); ++it_node) {
        for (const auto& wacs : it_node->second->getWalkingAreaCustomShapes()) {
            cdevice.openTag(SUMO_TAG_WALKINGAREA);
            cdevice.writeAttr(SUMO_ATTR_NODE, it_node->first);
            cdevice.writeAttr(SUMO_ATTR_EDGES, joinNamedToString(wacs.edges, " "));
            writeShape(cdevice, gch, wacs.shape, SUMO_ATTR_SHAPE, useGeo, geoAccuracy);
            cdevice.closeTag();
        }
    }

    edevice.close();
    cdevice.close();
}


void
NWWriter_XML::writeTrafficLights(const OptionsCont& oc, NBTrafficLightLogicCont& tc, NBEdgeCont& ec) {
    std::map<SumoXMLAttr, std::string> attrs;
    attrs[SUMO_ATTR_VERSION] = toString(NETWORK_VERSION, 1);
    OutputDevice& device = OutputDevice::getDevice(oc.getString("plain-output-prefix") + ".tll.xml");
    device.writeXMLHeader("tlLogics", "tllogic_file.xsd", attrs);
    NWWriter_SUMO::writeTrafficLights(device, tc);
    // we also need to remember the associations between tlLogics and connections
    // since the information in con.xml is insufficient
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        NBEdge* e = (*i).second;
        // write this edge's tl-controlled connections
        const std::vector<NBEdge::Connection> connections = e->getConnections();
        for (std::vector<NBEdge::Connection>::const_iterator c = connections.begin(); c != connections.end(); ++c) {
            if (c->tlID != "") {
                NWWriter_SUMO::writeConnection(device, *e, *c, false, NWWriter_SUMO::TLL);
            }
        }
    }
    device.close();
}


void
NWWriter_XML::writeJoinedJunctions(const OptionsCont& oc, NBNodeCont& nc) {
    std::map<SumoXMLAttr, std::string> attrs;
    attrs[SUMO_ATTR_VERSION] = toString(NETWORK_VERSION, 1);
    OutputDevice& device = OutputDevice::getDevice(oc.getString("junctions.join-output"));
    device.writeXMLHeader("nodes", "nodes_file.xsd", attrs);
    const std::vector<std::set<std::string> >& clusters = nc.getJoinedClusters();
    for (std::vector<std::set<std::string> >::const_iterator it = clusters.begin(); it != clusters.end(); it++) {
        assert((*it).size() > 0);
        device.openTag(SUMO_TAG_JOIN);
        // prepare string
        std::ostringstream oss;
        for (std::set<std::string>::const_iterator it_id = it->begin(); it_id != it->end(); it_id++) {
            oss << *it_id << " ";
        }
        // remove final space
        std::string ids = oss.str();
        device.writeAttr(SUMO_ATTR_NODES, ids.substr(0, ids.size() - 1));
        device.closeTag();
    }
    device.close();
}


void
NWWriter_XML::writeStreetSigns(const OptionsCont& oc, NBEdgeCont& ec) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("street-sign-output"));
    device.writeXMLHeader("additional", "additional_file.xsd");
    for (std::map<std::string, NBEdge*>::const_iterator i = ec.begin(); i != ec.end(); ++i) {
        NBEdge* e = (*i).second;
        const std::vector<NBSign>& signs =  e->getSigns();
        for (std::vector<NBSign>::const_iterator it = signs.begin(); it != signs.end(); ++it) {
            it->writeAsPOI(device, e);
        }
    }
    device.close();
}
void
NWWriter_XML::writePTStops(const OptionsCont& oc, NBPTStopCont& sc) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("ptstop-output"));
    device.writeXMLHeader("additional", "additional_file.xsd");
    for (std::map<std::string, NBPTStop*>::const_iterator i = sc.begin(); i != sc.end(); ++i) {
        i->second->write(device);
    }
    device.close();
}
void NWWriter_XML::writePTLines(const OptionsCont& oc, NBPTLineCont& lc, NBEdgeCont& ec) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("ptline-output"));
    device.writeXMLHeader("ptLines", "ptlines_file.xsd");
    for (const auto& item : lc.getLines()) {
        item.second->write(device, ec);
    }
    device.close();
}

void NWWriter_XML::writeParkingAreas(const OptionsCont& oc, NBParkingCont& pc, NBEdgeCont& ec) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("parking-output"));
    device.writeXMLHeader("additional", "additional_file.xsd");
    for (NBParking& p : pc) {
        p.write(device, ec);
    }
    device.close();
}

void
NWWriter_XML::writeDistricts(const OptionsCont& oc, NBDistrictCont& dc) {
    OutputDevice& device = OutputDevice::getDevice(oc.getString("taz-output"));
    device.writeXMLHeader("additional", "additional_file.xsd");
    for (std::map<std::string, NBDistrict*>::const_iterator i = dc.begin(); i != dc.end(); i++) {
        NWWriter_SUMO::writeDistrict(device, *(*i).second);
    }
}

void
NWWriter_XML::writeShape(OutputDevice& out, const GeoConvHelper& gch, PositionVector shape, SumoXMLAttr attr, bool useGeo, bool geoAccuracy) {
    if (useGeo) {
        for (int i = 0; i < (int) shape.size(); i++) {
            gch.cartesian2geo(shape[i]);
        }
    }
    if (geoAccuracy) {
        out.setPrecision(gPrecisionGeo);
    }
    out.writeAttr(attr, shape);
    if (geoAccuracy) {
        out.setPrecision();
    }
}

/****************************************************************************/


/****************************************************************************/
