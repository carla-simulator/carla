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
/// @file    NIImporter_Vissim.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Lukas Grohmann (AIT)
/// @author  Gerald Richter (AIT)
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>


#include <string>
#include <fstream>
#include <utils/common/StringUtils.h>
#include <utils/common/StringUtils.h>
#include <utils/common/MsgHandler.h>
#include <utils/options/OptionsCont.h>
#include <netbuild/NBNetBuilder.h>
#include "NIImporter_Vissim.h"
#include "typeloader/NIVissimSingleTypeParser_Simdauer.h"
#include "typeloader/NIVissimSingleTypeParser_Startuhrzeit.h"
#include "typeloader/NIVissimSingleTypeParser_DynUml.h"
#include "typeloader/NIVissimSingleTypeParser_Streckendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Verbindungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Richtungsentscheidungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Routenentscheidungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_VWunschentscheidungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Langsamfahrbereichdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Zuflussdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Fahrzeugtypdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Fahrzeugklassendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Verkehrszusammensetzungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Geschwindigkeitsverteilungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Laengenverteilungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Zeitenverteilungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Lichtsignalanlagendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Signalgruppendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Stopschilddefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Knotendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Signalgeberdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Detektordefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Liniendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Haltestellendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Reisezeitmessungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Querschnittsmessungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Messungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Verlustzeitmessungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Stauzaehlerdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Richtungspfeildefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Parkplatzdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Fahrverhaltendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Streckentypdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Kennungszeile.h"
#include "typeloader/NIVissimSingleTypeParser_Fensterdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Auswertungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Zusammensetzungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Startzufallszahl.h"
#include "typeloader/NIVissimSingleTypeParser_SimRate.h"
#include "typeloader/NIVissimSingleTypeParser_Zeitschrittfaktor.h"
#include "typeloader/NIVissimSingleTypeParser_Linksverkehr.h"
#include "typeloader/NIVissimSingleTypeParser_Stauparameterdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Gelbverhaltendefinition.h"
#include "typeloader/NIVissimSingleTypeParser_LSAKopplungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Gefahrwarnungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_TEAPACDefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Netzobjektdefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Fahrtverlaufdateien.h"
#include "typeloader/NIVissimSingleTypeParser_Emission.h"
#include "typeloader/NIVissimSingleTypeParser_Einheitendefinition.h"
#include "typeloader/NIVissimSingleTypeParser__XVerteilungsdefinition.h"
#include "typeloader/NIVissimSingleTypeParser__XKurvedefinition.h"
#include "typeloader/NIVissimSingleTypeParser_Kantensperrung.h"
#include "typeloader/NIVissimSingleTypeParser_Rautedefinition.h"


#include "tempstructs/NIVissimTL.h"
#include "tempstructs/NIVissimClosures.h"
#include "tempstructs/NIVissimSource.h"
#include "tempstructs/NIVissimTrafficDescription.h"
#include "tempstructs/NIVissimVehTypeClass.h"
#include "tempstructs/NIVissimConnection.h"
#include "tempstructs/NIVissimDisturbance.h"
#include "tempstructs/NIVissimConnectionCluster.h"
#include "tempstructs/NIVissimNodeDef.h"
#include "tempstructs/NIVissimEdge.h"
#include "tempstructs/NIVissimConflictArea.h"
#include "tempstructs/NIVissimDistrictConnection.h"
#include "tempstructs/NIVissimVehicleType.h"

#include <utils/xml/SUMOSAXHandler.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/FileHelpers.h>
#include <utils/common/StringTokenizer.h>
#include <utils/distribution/Distribution_Points.h>
#include <utils/distribution/DistributionCont.h>

#include <netbuild/NBEdgeCont.h> // !!! only for debugging purposes


// ===========================================================================
// static variables
// ===========================================================================
StringBijection<int>::Entry NIImporter_Vissim::vissimTags[] = {
    { "network",          NIImporter_Vissim::VISSIM_TAG_NETWORK },
    { "lanes",            NIImporter_Vissim::VISSIM_TAG_LANES },
    { "lane",             NIImporter_Vissim::VISSIM_TAG_LANE },
    { "link",             NIImporter_Vissim::VISSIM_TAG_LINK },
    { "links",            NIImporter_Vissim::VISSIM_TAG_LINKS },
    { "points3D",         NIImporter_Vissim::VISSIM_TAG_POINTS3D },
    { "point3D",          NIImporter_Vissim::VISSIM_TAG_POINT3D },
    { "linkPolyPoint",    NIImporter_Vissim::VISSIM_TAG_LINKPOLYPOINT },
    { "linkPolyPts",      NIImporter_Vissim::VISSIM_TAG_LINKPOLYPTS },
    { "fromLinkEndPt",    NIImporter_Vissim::VISSIM_TAG_FROM },
    { "toLinkEndPt",      NIImporter_Vissim::VISSIM_TAG_TO },
    { "vehicleInput",     NIImporter_Vissim::VISSIM_TAG_VEHICLE_INPUT },
    { "parkingLot",       NIImporter_Vissim::VISSIM_TAG_PARKINGLOT },
    { "vehicleClass",     NIImporter_Vissim::VISSIM_TAG_VEHICLE_CLASS },
    { "intObjectRef",     NIImporter_Vissim::VISSIM_TAG_INTOBJECTREF },
    { "desSpeedDecision", NIImporter_Vissim::VISSIM_TAG_SPEED_DECISION },
    {
        "desSpeedDistribution",
        NIImporter_Vissim::VISSIM_TAG_SPEED_DIST
    },
    {
        "speedDistributionDataPoint",
        NIImporter_Vissim::VISSIM_TAG_DATAPOINT
    },
    {
        "vehicleRoutingDecisionStatic",
        NIImporter_Vissim::VISSIM_TAG_DECISION_STATIC
    },
    {
        "vehicleRouteStatic",
        NIImporter_Vissim::VISSIM_TAG_ROUTE_STATIC
    },
    { "conflictArea",     NIImporter_Vissim::VISSIM_TAG_CA },
    { "",                 NIImporter_Vissim::VISSIM_TAG_NOTHING }
};


