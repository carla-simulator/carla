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
/// @file    NIVissimDistrictConnection.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#include <config.h>

#include <map>
#include <string>
#include <algorithm>
#include <cassert>
#include <utils/common/VectorHelper.h>
#include <utils/common/ToString.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/PositionVector.h>
#include <utils/options/OptionsCont.h>
#include "NIVissimAbstractEdge.h"
#include "NIVissimEdge.h"
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBDistrict.h>
#include <netbuild/NBDistrictCont.h>
#include "NIVissimDistrictConnection.h"
#include <utils/distribution/DistributionCont.h>
#include <utils/common/MsgHandler.h>


// ===========================================================================
// static member definitions
// ===========================================================================
NIVissimDistrictConnection::DictType NIVissimDistrictConnection::myDict;
std::map<int, std::vector<int> > NIVissimDistrictConnection::myDistrictsConnections;


// ===========================================================================
// method definitions
// ===========================================================================
NIVissimDistrictConnection::NIVissimDistrictConnection(int id,
        const std::string& name,
        const std::vector<int>& districts, const std::vector<double>& percentages,
        int edgeid, double position,
        const std::vector<std::pair<int, int> >& assignedVehicles)
    : myID(id), myName(name), myDistricts(districts),
      myEdgeID(edgeid), myPosition(position),
      myAssignedVehicles(assignedVehicles) {
    std::vector<int>::iterator i = myDistricts.begin();
    std::vector<double>::const_iterator j = percentages.begin();
    while (i != myDistricts.end()) {
        myPercentages[*i] = *j;
        i++;
        j++;
    }
}


NIVissimDistrictConnection::~NIVissimDistrictConnection() {}



bool
NIVissimDistrictConnection::dictionary(int id, const std::string& name,
                                       const std::vector<int>& districts, const std::vector<double>& percentages,
                                       int edgeid, double position,
                                       const std::vector<std::pair<int, int> >& assignedVehicles) {
    NIVissimDistrictConnection* o =
        new NIVissimDistrictConnection(id, name, districts, percentages,
                                       edgeid, position, assignedVehicles);
    if (!dictionary(id, o)) {
        delete o;
        return false;
    }
    return true;
}


bool
NIVissimDistrictConnection::dictionary(int id, NIVissimDistrictConnection* o) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        myDict[id] = o;
        return true;
    }
    return false;
}


NIVissimDistrictConnection*
NIVissimDistrictConnection::dictionary(int id) {
    DictType::iterator i = myDict.find(id);
    if (i == myDict.end()) {
        return nullptr;
    }
    return (*i).second;
}

void
NIVissimDistrictConnection::dict_BuildDistrictConnections() {
    //  pre-assign connections to districts
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        NIVissimDistrictConnection* c = (*i).second;
        const std::vector<int>& districts = c->myDistricts;
        for (std::vector<int>::const_iterator j = districts.begin(); j != districts.end(); j++) {
            // assign connection to district
            myDistrictsConnections[*j].push_back((*i).first);
        }
    }
}


void
NIVissimDistrictConnection::dict_CheckEdgeEnds() {
    for (std::map<int, std::vector<int> >::iterator k = myDistrictsConnections.begin(); k != myDistrictsConnections.end(); k++) {
        const std::vector<int>& connections = (*k).second;
        for (std::vector<int>::const_iterator j = connections.begin(); j != connections.end(); j++) {
            NIVissimDistrictConnection* c = dictionary(*j);
            c->checkEdgeEnd();
        }
    }
}


void
NIVissimDistrictConnection::checkEdgeEnd() {
    NIVissimEdge* edge = NIVissimEdge::dictionary(myEdgeID);
    assert(edge != 0);
    edge->checkDistrictConnectionExistanceAt(myPosition);
}


