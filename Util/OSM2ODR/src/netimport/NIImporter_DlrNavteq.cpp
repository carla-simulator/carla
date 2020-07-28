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
/// @file    NIImporter_DlrNavteq.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 14.04.2008
///
// Importer for networks stored in Elmar's format
/****************************************************************************/
#include <config.h>

#include <string>
#include <sstream>
#include <limits>
#include <utils/importio/LineHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/importio/LineReader.h>
#include <utils/geom/GeoConvHelper.h>
#include <netbuild/NBNetBuilder.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBTypeCont.h>
#include <netbuild/NBOwnTLDef.h>
#include <netimport/NINavTeqHelper.h>
#include "NILoader.h"
#include "NIImporter_DlrNavteq.h"


// ---------------------------------------------------------------------------
// static members
// ---------------------------------------------------------------------------
const std::string NIImporter_DlrNavteq::GEO_SCALE("1e-5");
const int NIImporter_DlrNavteq::EdgesHandler::MISSING_COLUMN = std::numeric_limits<int>::max();
const std::string NIImporter_DlrNavteq::UNDEFINED("-1");

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods
// ---------------------------------------------------------------------------
void
NIImporter_DlrNavteq::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isSet("dlr-navteq-prefix")) {
        return;
    }
    time_t csTime;
    time(&csTime);
    // parse file(s)
    LineReader lr;
    // load nodes
    std::map<std::string, PositionVector> myGeoms;
    PROGRESS_BEGIN_MESSAGE("Loading nodes");
    std::string file = oc.getString("dlr-navteq-prefix") + "_nodes_unsplitted.txt";
    NodesHandler handler1(nb.getNodeCont(), file, myGeoms);
    if (!lr.setFile(file)) {
        throw ProcessError("The file '" + file + "' could not be opened.");
    }
    lr.readAll(handler1);
    PROGRESS_DONE_MESSAGE();

    // load street names if given and wished
    std::map<std::string, std::string> streetNames; // nameID : name
    if (oc.getBool("output.street-names")) {
        file = oc.getString("dlr-navteq-prefix") + "_names.txt";
        if (lr.setFile(file)) {
            PROGRESS_BEGIN_MESSAGE("Loading street names");
            NamesHandler handler4(file, streetNames);
            lr.readAll(handler4);
            PROGRESS_DONE_MESSAGE();
        } else {
            WRITE_WARNING("Output will not contain street names because the file '" + file + "' was not found");
        }
    }

    // load edges
    PROGRESS_BEGIN_MESSAGE("Loading edges");
    file = oc.getString("dlr-navteq-prefix") + "_links_unsplitted.txt";
    // parse the file
    EdgesHandler handler2(nb.getNodeCont(), nb.getEdgeCont(), nb.getTypeCont(), file, myGeoms, streetNames);
    if (!lr.setFile(file)) {
        throw ProcessError("The file '" + file + "' could not be opened.");
    }
    lr.readAll(handler2);
    nb.getEdgeCont().recheckLaneSpread();
    PROGRESS_DONE_MESSAGE();

    // load traffic lights if given
    file = oc.getString("dlr-navteq-prefix") + "_traffic_signals.txt";
    if (lr.setFile(file)) {
        PROGRESS_BEGIN_MESSAGE("Loading traffic lights");
        TrafficlightsHandler handler3(nb.getNodeCont(), nb.getTLLogicCont(), nb.getEdgeCont(), file);
        lr.readAll(handler3);
        PROGRESS_DONE_MESSAGE();
    }

    // load prohibited manoeuvres if given
    file = oc.getString("dlr-navteq-prefix") + "_prohibited_manoeuvres.txt";
    if (lr.setFile(file)) {
        PROGRESS_BEGIN_MESSAGE("Loading prohibited manoeuvres");
        ProhibitionHandler handler6(nb.getEdgeCont(), file, csTime);
        lr.readAll(handler6);
        PROGRESS_DONE_MESSAGE();
    }

    // load connected lanes if given
    file = oc.getString("dlr-navteq-prefix") + "_connected_lanes.txt";
    if (lr.setFile(file)) {
        PROGRESS_BEGIN_MESSAGE("Loading connected lanes");
        ConnectedLanesHandler handler7(nb.getEdgeCont());
        lr.readAll(handler7);
        PROGRESS_DONE_MESSAGE();
    }

    // load time restrictions if given
    file = oc.getString("dlr-navteq-prefix") + "_links_timerestrictions.txt";
    if (lr.setFile(file)) {
        PROGRESS_BEGIN_MESSAGE("Loading time restrictions");
        if (!oc.isDefault("construction-date")) {
            csTime = readDate(oc.getString("construction-date"));
        }
        TimeRestrictionsHandler handler5(nb.getEdgeCont(), nb.getDistrictCont(), csTime);
        lr.readAll(handler5);
        handler5.printSummary();
        PROGRESS_DONE_MESSAGE();
    }
}

