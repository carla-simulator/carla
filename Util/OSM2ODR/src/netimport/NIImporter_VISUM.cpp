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
/// @file    NIImporter_VISUM.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @date    Fri, 19 Jul 2002
///
// A VISUM network importer
/****************************************************************************/
#include <config.h>

#include <string>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/ToString.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/geom/GeoConvHelper.h>
#include <netbuild/NBDistrict.h>

#include <netbuild/NBNetBuilder.h>
#include "NILoader.h"
#include "NIImporter_VISUM.h"


StringBijection<NIImporter_VISUM::VISUM_KEY>::Entry NIImporter_VISUM::KEYS_DE[] = {
    { "VSYS", VISUM_SYS },
    { "STRECKENTYP", VISUM_LINKTYPE },
    { "KNOTEN", VISUM_NODE },
    { "BEZIRK", VISUM_DISTRICT },
    { "PUNKT", VISUM_POINT },
    { "STRECKE", VISUM_LINK },
    { "V0IV", VISUM_V0 },
    { "VSYSSET", VISUM_TYPES },
    { "RANG", VISUM_RANK },
    { "KAPIV", VISUM_CAPACITY },
    { "XKOORD", VISUM_XCOORD },
    { "YKOORD", VISUM_YCOORD },
    { "ID", VISUM_ID },
    { "CODE", VISUM_CODE },
    { "VONKNOTNR", VISUM_FROMNODE },
    { "NACHKNOTNR", VISUM_TONODE },
    { "TYPNR", VISUM_TYPE },
    { "TYP", VISUM_TYP },
    { "ANBINDUNG", VISUM_DISTRICT_CONNECTION },
    { "BEZNR", VISUM_SOURCE_DISTRICT },
    { "KNOTNR",  VISUM_FROMNODENO },
    { "RICHTUNG",  VISUM_DIRECTION },
    { "FLAECHEID",  VISUM_SURFACEID },
    { "TFLAECHEID",  VISUM_FACEID },
    { "VONPUNKTID",  VISUM_FROMPOINTID },
    { "NACHPUNKTID",  VISUM_TOPOINTID },
    { "KANTE",  VISUM_EDGE },
    { "ABBIEGER",  VISUM_TURN },
    { "UEBERKNOTNR",  VISUM_VIANODENO },
    { "ANZFAHRSTREIFEN",  VISUM_NUMLANES },
    { "INDEX",  VISUM_INDEX },
    { "STRECKENPOLY",  VISUM_LINKPOLY },
    { "FLAECHENELEMENT",  VISUM_SURFACEITEM },
    { "TEILFLAECHENELEMENT",  VISUM_FACEITEM },
    { "KANTEID",  VISUM_EDGEID },
    { "Q",  VISUM_ORIGIN },
    { "Z",  VISUM_DESTINATION },
    { "NR", VISUM_NO } // must be the last one
};



StringBijection<NIImporter_VISUM::VISUM_KEY> NIImporter_VISUM::KEYS(NIImporter_VISUM::KEYS_DE, VISUM_NO);

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods (interface in this case)
// ---------------------------------------------------------------------------
void
NIImporter_VISUM::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    // check whether the option is set (properly)
    if (!oc.isSet("visum-file")) {
        return;
    }
    // build the handler
    NIImporter_VISUM loader(nb, oc.getString("visum-file"),
                            NBCapacity2Lanes(oc.getFloat("lanes-from-capacity.norm")),
                            oc.getBool("visum.use-type-priority"),
                            oc.getString("visum.language-file"));
    loader.load();
}



// ---------------------------------------------------------------------------
// loader methods
// ---------------------------------------------------------------------------
NIImporter_VISUM::NIImporter_VISUM(NBNetBuilder& nb,
                                   const std::string& file,
                                   NBCapacity2Lanes capacity2Lanes,
                                   bool useVisumPrio,
                                   const std::string& languageFile) :
    myNetBuilder(nb), myFileName(file),
    myCapacity2Lanes(capacity2Lanes), myUseVisumPrio(useVisumPrio) {
    if (languageFile != "") {
        loadLanguage(languageFile);
    }

    // the order of process is important!
    // set1
    addParser(KEYS.getString(VISUM_SYS), &NIImporter_VISUM::parse_VSysTypes);
    addParser(KEYS.getString(VISUM_LINKTYPE), &NIImporter_VISUM::parse_Types);
    addParser(KEYS.getString(VISUM_NODE), &NIImporter_VISUM::parse_Nodes);
    addParser(KEYS.getString(VISUM_DISTRICT), &NIImporter_VISUM::parse_Districts);
    addParser(KEYS.getString(VISUM_POINT), &NIImporter_VISUM::parse_Point);

    // set2
    // two types of "strecke"
    addParser(KEYS.getString(VISUM_LINK), &NIImporter_VISUM::parse_Edges);
    addParser(KEYS.getString(VISUM_EDGE), &NIImporter_VISUM::parse_Kante);

    // set3
    if (OptionsCont::getOptions().getBool("visum.no-connectors")) {
        addParser(KEYS.getString(VISUM_DISTRICT_CONNECTION), &NIImporter_VISUM::parse_Connectors);
    } else {
        addParser(KEYS.getString(VISUM_DISTRICT_CONNECTION), &NIImporter_VISUM::parse_Connectors_legacy);
    }
    // two types of "abbieger"
    addParser("ABBIEGEBEZIEHUNG", &NIImporter_VISUM::parse_Turns);
    addParser(KEYS.getString(VISUM_TURN), &NIImporter_VISUM::parse_Turns);

    addParser(KEYS.getString(VISUM_LINKPOLY), &NIImporter_VISUM::parse_EdgePolys);
    addParser("FAHRSTREIFEN", &NIImporter_VISUM::parse_Lanes);
    addParser(KEYS.getString(VISUM_SURFACEITEM), &NIImporter_VISUM::parse_PartOfArea);


    // set4
    // two types of lsa
    addParser("LSA", &NIImporter_VISUM::parse_TrafficLights);
    addParser("SIGNALANLAGE", &NIImporter_VISUM::parse_TrafficLights);
    // two types of knotenzulsa
    addParser("KNOTENZULSA", &NIImporter_VISUM::parse_NodesToTrafficLights);
    addParser("LSAZUKNOTEN", &NIImporter_VISUM::parse_NodesToTrafficLights);
    addParser("SIGNALANLAGEZUKNOTEN", &NIImporter_VISUM::parse_NodesToTrafficLights);
    // two types of signalgruppe
    addParser("LSASIGNALGRUPPE", &NIImporter_VISUM::parse_SignalGroups);
    addParser("SIGNALGRUPPE", &NIImporter_VISUM::parse_SignalGroups);
    // three types of ABBZULSASIGNALGRUPPE
    addParser("ABBZULSASIGNALGRUPPE", &NIImporter_VISUM::parse_TurnsToSignalGroups);
    addParser("SIGNALGRUPPEZUABBIEGER", &NIImporter_VISUM::parse_TurnsToSignalGroups);
    addParser("SIGNALGRUPPEZUFSABBIEGER", &NIImporter_VISUM::parse_TurnsToSignalGroups);

    addParser(KEYS.getString(VISUM_FACEITEM), &NIImporter_VISUM::parse_AreaSubPartElement);

    // two types of LSAPHASE
    addParser("LSAPHASE", &NIImporter_VISUM::parse_Phases);
    addParser("PHASE", &NIImporter_VISUM::parse_Phases);

    addParser("LSASIGNALGRUPPEZULSAPHASE", &NIImporter_VISUM::parse_SignalGroupsToPhases);
    addParser("FAHRSTREIFENABBIEGER", &NIImporter_VISUM::parse_LanesConnections);
}