void
NIVissimDistrictConnection::dict_BuildDistrictNodes(NBDistrictCont& dc,
        NBNodeCont& nc) {
    for (std::map<int, std::vector<int> >::iterator k = myDistrictsConnections.begin(); k != myDistrictsConnections.end(); k++) {
        // get the connections
        const std::vector<int>& connections = (*k).second;
        // retrieve the current district
        std::string dsid = toString<int>((*k).first);
        NBDistrict* district = new NBDistrict(dsid);
        dc.insert(district);
        // compute the middle of the district
        PositionVector pos;
        for (std::vector<int>::const_iterator j = connections.begin(); j != connections.end(); j++) {
            NIVissimDistrictConnection* c = dictionary(*j);
            pos.push_back(c->geomPosition());
        }
        Position distCenter = pos.getPolygonCenter();
        if (connections.size() == 1) { // !!! ok, ok, maybe not the best way just to add an offset
            distCenter.add(10, 10);
        }
        district->setCenter(distCenter);
        // build the node
        std::string id = "District" + district->getID();
        NBNode* districtNode =
            new NBNode(id, district->getPosition(), district);
        if (!nc.insert(districtNode)) {
            throw 1;
        }
    }
}

void
NIVissimDistrictConnection::dict_BuildDistricts(NBDistrictCont& dc,
        NBEdgeCont& ec,
        NBNodeCont& nc) {
    // add the sources and sinks
    //  their normalised probability is computed within NBDistrict
    //   to avoid double code writing and more securty within the converter
    //  go through the district table
    for (std::map<int, std::vector<int> >::iterator k = myDistrictsConnections.begin(); k != myDistrictsConnections.end(); k++) {
        // get the connections
        const std::vector<int>& connections = (*k).second;
        // retrieve the current district
        NBDistrict* district =
            dc.retrieve(toString<int>((*k).first));
        NBNode* districtNode = nc.retrieve("District" + district->getID());
        assert(district != 0 && districtNode != 0);

        for (std::vector<int>::const_iterator l = connections.begin(); l != connections.end(); l++) {
            NIVissimDistrictConnection* c = dictionary(*l);
            // get the edge to connect the parking place to
            NBEdge* e = ec.retrieve(toString<int>(c->myEdgeID));
            if (e == nullptr) {
                e = ec.retrievePossiblySplit(toString<int>(c->myEdgeID), c->myPosition);
            }
            if (e == nullptr) {
                WRITE_WARNING("Could not build district '" + toString<int>((*k).first) + "' - edge '" + toString<int>(c->myEdgeID) + "' is missing.");
                continue;
            }
            std::string id = "ParkingPlace" + toString<int>(*l);
            NBNode* parkingPlace = nc.retrieve(id);
            if (parkingPlace == nullptr) {
                double pos = c->getPosition();
                if (pos < e->getLength() - pos) {
                    parkingPlace = e->getFromNode();
                    parkingPlace->invalidateIncomingConnections();
                } else {
                    parkingPlace = e->getToNode();
                    parkingPlace->invalidateOutgoingConnections();
                }
            }
            assert(
                e->getToNode() == parkingPlace
                ||
                e->getFromNode() == parkingPlace);

            // build the connection to the source
            if (e->getFromNode() == parkingPlace) {
                id = "VissimFromParkingplace" + toString<int>((*k).first) + "-" + toString<int>(c->myID);
                NBEdge* source =
                    new NBEdge(id, districtNode, parkingPlace,
                               "Connection", c->getMeanSpeed(/*distc*/) / (double) 3.6, 3, -1,
                               NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET);
                if (!ec.insert(source)) { // !!! in den Konstruktor
                    throw 1; // !!!
                }
                double percNormed =
                    c->myPercentages[(*k).first];
                if (!district->addSource(source, percNormed)) {
                    throw 1;
                }
            }

            // build the connection to the destination
            if (e->getToNode() == parkingPlace) {
                id = "VissimToParkingplace"  + toString<int>((*k).first) + "-" + toString<int>(c->myID);
                NBEdge* destination =
                    new NBEdge(id, parkingPlace, districtNode,
                               "Connection", (double) 100 / (double) 3.6, 2, -1,
                               NBEdge::UNSPECIFIED_WIDTH, NBEdge::UNSPECIFIED_OFFSET);
                if (!ec.insert(destination)) { // !!! (in den Konstruktor)
                    throw 1; // !!!
                }
                double percNormed2 =
                    c->myPercentages[(*k).first];
                if (!district->addSink(destination, percNormed2)) {
                    throw 1; // !!!
                }
            }

            /*
            if(e->getToNode()==districtNode) {
            double percNormed =
                c->myPercentages[(*k).first];
            district->addSink(e, percNormed);
            }
            if(e->getFromNode()==districtNode) {
            double percNormed =
                c->myPercentages[(*k).first];
            district->addSource(e, percNormed);
            }
            */
        }

        /*
        // add them as sources and sinks to the current district
        for(std::vector<int>::const_iterator l=connections.begin(); l!=connections.end(); l++) {
            // get the current connections
            NIVissimDistrictConnection *c = dictionary(*l);
            // get the edge to connect the parking place to
            NBEdge *e = NBEdgeCont::retrieve(toString<int>(c->myEdgeID));
            Position edgepos = c->geomPosition();
            NBNode *edgeend = e->tryGetNodeAtPosition(c->myPosition,
                e->getLength()/4.0);
            if(edgeend==0) {
                // Edge splitting omitted on build district connections by now
                assert(false);
            }

            // build the district-node if not yet existing
            std::string id = "VissimParkingplace" + district->getID();
            NBNode *districtNode = nc.retrieve(id);
            assert(districtNode!=0);

            if(e->getToNode()==edgeend) {
                // build the connection to the source
                id = std::string("VissimFromParkingplace")
                    + toString<int>((*k).first) + "-"
                    + toString<int>(c->myID);
                NBEdge *source =
                    new NBEdge(id, id, districtNode, edgeend,
                    "Connection", 100/3.6, 2, 100, 0,
                    NBEdge::EDGEFUNCTION_SOURCE);
                NBEdgeCont::insert(source); // !!! (in den Konstruktor)
                double percNormed =
                    c->myPercentages[(*k).first];
                district->addSource(source, percNormed);
            } else {
                // build the connection to the destination
                id = std::string("VissimToParkingplace")
                    + toString<int>((*k).first) + "-"
                    + toString<int>(c->myID);
                NBEdge *destination =
                    new NBEdge(id, id, edgeend, districtNode,
                    "Connection", 100/3.6, 2, 100, 0,
                    NBEdge::EDGEFUNCTION_SINK);
                NBEdgeCont::insert(destination); // !!! (in den Konstruktor)

                // add both the source and the sink to the district
                double percNormed =
                    c->myPercentages[(*k).first];
                district->addSink(destination, percNormed);
            }
        }
        */
    }
}