StringBijection<int>::Entry NIImporter_Vissim::vissimAttrs[] = {
    { "no",             NIImporter_Vissim::VISSIM_ATTR_NO }, //id
    { "name",           NIImporter_Vissim::VISSIM_ATTR_NAME },
    { "x",              NIImporter_Vissim::VISSIM_ATTR_X },
    { "y",              NIImporter_Vissim::VISSIM_ATTR_Y },
    { "zOffset",        NIImporter_Vissim::VISSIM_ATTR_ZOFFSET },
    { "surch1",         NIImporter_Vissim::VISSIM_ATTR_ZUSCHLAG1 },
    { "surch2",         NIImporter_Vissim::VISSIM_ATTR_ZUSCHLAG2 },
    { "width",          NIImporter_Vissim::VISSIM_ATTR_WIDTH },
    { "linkBehavType",  NIImporter_Vissim::VISSIM_ATTR_LINKBEHAVETYPE},
    { "lane",           NIImporter_Vissim::VISSIM_ATTR_LANE },
    { "pos",            NIImporter_Vissim::VISSIM_ATTR_POS },
    { "link",           NIImporter_Vissim::VISSIM_ATTR_LINK },
    { "intLink",        NIImporter_Vissim::VISSIM_ATTR_INTLINK }, //edgeID
    { "relFlow",        NIImporter_Vissim::VISSIM_ATTR_PERCENTAGE },
    { "zone",           NIImporter_Vissim::VISSIM_ATTR_DISTRICT },
    { "color",          NIImporter_Vissim::VISSIM_ATTR_COLOR },
    { "key",            NIImporter_Vissim::VISSIM_ATTR_KEY },
    { "fx",             NIImporter_Vissim::VISSIM_ATTR_FX },
    { "destLink",       NIImporter_Vissim::VISSIM_ATTR_DESTLINK },
    { "destPos",        NIImporter_Vissim::VISSIM_ATTR_DESTPOS },
    { "link1",          NIImporter_Vissim::VISSIM_ATTR_LINK1 },
    { "link2",          NIImporter_Vissim::VISSIM_ATTR_LINK2 },
    { "status",         NIImporter_Vissim::VISSIM_ATTR_STATUS },
    { "",               NIImporter_Vissim::VISSIM_ATTR_NOTHING }
};


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods (interface in this case)
// ---------------------------------------------------------------------------
void
NIImporter_Vissim::loadNetwork(const OptionsCont& oc, NBNetBuilder& nb) {
    if (!oc.isSet("vissim-file")) {
        return;
    }
    NIImporter_Vissim(nb).load(oc);
}


// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Streckendefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Streckendefinition::NIVissimXMLHandler_Streckendefinition(
    //std::map<int, VissimXMLEdge>& toFill)
    nodeMap& elemData)
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file"),
      myElemData(elemData),
      myHierarchyLevel(0),
      isConnector(false) {
    myElemData.clear();
}

NIImporter_Vissim::NIVissimXMLHandler_Streckendefinition::~NIVissimXMLHandler_Streckendefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Streckendefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    myHierarchyLevel++;

    // finding an actual LINK
    if (element == VISSIM_TAG_LINK) {
        //parse all links
        bool ok = true;
        int id = attrs.get<int>(VISSIM_ATTR_NO, nullptr, ok);
        myLastNodeID = id;

        // !!! assuming empty myElemData
        myElemData["id"].push_back(attrs.get<std::string>(VISSIM_ATTR_NO, nullptr, ok));
        // error ignored if name is empty
        myElemData["name"].push_back(attrs.get<std::string>(VISSIM_ATTR_NAME, nullptr, ok, false));
        myElemData["type"].push_back(attrs.get<std::string>(VISSIM_ATTR_LINKBEHAVETYPE, nullptr, ok));
        myElemData["zuschlag1"].push_back(attrs.get<std::string>(VISSIM_ATTR_ZUSCHLAG1, nullptr, ok));
        myElemData["zuschlag2"].push_back(attrs.get<std::string>(VISSIM_ATTR_ZUSCHLAG2, nullptr, ok));
    }

    if (element == VISSIM_TAG_LANE) {
        bool ok = true;
        // appends empty element if no width found
        // error ignored if name is empty
        myElemData["width"].push_back(attrs.get<std::string>(VISSIM_ATTR_WIDTH, nullptr, ok, false));
    }

    if (element == VISSIM_TAG_FROM) {
        if (isConnector != true) {
            isConnector = true;
        }
        bool ok = true;
        std::vector<std::string> from(StringTokenizer(attrs.get<std::string>(
                                          VISSIM_ATTR_LANE, nullptr, ok), " ").getVector());
        myElemData["from_pos"].push_back(attrs.get<std::string>(VISSIM_ATTR_POS, nullptr, ok));
        myElemData["from_id"].push_back(from[0]);
        myElemData["from_lane"].push_back(from[1]);
    }

    if (element == VISSIM_TAG_TO) {
        bool ok = true;
        std::vector<std::string> to(StringTokenizer(attrs.get<std::string>(
                                        VISSIM_ATTR_LANE, nullptr, ok), " ").getVector());
        myElemData["to_pos"].push_back(attrs.get<std::string>(VISSIM_ATTR_POS, nullptr, ok));
        myElemData["to_id"].push_back(to[0]);
        myElemData["to_lane"].push_back(to[1]);
    }

    if (element == VISSIM_TAG_POINT3D || element == VISSIM_TAG_LINKPOLYPOINT) {
        bool ok = true;
        // create a <sep> separated string of coordinate data
        std::string sep(" ");

        std::string posS(attrs.get<std::string>(VISSIM_ATTR_X, nullptr, ok));
        posS += sep;
        posS.append(attrs.get<std::string>(VISSIM_ATTR_Y, nullptr, ok));
        // allow for no Z
        std::string z(attrs.get<std::string>(VISSIM_ATTR_ZOFFSET, nullptr, ok, false));
        if (z.length() > 0) {
            posS += sep;
            posS.append(z);
        }
        myElemData["pos"].push_back(posS);
    }


}