NIImporter_VISUM::~NIImporter_VISUM() {
    for (NIVisumTL_Map::iterator j = myTLS.begin(); j != myTLS.end(); j++) {
        delete j->second;
    }
}


void
NIImporter_VISUM::addParser(const std::string& name, ParsingFunction function) {
    TypeParser p;
    p.name = name;
    p.function = function;
    p.position = -1;
    mySingleDataParsers.push_back(p);
}


void
NIImporter_VISUM::load() {
    // open the file
    if (!myLineReader.setFile(myFileName)) {
        throw ProcessError("Can not open visum-file '" + myFileName + "'.");
    }
    // scan the file for data positions
    while (myLineReader.hasMore()) {
        std::string line = myLineReader.readLine();
        if (line.length() > 0 && line[0] == '$') {
            ParserVector::iterator i;
            for (i = mySingleDataParsers.begin(); i != mySingleDataParsers.end(); i++) {
                std::string dataName = "$" + (*i).name + ":";
                if (line.substr(0, dataName.length()) == dataName) {
                    (*i).position = myLineReader.getPosition();
                    (*i).pattern = line.substr(dataName.length());
                    WRITE_MESSAGE("Found: " + dataName + " at " + toString<int>(myLineReader.getPosition()));
                }
            }
        }
    }
    // go through the parsers and process all entries
    for (ParserVector::iterator i = mySingleDataParsers.begin(); i != mySingleDataParsers.end(); i++) {
        if ((*i).position < 0) {
            // do not process using parsers for which no information was found
            continue;
        }
        // ok, the according information is stored in the file
        PROGRESS_BEGIN_MESSAGE("Parsing " + (*i).name);
        // reset the line reader and let it point to the begin of the according data field
        myLineReader.reinit();
        myLineReader.setPos((*i).position);
        // prepare the line parser
        myLineParser.reinit((*i).pattern);
        // read
        bool singleDataEndFound = false;
        while (myLineReader.hasMore() && !singleDataEndFound) {
            std::string line = myLineReader.readLine();
            if (line.length() == 0 || line[0] == '*' || line[0] == '$') {
                singleDataEndFound = true;
            } else {
                myLineParser.parseLine(line);
                try {
                    myCurrentID = "<unknown>";
                    (this->*(*i).function)();
                } catch (OutOfBoundsException&) {
                    WRITE_ERROR("Too short value line in " + (*i).name + " occurred.");
                } catch (NumberFormatException&) {
                    WRITE_ERROR("A value in " + (*i).name + " should be numeric but is not (id='" + myCurrentID + "').");
                } catch (UnknownElement& e) {
                    WRITE_ERROR("One of the needed values ('" + std::string(e.what()) + "') is missing in " + (*i).name + ".");
                }
            }
        }
        // close single reader processing
        PROGRESS_DONE_MESSAGE();
    }
    myNetBuilder.getEdgeCont().reduceGeometries(POSITION_EPS);

    // build traffic lights
    for (NIVisumTL_Map::iterator j = myTLS.begin(); j != myTLS.end(); j++) {
        j->second->build(myNetBuilder.getEdgeCont(), myNetBuilder.getTLLogicCont());
    }
    // build district shapes
    for (std::map<NBDistrict*, PositionVector>::const_iterator k = myDistrictShapes.begin(); k != myDistrictShapes.end(); ++k) {
        (*k).first->addShape((*k).second);
    }
}





void
NIImporter_VISUM::parse_VSysTypes() {
    std::string name = myLineParser.know("VSysCode") ? myLineParser.get("VSysCode").c_str() : myLineParser.get(KEYS.getString(VISUM_CODE)).c_str();
    std::string type = myLineParser.know("VSysMode") ? myLineParser.get("VSysMode").c_str() : myLineParser.get(KEYS.getString(VISUM_TYP)).c_str();
    myVSysTypes[name] = type;
}


void
NIImporter_VISUM::parse_Types() {
    // get the id
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    // get the maximum speed
    double speed = getWeightedFloat2("v0-IV", KEYS.getString(VISUM_V0), "km/h");
    if (speed == 0) {
        // unlimited speed
        speed = 3600;
    } else if (speed < 0) {
        WRITE_ERROR("Type '" + myCurrentID + "' has speed " + toString(speed));
    }
    // get the permissions
    SVCPermissions permissions = getPermissions(KEYS.getString(VISUM_TYPES), true);
    // get the priority
    const int priority = 1000 - StringUtils::toInt(myLineParser.get(KEYS.getString(VISUM_RANK)));
    // try to retrieve the number of lanes
    const int numLanes = myCapacity2Lanes.get(getNamedFloat("Kap-IV", KEYS.getString(VISUM_CAPACITY)));
    // insert the type
    myNetBuilder.getTypeCont().insert(myCurrentID, numLanes, speed / (double) 3.6, priority, permissions, NBEdge::UNSPECIFIED_WIDTH, false, NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_WIDTH, 0, 0, 0);
    myNetBuilder.getTypeCont().markAsSet(myCurrentID, SUMO_ATTR_NUMLANES);
    myNetBuilder.getTypeCont().markAsSet(myCurrentID, SUMO_ATTR_SPEED);
    myNetBuilder.getTypeCont().markAsSet(myCurrentID, SUMO_ATTR_PRIORITY);
    myNetBuilder.getTypeCont().markAsSet(myCurrentID, SUMO_ATTR_ONEWAY);
    myNetBuilder.getTypeCont().markAsSet(myCurrentID, SUMO_ATTR_ALLOW);
}


void
NIImporter_VISUM::parse_Nodes() {
    // get the id
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    // get the position
    double x = getNamedFloat(KEYS.getString(VISUM_XCOORD));
    double y = getNamedFloat(KEYS.getString(VISUM_YCOORD));
    Position pos(x, y);
    if (!NBNetBuilder::transformCoordinate(pos)) {
        WRITE_ERROR("Unable to project coordinates for node " + myCurrentID + ".");
        return;
    }
    // add to the list
    if (!myNetBuilder.getNodeCont().insert(myCurrentID, pos)) {
        WRITE_ERROR("Duplicate node occurred ('" + myCurrentID + "').");
    }
}


void
NIImporter_VISUM::parse_Districts() {
    // get the id
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    // get the information whether the source and the destination
    //  connections are weighted
    //bool sourcesWeighted = getWeightedBool("Proz_Q");
    //bool destWeighted = getWeightedBool("Proz_Z");
    // get the node information
    double x = getNamedFloat(KEYS.getString(VISUM_XCOORD));
    double y = getNamedFloat(KEYS.getString(VISUM_YCOORD));
    Position pos(x, y);
    if (!NBNetBuilder::transformCoordinate(pos, false)) {
        WRITE_ERROR("Unable to project coordinates for district " + myCurrentID + ".");
        return;
    }
    // build the district
    NBDistrict* district = new NBDistrict(myCurrentID, pos);
    if (!myNetBuilder.getDistrictCont().insert(district)) {
        WRITE_ERROR("Duplicate district occurred ('" + myCurrentID + "').");
        delete district;
        return;
    }
    if (myLineParser.know(KEYS.getString(VISUM_SURFACEID))) {
        long long int flaecheID = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_SURFACEID)));
        myShapeDistrictMap[flaecheID] = district;
    }
}


