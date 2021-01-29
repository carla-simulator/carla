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
/// @file    NIImporter_MATSim.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, 26.04.2011
///
// Importer for networks stored in MATSim format
/****************************************************************************/
#include <config.h>
#include <set>
#include <functional>
#include <sstream>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/geom/GeoConvHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/xml/XMLSubSys.h>
#include "NILoader.h"
#include "NIImporter_MATSim.h"



// ===========================================================================
// static variables
// ===========================================================================
StringBijection<int>::Entry NIImporter_MATSim::matsimTags[] = {
    { "network",          NIImporter_MATSim::MATSIM_TAG_NETWORK },
    { "node",             NIImporter_MATSim::MATSIM_TAG_NODE },
    { "link",             NIImporter_MATSim::MATSIM_TAG_LINK },
    { "links",            NIImporter_MATSim::MATSIM_TAG_LINKS },
    { "",                 NIImporter_MATSim::MATSIM_TAG_NOTHING }
};


StringBijection<int>::Entry NIImporter_MATSim::matsimAttrs[] = {
    { "id",             NIImporter_MATSim::MATSIM_ATTR_ID },
    { "x",              NIImporter_MATSim::MATSIM_ATTR_X },
    { "y",              NIImporter_MATSim::MATSIM_ATTR_Y },
    { "from",           NIImporter_MATSim::MATSIM_ATTR_FROM },
    { "to",             NIImporter_MATSim::MATSIM_ATTR_TO },
    { "length",         NIImporter_MATSim::MATSIM_ATTR_LENGTH },
    { "freespeed",      NIImporter_MATSim::MATSIM_ATTR_FREESPEED },
    { "capacity",       NIImporter_MATSim::MATSIM_ATTR_CAPACITY },
    { "permlanes",      NIImporter_MATSim::MATSIM_ATTR_PERMLANES },
    { "oneway",         NIImporter_MATSim::MATSIM_ATTR_ONEWAY },
    { "modes",          NIImporter_MATSim::MATSIM_ATTR_MODES },
    { "origid",         NIImporter_MATSim::MATSIM_ATTR_ORIGID },
    { "capperiod",      NIImporter_MATSim::MATSIM_ATTR_CAPPERIOD },
    { "capDivider",     NIImporter_MATSim::MATSIM_ATTR_CAPDIVIDER },

    { "",               NIImporter_MATSim::MATSIM_ATTR_NOTHING }
};


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NIImporter_MATSim::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isSet("matsim-files")) {
        return;
    }
    /* Parse file(s)
     * Each file is parsed twice: first for nodes, second for edges. */
    std::vector<std::string> files = oc.getStringVector("matsim-files");
    // load nodes, first
    NodesHandler nodesHandler(nb.getNodeCont());
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        // nodes
        if (!FileHelpers::isReadable(*file)) {
            WRITE_ERROR("Could not open matsim-file '" + *file + "'.");
            return;
        }
        nodesHandler.setFileName(*file);
        PROGRESS_BEGIN_MESSAGE("Parsing nodes from matsim-file '" + *file + "'");
        if (!XMLSubSys::runParser(nodesHandler, *file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();
    }
    // load edges, then
    EdgesHandler edgesHandler(nb.getNodeCont(), nb.getEdgeCont(), oc.getBool("matsim.keep-length"),
                              oc.getBool("matsim.lanes-from-capacity"), NBCapacity2Lanes(oc.getFloat("lanes-from-capacity.norm")));
    for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file) {
        // edges
        edgesHandler.setFileName(*file);
        PROGRESS_BEGIN_MESSAGE("Parsing edges from matsim-file '" + *file + "'");
        XMLSubSys::runParser(edgesHandler, *file);
        PROGRESS_DONE_MESSAGE();
    }
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_MATSim::NodesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_MATSim::NodesHandler::NodesHandler(NBNodeCont& toFill)
    : GenericSAXHandler(matsimTags, MATSIM_TAG_NOTHING,
                        matsimAttrs, MATSIM_ATTR_NOTHING,
                        "matsim - file"), myNodeCont(toFill) {
}


NIImporter_MATSim::NodesHandler::~NodesHandler() {}


void
NIImporter_MATSim::NodesHandler::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    if (element != MATSIM_TAG_NODE) {
        return;
    }
    // get the id, report a warning if not given or empty...
    bool ok = true;
    std::string id = attrs.get<std::string>(MATSIM_ATTR_ID, nullptr, ok);
    double x = attrs.get<double>(MATSIM_ATTR_X, id.c_str(), ok);
    double y = attrs.get<double>(MATSIM_ATTR_Y, id.c_str(), ok);
    if (!ok) {
        return;
    }
    Position pos(x, y);
    if (!NBNetBuilder::transformCoordinate(pos)) {
        WRITE_ERROR("Unable to project coordinates for node '" + id + "'.");
    }
    NBNode* node = new NBNode(id, pos);
    if (!myNodeCont.insert(node)) {
        delete node;
        WRITE_ERROR("Could not add node '" + id + "'. Probably declared twice.");
    }
}