void
NIImporter_Vissim::NIVissimXMLHandler_Streckendefinition::myEndElement(int element) {
    if (element == VISSIM_TAG_LINK && myHierarchyLevel == 3) {
        //std::cout << "elemData len:" << myElemData.size() << std::endl;

        NIVissimClosedLanesVector clv;          //FIXME -> clv einlesen
        std::vector<int> assignedVehicles;      //FIXME -> assignedVehicles einlesen
        int id(StringUtils::toInt(myElemData["id"].front()));

        PositionVector geom;
        // convert all position coordinate strings to PositionVectors
        while (!myElemData["pos"].empty()) {
            std::vector<std::string> sPos_v(StringTokenizer(
                                                myElemData["pos"].front(), " ").getVector());
            myElemData["pos"].pop_front();
            std::vector<double> pos_v(3);

            // doing a transform with explicit hint on function signature
            std::transform(sPos_v.begin(), sPos_v.end(), pos_v.begin(),
                           StringUtils::toDouble);
            geom.push_back_noDoublePos(Position(pos_v[0], pos_v[1], pos_v[2]));
        }
        // FIXME: a length = 0 PosVec seems fatal -> segfault
        double length(geom.length());

        if (isConnector == false) {
            // Add Edge
            std::vector<double> laneWidths;
            for (std::string& w : myElemData["width"]) {
                laneWidths.push_back(StringUtils::toDouble(w));
            }
            NIVissimEdge* edge = new NIVissimEdge(id,
                                                  myElemData["name"].front(),
                                                  myElemData["type"].front(),
                                                  laneWidths,
                                                  StringUtils::toDouble(myElemData["zuschlag1"].front()),
                                                  StringUtils::toDouble(myElemData["zuschlag2"].front()),
                                                  length, geom, clv);
            NIVissimEdge::dictionary(id, edge);
        } else {
            int numLanes = (int)myElemData["width"].size();
            std::vector<int> laneVec(numLanes);
            // Add Connector

            //NOTE: there should be only 1 lane number in XML
            // subtraction of 1 as in readExtEdgePointDef()
            laneVec[0] = StringUtils::toInt(myElemData["from_lane"].front()) - 1;
            // then count up, building lane number vector
            for (std::vector<int>::iterator each = ++laneVec.begin(); each != laneVec.end(); ++each) {
                *each = *(each - 1) + 1;
            }

            NIVissimExtendedEdgePoint from_def(
                StringUtils::toInt(myElemData["from_id"].front()),
                laneVec,
                StringUtils::toDouble(myElemData["from_pos"].front()),
                assignedVehicles);

            //NOTE: there should be only 1 lane number in XML
            // subtraction of 1 as in readExtEdgePointDef()
            laneVec[0] = StringUtils::toInt(myElemData["to_lane"].front()) - 1;
            // then count up, building lane number vector
            for (std::vector<int>::iterator each = ++laneVec.begin(); each != laneVec.end(); ++each) {
                *each = *(each - 1) + 1;
            }

            NIVissimExtendedEdgePoint to_def(
                StringUtils::toInt(myElemData["to_id"].front()),
                laneVec,
                StringUtils::toDouble(myElemData["to_pos"].front()),
                assignedVehicles);

            NIVissimConnection* connector = new
            NIVissimConnection(id,
                               myElemData["name"].front(),
                               from_def, to_def,
                               geom, assignedVehicles, clv);

            NIVissimConnection::dictionary(id, connector);
        }
        // clear the element data
        myElemData.clear();
        isConnector = false;
        //std::cout << "elemData len (clear):" << myElemData.size() << std::endl;
        //std::cout.flush();

    }
    --myHierarchyLevel;
}


// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Zuflussdefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Zuflussdefinition::NIVissimXMLHandler_Zuflussdefinition()
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file") {
}

NIImporter_Vissim::NIVissimXMLHandler_Zuflussdefinition::~NIVissimXMLHandler_Zuflussdefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Zuflussdefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // finding an actual flow
    if (element == VISSIM_TAG_VEHICLE_INPUT) {
        //parse all flows
        bool ok = true;
        std::string id = attrs.get<std::string>(VISSIM_ATTR_NO, nullptr, ok);
        std::string edgeid = attrs.get<std::string>(VISSIM_ATTR_LINK, nullptr, ok);
        std::string name = attrs.get<std::string>(VISSIM_ATTR_NAME, nullptr, ok, false);

        NIVissimSource::dictionary(id,
                                   name,
                                   edgeid);
    }
}

// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Parkplatzdefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Parkplatzdefinition::NIVissimXMLHandler_Parkplatzdefinition()
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file") {
}

NIImporter_Vissim::NIVissimXMLHandler_Parkplatzdefinition::~NIVissimXMLHandler_Parkplatzdefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Parkplatzdefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // finding an actual parkinglot
    if (element == VISSIM_TAG_PARKINGLOT) {
        //parse all parkinglots
        bool ok = true;
        int id = attrs.get<int>(VISSIM_ATTR_NO, nullptr, ok);
        int edgeid = attrs.get<int>(VISSIM_ATTR_INTLINK, nullptr, ok);
        std::string name = attrs.get<std::string>(VISSIM_ATTR_NAME, nullptr, ok, false);
        double position = attrs.get<double>(VISSIM_ATTR_POS, nullptr, ok);
        std::vector<std::pair<int, int> > assignedVehicles; // (vclass, vwunsch)
        //FIXME: vWunsch + Fahzeugklassen einlesen
        // There can be s
        std::vector<int> districts;
        //FIXME: Parkplatzdefinition für mehrere Zonen implementieren
        std::vector<double> percentages;
        districts.push_back(attrs.get<int>(VISSIM_ATTR_DISTRICT, nullptr, ok));
        percentages.push_back(attrs.get<double>(VISSIM_ATTR_PERCENTAGE, nullptr, ok));

        NIVissimDistrictConnection::dictionary(id,
                                               name,
                                               districts,
                                               percentages,
                                               edgeid,
                                               position,
                                               assignedVehicles);
    }
}


// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Fahrzeugklassendefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Fahrzeugklassendefinition::NIVissimXMLHandler_Fahrzeugklassendefinition(nodeMap& elemData)
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file"),
      myElemData(elemData),
      myHierarchyLevel(0) {
    myElemData.clear();
}

NIImporter_Vissim::NIVissimXMLHandler_Fahrzeugklassendefinition::~NIVissimXMLHandler_Fahrzeugklassendefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Fahrzeugklassendefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    myHierarchyLevel++;

    if (element == VISSIM_TAG_VEHICLE_CLASS) {
        bool ok = true;
        myElemData["id"].push_back(attrs.get<std::string>(VISSIM_ATTR_NO, nullptr, ok));
        myElemData["name"].push_back(attrs.get<std::string>(VISSIM_ATTR_NAME, nullptr, ok, false));
        std::string colorStr(attrs.get<std::string>(VISSIM_ATTR_COLOR, nullptr, ok));
        for (int pos = (int)colorStr.size() - 2; pos > 0; pos -= 2) {
            colorStr.insert(pos, " ");
        }
        myElemData["color"].push_back(colorStr);
    }
    if (element == VISSIM_TAG_INTOBJECTREF) {
        bool ok = true;
        myElemData["types"].push_back(attrs.get<std::string>(VISSIM_ATTR_KEY, nullptr, ok));


    }
}