void
NIImporter_VISUM::parse_Point() {
    long long int id = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_ID)));
    double x = StringUtils::toDouble(myLineParser.get(KEYS.getString(VISUM_XCOORD)));
    double y = StringUtils::toDouble(myLineParser.get(KEYS.getString(VISUM_YCOORD)));
    Position pos(x, y);
    if (!NBNetBuilder::transformCoordinate(pos, false)) {
        WRITE_ERROR("Unable to project coordinates for point " + toString(id) + ".");
        return;
    }
    myPoints[id] = pos;
}


void
NIImporter_VISUM::parse_Edges() {
    if (myLineParser.know(KEYS.getString(VISUM_TYPES)) && myLineParser.get(KEYS.getString(VISUM_TYPES)) == "") {
        // no vehicle allowed; don't add
        return;
    }
    // get the id
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    // get the from- & to-node and validate them
    NBNode* from = getNamedNode("VonKnot", KEYS.getString(VISUM_FROMNODE));
    NBNode* to = getNamedNode("NachKnot", KEYS.getString(VISUM_TONODE));
    if (!checkNodes(from, to)) {
        return;
    }
    // get the type
    std::string type = myLineParser.know(KEYS.getString(VISUM_TYP)) ? myLineParser.get(KEYS.getString(VISUM_TYP)) : myLineParser.get(KEYS.getString(VISUM_TYPE));
    // get the speed
    double speed = myNetBuilder.getTypeCont().getSpeed(type);
    if (!OptionsCont::getOptions().getBool("visum.use-type-speed")) {
        try {
            std::string speedS = myLineParser.know("v0-IV") ? myLineParser.get("v0-IV") : myLineParser.get(KEYS.getString(VISUM_V0));
            if (speedS.find("km/h") != std::string::npos) {
                speedS = speedS.substr(0, speedS.find("km/h"));
            }
            speed = StringUtils::toDouble(speedS) / 3.6;
        } catch (OutOfBoundsException&) {}
    }
    if (speed <= 0) {
        speed = myNetBuilder.getTypeCont().getSpeed(type);
    }

    // get the information whether the edge is a one-way
    bool oneway = myLineParser.know("Einbahn")
                  ? StringUtils::toBool(myLineParser.get("Einbahn"))
                  : true;
    // get the number of lanes
    int nolanes = myNetBuilder.getTypeCont().getNumLanes(type);
    if (!OptionsCont::getOptions().getBool("visum.recompute-lane-number")) {
        if (!OptionsCont::getOptions().getBool("visum.use-type-laneno")) {
            if (myLineParser.know("Fahrstreifen")) {
                nolanes = StringUtils::toInt(myLineParser.get("Fahrstreifen"));
            } else if (myLineParser.know(KEYS.getString(VISUM_NUMLANES))) {
                nolanes = StringUtils::toInt(myLineParser.get(KEYS.getString(VISUM_NUMLANES)));
            }
        }
    } else {
        if (myLineParser.know(KEYS.getString(VISUM_CAPACITY))) {
            nolanes = myCapacity2Lanes.get(StringUtils::toDouble(myLineParser.get(KEYS.getString(VISUM_CAPACITY))));
        } else if (myLineParser.know("KAP-IV")) {
            nolanes = myCapacity2Lanes.get(StringUtils::toDouble(myLineParser.get("KAP-IV")));
        }
    }
    // check whether the id is already used
    //  (should be the opposite direction)
    bool oneway_checked = oneway;
    NBEdge* previous = myNetBuilder.getEdgeCont().retrieve(myCurrentID);
    if (previous != nullptr) {
        myCurrentID = '-' + myCurrentID;
        previous->setLaneSpreadFunction(LaneSpreadFunction::RIGHT);
        oneway_checked = false;
    }
    if (find(myTouchedEdges.begin(), myTouchedEdges.end(), myCurrentID) != myTouchedEdges.end()) {
        oneway_checked = false;
    }
    std::string tmpid = '-' + myCurrentID;
    if (find(myTouchedEdges.begin(), myTouchedEdges.end(), tmpid) != myTouchedEdges.end()) {
        previous = myNetBuilder.getEdgeCont().retrieve(tmpid);
        if (previous != nullptr) {
            previous->setLaneSpreadFunction(LaneSpreadFunction::RIGHT);
        }
        oneway_checked = false;
    }
    // add the edge
    const SVCPermissions permissions = getPermissions(KEYS.getString(VISUM_TYPES), false, myNetBuilder.getTypeCont().getPermissions(type));
    int prio = myUseVisumPrio ? myNetBuilder.getTypeCont().getPriority(type) : -1;
    if (nolanes != 0 && speed != 0) {
        LaneSpreadFunction lsf = oneway_checked ? LaneSpreadFunction::CENTER : LaneSpreadFunction::RIGHT;
        // @todo parse name from visum files
        NBEdge* e = new NBEdge(myCurrentID, from, to, type, speed, nolanes, prio,
                               NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, "", lsf);
        e->setPermissions(permissions);
        if (!myNetBuilder.getEdgeCont().insert(e)) {
            delete e;
            WRITE_ERROR("Duplicate edge occurred ('" + myCurrentID + "').");
        }
    }
    myTouchedEdges.push_back(myCurrentID);
    // nothing more to do, when the edge is a one-way street
    if (oneway) {
        return;
    }
    // add the opposite edge
    myCurrentID = '-' + myCurrentID;
    if (nolanes != 0 && speed != 0) {
        LaneSpreadFunction lsf = oneway_checked ? LaneSpreadFunction::CENTER : LaneSpreadFunction::RIGHT;
        // @todo parse name from visum files
        NBEdge* e = new NBEdge(myCurrentID, from, to, type, speed, nolanes, prio,
                               NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET, "", lsf);
        e->setPermissions(permissions);
        if (!myNetBuilder.getEdgeCont().insert(e)) {
            delete e;
            WRITE_ERROR("Duplicate edge occurred ('" + myCurrentID + "').");
        }
    }
    myTouchedEdges.push_back(myCurrentID);
}


void
NIImporter_VISUM::parse_Kante() {
    long long int id = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_ID)));
    long long int from = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_FROMPOINTID)));
    long long int to = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_TOPOINTID)));
    myEdges[id] = std::make_pair(from, to);
}


void
NIImporter_VISUM::parse_PartOfArea() {
    long long int flaecheID = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_SURFACEID)));
    long long int flaechePartID = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_FACEID)));
    if (mySubPartsAreas.find(flaechePartID) == mySubPartsAreas.end()) {
        mySubPartsAreas[flaechePartID] = std::vector<long long int>();
    }
    mySubPartsAreas[flaechePartID].push_back(flaecheID);
}


void
NIImporter_VISUM::parse_Connectors() {
    // get the source district
    std::string bez = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_SOURCE_DISTRICT)));
    // get the destination node
    NBNode* dest = getNamedNode(KEYS.getString(VISUM_FROMNODENO));
    if (dest == nullptr) {
        return;
    }
    // get the weight of the connection
    double proz = 1;
    if (myLineParser.know("Proz") || myLineParser.know("Proz(IV)")) {
        proz = getNamedFloat("Proz", "Proz(IV)") / 100;
    }
    // get the information whether this is a sink or a source
    std::string dir = myLineParser.get(KEYS.getString(VISUM_DIRECTION));
    if (dir.length() == 0) {
        dir = KEYS.getString(VISUM_ORIGIN) + KEYS.getString(VISUM_DESTINATION);
    }
    // build the source when needed
    if (dir.find(KEYS.getString(VISUM_ORIGIN)) != std::string::npos) {
        for (NBEdge* edge : dest->getOutgoingEdges()) {
            myNetBuilder.getDistrictCont().addSource(bez, edge, proz);
        }
    }
    // build the sink when needed
    if (dir.find(KEYS.getString(VISUM_DESTINATION)) != std::string::npos) {
        for (NBEdge* edge : dest->getIncomingEdges()) {
            myNetBuilder.getDistrictCont().addSink(bez, edge, proz);
        }
    }
}