double
NIImporter_DlrNavteq::readVersion(const std::string& line, const std::string& file) {
    assert(line[0] == '#');
    const std::string marker = "extraction version: v";
    const std::string lowerCase = StringUtils::to_lower_case(line);
    if (lowerCase.find(marker) == std::string::npos) {
        return -1;
    }
    const int vStart = (int)(lowerCase.find(marker) + marker.size());
    const int vEnd = (int)line.find(" ", vStart);
    try {
        const double version = StringUtils::toDouble(line.substr(vStart, vEnd - vStart));
        if (version < 0) {
            throw ProcessError("Invalid version number '" + toString(version) + "' in file '" + file + "'.");
        }
        return version;
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value '" + line.substr(vStart, vEnd - vStart) + "' for version string in file '" + file + "'.");
    }
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::NodesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::NodesHandler::NodesHandler(NBNodeCont& nc,
        const std::string& file,
        std::map<std::string, PositionVector>& geoms)
    : myNodeCont(nc), myGeoms(geoms) {
    UNUSED_PARAMETER(file);
}


NIImporter_DlrNavteq::NodesHandler::~NodesHandler() {}


bool
NIImporter_DlrNavteq::NodesHandler::report(const std::string& result) {
    if (result[0] == '#') {
        return true;
    }
    std::string id;
    double x, y;
    int no_geoms, intermediate;
    // parse
    std::istringstream stream(result);
    // id
    stream >> id;
    if (stream.fail()) {
        throw ProcessError("Something is wrong with the following data line\n" + result);
    }
    // intermediate?
    stream >> intermediate;
    if (stream.fail()) {
        if (myNodeCont.size() == 0) { // be generous with extra data at beginning of file
            return true;
        }
        throw ProcessError("Non-numerical value for intermediate status in node " + id + ".");
    }
    // number of geometrical information
    stream >> no_geoms;
    if (stream.fail()) {
        throw ProcessError("Non-numerical value for number of geometries in node " + id + ".");
    }
    // geometrical information
    PositionVector geoms;
    for (int i = 0; i < no_geoms; i++) {
        stream >> x;
        if (stream.fail()) {
            throw ProcessError("Non-numerical value for x-position in node " + id + ".");
        }
        stream >> y;
        if (stream.fail()) {
            throw ProcessError("Non-numerical value for y-position in node " + id + ".");
        }
        Position pos(x, y);
        if (!NBNetBuilder::transformCoordinate(pos, true)) {
            throw ProcessError("Unable to project coordinates for node " + id + ".");
        }
        geoms.push_back(pos);
    }

    if (intermediate == 0) {
        NBNode* n = new NBNode(id, geoms[0]);
        if (!myNodeCont.insert(n)) {
            delete n;
            throw ProcessError("Could not add node '" + id + "'.");
        }
    } else {
        myGeoms[id] = geoms;
    }
    return true;
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::EdgesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::EdgesHandler::EdgesHandler(NBNodeCont& nc, NBEdgeCont& ec,
        NBTypeCont& tc, const std::string& file,
        std::map<std::string, PositionVector>& geoms,
        std::map<std::string, std::string>& streetNames):
    myNodeCont(nc),
    myEdgeCont(ec),
    myTypeCont(tc),
    myGeoms(geoms),
    myStreetNames(streetNames),
    myVersion(0),
    myFile(file) {
}


NIImporter_DlrNavteq::EdgesHandler::~EdgesHandler() {}


bool
NIImporter_DlrNavteq::EdgesHandler::report(const std::string& result) {
    // parse version number from first comment line and initialize column definitions
    if (result[0] == '#') {
        if (!myColumns.empty()) {
            return true;
        }
        const double version = readVersion(result, myFile);
        if (version > 0) {
            myVersion = version;
            // init columns
            const int NUM_COLUMNS = 25; // @note arrays must match this size!
            const int MC = MISSING_COLUMN;
            if (myVersion < 3) {
                const int columns[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, MC, 12, 13, 14, 15, 16, 17, 18, 19, 20, MC, MC, -21};
                myColumns = std::vector<int>(columns, columns + NUM_COLUMNS);
            } else if (myVersion < 6) {
                const int columns[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, MC, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, -23};
                myColumns = std::vector<int>(columns, columns + NUM_COLUMNS);
            } else {
                const int columns[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
                myColumns = std::vector<int>(columns, columns + NUM_COLUMNS);
            }
        }
        return true;
    }
    if (myColumns.empty()) {
        throw ProcessError("Missing version string in file '" + myFile + "'.");
    }
    // interpret link attributes
    StringTokenizer st(result, StringTokenizer::WHITECHARS);
    const std::string id = getColumn(st, LINK_ID);
    // form of way (for priority and permissions)
    int form_of_way;
    try {
        form_of_way = StringUtils::toInt(getColumn(st, FORM_OF_WAY));
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for form_of_way of link '" + id + "'.");
    }
    // brunnel type (bridge/tunnel/ferry (for permissions)
    int brunnel_type;
    try {
        brunnel_type = StringUtils::toInt(getColumn(st, BRUNNEL_TYPE));
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for brunnel_type of link '" + id + "'.");
    }
    // priority based on street_type / frc
    int priority;
    try {
        priority = -StringUtils::toInt(getColumn(st, FUNCTIONAL_ROAD_CLASS));
        // lower priority using form_of_way
        if (form_of_way == 11) {
            priority -= 1; // frontage road, very often with lowered curb
        } else if (form_of_way > 11) {
            priority -= 2; // parking/service access assume lowered curb
        }
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for street_type of link '" + id + "').");
    }
    // street name
    std::string streetName = getStreetNameFromIDs(
                                 getColumn(st, NAME_ID1_REGIONAL),
                                 getColumn(st, NAME_ID2_LOCAL));
    // try to get the nodes
    const std::string fromID = getColumn(st, NODE_ID_FROM);
    const std::string toID = getColumn(st, NODE_ID_TO);
    NBNode* from = myNodeCont.retrieve(fromID);
    NBNode* to = myNodeCont.retrieve(toID);
    if (from == nullptr) {
        throw ProcessError("The from-node '" + fromID + "' of link '" + id + "' could not be found");
    }
    if (to == nullptr) {
        throw ProcessError("The to-node '" + toID + "' of link '" + id + "' could not be found");
    }
    // speed
    double speed;
    try {
        speed = StringUtils::toInt(getColumn(st, SPEED_RESTRICTION, "-1")) / 3.6;
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for the SPEED_RESTRICTION of link '" + id + "'.");
    }
    if (speed < 0) {
        // speed category as fallback
        speed = NINavTeqHelper::getSpeed(id, getColumn(st, SPEED_CATEGORY));
    }
    // number of lanes
    int numLanes;
    try {
        // EXTENDED_NUMBER_OF_LANES is prefered but may not be defined
        numLanes = StringUtils::toInt(getColumn(st, EXTENDED_NUMBER_OF_LANES, "-1"));
        if (numLanes == -1) {
            numLanes = NINavTeqHelper::getLaneNumber(id, getColumn(st, NUMBER_OF_LANES), speed);
        }
    } catch (NumberFormatException&) {
        throw ProcessError("Non-numerical value for the number of lanes of link '" + id + "'.");
    }

    const std::string navTeqTypeId = getColumn(st, VEHICLE_TYPE) + "_" + getColumn(st, FORM_OF_WAY);
    // build the edge
    NBEdge* e = nullptr;
    const std::string interID = getColumn(st, BETWEEN_NODE_ID);
    if (interID == "-1") {
        e = new NBEdge(id, from, to, myTypeCont.knows(navTeqTypeId) ? navTeqTypeId : "", speed, numLanes, priority,
                       NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, streetName);
    } else {
        PositionVector geoms = myGeoms[interID];
        if (getColumn(st, CONNECTION, "0") == "1") {
            geoms = geoms.reverse();
        }
        geoms.insert(geoms.begin(), from->getPosition());
        geoms.push_back(to->getPosition());
        const std::string origID = OptionsCont::getOptions().getBool("output.original-names") ? id : "";
        e = new NBEdge(id, from, to, myTypeCont.knows(navTeqTypeId) ? navTeqTypeId : "", speed, numLanes, priority,
                       NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, geoms, streetName, origID, LaneSpreadFunction::CENTER);
    }

    // NavTeq imports can be done with a typemap (if supplied), if not, the old defaults are used
    if (myTypeCont.knows(navTeqTypeId)) {
        e->setPermissions(myTypeCont.getPermissions(navTeqTypeId));
    } else {
        // add vehicle type information to the edge
        const SVCPermissions allPermissions = myTypeCont.getPermissions("");
        const SVCPermissions defaultPermissions = OptionsCont::getOptions().getBool("dlr-navteq.tolerant-permissions") ? allPermissions : 0;
        if (myVersion < 6.0) {
            NINavTeqHelper::addVehicleClasses(*e, getColumn(st, VEHICLE_TYPE), allPermissions, defaultPermissions);
        } else {
            NINavTeqHelper::addVehicleClassesV6(*e, getColumn(st, VEHICLE_TYPE), allPermissions, defaultPermissions);
        }
        // permission modifications based on form_of_way
        if (form_of_way == 14) { // pedestrian area (fussgaengerzone)
            // unfortunately, the veh_type string is misleading in this case
            e->disallowVehicleClass(-1, SVC_PASSENGER);
        }
        // permission modifications based on brunnel_type
        if (brunnel_type == 10) { // ferry
            e->setPermissions(SVC_SHIP, -1);
        }
    }

    // insert the edge to the network
    if (!myEdgeCont.insert(e)) {
        delete e;
        throw ProcessError("Could not add edge '" + id + "'.");
    }
    return true;
}


std::string
NIImporter_DlrNavteq::EdgesHandler::getColumn(const StringTokenizer& st, ColumnName name, const std::string fallback) {
    assert(!myColumns.empty());
    if (myColumns[name] == MISSING_COLUMN) {
        if (fallback == "") {
            throw ProcessError("Missing column " + toString(name) + ".");
        } else {
            return fallback;
        }
    } else if (myColumns[name] >= 0) {
        return st.get((int)(myColumns[name]));
    } else {
        // negative column number implies an optional column
        if ((int) st.size() <= -myColumns[name]) {
            // the column is not present
            if (fallback == "") {
                throw ProcessError("Missing optional column " + toString(name) + " without default value.");
            } else {
                return fallback;
            }
        } else {
            return st.get((int)(-myColumns[name]));
        }
    }
}


std::string
NIImporter_DlrNavteq::EdgesHandler::getStreetNameFromIDs(
    const std::string& regionalID, const std::string& localID) const {
    std::string result = "";
    bool hadRegional = false;
    if (myStreetNames.count(regionalID) > 0) {
        hadRegional = true;
        result += myStreetNames[regionalID];
    }
    if (myStreetNames.count(localID) > 0) {
        if (hadRegional) {
            result += " / ";
        }
        result += myStreetNames[localID];
    }
    return result;
}

// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::TrafficlightsHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::TrafficlightsHandler::TrafficlightsHandler(NBNodeCont& nc,
        NBTrafficLightLogicCont& tlc,
        NBEdgeCont& ne,
        const std::string& file) :
    myNodeCont(nc),
    myTLLogicCont(tlc),
    myEdgeCont(ne) {
    UNUSED_PARAMETER(file);
}


NIImporter_DlrNavteq::TrafficlightsHandler::~TrafficlightsHandler() {}


bool
NIImporter_DlrNavteq::TrafficlightsHandler::report(const std::string& result) {
// #ID     POICOL-TYPE     DESCRIPTION     LONGITUDE       LATITUDE        NAVTEQ_LINK_ID  NODEID

    if (result[0] == '#') {
        return true;
    }
    StringTokenizer st(result, StringTokenizer::WHITECHARS);
    const std::string edgeID = st.get(5);
    NBEdge* edge = myEdgeCont.retrieve(edgeID);
    if (edge == nullptr) {
        WRITE_WARNINGF("The traffic light edge '%' could not be found.", edgeID);
    } else {
        NBNode* node = edge->getToNode();
        if (node->getType() != SumoXMLNodeType::TRAFFIC_LIGHT) {
            node->reinit(node->getPosition(), SumoXMLNodeType::TRAFFIC_LIGHT);
            // @note. There may be additional information somewhere in the GDF files about traffic light type ...
            TrafficLightType type = SUMOXMLDefinitions::TrafficLightTypes.get(OptionsCont::getOptions().getString("tls.default-type"));
            // @note actually we could use the navteq node ID here
            NBTrafficLightDefinition* tlDef = new NBOwnTLDef(node->getID(), node, 0, type);
            if (!myTLLogicCont.insert(tlDef)) {
                // actually, nothing should fail here
                delete tlDef;
                throw ProcessError("Could not allocate tls for '" + node->getID() + "'.");
            }
        }
    }
    return true;
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::NamesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::NamesHandler::NamesHandler(
    const std::string& file, std::map<std::string, std::string>& streetNames) :
    myStreetNames(streetNames) {
    UNUSED_PARAMETER(file);
}


NIImporter_DlrNavteq::NamesHandler::~NamesHandler() {}


bool
NIImporter_DlrNavteq::NamesHandler::report(const std::string& result) {
// # NAME_ID    Name
    if (result[0] == '#') {
        return true;
    }
    StringTokenizer st(result, StringTokenizer::TAB);
    if (st.size() == 1) {
        return true; // one line with the number of data containing lines in it (also starts with a comment # since ersion 6.5)
    }
    assert(st.size() >= 2);
    const std::string id = st.next();
    if (st.size() > 2) {
        const std::string permanent_id_info = st.next();
    }
    myStreetNames[id] = st.next();
    return true;
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::TimeRestrictionsHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::TimeRestrictionsHandler::TimeRestrictionsHandler(NBEdgeCont& ec, NBDistrictCont& dc, time_t constructionTime):
    myEdgeCont(ec),
    myDistrictCont(dc),
    myConstructionTime(constructionTime),
    myCS_min(std::numeric_limits<time_t>::max()),
    myCS_max(std::numeric_limits<time_t>::min()),
    myConstructionEntries(0),
    myNotStarted(0),
    myUnderConstruction(0),
    myFinished(0),
    myRemovedEdges(0) {
}


NIImporter_DlrNavteq::TimeRestrictionsHandler::~TimeRestrictionsHandler() {}


bool
NIImporter_DlrNavteq::TimeRestrictionsHandler::report(const std::string& result) {
// # NAME_ID    Name
    if (result[0] == '#') {
        return true;
    }
    StringTokenizer st(result, StringTokenizer::WHITECHARS);
    const std::string id = st.next();
    const std::string type = st.next();
    const std::string directionOfFlow = st.next(); // can be ignored since unidirectional edge ids are referenced in the file
    const std::string throughTraffic = st.next();
    const std::string vehicleType = st.next();
    const std::string validityPeriod = st.next();
    const std::string warning = "Unrecognized TIME_REC '" + validityPeriod + "'";
    if (type == "CS") {
        myConstructionEntries++;
        if (validityPeriod.size() > 1024) {
            WRITE_WARNING(warning);
        }
        // construction
        char start[1024];
        char duration[1024];

        int matched;

        matched = sscanf(validityPeriod.c_str(), "[(%[^)]){%[^}]}]", start, duration);
        if (matched == 2) {
            time_t tStart = readTimeRec(start, "");
            time_t tEnd = readTimeRec(start, duration);
            myCS_min = MIN2(myCS_min, tStart);
            myCS_max = MAX2(myCS_max, tEnd);
            //std::cout << " start=" << start << " tStart=" << tStart<< " translation=" << asctime(localtime(&tStart)) << "";
            //std::cout << " duration=" << duration << " tEnd=" << tEnd << " translation=" << asctime(localtime(&tEnd)) << "\n";
            if (myConstructionTime < tEnd) {
                NBEdge* edge = myEdgeCont.retrieve(id);
                if (edge != nullptr) {
                    myRemovedEdges++;
                    myEdgeCont.extract(myDistrictCont, edge, true);
                }
                if (myConstructionTime < tStart) {
                    myNotStarted++;
                } else {
                    myUnderConstruction++;
                }
            } else {
                myFinished++;
            }
        } else {
            WRITE_WARNING(warning);
        };
    }
    return true;
}


void
NIImporter_DlrNavteq::TimeRestrictionsHandler::printSummary() {
    if (myConstructionEntries > 0) {
        char buff[1024];
        std::ostringstream msg;
        strftime(buff, 1024, "%Y-%m-%d", localtime(&myCS_min));
        msg << "Parsed " << myConstructionEntries << " construction entries between " << buff;
        strftime(buff, 1024, "%Y-%m-%d", localtime(&myCS_max));
        msg << " and " << buff << ".\n";
        strftime(buff, 1024, "%Y-%m-%d", localtime(&myConstructionTime));
        msg << "Removed " << myRemovedEdges << " edges not yet constructed at " << buff << ".\n";
        msg << "   not yet started: " << myNotStarted << "\n";
        msg << "   under construction: " << myUnderConstruction << "\n";
        msg << "   finished: " << myFinished << "\n";
        WRITE_MESSAGE(msg.str());
    }
}


int
NIImporter_DlrNavteq::readPrefixedInt(const std::string& s, const std::string& prefix, int fallBack) {
    int result = fallBack;
    size_t pos = s.find(prefix);
    if (pos != std::string::npos) {
        sscanf(s.substr(pos).c_str(), (prefix + "%i").c_str(), &result);
    }
    return result;
}

time_t
NIImporter_DlrNavteq::readTimeRec(const std::string& start, const std::string& duration) {
    // http://www.cplusplus.com/reference/ctime/mktime/
    struct tm timeinfo;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_year = 0;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 1;
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_isdst = 0;

    timeinfo.tm_year = readPrefixedInt(start, "y") + readPrefixedInt(duration, "y") - 1900;
    timeinfo.tm_mon = readPrefixedInt(start, "M") + readPrefixedInt(duration, "M") - 1;
    timeinfo.tm_mday = 7 * (readPrefixedInt(start, "w") + readPrefixedInt(duration, "w"));
    timeinfo.tm_mday += readPrefixedInt(start, "d") + readPrefixedInt(duration, "d");

    time_t result =  mktime(&timeinfo);
    return result;
}


time_t
NIImporter_DlrNavteq::readDate(const std::string& yyyymmdd) {
    struct tm timeinfo;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_wday = 0;
    timeinfo.tm_yday = 0;
    timeinfo.tm_isdst = 0;

    if (yyyymmdd.size() == 10
            && yyyymmdd[4] == '-'
            && yyyymmdd[7] == '-') {
        try {
            timeinfo.tm_year = StringUtils::toInt(yyyymmdd.substr(0, 4)) - 1900;
            timeinfo.tm_mon = StringUtils::toInt(yyyymmdd.substr(5, 2)) - 1;
            timeinfo.tm_mday = StringUtils::toInt(yyyymmdd.substr(8, 2));
            return mktime(&timeinfo);
        } catch (...) {
        }
    }
    WRITE_ERROR("Could not parse YYYY-MM-DD date '" + yyyymmdd + "'");
    time_t now;
    time(&now);
    return now;
}

// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::ProhibitionHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::ProhibitionHandler::ProhibitionHandler(
    NBEdgeCont& ec, const std::string& file, time_t constructionTime) :
    myEdgeCont(ec),
    myFile(file),
    myVersion(0),
    myConstructionTime(constructionTime) {
}


NIImporter_DlrNavteq::ProhibitionHandler::~ProhibitionHandler() {}


bool
NIImporter_DlrNavteq::ProhibitionHandler::report(const std::string& result) {
// # NAME_ID    Name
    if (result[0] == '#') {
        if (myVersion == 0) {
            const double version = readVersion(result, myFile);
            if (version > 0) {
                myVersion = version;
            }
        }
        return true;
    }
    StringTokenizer st(result, StringTokenizer::TAB);
    if (st.size() == 1) {
        return true; // one line with the number of data containing lines in it (also starts with a comment # since ersion 6.5)
    }
    if (myVersion >= 6) {
        assert(st.size() >= 7);
        const std::string id = st.next();
        const std::string permanent = st.next();
        const std::string validityPeriod = st.next();
        const std::string throughTraffic = st.next();
        const std::string vehicleType = st.next();
        if (validityPeriod != UNDEFINED) {
            WRITE_WARNINGF("Ignoring temporary prohibited manoeuvre (%).", validityPeriod);
            return true;
        }
    }
    const std::string startEdge = st.next();
    const std::string endEdge = st.get(st.size() - 1);

    NBEdge* from = myEdgeCont.retrieve(startEdge);
    if (from == nullptr) {
        WRITE_WARNINGF("Ignoring prohibition from unknown start edge '%'.", startEdge);
        return true;
    }
    NBEdge* to = myEdgeCont.retrieve(endEdge);
    if (to == nullptr) {
        WRITE_WARNINGF("Ignoring prohibition from unknown end edge '%'.", endEdge);
        return true;
    }
    from->removeFromConnections(to, -1, -1, true);
    return true;
}


// ---------------------------------------------------------------------------
// definitions of NIImporter_DlrNavteq::ConnectedLanesHandler-methods
// ---------------------------------------------------------------------------
NIImporter_DlrNavteq::ConnectedLanesHandler::ConnectedLanesHandler(
    NBEdgeCont& ec) :
    myEdgeCont(ec) {
}


NIImporter_DlrNavteq::ConnectedLanesHandler::~ConnectedLanesHandler() {}


bool
NIImporter_DlrNavteq::ConnectedLanesHandler::report(const std::string& result) {
    if (result[0] == '#') {
        return true;
    }
    StringTokenizer st(result, StringTokenizer::TAB);
    if (st.size() == 1) {
        return true; // one line with the number of data containing lines in it (also starts with a comment # since ersion 6.5)
    }
    assert(st.size() >= 7);
    const std::string nodeID = st.next();
    const std::string vehicleType = st.next();
    const std::string fromLaneS = st.next();
    const std::string toLaneS = st.next();
    const std::string throughTraffic = st.next();
    const std::string startEdge = st.next();
    const std::string endEdge = st.get(st.size() - 1);

    NBEdge* from = myEdgeCont.retrieve(startEdge);
    if (from == nullptr) {
        WRITE_WARNINGF("Ignoring prohibition from unknown start edge '%'.", startEdge);
        return true;
    }
    NBEdge* to = myEdgeCont.retrieve(endEdge);
    if (to == nullptr) {
        WRITE_WARNINGF("Ignoring prohibition from unknown end edge '%'.", endEdge);
        return true;
    }
    int fromLane = StringUtils::toInt(fromLaneS) - 1; // one based
    if (fromLane < 0 || fromLane >= from->getNumLanes()) {
        WRITE_WARNINGF("Ignoring invalid lane index '%' in connection from edge '%' with % lanes.", fromLaneS, startEdge, from->getNumLanes());
        return true;
    }
    int toLane = StringUtils::toInt(toLaneS) - 1; // one based
    if (toLane < 0 || toLane >= to->getNumLanes()) {
        WRITE_WARNINGF("Ignoring invalid lane index '%' in connection to edge '%' with % lanes", toLaneS, endEdge, to->getNumLanes());
        return true;
    }
    if (!from->addLane2LaneConnection(fromLane, to, toLane, NBEdge::Lane2LaneInfoType::USER, true)) {
        if (OptionsCont::getOptions().getBool("show-errors.connections-first-try")) {
            WRITE_WARNINGF("Could not set loaded connection from '%' to '%'.", from->getLaneID(fromLane), to->getLaneID(toLane));
        }
        // set as to be re-applied after network processing
        // if this connection runs across a node cluster it may not be possible to set this
        const bool warnOnly = st.size() > 7;
        myEdgeCont.addPostProcessConnection(from->getID(), fromLane, to->getID(), toLane, false, true,
                                            NBEdge::UNSPECIFIED_CONTPOS, NBEdge::UNSPECIFIED_VISIBILITY_DISTANCE,
                                            NBEdge::UNSPECIFIED_SPEED, NBEdge::UNSPECIFIED_LOADED_LENGTH, PositionVector::EMPTY, false, warnOnly);
    }
    // ensure that connections for other lanes are guessed if not specified
    from->declareConnectionsAsLoaded(NBEdge::EdgeBuildingStep::INIT);
    from->getLaneStruct(fromLane).connectionsDone = true;
    return true;
}


/****************************************************************************/