// ---------------------------------------------------------------------------
// definitions of NIImporter_MATSim::EdgesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_MATSim::EdgesHandler::EdgesHandler(const NBNodeCont& nc, NBEdgeCont& toFill,
        bool keepEdgeLengths, bool lanesFromCapacity,
        NBCapacity2Lanes capacity2Lanes)
    : GenericSAXHandler(matsimTags, MATSIM_TAG_NOTHING,
                        matsimAttrs, MATSIM_ATTR_NOTHING, "matsim - file"),
      myNodeCont(nc), myEdgeCont(toFill), myCapacityNorm(3600),
      myKeepEdgeLengths(keepEdgeLengths), myLanesFromCapacity(lanesFromCapacity),
      myCapacity2Lanes(capacity2Lanes) {
}


NIImporter_MATSim::EdgesHandler::~EdgesHandler() {
}


void
NIImporter_MATSim::EdgesHandler::myStartElement(int element,
        const SUMOSAXAttributes& attrs) {
    if (element == MATSIM_TAG_NETWORK) {
        if (attrs.hasAttribute(MATSIM_ATTR_CAPDIVIDER)) {
            bool ok = true;
            int capDivider = attrs.get<int>(MATSIM_ATTR_CAPDIVIDER, "network", ok);
            if (ok) {
                myCapacityNorm = (double)(capDivider * 3600);
            }
        }
    }
    if (element == MATSIM_TAG_LINKS) {
        bool ok = true;
        std::string capperiod = attrs.get<std::string>(MATSIM_ATTR_CAPPERIOD, "links", ok);
        StringTokenizer st(capperiod, ":");
        if (st.size() != 3) {
            WRITE_ERROR("Bogus capacity period format; requires 'hh:mm:ss'.");
            return;
        }
        try {
            int hours = StringUtils::toInt(st.next());
            int minutes = StringUtils::toInt(st.next());
            int seconds = StringUtils::toInt(st.next());
            myCapacityNorm = (double)(hours * 3600 + minutes * 60 + seconds);
        } catch (NumberFormatException&) {
        } catch (EmptyData&) {
        }
        return;
    }

    // parse "link" elements
    if (element != MATSIM_TAG_LINK) {
        return;
    }
    bool ok = true;
    std::string id = attrs.get<std::string>(MATSIM_ATTR_ID, nullptr, ok);
    std::string fromNodeID = attrs.get<std::string>(MATSIM_ATTR_FROM, id.c_str(), ok);
    std::string toNodeID = attrs.get<std::string>(MATSIM_ATTR_TO, id.c_str(), ok);
    double length = attrs.get<double>(MATSIM_ATTR_LENGTH, id.c_str(), ok); // override computed?
    double freeSpeed = attrs.get<double>(MATSIM_ATTR_FREESPEED, id.c_str(), ok); //
    double capacity = attrs.get<double>(MATSIM_ATTR_CAPACITY, id.c_str(), ok); // override permLanes?
    double permLanes = attrs.get<double>(MATSIM_ATTR_PERMLANES, id.c_str(), ok);
    //bool oneWay = attrs.getOpt<bool>(MATSIM_ATTR_ONEWAY, id.c_str(), ok, true); // mandatory?
    std::string modes = attrs.getOpt<std::string>(MATSIM_ATTR_MODES, id.c_str(), ok, ""); // which values?
    std::string origid = attrs.getOpt<std::string>(MATSIM_ATTR_ORIGID, id.c_str(), ok, "");
    NBNode* fromNode = myNodeCont.retrieve(fromNodeID);
    NBNode* toNode = myNodeCont.retrieve(toNodeID);
    if (fromNode == nullptr) {
        WRITE_ERROR("Could not find from-node for edge '" + id + "'.");
    }
    if (toNode == nullptr) {
        WRITE_ERROR("Could not find to-node for edge '" + id + "'.");
    }
    if (fromNode == nullptr || toNode == nullptr) {
        return;
    }
    if (myLanesFromCapacity) {
        permLanes = myCapacity2Lanes.get(capacity);
    }
    NBEdge* edge = new NBEdge(id, fromNode, toNode, "", freeSpeed, (int) permLanes, -1, NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET);
    edge->setParameter("capacity", toString(capacity));
    if (myKeepEdgeLengths) {
        edge->setLoadedLength(length);
    }
    if (!myEdgeCont.insert(edge)) {
        delete edge;
        WRITE_ERROR("Could not add edge '" + id + "'. Probably declared twice.");
    }
}


/****************************************************************************/