void
NIImporter_VISUM::parse_Connectors_legacy() {
    // get the source district
    std::string bez = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_SOURCE_DISTRICT)));
    // get the destination node
    NBNode* dest = getNamedNode(KEYS.getString(VISUM_FROMNODENO));
    if (dest == nullptr) {
        return;
    }
    // get the weight of the connection
    double proz = 1;
    if (myLineParser.know("Proz") || myLineParser.know("Proz(IV)")) {
        proz = getNamedFloat("Proz", "Proz(IV)") / 100;
    }
    // get the duration to wait (unused)
//     double retard = -1;
//     if (myLineParser.know("t0-IV")) {
//         retard = getNamedFloat("t0-IV", -1);
//     }
    // get the type;
    //  use a standard type with a large speed when a type is not given

    std::string type = myLineParser.know(KEYS.getString(VISUM_TYP))
                       ? NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_TYP)))
                       : "";
    // add the connectors as an edge
    std::string id = bez + "-" + dest->getID();
    // get the information whether this is a sink or a source
    std::string dir = myLineParser.get(KEYS.getString(VISUM_DIRECTION));
    if (dir.length() == 0) {
        dir = KEYS.getString(VISUM_ORIGIN) + KEYS.getString(VISUM_DESTINATION);
    }
    // build the source when needed
    if (dir.find(KEYS.getString(VISUM_ORIGIN)) != std::string::npos) {
        const EdgeVector& edges = dest->getOutgoingEdges();
        bool hasContinuation = false;
        for (EdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            if (!(*i)->isMacroscopicConnector()) {
                hasContinuation = true;
            }
        }
        if (!hasContinuation) {
            // obviously, there is no continuation on the net
            WRITE_WARNING("Incoming connector '" + id + "' will not be build - would be not connected to network.");
        } else {
            NBNode* src = buildDistrictNode(bez, dest, true);
            if (src == nullptr) {
                WRITE_ERROR("The district '" + bez + "' could not be built.");
                return;
            }
            NBEdge* edge = new NBEdge(id, src, dest, "VisumConnector",
                                      OptionsCont::getOptions().getFloat("visum.connector-speeds"),
                                      OptionsCont::getOptions().getInt("visum.connectors-lane-number"),
                                      -1, NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET,
                                      "", LaneSpreadFunction::RIGHT);
            edge->setAsMacroscopicConnector();
            if (!myNetBuilder.getEdgeCont().insert(edge)) {
                WRITE_ERROR("A duplicate edge id occurred (ID='" + id + "').");
                return;
            }
            edge = myNetBuilder.getEdgeCont().retrieve(id);
            if (edge != nullptr) {
                myNetBuilder.getDistrictCont().addSource(bez, edge, proz);
            }
        }
    }
    // build the sink when needed
    if (dir.find(KEYS.getString(VISUM_DESTINATION)) != std::string::npos) {
        const EdgeVector& edges = dest->getIncomingEdges();
        bool hasPredeccessor = false;
        for (EdgeVector::const_iterator i = edges.begin(); i != edges.end(); ++i) {
            if (!(*i)->isMacroscopicConnector()) {
                hasPredeccessor = true;
            }
        }
        if (!hasPredeccessor) {
            // obviously, the network is not connected to this node
            WRITE_WARNING("Outgoing connector '" + id + "' will not be build - would be not connected to network.");
        } else {
            NBNode* src = buildDistrictNode(bez, dest, false);
            if (src == nullptr) {
                WRITE_ERROR("The district '" + bez + "' could not be built.");
                return;
            }
            id = "-" + id;
            NBEdge* edge = new NBEdge(id, dest, src, "VisumConnector",
                                      OptionsCont::getOptions().getFloat("visum.connector-speeds"),
                                      OptionsCont::getOptions().getInt("visum.connectors-lane-number"),
                                      -1, NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET,
                                      "", LaneSpreadFunction::RIGHT);
            edge->setAsMacroscopicConnector();
            if (!myNetBuilder.getEdgeCont().insert(edge)) {
                WRITE_ERROR("A duplicate edge id occurred (ID='" + id + "').");
                return;
            }
            edge = myNetBuilder.getEdgeCont().retrieve(id);
            if (edge != nullptr) {
                myNetBuilder.getDistrictCont().addSink(bez, edge, proz);
            }
        }
    }
}


void
NIImporter_VISUM::parse_Turns() {
    if (myLineParser.know(KEYS.getString(VISUM_TYPES)) && myLineParser.get(KEYS.getString(VISUM_TYPES)) == "") {
        // no vehicle allowed; don't add
        return;
    }
    // retrieve the nodes
    NBNode* from = getNamedNode("VonKnot", KEYS.getString(VISUM_FROMNODE));
    NBNode* via = getNamedNode("UeberKnot", KEYS.getString(VISUM_VIANODENO));
    NBNode* to = getNamedNode("NachKnot", KEYS.getString(VISUM_TONODE));
    if (from == nullptr || via == nullptr || to == nullptr) {
        return;
    }
    // all nodes are known
    std::string type = myLineParser.know("VSysCode")
                       ? myLineParser.get("VSysCode")
                       : myLineParser.get(KEYS.getString(VISUM_TYPES));
    if (myVSysTypes.find(type) != myVSysTypes.end() && myVSysTypes.find(type)->second == "IV") {
        // try to set the turning definition
        NBEdge* src = from->getConnectionTo(via);
        NBEdge* dest = via->getConnectionTo(to);
        // check both
        if (src == nullptr) {
            if (OptionsCont::getOptions().getBool("visum.verbose-warnings")) {
                WRITE_WARNING("There is no edge from node '" + from->getID() + "' to node '" + via->getID() + "'.");
            }
            return;
        }
        if (dest == nullptr) {
            if (OptionsCont::getOptions().getBool("visum.verbose-warnings")) {
                WRITE_WARNING("There is no edge from node '" + via->getID() + "' to node '" + to->getID() + "'.");
            }
            return;
        }
        // both edges found
        //  set them into the edge
        src->addEdge2EdgeConnection(dest);
    }
}


void
NIImporter_VISUM::parse_EdgePolys() {
    // get the from- & to-node and validate them
    NBNode* from = getNamedNode("VonKnot", KEYS.getString(VISUM_FROMNODE));
    NBNode* to = getNamedNode("NachKnot", KEYS.getString(VISUM_TONODE));
    if (!checkNodes(from, to)) {
        return;
    }
    bool failed = false;
    int index;
    double x, y;
    try {
        index = StringUtils::toInt(myLineParser.get(KEYS.getString(VISUM_INDEX)));
        x = getNamedFloat(KEYS.getString(VISUM_XCOORD));
        y = getNamedFloat(KEYS.getString(VISUM_YCOORD));
    } catch (NumberFormatException&) {
        WRITE_ERROR("Error in geometry description from node '" + from->getID() + "' to node '" + to->getID() + "'.");
        return;
    }
    Position pos(x, y);
    if (!NBNetBuilder::transformCoordinate(pos)) {
        WRITE_ERROR("Unable to project coordinates for node '" + from->getID() + "'.");
        return;
    }
    NBEdge* e = from->getConnectionTo(to);
    if (e != nullptr) {
        e->addGeometryPoint(index, pos);
    } else {
        failed = true;
    }
    e = to->getConnectionTo(from);
    if (e != nullptr) {
        e->addGeometryPoint(-index, pos);
        failed = false;
    }
    // check whether the operation has failed
    if (failed) {
        if (OptionsCont::getOptions().getBool("visum.verbose-warnings")) {
            WRITE_WARNING("There is no edge from node '" + from->getID() + "' to node '" + to->getID() + "'.");
        }
    }
}