void
NIImporter_Vissim::NIVissimXMLHandler_Fahrzeugklassendefinition::myEndElement(int element) {
    if (element == VISSIM_TAG_VEHICLE_CLASS && myHierarchyLevel == 3) {
        RGBColor color;
        std::istringstream iss(myElemData["color"].front());
        std::vector<std::string> sCol_v(StringTokenizer(
                                            myElemData["color"].front(), " ").getVector());
        std::vector<int> myColorVector(sCol_v.size());
        std::transform(sCol_v.begin(), sCol_v.end(), myColorVector.begin(), StringUtils::hexToInt);

        color = RGBColor((unsigned char)myColorVector[0],
                         (unsigned char)myColorVector[1],
                         (unsigned char)myColorVector[2],
                         (unsigned char)myColorVector[3]);
        std::vector<int> types;
        while (!myElemData["types"].empty()) {
            types.push_back(StringUtils::toInt(myElemData["types"].front()));
            myElemData["types"].pop_front();
        }

        NIVissimVehTypeClass::dictionary(StringUtils::toInt(myElemData["id"].front()),
                                         myElemData["name"].front(),
                                         color,
                                         types);
        myElemData.clear();
    }
    --myHierarchyLevel;
}

// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition::NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition(nodeMap& elemData)
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file"),
      myElemData(elemData),
      myHierarchyLevel(0) {
    myElemData.clear();
}

NIImporter_Vissim::NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition::~NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    myHierarchyLevel++;
    if (element == VISSIM_TAG_SPEED_DIST) {
        bool ok = true;
        myElemData["id"].push_back(attrs.get<std::string>(VISSIM_ATTR_NO, nullptr, ok));
    }

    if (element == VISSIM_TAG_DATAPOINT) {
        bool ok = true;
        std::string sep(" ");
        std::string posS(attrs.get<std::string>(VISSIM_ATTR_X, nullptr, ok));
        posS += sep;
        posS.append(attrs.get<std::string>(VISSIM_ATTR_FX, nullptr, ok));
        myElemData["points"].push_back(posS);

    }

}

void
NIImporter_Vissim::NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition::myEndElement(int element) {
    if (element == VISSIM_TAG_SPEED_DIST && myHierarchyLevel == 3) {
        Distribution_Points* points = new Distribution_Points(myElemData["id"].front());
        while (!myElemData["points"].empty()) {
            std::vector<std::string> sPos_v(StringTokenizer(
                                                myElemData["points"].front(), " ").getVector());
            myElemData["points"].pop_front();
            points->add(StringUtils::toDouble(sPos_v[0]), StringUtils::toDouble(sPos_v[1]));
        }
        DistributionCont::dictionary("speed", myElemData["id"].front(), points);
        myElemData.clear();
    }
    --myHierarchyLevel;
}

// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_VWunschentscheidungsdefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_VWunschentscheidungsdefinition::NIVissimXMLHandler_VWunschentscheidungsdefinition(nodeMap& elemData)
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file"),
      myElemData(elemData),
      myHierarchyLevel(0) {
    myElemData.clear();
}

NIImporter_Vissim::NIVissimXMLHandler_VWunschentscheidungsdefinition::~NIVissimXMLHandler_VWunschentscheidungsdefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_VWunschentscheidungsdefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    myHierarchyLevel++;
    if (element == VISSIM_TAG_SPEED_DECISION) {
        bool ok = true;
        myElemData["name"].push_back(attrs.get<std::string>(VISSIM_ATTR_NAME, nullptr, ok, false));
        //FIXME: 2 vWunsch in the xml file, but only 1 of them is set???
    }

}

void
NIImporter_Vissim::NIVissimXMLHandler_VWunschentscheidungsdefinition::myEndElement(int /* element */) {
    --myHierarchyLevel;
}


// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_Routenentscheidungsdefinition-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_Routenentscheidungsdefinition::NIVissimXMLHandler_Routenentscheidungsdefinition(nodeMap& elemData)
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file"),
      myElemData(elemData),
      myHierarchyLevel(0) {
    myElemData.clear();
}

NIImporter_Vissim::NIVissimXMLHandler_Routenentscheidungsdefinition::~NIVissimXMLHandler_Routenentscheidungsdefinition() { }

void
NIImporter_Vissim::NIVissimXMLHandler_Routenentscheidungsdefinition::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    myHierarchyLevel++;
    if (element == VISSIM_TAG_DECISION_STATIC) {
        bool ok = true;
        myElemData["startLink"].push_back(attrs.get<std::string>(VISSIM_ATTR_LINK, nullptr, ok));
        myElemData["startPos"].push_back(attrs.get<std::string>(VISSIM_ATTR_POS, nullptr, ok));
    }
    if (element == VISSIM_TAG_ROUTE_STATIC) {
        bool ok = true;
        myElemData["destLink"].push_back(attrs.get<std::string>(VISSIM_ATTR_DESTLINK, nullptr, ok));
        myElemData["destPos"].push_back(attrs.get<std::string>(VISSIM_ATTR_DESTPOS, nullptr, ok));
        myElemData["id"].push_back(attrs.get<std::string>(VISSIM_ATTR_NO, nullptr, ok));
    }
    if (element == VISSIM_TAG_INTOBJECTREF) {
        // bool ok = true;
    }

}

void
NIImporter_Vissim::NIVissimXMLHandler_Routenentscheidungsdefinition::myEndElement(int /* element */) {
    --myHierarchyLevel;
}

// ---------------------------------------------------------------------------
// definitions of NIVissimXMLHandler_ConflictArea-methods
// ---------------------------------------------------------------------------
NIImporter_Vissim::NIVissimXMLHandler_ConflictArea::NIVissimXMLHandler_ConflictArea()
    : GenericSAXHandler(vissimTags, VISSIM_TAG_NOTHING,
                        vissimAttrs, VISSIM_ATTR_NOTHING,
                        "vissim - file") {}

NIImporter_Vissim::NIVissimXMLHandler_ConflictArea::~NIVissimXMLHandler_ConflictArea() { }

void
NIImporter_Vissim::NIVissimXMLHandler_ConflictArea::myStartElement(int element, const SUMOSAXAttributes& attrs) {
    // finding an actual flow
    if (element == VISSIM_TAG_CA) {
        //parse all flows
        bool ok = true;
        std::string status = attrs.get<std::string>(VISSIM_ATTR_STATUS, nullptr, ok);
        //get only the conflict areas which were set in VISSIM
        if (status != "PASSIVE") {
            NIVissimConflictArea::dictionary(attrs.get<int>(VISSIM_ATTR_NO, nullptr, ok),
                                             attrs.get<std::string>(VISSIM_ATTR_LINK1, nullptr, ok),
                                             attrs.get<std::string>(VISSIM_ATTR_LINK2, nullptr, ok),
                                             status);
        }

    }
}


/* -------------------------------------------------------------------------
 * NIImporter_Vissim::VissimSingleTypeParser-methods
 * ----------------------------------------------------------------------- */
NIImporter_Vissim::VissimSingleTypeParser::VissimSingleTypeParser(NIImporter_Vissim& parent)
    : myVissimParent(parent) {}