Position
NIVissimDistrictConnection::geomPosition() const {
    NIVissimAbstractEdge* e = NIVissimEdge::dictionary(myEdgeID);
    return e->getGeomPosition(myPosition);
}


NIVissimDistrictConnection*
NIVissimDistrictConnection::dict_findForEdge(int edgeid) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        if ((*i).second->myEdgeID == edgeid) {
            return (*i).second;
        }
    }
    return nullptr;
}


void
NIVissimDistrictConnection::clearDict() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); i++) {
        delete (*i).second;
    }
    myDict.clear();
}


double
NIVissimDistrictConnection::getMeanSpeed() const {
    //assert(myAssignedVehicles.size()!=0);
    if (myAssignedVehicles.size() == 0) {
        WRITE_WARNING("No streams assigned at district'" + toString(myID) + "'.\n Using default speed 200km/h");
        return (double) 200 / (double) 3.6;
    }
    double speed = 0;
    std::vector<std::pair<int, int> >::const_iterator i;
    for (i = myAssignedVehicles.begin(); i != myAssignedVehicles.end(); i++) {
        speed += getRealSpeed((*i).second);
    }
    return speed / (double) myAssignedVehicles.size();
}


double
NIVissimDistrictConnection::getRealSpeed(int distNo) const {
    std::string id = toString<int>(distNo);
    Distribution* dist = DistributionCont::dictionary("speed", id);
    if (dist == nullptr) {
        WRITE_WARNING("The referenced speed distribution '" + id + "' is not known.");
        WRITE_WARNING(". Using default.");
        return OptionsCont::getOptions().getFloat("vissim.default-speed");
    }
    assert(dist != 0);
    double speed = dist->getMax();
    if (speed < 0 || speed > 1000) {
        WRITE_WARNING(" False speed at district '" + id);
        WRITE_WARNING(". Using default.");
        speed = OptionsCont::getOptions().getFloat("vissim.default-speed");
    }
    return speed;
}


/****************************************************************************/