void
NIImporter_VISUM::parse_Lanes() {
    // The base number of lanes for the edge was already defined in STRECKE
    // this refines lane specific attribute (width) and optionally introduces splits for additional lanes
    // It is permitted for KNOTNR to be 0
    //
    // get the edge
    NBEdge* baseEdge = getNamedEdge("STRNR");
    if (baseEdge == nullptr) {
        return;
    }
    NBEdge* edge = baseEdge;
    // get the node
    NBNode* node = getNamedNodeSecure("KNOTNR");
    if (node == nullptr) {
        node = edge->getToNode();
    } else {
        edge = getNamedEdgeContinuating("STRNR", node);
    }
    // check
    if (edge == nullptr) {
        return;
    }
    // get the lane
    std::string laneS = myLineParser.know("FSNR")
                        ? NBHelpers::normalIDRepresentation(myLineParser.get("FSNR"))
                        : NBHelpers::normalIDRepresentation(myLineParser.get("NR"));
    int lane = -1;
    try {
        lane = StringUtils::toInt(laneS);
    } catch (NumberFormatException&) {
        WRITE_ERROR("A lane number for edge '" + edge->getID() + "' is not numeric (" + laneS + ").");
        return;
    }
    lane -= 1;
    if (lane < 0) {
        WRITE_ERROR("A lane number for edge '" + edge->getID() + "' is not positive (" + laneS + ").");
        return;
    }
    // get the direction
    std::string dirS = NBHelpers::normalIDRepresentation(myLineParser.get("RICHTTYP"));
    int prevLaneNo = baseEdge->getNumLanes();
    if ((dirS == "1" && !(node->hasIncoming(edge))) || (dirS == "0" && !(node->hasOutgoing(edge)))) {
        // get the last part of the turnaround direction
        NBEdge* cand = getReversedContinuating(edge, node);
        if (cand) {
            edge = cand;
        }
    }
    // get the length
    std::string lengthS = NBHelpers::normalIDRepresentation(myLineParser.get("LAENGE"));
    double length = -1;
    try {
        length = StringUtils::toDouble(lengthS);
    } catch (NumberFormatException&) {
        WRITE_ERROR("A lane length for edge '" + edge->getID() + "' is not numeric (" + lengthS + ").");
        return;
    }
    if (length < 0) {
        WRITE_ERROR("A lane length for edge '" + edge->getID() + "' is not positive (" + lengthS + ").");
        return;
    }
    //
    if (dirS == "1") {
        lane -= prevLaneNo;
    }
    //
    if (length == 0) {
        if ((int) edge->getNumLanes() > lane) {
            // ok, we know this already...
            return;
        }
        // increment by one
        edge->incLaneNo(1);
    } else {
        // check whether this edge already has been created
        if (isSplitEdge(edge, node)) {
            if (edge->getID().substr(edge->getID().find('_')) == "_" + toString(length) + "_" + node->getID()) {
                if ((int) edge->getNumLanes() > lane) {
                    // ok, we know this already...
                    return;
                }
                // increment by one
                edge->incLaneNo(1);
                return;
            }
        }
        // nope, we have to split the edge...
        //  maybe it is not the proper edge to split - VISUM seems not to sort the splits...
        bool mustRecheck = true;
        double seenLength = 0;
        while (mustRecheck) {
            if (isSplitEdge(edge, node)) {
                // ok, we have a previously created edge here
                std::string sub = edge->getID();
                sub = sub.substr(sub.rfind('_', sub.rfind('_') - 1));
                sub = sub.substr(1, sub.find('_', 1) - 1);
                double dist = StringUtils::toDouble(sub);
                if (dist < length) {
                    seenLength += edge->getLength();
                    if (dirS == "1") {
                        // incoming -> move back
                        edge = edge->getFromNode()->getIncomingEdges()[0];
                    } else {
                        // outgoing -> move forward
                        edge = edge->getToNode()->getOutgoingEdges()[0];
                    }
                } else {
                    mustRecheck = false;
                }
            } else {
                // we have the center edge - do not continue...
                mustRecheck = false;
            }
        }
        // compute position
        Position p;
        double useLength = length - seenLength;
        useLength = edge->getLength() - useLength;
        if (useLength < 0 || useLength > edge->getLength()) {
            WRITE_WARNING("Could not find split position for edge '" + edge->getID() + "'.");
            return;
        }
        std::string edgeID = edge->getID();
        p = edge->getGeometry().positionAtOffset(useLength);
        if (isSplitEdge(edge, node)) {
            edgeID = edgeID.substr(0, edgeID.find('_'));
        }
        NBNode* rn = new NBNode(edgeID + "_" +  toString((int) length) + "_" + node->getID(), p);
        if (!myNetBuilder.getNodeCont().insert(rn)) {
            throw ProcessError("Ups - could not insert node!");
        }
        std::string nid = edgeID + "_" +  toString((int) length) + "_" + node->getID();
        myNetBuilder.getEdgeCont().splitAt(myNetBuilder.getDistrictCont(), edge, useLength, rn,
                                           edge->getID(), nid, edge->getNumLanes() + 0, edge->getNumLanes() + 1);
        // old edge is deleted and a new edge with the same name created
        edge = myNetBuilder.getEdgeCont().retrieve(edgeID);
        NBEdge* nedge = myNetBuilder.getEdgeCont().retrieve(nid);
        nedge = nedge->getToNode()->getOutgoingEdges()[0];
        while (isSplitEdge(edge, node)) {
            assert(nedge->getToNode()->getOutgoingEdges().size() > 0);
            nedge->incLaneNo(1);
            nedge = nedge->getToNode()->getOutgoingEdges()[0];
        }
    }
}


void
NIImporter_VISUM::parse_TrafficLights() {
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    SUMOTime cycleTime = (SUMOTime) getWeightedFloat2("Umlaufzeit", "UMLZEIT", "s");
    SUMOTime intermediateTime = (SUMOTime) getWeightedFloat2("StdZwischenzeit", "STDZWZEIT", "s");
    bool phaseBased = myLineParser.know("PhasenBasiert")
                      ? StringUtils::toBool(myLineParser.get("PhasenBasiert"))
                      : false;
    SUMOTime offset = myLineParser.know("ZEITVERSATZ") ? TIME2STEPS(getWeightedFloat("ZEITVERSATZ", "s")) : 0;
    // add to the list
    myTLS[myCurrentID] = new NIVisumTL(myCurrentID, cycleTime, offset, intermediateTime, phaseBased);
}


void
NIImporter_VISUM::parse_NodesToTrafficLights() {
    std::string node = myLineParser.get("KnotNr").c_str();
    if (node == "0") {
        // this is a dummy value which cannot be assigned to
        return;
    }
    std::string trafficLight = myLineParser.get("LsaNr").c_str();
    // add to the list
    NBNode* n = myNetBuilder.getNodeCont().retrieve(node);
    auto tlIt = myTLS.find(trafficLight);
    if (n != nullptr && tlIt != myTLS.end()) {
        tlIt->second->addNode(n);
    } else {
        WRITE_ERROR("Could not assign" + std::string(n == nullptr ? " missing" : "") + " node '" + node
                    + "' to" + std::string(tlIt == myTLS.end() ? " missing" : "") + " traffic light '" + trafficLight + "'");
    }
}