NIImporter_Vissim::VissimSingleTypeParser::~VissimSingleTypeParser() {}


std::string
NIImporter_Vissim::VissimSingleTypeParser::myRead(std::istream& from) {
    std::string tmp;
    from >> tmp;
    return StringUtils::to_lower_case(tmp);
}



std::string
NIImporter_Vissim::VissimSingleTypeParser::readEndSecure(std::istream& from,
        const std::string& excl) {
    std::string myExcl = StringUtils::to_lower_case(excl);
    std::string tmp = myRead(from);
    if (tmp == "") {
        return "DATAEND";
    }
    if (tmp != myExcl
            &&
            (tmp.substr(0, 2) == "--" || !myVissimParent.admitContinue(tmp))
       ) {
        return "DATAEND";
    }
    return StringUtils::to_lower_case(tmp);
}


std::string
NIImporter_Vissim::VissimSingleTypeParser::readEndSecure(std::istream& from,
        const std::vector<std::string>& excl) {
    std::vector<std::string> myExcl;
    std::vector<std::string>::const_iterator i;
    for (i = excl.begin(); i != excl.end(); i++) {
        std::string mes = StringUtils::to_lower_case(*i);
        myExcl.push_back(mes);
    }
    std::string tmp = myRead(from);
    if (tmp == "") {
        return "DATAEND";
    }

    bool equals = false;
    for (i = myExcl.begin(); i != myExcl.end() && !equals; i++) {
        if ((*i) == tmp) {
            equals = true;
        }
    }
    if (!equals
            &&
            (tmp.substr(0, 2) == "--" || !myVissimParent.admitContinue(tmp))
       ) {
        return "DATAEND";
    }
    return StringUtils::to_lower_case(tmp);
}


std::string
NIImporter_Vissim::VissimSingleTypeParser::overrideOptionalLabel(std::istream& from,
        const std::string& tag) {
    std::string tmp;
    if (tag == "") {
        tmp = myRead(from);
    } else {
        tmp = tag;
    }
    if (tmp == "beschriftung") {
        tmp = myRead(from);
        if (tmp == "keine") {
            from >> tmp;
        }
        tmp = myRead(from);
        tmp = myRead(from);
    }
    return tmp;
}


Position
NIImporter_Vissim::VissimSingleTypeParser::getPosition(std::istream& from) {
    double x, y;
    from >> x; // type-checking is missing!
    from >> y; // type-checking is missing!
    return Position(x, y);
}


std::vector<int>
NIImporter_Vissim::VissimSingleTypeParser::parseAssignedVehicleTypes(
    std::istream& from, const std::string& next) {
    std::string tmp = readEndSecure(from);
    std::vector<int> ret;
    if (tmp == "alle") {
        ret.push_back(-1);
        return ret;
    }
    while (tmp != "DATAEND" && tmp != next) {
        ret.push_back(StringUtils::toInt(tmp));
        tmp = readEndSecure(from);
    }
    return ret;
}


NIVissimExtendedEdgePoint
NIImporter_Vissim::VissimSingleTypeParser::readExtEdgePointDef(
    std::istream& from) {
    std::string tag;
    from >> tag; // "Strecke"
    int edgeid;
    from >> edgeid; // type-checking is missing!
    from >> tag; // "Spuren"
    std::vector<int> lanes;
    while (tag != "bei") {
        tag = readEndSecure(from);
        if (tag != "bei") {
            int lane = StringUtils::toInt(tag);
            lanes.push_back(lane - 1);
        }
    }
    double position;
    from >> position;
    std::vector<int> dummy;
    return NIVissimExtendedEdgePoint(edgeid, lanes, position, dummy);
}


std::string
NIImporter_Vissim::VissimSingleTypeParser::readName(std::istream& from) {
    std::string name;
    from >> name;
    if (name[0] == '"') {
        while (name[name.length() - 1] != '"') {
            std::string tmp;
            from >> tmp;
            name = name + " " + tmp;
        }
        name = name.substr(1, name.length() - 2);
    }
    return StringUtils::convertUmlaute(name);
}


void
NIImporter_Vissim::VissimSingleTypeParser::readUntil(std::istream& from,
        const std::string& name) {
    std::string tag;
    while (tag != name) {
        tag = myRead(from);
    }
}

bool
NIImporter_Vissim::VissimSingleTypeParser::skipOverreading(std::istream& from,
        const std::string& name) {
    std::string tag;
    while (tag != name) {
        tag = myRead(from);
    }
    while (tag != "DATAEND") {
        tag = readEndSecure(from);
    }
    return true;
}



/* -------------------------------------------------------------------------
 * NIImporter_Vissim-methods
 * ----------------------------------------------------------------------- */
NIImporter_Vissim::NIImporter_Vissim(NBNetBuilder& nb) : myNetBuilder(nb) {
    insertKnownElements();
    buildParsers();
    myColorMap["blau"] = RGBColor(77, 77, 255, 255);
    myColorMap["gelb"] = RGBColor::YELLOW;
    myColorMap["grau"] = RGBColor::GREY;
    myColorMap["lila"] = RGBColor::MAGENTA;
    myColorMap["gruen"] = RGBColor::GREEN;
    myColorMap["rot"] = RGBColor::RED;
    myColorMap["schwarz"] = RGBColor::BLACK;
    myColorMap["tuerkis"] = RGBColor::CYAN;
    myColorMap["weiss"] = RGBColor::WHITE;
    myColorMap["keine"] = RGBColor::WHITE;
}




NIImporter_Vissim::~NIImporter_Vissim() {
    NIVissimAbstractEdge::clearDict();
    NIVissimClosures::clearDict();
    NIVissimDistrictConnection::clearDict();
    NIVissimDisturbance::clearDict();
    NIVissimNodeCluster::clearDict();
    NIVissimNodeDef::clearDict();
    NIVissimSource::clearDict();
    NIVissimTL::clearDict();
    NIVissimTL::NIVissimTLSignal::clearDict();
    NIVissimTL::NIVissimTLSignalGroup::clearDict();
    NIVissimTrafficDescription::clearDict();
    NIVissimVehTypeClass::clearDict();
    NIVissimVehicleType::clearDict();
    NIVissimConnectionCluster::clearDict();
    NIVissimEdge::clearDict();
    NIVissimAbstractEdge::clearDict();
    NIVissimConnection::clearDict();
    NIVissimConflictArea::clearDict();
    for (ToParserMap::iterator i = myParsers.begin(); i != myParsers.end(); i++) {
        delete (*i).second;
    }
}


void
NIImporter_Vissim::load(const OptionsCont& options) {
    const std::string file = options.getString("vissim-file");
    // try to open the file
    std::ifstream strm(file.c_str());
    if (!strm.good()) {
        WRITE_ERROR("Could not open vissim-file '" + file + "'.");
        return;
    }
    std::string token;
    strm >> token;
    if (StringUtils::endsWith(file, ".inpx") || StringUtils::endsWith(token, "<?xml") || StringUtils::endsWith(token, "<network")) {
        // Create NIVissimXMLHandlers
        NIVissimXMLHandler_Streckendefinition XMLHandler_Streckendefinition(elementData);
        NIVissimXMLHandler_Zuflussdefinition XMLHandler_Zuflussdefinition;
        //NIVissimXMLHandler_Parkplatzdefinition XMLHandler_Parkplatzdefinition;
        NIVissimXMLHandler_Fahrzeugklassendefinition XMLHandler_Fahrzeugklassendefinition(elementData);
        NIVissimXMLHandler_Geschwindigkeitsverteilungsdefinition XMLHandler_Geschwindigkeitsverteilung(elementData);
        NIVissimXMLHandler_ConflictArea XMLHandler_ConflictAreas;

        // Strecken + Verbinder
        XMLHandler_Streckendefinition.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing strecken+verbinder from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_Streckendefinition, file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();

        // Zuflüsse
        XMLHandler_Zuflussdefinition.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing zuflüsse from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_Zuflussdefinition, file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();

        //Geschwindigkeitsverteilungen
        XMLHandler_Geschwindigkeitsverteilung.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing parkplätze from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_Geschwindigkeitsverteilung, file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();


        //Fahrzeugklassen
        XMLHandler_Fahrzeugklassendefinition.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing parkplätze from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_Fahrzeugklassendefinition, file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();

        //Parkplätze
        /*XMLHandler_Parkplatzdefinition.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing parkplätze from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_Parkplatzdefinition, file)) {
        return;
        }
        PROGRESS_DONE_MESSAGE();*/


        //Konfliktflächen
        XMLHandler_ConflictAreas.setFileName(file);
        PROGRESS_BEGIN_MESSAGE("Parsing conflict areas from vissim-file '" + file + "'");
        if (!XMLSubSys::runParser(XMLHandler_ConflictAreas, file)) {
            return;
        }
        PROGRESS_DONE_MESSAGE();
    } else {
        strm.seekg(strm.beg);
        if (!readContents(strm)) {
            return;
        }
    }
    postLoadBuild(options.getFloat("vissim.join-distance"));
}


bool
NIImporter_Vissim::admitContinue(const std::string& tag) {
    ToElemIDMap::const_iterator i = myKnownElements.find(tag);
    if (i == myKnownElements.end()) {
        return true;
    }
    myLastSecure = tag;
    return false;
}


bool
NIImporter_Vissim::readContents(std::istream& strm) {
    // read contents
    bool ok = true;
    while (strm.good() && ok) {
        std::string tag;
        if (myLastSecure != "") {
            tag = myLastSecure;
        } else {
            strm >> tag;
        }
        myLastSecure = "";
        bool parsed = false;
        while (!parsed && strm.good() && ok) {
            ToElemIDMap::iterator i = myKnownElements.find(StringUtils::to_lower_case(tag));
            if (i != myKnownElements.end()) {
                ToParserMap::iterator j = myParsers.find((*i).second);
                if (j != myParsers.end()) {
                    VissimSingleTypeParser* parser = (*j).second;
                    ok = parser->parse(strm);
                    parsed = true;
                }
            }
            if (!parsed) {
                std::string line;
                std::streamoff pos;
                do {
                    pos = strm.tellg();
                    getline(strm, line);
                } while (strm.good() && (line == "" || line[0] == ' ' || line[0] == '-'));
                if (!strm.good()) {
                    return true;
                }
                strm.seekg(pos);
                strm >> tag;
            }
        }
    }
    return ok;
}


void
NIImporter_Vissim::postLoadBuild(double offset) {
    // close the loading process
    NIVissimBoundedClusterObject::closeLoading();
    NIVissimConnection::dict_assignToEdges();
    NIVissimDisturbance::dict_SetDisturbances();
    // build district->connections map
    NIVissimDistrictConnection::dict_BuildDistrictConnections();
    // build clusters around nodes
//    NIVissimNodeDef::buildNodeClusters();
    // build node clusters around traffic lights
//    NIVissimTL::buildNodeClusters();

    // when connections or disturbances are left, build nodes around them

    // try to assign connection clusters to nodes
    //  only left connections will be processed in
    //   buildConnectionClusters & join
//30.4. brauchen wir noch!    NIVissimNodeDef::dict_assignConnectionsToNodes();

    // build clusters of connections with the same direction and a similar position along the streets
    NIVissimEdge::buildConnectionClusters();
    // check whether further nodes (connection clusters by now) must be added
    NIVissimDistrictConnection::dict_CheckEdgeEnds();

    // join clusters when overlapping (different streets are possible)
    NIVissimEdge::dict_checkEdges2Join();
    NIVissimConnectionCluster::joinBySameEdges(offset);
//    NIVissimConnectionCluster::joinByDisturbances(offset);

//    NIVissimConnectionCluster::addTLs(offset);

    // build nodes from clusters
    NIVissimNodeCluster::setCurrentVirtID(NIVissimNodeDef::getMaxID());
    NIVissimConnectionCluster::buildNodeClusters();

//    NIVissimNodeCluster::dict_recheckEdgeChanges();
    NIVissimNodeCluster::buildNBNodes(myNetBuilder.getNodeCont());
    NIVissimDistrictConnection::dict_BuildDistrictNodes(
        myNetBuilder.getDistrictCont(), myNetBuilder.getNodeCont());
    NIVissimEdge::dict_propagateSpeeds();
    NIVissimEdge::dict_buildNBEdges(myNetBuilder.getDistrictCont(), myNetBuilder.getNodeCont(), myNetBuilder.getEdgeCont(), offset);
    if (OptionsCont::getOptions().getBool("vissim.report-unset-speeds")) {
        NIVissimEdge::reportUnsetSpeeds();
    }
    NIVissimDistrictConnection::dict_BuildDistricts(myNetBuilder.getDistrictCont(), myNetBuilder.getEdgeCont(), myNetBuilder.getNodeCont());
    NIVissimConnection::dict_buildNBEdgeConnections(myNetBuilder.getEdgeCont());
    NIVissimNodeCluster::dict_addDisturbances(myNetBuilder.getDistrictCont(), myNetBuilder.getNodeCont(), myNetBuilder.getEdgeCont());
    NIVissimConflictArea::setPriorityRegulation(myNetBuilder.getEdgeCont());
    NIVissimTL::dict_SetSignals(myNetBuilder.getTLLogicCont(), myNetBuilder.getEdgeCont());
}