void
NIImporter_VISUM::parse_SignalGroups() {
    myCurrentID = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    std::string LSAid = NBHelpers::normalIDRepresentation(myLineParser.get("LsaNr"));
    double startTime = getNamedFloat("GzStart", "GRUENANF");
    double endTime = getNamedFloat("GzEnd", "GRUENENDE");
    double yellowTime = myLineParser.know("GELB") ? getNamedFloat("GELB") : -1;
    // add to the list
    if (myTLS.find(LSAid) == myTLS.end()) {
        WRITE_ERROR("Could not find TLS '" + LSAid + "' for setting the signal group.");
        return;
    }
    myTLS.find(LSAid)->second->addSignalGroup(myCurrentID, (SUMOTime) startTime, (SUMOTime) endTime, (SUMOTime) yellowTime);
}


void
NIImporter_VISUM::parse_TurnsToSignalGroups() {
    // get the id
    std::string SGid = getNamedString("SGNR", "SIGNALGRUPPENNR");
    if (!myLineParser.know("LsaNr")) {
        /// XXX could be retrieved from context
        WRITE_WARNING("Ignoring SIGNALGRUPPEZUFSABBIEGER because LsaNr is not known");
        return;
    }
    std::string LSAid = getNamedString("LsaNr");
    // nodes
    NBNode* from = myLineParser.know("VonKnot") ? getNamedNode("VonKnot") : nullptr;
    NBNode* via = myLineParser.know("KNOTNR")
                  ? getNamedNode("KNOTNR")
                  : getNamedNode("UeberKnot", "UeberKnotNr");
    NBNode* to = myLineParser.know("NachKnot") ? getNamedNode("NachKnot") : nullptr;
    // edges
    NBEdge* edg1 = nullptr;
    NBEdge* edg2 = nullptr;
    if (from == nullptr && to == nullptr) {
        edg1 = getNamedEdgeContinuating("VONSTRNR", via);
        edg2 = getNamedEdgeContinuating("NACHSTRNR", via);
    } else {
        edg1 = getEdge(from, via);
        edg2 = getEdge(via, to);
    }
    // add to the list
    NIVisumTL::SignalGroup& SG = myTLS.find(LSAid)->second->getSignalGroup(SGid);
    if (edg1 != nullptr && edg2 != nullptr) {
        if (!via->hasIncoming(edg1)) {
            std::string sid;
            if (edg1->getID()[0] == '-') {
                sid = edg1->getID().substr(1);
            } else {
                sid = "-" + edg1->getID();
            }
            if (sid.find('_') != std::string::npos) {
                sid = sid.substr(0, sid.find('_'));
            }
            edg1 = getNamedEdgeContinuating(myNetBuilder.getEdgeCont().retrieve(sid),  via);
        }
        if (!via->hasOutgoing(edg2)) {
            std::string sid;
            if (edg2->getID()[0] == '-') {
                sid = edg2->getID().substr(1);
            } else {
                sid = "-" + edg2->getID();
            }
            if (sid.find('_') != std::string::npos) {
                sid = sid.substr(0, sid.find('_'));
            }
            edg2 = getNamedEdgeContinuating(myNetBuilder.getEdgeCont().retrieve(sid),  via);
        }
        SG.connections().push_back(NBConnection(edg1, edg2));
    }
}


void
NIImporter_VISUM::parse_AreaSubPartElement() {
    long long int id = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_FACEID)));
    long long int edgeid = StringUtils::toLong(myLineParser.get(KEYS.getString(VISUM_EDGEID)));
    if (myEdges.find(edgeid) == myEdges.end()) {
        WRITE_ERROR("Unknown edge in TEILFLAECHENELEMENT");
        return;
    }
    std::string dir = myLineParser.get(KEYS.getString(VISUM_DIRECTION));
// get index (unused)
//     std::string indexS = NBHelpers::normalIDRepresentation(myLineParser.get("INDEX"));
//     int index = -1;
//     try {
//         index = StringUtils::toInt(indexS) - 1;
//     } catch (NumberFormatException&) {
//         WRITE_ERROR("An index for a TEILFLAECHENELEMENT is not numeric (id='" + toString(id) + "').");
//         return;
//     }
    PositionVector shape;
    shape.push_back(myPoints[myEdges[edgeid].first]);
    shape.push_back(myPoints[myEdges[edgeid].second]);
    if (dir.length() > 0 && dir[0] == '1') {
        shape = shape.reverse();
    }
    if (mySubPartsAreas.find(id) == mySubPartsAreas.end()) {
        WRITE_ERROR("Unkown are for area part '" + myCurrentID + "'.");
        return;
    }

    const std::vector<long long int>& areas = mySubPartsAreas.find(id)->second;
    for (std::vector<long long int>::const_iterator i = areas.begin(); i != areas.end(); ++i) {
        NBDistrict* d = myShapeDistrictMap[*i];
        if (d == nullptr) {
            continue;
        }
        if (myDistrictShapes.find(d) == myDistrictShapes.end()) {
            myDistrictShapes[d] = PositionVector();
        }
        if (dir.length() > 0 && dir[0] == '1') {
            myDistrictShapes[d].push_back(myPoints[myEdges[edgeid].second]);
            myDistrictShapes[d].push_back(myPoints[myEdges[edgeid].first]);
        } else {
            myDistrictShapes[d].push_back(myPoints[myEdges[edgeid].first]);
            myDistrictShapes[d].push_back(myPoints[myEdges[edgeid].second]);
        }
    }
}


void
NIImporter_VISUM::parse_Phases() {
    // get the id
    std::string phaseid = NBHelpers::normalIDRepresentation(myLineParser.get(KEYS.getString(VISUM_NO)));
    std::string LSAid = NBHelpers::normalIDRepresentation(myLineParser.get("LsaNr"));
    double startTime = getNamedFloat("GzStart", "GRUENANF");
    double endTime = getNamedFloat("GzEnd", "GRUENENDE");
    double yellowTime = myLineParser.know("GELB") ? getNamedFloat("GELB") : -1;
    myTLS.find(LSAid)->second->addPhase(phaseid, (SUMOTime) startTime, (SUMOTime) endTime, (SUMOTime) yellowTime);
}


void NIImporter_VISUM::parse_SignalGroupsToPhases() {
    // get the id
    std::string Phaseid = NBHelpers::normalIDRepresentation(myLineParser.get("PsNr"));
    std::string LSAid = NBHelpers::normalIDRepresentation(myLineParser.get("LsaNr"));
    std::string SGid = NBHelpers::normalIDRepresentation(myLineParser.get("SGNR"));
    // insert
    NIVisumTL* LSA = myTLS.find(LSAid)->second;
    NIVisumTL::SignalGroup& SG = LSA->getSignalGroup(SGid);
    NIVisumTL::Phase* PH = LSA->getPhases().find(Phaseid)->second;
    SG.phases()[Phaseid] = PH;
}