void
NIImporter_Vissim::insertKnownElements() {
    myKnownElements["kennung"] = VE_Kennungszeile;
    myKnownElements["zufallszahl"] = VE_Startzufallszahl;
    myKnownElements["simulationsdauer"] = VE_Simdauer;
    myKnownElements["startuhrzeit"] = VE_Startuhrzeit;
    myKnownElements["simulationsrate"] = VE_SimRate;
    myKnownElements["zeitschritt"] = VE_Zeitschrittfaktor;
    myKnownElements["linksverkehr"] = VE_Linksverkehr;
    myKnownElements["dynuml"] = VE_DynUml;
    myKnownElements["stau"] = VE_Stauparameterdefinition;
    myKnownElements["gelbverhalten"] = VE_Gelbverhaltendefinition;
    myKnownElements["strecke"] = VE_Streckendefinition;
    myKnownElements["verbindung"] = VE_Verbindungsdefinition;
    myKnownElements["richtungsentscheidung"] = VE_Richtungsentscheidungsdefinition;
    myKnownElements["routenentscheidung"] = VE_Routenentscheidungsdefinition;
    myKnownElements["vwunschentscheidung"] = VE_VWunschentscheidungsdefinition;
    myKnownElements["langsamfahrbereich"] = VE_Langsamfahrbereichdefinition;
    myKnownElements["zufluss"] = VE_Zuflussdefinition;
    myKnownElements["fahrzeugtyp"] = VE_Fahrzeugtypdefinition;
    myKnownElements["fahrzeugklasse"] = VE_Fahrzeugklassendefinition;
    myKnownElements["zusammensetzung"] = VE_Verkehrszusammensetzungsdefinition;
    myKnownElements["vwunsch"] = VE_Geschwindigkeitsverteilungsdefinition;
    myKnownElements["laengen"] = VE_Laengenverteilungsdefinition;
    myKnownElements["zeiten"] = VE_Zeitenverteilungsdefinition;
    myKnownElements["baujahre"] = VE_Baujahrverteilungsdefinition;
    myKnownElements["leistungen"] = VE_Laufleistungsverteilungsdefinition;
    myKnownElements["massen"] = VE_Massenverteilungsdefinition;
    myKnownElements["leistungen"] = VE_Leistungsverteilungsdefinition;
    myKnownElements["maxbeschleunigung"] = VE_Maxbeschleunigungskurvedefinition;
    myKnownElements["wunschbeschleunigung"] = VE_Wunschbeschleunigungskurvedefinition;
    myKnownElements["maxverzoegerung"] = VE_Maxverzoegerungskurvedefinition;
    myKnownElements["wunschverzoegerung"] = VE_Wunschverzoegerungskurvedefinition;
    myKnownElements["querverkehrsstoerung"] = VE_Querverkehrsstoerungsdefinition;
    myKnownElements["lsa"] = VE_Lichtsignalanlagendefinition;
    myKnownElements["signalgruppe"] = VE_Signalgruppendefinition;
    myKnownElements["signalgeber"] = VE_Signalgeberdefinition;
    myKnownElements["lsakopplung"] = VE_LSAKopplungdefinition;
    myKnownElements["detektor"] = VE_Detektorendefinition;
    myKnownElements["haltestelle"] = VE_Haltestellendefinition;
    myKnownElements["linie"] = VE_Liniendefinition;
    myKnownElements["stopschild"] = VE_Stopschilddefinition;
    myKnownElements["messung"] = VE_Messungsdefinition;
    myKnownElements["reisezeit"] = VE_Reisezeitmessungsdefinition;
    myKnownElements["verlustzeit"] = VE_Verlustzeitmessungsdefinition;
    myKnownElements["querschnittsmessung"] = VE_Querschnittsmessungsdefinition;
    myKnownElements["stauzaehler"] = VE_Stauzaehlerdefinition;
    myKnownElements["auswertung"] = VE_Auswertungsdefinition;
    myKnownElements["fenster"] = VE_Fensterdefinition;
    myKnownElements["motiv"] = VE_Gefahrenwarnsystemdefinition;
    myKnownElements["parkplatz"] = VE_Parkplatzdefinition;
    myKnownElements["knoten"] = VE_Knotendefinition;
    myKnownElements["teapac"] = VE_TEAPACdefinition;
    myKnownElements["netzobjekt"] = VE_Netzobjektdefinition;
    myKnownElements["richtungspfeil"] = VE_Richtungspfeildefinition;
    myKnownElements["raute"] = VE_Rautedefinition;
    myKnownElements["fahrverhalten"] = VE_Fahrverhaltendefinition;
    myKnownElements["fahrtverlaufdateien"] = VE_Fahrtverlaufdateien;
    myKnownElements["emission"] = VE_Emission;
    myKnownElements["einheit"] = VE_Einheitendefinition;
    myKnownElements["streckentyp"] = VE_Streckentypdefinition;
    myKnownElements["kantensperrung"] = VE_Kantensperrung;
    myKnownElements["kante"] = VE_Kantensperrung;


    myKnownElements["advance"] = VE_DUMMY;
    myKnownElements["temperatur"] = VE_DUMMY;

}