void NIImporter_VISUM::parse_LanesConnections() {
    NBNode* node = nullptr;
    NBEdge* fromEdge = nullptr;
    NBEdge* toEdge = nullptr;
    // get the node and edges depending on network format
    const std::string nodeID = getNamedString("KNOTNR", "KNOT");
    if (nodeID == "0") {
        fromEdge = getNamedEdge("VONSTRNR", "VONSTR");
        toEdge = getNamedEdge("NACHSTRNR", "NACHSTR");
        if (fromEdge == nullptr) {
            return;
        }
        node = fromEdge->getToNode();
        WRITE_WARNING("Ignoring lane-to-lane connection (not yet implemented for this format version)");
        return;
    } else {
        node = getNamedNode("KNOTNR", "KNOT");
        if (node  == nullptr) {
            return;
        }
        fromEdge = getNamedEdgeContinuating("VONSTRNR", "VONSTR", node);
        toEdge = getNamedEdgeContinuating("NACHSTRNR", "NACHSTR", node);
    }
    if (fromEdge == nullptr || toEdge == nullptr) {
        return;
    }

    int fromLaneOffset = 0;
    if (!node->hasIncoming(fromEdge)) {
        fromLaneOffset = fromEdge->getNumLanes();
        fromEdge = getReversedContinuating(fromEdge, node);
    } else {
        fromEdge = getReversedContinuating(fromEdge, node);
        NBEdge* tmp = myNetBuilder.getEdgeCont().retrieve(fromEdge->getID().substr(0, fromEdge->getID().find('_')));
        fromLaneOffset = tmp->getNumLanes();
    }

    int toLaneOffset = 0;
    if (!node->hasOutgoing(toEdge)) {
        toLaneOffset = toEdge->getNumLanes();
        toEdge = getReversedContinuating(toEdge, node);
    } else {
        NBEdge* tmp = myNetBuilder.getEdgeCont().retrieve(toEdge->getID().substr(0, toEdge->getID().find('_')));
        toLaneOffset = tmp->getNumLanes();
    }
    // get the from-lane
    std::string fromLaneS = NBHelpers::normalIDRepresentation(myLineParser.get("VONFSNR"));
    int fromLane = -1;
    try {
        fromLane = StringUtils::toInt(fromLaneS);
    } catch (NumberFormatException&) {
        WRITE_ERROR("A from-lane number for edge '" + fromEdge->getID() + "' is not numeric (" + fromLaneS + ").");
        return;
    }
    fromLane -= 1;
    if (fromLane < 0) {
        WRITE_ERROR("A from-lane number for edge '" + fromEdge->getID() + "' is not positive (" + fromLaneS + ").");
        return;
    }
    // get the from-lane
    std::string toLaneS = NBHelpers::normalIDRepresentation(myLineParser.get("NACHFSNR"));
    int toLane = -1;
    try {
        toLane = StringUtils::toInt(toLaneS);
    } catch (NumberFormatException&) {
        WRITE_ERROR("A to-lane number for edge '" + toEdge->getID() + "' is not numeric (" + toLaneS + ").");
        return;
    }
    toLane -= 1;
    if (toLane < 0) {
        WRITE_ERROR("A to-lane number for edge '" + toEdge->getID() + "' is not positive (" + toLaneS + ").");
        return;
    }
    // !!! the next is probably a hack
    if (fromLane - fromLaneOffset < 0) {
        //fromLaneOffset = 0;
    } else {
        fromLane = (int)fromEdge->getNumLanes() - (fromLane - fromLaneOffset) - 1;
    }
    if (toLane - toLaneOffset < 0) {
        //toLaneOffset = 0;
    } else {
        toLane = (int)toEdge->getNumLanes() - (toLane - toLaneOffset) - 1;
    }
    //
    if ((int) fromEdge->getNumLanes() <= fromLane) {
        WRITE_ERROR("A from-lane number for edge '" + fromEdge->getID() + "' is larger than the edge's lane number (" + fromLaneS + ").");
        return;
    }
    if ((int) toEdge->getNumLanes() <= toLane) {
        WRITE_ERROR("A to-lane number for edge '" + toEdge->getID() + "' is larger than the edge's lane number (" + toLaneS + ").");
        return;
    }
    //
    fromEdge->addLane2LaneConnection(fromLane, toEdge, toLane, NBEdge::Lane2LaneInfoType::VALIDATED);
}













double
NIImporter_VISUM::getWeightedFloat(const std::string& name, const std::string& suffix) {
    try {
        std::string val = myLineParser.get(name);
        if (val.find(suffix) != std::string::npos) {
            val = val.substr(0, val.find(suffix));
        }
        return StringUtils::toDouble(val);
    } catch (...) {}
    return -1;
}


double
NIImporter_VISUM::getWeightedFloat2(const std::string& name, const std::string& name2, const std::string& suffix) {
    double result = getWeightedFloat(name, suffix);
    if (result != -1) {
        return result;
    } else {
        return getWeightedFloat(name2, suffix);
    }
}

bool
NIImporter_VISUM::getWeightedBool(const std::string& name) {
    try {
        return StringUtils::toBool(myLineParser.get(name));
    } catch (...) {}
    try {
        return StringUtils::toBool(myLineParser.get((name + "(IV)")));
    } catch (...) {}
    return false;
}

SVCPermissions
NIImporter_VISUM::getPermissions(const std::string& name, bool warn, SVCPermissions unknown) {
    SVCPermissions result = 0;
    for (std::string v : StringTokenizer(myLineParser.get(name), ",").getVector()) {
        // common values in english and german
        // || v == "funiculaire-telecabine" ---> no matching
        std::transform(v.begin(), v.end(), v.begin(), tolower);
        if (v == "bus" || v == "tcsp" || v == "acces tc" || v == "Accès tc" || v == "accès tc") {
            result |= SVC_BUS;
        } else if (v == "walk" || v == "w" || v == "f" || v == "ped" || v == "map") {
            result |= SVC_PEDESTRIAN;
        } else if (v == "l" || v == "lkw" || v == "h" || v == "hgv" || v == "lw" || v == "truck" || v == "tru" || v == "pl") {
            result |= SVC_TRUCK;
        } else if (v == "b" || v == "bike" || v == "velo") {
            result |= SVC_BICYCLE;
        } else if (v == "train" || v == "rail") {
            result |= SVC_RAIL;
        } else if (v == "tram") {
            result |= SVC_TRAM;
        } else if (v == "p" || v == "pkw" || v == "car" || v == "c" || v == "vp" || v == "2rm") {
            result |= SVC_PASSENGER;
        } else {
            if (warn) {
                WRITE_WARNING("Encountered unknown vehicle category '" + v + "' in type '" + myLineParser.get(KEYS.getString(VISUM_NO)) + "'");
            }
            result |= unknown;
        }
    }
    return result;
}

NBNode*
NIImporter_VISUM::getNamedNode(const std::string& fieldName) {
    std::string nodeS = NBHelpers::normalIDRepresentation(myLineParser.get(fieldName));
    NBNode* node = myNetBuilder.getNodeCont().retrieve(nodeS);
    if (node == nullptr) {
        WRITE_ERROR("The node '" + nodeS + "' is not known.");
    }
    return node;
}

NBNode*
NIImporter_VISUM::getNamedNodeSecure(const std::string& fieldName, NBNode* fallback) {
    std::string nodeS = NBHelpers::normalIDRepresentation(myLineParser.get(fieldName));
    NBNode* node = myNetBuilder.getNodeCont().retrieve(nodeS);
    if (node == nullptr) {
        return fallback;
    }
    return node;
}


NBNode*
NIImporter_VISUM::getNamedNode(const std::string& fieldName1, const std::string& fieldName2) {
    if (myLineParser.know(fieldName1)) {
        return getNamedNode(fieldName1);
    } else {
        return getNamedNode(fieldName2);
    }
}


NBEdge*
NIImporter_VISUM::getNamedEdge(const std::string& fieldName) {
    std::string edgeS = NBHelpers::normalIDRepresentation(myLineParser.get(fieldName));
    NBEdge* edge = myNetBuilder.getEdgeCont().retrieve(edgeS);
    if (edge == nullptr) {
        WRITE_ERROR("The edge '" + edgeS + "' is not known.");
    }
    return edge;
}