void
NIImporter_Vissim::buildParsers() {
    myParsers[VE_Simdauer] =
        new NIVissimSingleTypeParser_Simdauer(*this);
    myParsers[VE_Startuhrzeit] =
        new NIVissimSingleTypeParser_Startuhrzeit(*this);
    myParsers[VE_DynUml] =
        new NIVissimSingleTypeParser_DynUml(*this);
    myParsers[VE_Streckendefinition] =
        new NIVissimSingleTypeParser_Streckendefinition(*this);
    myParsers[VE_Verbindungsdefinition] =
        new NIVissimSingleTypeParser_Verbindungsdefinition(*this);
    myParsers[VE_Richtungsentscheidungsdefinition] =
        new NIVissimSingleTypeParser_Richtungsentscheidungsdefinition(*this);
    myParsers[VE_Routenentscheidungsdefinition] =
        new NIVissimSingleTypeParser_Routenentscheidungsdefinition(*this);
    myParsers[VE_VWunschentscheidungsdefinition] =
        new NIVissimSingleTypeParser_VWunschentscheidungsdefinition(*this);
    myParsers[VE_Langsamfahrbereichdefinition] =
        new NIVissimSingleTypeParser_Langsamfahrbereichdefinition(*this);
    myParsers[VE_Zuflussdefinition] =
        new NIVissimSingleTypeParser_Zuflussdefinition(*this);
    myParsers[VE_Fahrzeugtypdefinition] =
        new NIVissimSingleTypeParser_Fahrzeugtypdefinition(*this, myColorMap);
    myParsers[VE_Fahrzeugklassendefinition] =
        new NIVissimSingleTypeParser_Fahrzeugklassendefinition(*this, myColorMap);
    myParsers[VE_Geschwindigkeitsverteilungsdefinition] =
        new NIVissimSingleTypeParser_Geschwindigkeitsverteilungsdefinition(*this);
    myParsers[VE_Laengenverteilungsdefinition] =
        new NIVissimSingleTypeParser_Laengenverteilungsdefinition(*this);
    myParsers[VE_Zeitenverteilungsdefinition] =
        new NIVissimSingleTypeParser_Zeitenverteilungsdefinition(*this);
    myParsers[VE_Querverkehrsstoerungsdefinition] =
        new NIVissimSingleTypeParser_Querverkehrsstoerungsdefinition(*this);
    myParsers[VE_Lichtsignalanlagendefinition] =
        new NIVissimSingleTypeParser_Lichtsignalanlagendefinition(*this);
    myParsers[VE_Signalgruppendefinition] =
        new NIVissimSingleTypeParser_Signalgruppendefinition(*this);
    myParsers[VE_Stopschilddefinition] =
        new NIVissimSingleTypeParser_Stopschilddefinition(*this);
    myParsers[VE_Knotendefinition] =
        new NIVissimSingleTypeParser_Knotendefinition(*this);
    myParsers[VE_Signalgeberdefinition] =
        new NIVissimSingleTypeParser_Signalgeberdefinition(*this);
    myParsers[VE_Detektorendefinition] =
        new NIVissimSingleTypeParser_Detektordefinition(*this);
    myParsers[VE_Haltestellendefinition] =
        new NIVissimSingleTypeParser_Haltestellendefinition(*this);
    myParsers[VE_Liniendefinition] =
        new NIVissimSingleTypeParser_Liniendefinition(*this);
    myParsers[VE_Reisezeitmessungsdefinition] =
        new NIVissimSingleTypeParser_Reisezeitmessungsdefinition(*this);
    myParsers[VE_Querschnittsmessungsdefinition] =
        new NIVissimSingleTypeParser_Querschnittsmessungsdefinition(*this);
    myParsers[VE_Messungsdefinition] =
        new NIVissimSingleTypeParser_Messungsdefinition(*this);
    myParsers[VE_Verlustzeitmessungsdefinition] =
        new NIVissimSingleTypeParser_Verlustzeitmessungsdefinition(*this);
    myParsers[VE_Stauzaehlerdefinition] =
        new NIVissimSingleTypeParser_Stauzaehlerdefinition(*this);
    myParsers[VE_Rautedefinition] =
        new NIVissimSingleTypeParser_Rautedefinition(*this);
    myParsers[VE_Richtungspfeildefinition] =
        new NIVissimSingleTypeParser_Richtungspfeildefinition(*this);
    myParsers[VE_Parkplatzdefinition] =
        new NIVissimSingleTypeParser_Parkplatzdefinition(*this);
    myParsers[VE_Fahrverhaltendefinition] =
        new NIVissimSingleTypeParser_Fahrverhaltendefinition(*this);
    myParsers[VE_Streckentypdefinition] =
        new NIVissimSingleTypeParser_Streckentypdefinition(*this);
    myParsers[VE_Kennungszeile] =
        new NIVissimSingleTypeParser_Kennungszeile(*this);
    myParsers[VE_Fensterdefinition] =
        new NIVissimSingleTypeParser_Fensterdefinition(*this);
    myParsers[VE_Auswertungsdefinition] =
        new NIVissimSingleTypeParser_Auswertungsdefinition(*this);
    myParsers[VE_Verkehrszusammensetzungsdefinition] =
        new NIVissimSingleTypeParser_Zusammensetzungsdefinition(*this);
    myParsers[VE_Kantensperrung] =
        new NIVissimSingleTypeParser_Kantensperrung(*this);

    myParsers[VE_Startzufallszahl] =
        new NIVissimSingleTypeParser_Startzufallszahl(*this);
    myParsers[VE_SimRate] =
        new NIVissimSingleTypeParser_SimRate(*this);
    myParsers[VE_Zeitschrittfaktor] =
        new NIVissimSingleTypeParser_Zeitschrittfaktor(*this);
    myParsers[VE_Linksverkehr] =
        new NIVissimSingleTypeParser_Linksverkehr(*this);
    myParsers[VE_Stauparameterdefinition] =
        new NIVissimSingleTypeParser_Stauparameterdefinition(*this);
    myParsers[VE_Gelbverhaltendefinition] =
        new NIVissimSingleTypeParser_Gelbverhaltendefinition(*this);
    myParsers[VE_LSAKopplungdefinition] =
        new NIVissimSingleTypeParser_LSAKopplungsdefinition(*this);
    myParsers[VE_Gefahrenwarnsystemdefinition] =
        new NIVissimSingleTypeParser_Gefahrwarnungsdefinition(*this);
    myParsers[VE_TEAPACdefinition] =
        new NIVissimSingleTypeParser_TEAPACDefinition(*this);
    myParsers[VE_Netzobjektdefinition] =
        new NIVissimSingleTypeParser_Netzobjektdefinition(*this);
    myParsers[VE_Fahrtverlaufdateien] =
        new NIVissimSingleTypeParser_Fahrtverlaufdateien(*this);
    myParsers[VE_Emission] =
        new NIVissimSingleTypeParser_Emission(*this);
    myParsers[VE_Einheitendefinition] =
        new NIVissimSingleTypeParser_Einheitendefinition(*this);
    myParsers[VE_Baujahrverteilungsdefinition] =
        new NIVissimSingleTypeParser__XVerteilungsdefinition(*this);
    myParsers[VE_Laufleistungsverteilungsdefinition] =
        new NIVissimSingleTypeParser__XVerteilungsdefinition(*this);
    myParsers[VE_Massenverteilungsdefinition] =
        new NIVissimSingleTypeParser__XVerteilungsdefinition(*this);
    myParsers[VE_Leistungsverteilungsdefinition] =
        new NIVissimSingleTypeParser__XVerteilungsdefinition(*this);
    myParsers[VE_Maxbeschleunigungskurvedefinition] =
        new NIVissimSingleTypeParser__XKurvedefinition(*this);
    myParsers[VE_Wunschbeschleunigungskurvedefinition] =
        new NIVissimSingleTypeParser__XKurvedefinition(*this);
    myParsers[VE_Maxverzoegerungskurvedefinition] =
        new NIVissimSingleTypeParser__XKurvedefinition(*this);
    myParsers[VE_Wunschverzoegerungskurvedefinition] =
        new NIVissimSingleTypeParser__XKurvedefinition(*this);

}


/****************************************************************************/