NBEdge*
NIImporter_VISUM::getNamedEdge(const std::string& fieldName1, const std::string& fieldName2) {
    if (myLineParser.know(fieldName1)) {
        return getNamedEdge(fieldName1);
    } else {
        return getNamedEdge(fieldName2);
    }
}



NBEdge*
NIImporter_VISUM::getReversedContinuating(NBEdge* edge, NBNode* node) {
    std::string sid;
    if (edge->getID()[0] == '-') {
        sid = edge->getID().substr(1);
    } else {
        sid = "-" + edge->getID();
    }
    if (sid.find('_') != std::string::npos) {
        sid = sid.substr(0, sid.find('_'));
    }
    return getNamedEdgeContinuating(myNetBuilder.getEdgeCont().retrieve(sid),  node);
}


NBEdge*
NIImporter_VISUM::getNamedEdgeContinuating(NBEdge* begin, NBNode* node) {
    if (begin == nullptr) {
        return nullptr;
    }
    NBEdge* ret = begin;
    std::string edgeID = ret->getID();
    // hangle forward
    while (ret != nullptr) {
        // ok, this is the edge we are looking for
        if (ret->getToNode() == node) {
            return ret;
        }
        const EdgeVector& nedges = ret->getToNode()->getOutgoingEdges();
        if (nedges.size() != 1) {
            // too many edges follow
            ret = nullptr;
            continue;
        }
        NBEdge* next = nedges[0];
        if (ret->getID().substr(0, edgeID.length()) != next->getID().substr(0, edgeID.length())) {
            // ok, another edge is next...
            ret = nullptr;
            continue;
        }
        if (next->getID().substr(next->getID().length() - node->getID().length()) != node->getID()) {
            ret = nullptr;
            continue;
        }
        ret = next;
    }

    ret = begin;
    // hangle backward
    while (ret != nullptr) {
        // ok, this is the edge we are looking for
        if (ret->getFromNode() == node) {
            return ret;
        }
        const EdgeVector& nedges = ret->getFromNode()->getIncomingEdges();
        if (nedges.size() != 1) {
            // too many edges follow
            ret = nullptr;
            continue;
        }
        NBEdge* next = nedges[0];
        if (ret->getID().substr(0, edgeID.length()) != next->getID().substr(0, edgeID.length())) {
            // ok, another edge is next...
            ret = nullptr;
            continue;
        }
        if (next->getID().substr(next->getID().length() - node->getID().length()) != node->getID()) {
            ret = nullptr;
            continue;
        }
        ret = next;
    }
    return nullptr;
}


NBEdge*
NIImporter_VISUM::getNamedEdgeContinuating(const std::string& fieldName, NBNode* node) {
    std::string edgeS = NBHelpers::normalIDRepresentation(myLineParser.get(fieldName));
    NBEdge* edge = myNetBuilder.getEdgeCont().retrieve(edgeS);
    if (edge == nullptr) {
        WRITE_ERROR("The edge '" + edgeS + "' is not known.");
    }
    return getNamedEdgeContinuating(edge, node);
}


NBEdge*
NIImporter_VISUM::getNamedEdgeContinuating(const std::string& fieldName1, const std::string& fieldName2,
        NBNode* node) {
    if (myLineParser.know(fieldName1)) {
        return getNamedEdgeContinuating(fieldName1, node);
    } else {
        return getNamedEdgeContinuating(fieldName2, node);
    }
}


NBEdge*
NIImporter_VISUM::getEdge(NBNode* FromNode, NBNode* ToNode) {
    EdgeVector::const_iterator i;
    for (i = FromNode->getOutgoingEdges().begin(); i != FromNode->getOutgoingEdges().end(); i++) {
        if (ToNode == (*i)->getToNode()) {
            return (*i);
        }
    }
    //!!!
    return nullptr;
}


double
NIImporter_VISUM::getNamedFloat(const std::string& fieldName) {
    std::string value = myLineParser.get(fieldName);
    if (StringUtils::endsWith(myLineParser.get(fieldName), "km/h")) {
        value = value.substr(0, value.length() - 4);
    }
    return StringUtils::toDouble(value);
}


double
NIImporter_VISUM::getNamedFloat(const std::string& fieldName, double defaultValue) {
    try {
        return StringUtils::toDouble(myLineParser.get(fieldName));
    } catch (...) {
        return defaultValue;
    }
}


double
NIImporter_VISUM::getNamedFloat(const std::string& fieldName1, const std::string& fieldName2) {
    if (myLineParser.know(fieldName1)) {
        return getNamedFloat(fieldName1);
    } else {
        return getNamedFloat(fieldName2);
    }
}


double
NIImporter_VISUM::getNamedFloat(const std::string& fieldName1, const std::string& fieldName2,
                                double defaultValue) {
    if (myLineParser.know(fieldName1)) {
        return getNamedFloat(fieldName1, defaultValue);
    } else {
        return getNamedFloat(fieldName2, defaultValue);
    }
}


std::string
NIImporter_VISUM::getNamedString(const std::string& fieldName) {
    return NBHelpers::normalIDRepresentation(myLineParser.get(fieldName));
}


std::string
NIImporter_VISUM::getNamedString(const std::string& fieldName1,
                                 const std::string& fieldName2) {
    if (myLineParser.know(fieldName1)) {
        return getNamedString(fieldName1);
    } else {
        return getNamedString(fieldName2);
    }
}






NBNode*
NIImporter_VISUM::buildDistrictNode(const std::string& id, NBNode* dest,
                                    bool isSource) {
    // get the district
    NBDistrict* dist = myNetBuilder.getDistrictCont().retrieve(id);
    if (dist == nullptr) {
        return nullptr;
    }
    // build the id
    std::string nid;
    nid = id + "-" + dest->getID();
    if (!isSource) {
        nid = "-" + nid;
    }
    // insert the node
    if (!myNetBuilder.getNodeCont().insert(nid, dist->getPosition())) {
        WRITE_ERROR("Could not build connector node '" + nid + "'.");
    }
    // return the node
    return myNetBuilder.getNodeCont().retrieve(nid);
}


bool
NIImporter_VISUM::checkNodes(NBNode* from, NBNode* to)  {
    if (from == nullptr) {
        WRITE_ERROR(" The from-node was not found within the net");
    }
    if (to == nullptr) {
        WRITE_ERROR(" The to-node was not found within the net");
    }
    if (from == to) {
        WRITE_ERROR(" Both nodes are the same");
    }
    return from != nullptr && to != nullptr && from != to;
}

bool
NIImporter_VISUM::isSplitEdge(NBEdge* edge, NBNode* node) {
    return (edge->getID().length() > node->getID().length() + 1
            && (edge->getID().substr(edge->getID().length() - node->getID().length() - 1) == "_" + node->getID()));
}

void
NIImporter_VISUM::loadLanguage(const std::string& file) {
    std::ifstream strm(file.c_str());
    if (!strm.good()) {
        throw ProcessError("Could not load VISUM language map from '" + file + "'.");
    }
    while (strm.good()) {
        std::string keyDE;
        std::string keyNew;
        strm >> keyDE;
        strm >> keyNew;
        if (KEYS.hasString(keyDE)) {
            VISUM_KEY key = KEYS.get(keyDE);
            KEYS.remove(keyDE, key);
            KEYS.insert(keyNew, key);
        } else if (keyDE != "") {
            WRITE_WARNING("Unknown entry '" + keyDE + "' in VISUM language map");
        }
    }

}


/****************************************************************************/
