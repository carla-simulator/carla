/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2009-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSDevice_Vehroutes.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @date    Fri, 30.01.2009
///
// A device which collects info on the vehicle trip
/****************************************************************************/
#include <config.h>

#include <microsim/MSGlobals.h>
#include <microsim/MSNet.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdge.h>
#include <microsim/MSRoute.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleType.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <utils/vehicle/SUMOVehicle.h>
#include <utils/options/OptionsCont.h>
#include <utils/iodevices/OutputDevice_String.h>
#include <utils/xml/SUMOSAXAttributes.h>
#include "MSDevice_Vehroutes.h"


// ===========================================================================
// static member variables
// ===========================================================================
bool MSDevice_Vehroutes::mySaveExits = false;
bool MSDevice_Vehroutes::myLastRouteOnly = false;
bool MSDevice_Vehroutes::myDUAStyle = false;
bool MSDevice_Vehroutes::myWriteCosts = false;
bool MSDevice_Vehroutes::mySorted = false;
bool MSDevice_Vehroutes::myIntendedDepart = false;
bool MSDevice_Vehroutes::myRouteLength = false;
bool MSDevice_Vehroutes::mySkipPTLines = false;
bool MSDevice_Vehroutes::myIncludeIncomplete = false;
bool MSDevice_Vehroutes::myWriteStopPriorEdges = false;
MSDevice_Vehroutes::StateListener MSDevice_Vehroutes::myStateListener;
std::map<const SUMOTime, int> MSDevice_Vehroutes::myDepartureCounts;
std::map<const SUMOTime, std::map<const std::string, std::string> > MSDevice_Vehroutes::myRouteInfos;


// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static initialisation methods
// ---------------------------------------------------------------------------
void
MSDevice_Vehroutes::init() {
    const OptionsCont& oc = OptionsCont::getOptions();
    if (oc.isSet("vehroute-output")) {
        OutputDevice::createDeviceByOption("vehroute-output", "routes", "routes_file.xsd");
        mySaveExits = oc.getBool("vehroute-output.exit-times");
        myLastRouteOnly = oc.getBool("vehroute-output.last-route");
        myDUAStyle = oc.getBool("vehroute-output.dua");
        myWriteCosts = oc.getBool("vehroute-output.cost");
        mySorted = myDUAStyle || oc.getBool("vehroute-output.sorted");
        myIntendedDepart = oc.getBool("vehroute-output.intended-depart");
        myRouteLength = oc.getBool("vehroute-output.route-length");
        mySkipPTLines = oc.getBool("vehroute-output.skip-ptlines");
        myIncludeIncomplete = oc.getBool("vehroute-output.incomplete");
        myWriteStopPriorEdges = oc.getBool("vehroute-output.stop-edges");
        MSNet::getInstance()->addVehicleStateListener(&myStateListener);
    }
}


MSDevice_Vehroutes*
MSDevice_Vehroutes::buildVehicleDevices(SUMOVehicle& v, std::vector<MSVehicleDevice*>& into, int maxRoutes) {
    if (maxRoutes < std::numeric_limits<int>::max()) {
        return new MSDevice_Vehroutes(v, "vehroute_" + v.getID(), maxRoutes);
    }
    if (mySkipPTLines && v.getParameter().line != "") {
        return nullptr;
    }
    OptionsCont& oc = OptionsCont::getOptions();
    if (equippedByDefaultAssignmentOptions(oc, "vehroute", v, oc.isSet("vehroute-output"))) {
        if (myLastRouteOnly) {
            maxRoutes = 0;
        }
        myStateListener.myDevices[&v] = new MSDevice_Vehroutes(v, "vehroute_" + v.getID(), maxRoutes);
        into.push_back(myStateListener.myDevices[&v]);
        return myStateListener.myDevices[&v];
    }
    return nullptr;
}


// ---------------------------------------------------------------------------
// MSDevice_Vehroutes::StateListener-methods
// ---------------------------------------------------------------------------
void
MSDevice_Vehroutes::StateListener::vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info) {
    if (to == MSNet::VEHICLE_STATE_NEWROUTE) {
        const auto& deviceEntry = myDevices.find(vehicle);
        if (deviceEntry != myDevices.end()) {
            deviceEntry->second->addRoute(info);
        }
    }
}


// ---------------------------------------------------------------------------
// MSDevice_Vehroutes-methods
// ---------------------------------------------------------------------------
MSDevice_Vehroutes::MSDevice_Vehroutes(SUMOVehicle& holder, const std::string& id, int maxRoutes) :
    MSVehicleDevice(holder, id),
    myCurrentRoute(&holder.getRoute()),
    myMaxRoutes(maxRoutes),
    myLastSavedAt(nullptr),
    myDepartLane(-1),
    myDepartPos(-1),
    myDepartSpeed(-1),
    myDepartPosLat(0),
    myStopOut(2) {
    myCurrentRoute->addReference();
}


MSDevice_Vehroutes::~MSDevice_Vehroutes() {
    for (const RouteReplaceInfo& rri : myReplacedRoutes) {
        rri.route->release();
    }
    myCurrentRoute->release();
    myStateListener.myDevices.erase(&myHolder);
}


bool
MSDevice_Vehroutes::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
    if (reason == MSMoveReminder::NOTIFICATION_DEPARTED) {
        if (mySorted && myStateListener.myDevices[static_cast<SUMOVehicle*>(&veh)] == this) {
            const SUMOTime departure = myIntendedDepart ? myHolder.getParameter().depart : MSNet::getInstance()->getCurrentTimeStep();
            myDepartureCounts[departure]++;
        }
        if (!MSGlobals::gUseMesoSim) {
            const MSVehicle& vehicle = static_cast<MSVehicle&>(veh);
            myDepartLane = vehicle.getLane()->getIndex();
            myDepartPosLat = vehicle.getLateralPositionOnLane();
        }
        myDepartSpeed = veh.getSpeed();
        myDepartPos = veh.getPositionOnLane();
    }
    if (myWriteStopPriorEdges) {
        myPriorEdges.push_back(&enteredLane->getEdge());
    }
    return mySaveExits || myWriteStopPriorEdges;
}


bool
MSDevice_Vehroutes::notifyLeave(SUMOTrafficObject& veh, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (mySaveExits && reason != NOTIFICATION_LANE_CHANGE) {
        if (reason != NOTIFICATION_TELEPORT && myLastSavedAt == veh.getEdge()) { // need to check this for internal lanes
            myExits.back() = MSNet::getInstance()->getCurrentTimeStep();
        } else if (myLastSavedAt != veh.getEdge()) {
            myExits.push_back(MSNet::getInstance()->getCurrentTimeStep());
            myLastSavedAt = veh.getEdge();
        }
    }
    return mySaveExits || myWriteStopPriorEdges;
}


void
MSDevice_Vehroutes::stopEnded(const SUMOVehicleParameter::Stop& stop) {
    stop.write(myStopOut, !myWriteStopPriorEdges);
    if (myWriteStopPriorEdges) {
        myStopOut.writeAttr("priorEdges", myPriorEdges);
        myPriorEdges.clear();
        myStopOut.closeTag();
    }
}


void
MSDevice_Vehroutes::writeXMLRoute(OutputDevice& os, int index) const {
    if (index == 0 && !myIncludeIncomplete && myReplacedRoutes[index].route->size() == 2 &&
            myReplacedRoutes[index].route->getEdges().front()->isTazConnector() &&
            myReplacedRoutes[index].route->getEdges().back()->isTazConnector()) {
        return;
    }
    // check if a previous route shall be written
    os.openTag(SUMO_TAG_ROUTE);
    if (index >= 0) {
        assert((int)myReplacedRoutes.size() > index);
        if (myDUAStyle || myWriteCosts) {
            os.writeAttr(SUMO_ATTR_COST, myReplacedRoutes[index].route->getCosts());
        }
        if (myWriteCosts) {
            os.writeAttr(SUMO_ATTR_SAVINGS, myReplacedRoutes[index].route->getSavings());
        }
        // write edge on which the vehicle was when the route was valid
        os.writeAttr("replacedOnEdge", (myReplacedRoutes[index].edge ?
                                        myReplacedRoutes[index].edge->getID() : ""));
        // write the reason for replacement
        os.writeAttr("reason", myReplacedRoutes[index].info);

        // write the time at which the route was replaced
        os.writeAttr("replacedAtTime", time2string(myReplacedRoutes[index].time));
        os.writeAttr(SUMO_ATTR_PROB, "0");
        os << " edges=\"";
        // get the route
        int i = index;
        while (i > 0 && myReplacedRoutes[i - 1].edge != nullptr && !myIncludeIncomplete) {
            i--;
        }
        const MSEdge* lastEdge = nullptr;
        for (; i < index; ++i) {
            myReplacedRoutes[i].route->writeEdgeIDs(os, lastEdge, myReplacedRoutes[i].edge);
            lastEdge = myReplacedRoutes[i].edge;
        }
        myReplacedRoutes[index].route->writeEdgeIDs(os, lastEdge);
        os << "\"";
    } else {
        if (myDUAStyle || myWriteCosts) {
            os.writeAttr(SUMO_ATTR_COST, myHolder.getRoute().getCosts());
        }
        if (myWriteCosts) {
            os.writeAttr(SUMO_ATTR_SAVINGS, myHolder.getRoute().getSavings());
        }
        os << " edges=\"";
        const MSEdge* lastEdge = nullptr;
        int numWritten = 0;
        if (myHolder.getNumberReroutes() > 0) {
            assert((int)myReplacedRoutes.size() <= myHolder.getNumberReroutes());
            int i = (int)myReplacedRoutes.size();
            while (i > 0 && myReplacedRoutes[i - 1].edge) {
                i--;
            }
            for (; i < (int)myReplacedRoutes.size(); ++i) {
                numWritten += myReplacedRoutes[i].route->writeEdgeIDs(os, lastEdge, myReplacedRoutes[i].edge);
                lastEdge = myReplacedRoutes[i].edge;
            }
        }
        numWritten += myCurrentRoute->writeEdgeIDs(os, lastEdge, nullptr);
        os << "\"";

        if (mySaveExits) {
            std::vector<std::string> exits;
            for (SUMOTime t : myExits) {
                exits.push_back(time2string(t));
            }
            assert(numWritten >= (int)myExits.size());
            std::vector<std::string> missing(numWritten - (int)myExits.size(), "-1");
            exits.insert(exits.end(), missing.begin(), missing.end());
            os.writeAttr("exitTimes", exits);
        }
    }
    os.closeTag();
}


void
MSDevice_Vehroutes::generateOutput(OutputDevice* /*tripinfoOut*/) const {
    writeOutput(true);
}


void
MSDevice_Vehroutes::writeOutput(const bool hasArrived) const {
    OutputDevice& routeOut = OutputDevice::getDeviceByOption("vehroute-output");
    OutputDevice_String od(1);
    SUMOVehicleParameter tmp = myHolder.getParameter();
    tmp.depart = myIntendedDepart ? myHolder.getParameter().depart : myHolder.getDeparture();
    if (!MSGlobals::gUseMesoSim) {
        if (tmp.wasSet(VEHPARS_DEPARTLANE_SET)) {
            tmp.departLaneProcedure = DepartLaneDefinition::GIVEN;
            tmp.departLane = myDepartLane;
        }
        if (tmp.wasSet(VEHPARS_DEPARTPOSLAT_SET)) {
            tmp.departPosLatProcedure = DepartPosLatDefinition::GIVEN;
            tmp.departPosLat = myDepartPosLat;
        }
    }
    if (tmp.wasSet(VEHPARS_DEPARTPOS_SET)) {
        tmp.departPosProcedure = DepartPosDefinition::GIVEN;
        tmp.departPos = myDepartPos;
    }
    if (tmp.wasSet(VEHPARS_DEPARTSPEED_SET)) {
        tmp.departSpeedProcedure = DepartSpeedDefinition::GIVEN;
        tmp.departSpeed = myDepartSpeed;
    }
    const std::string typeID = myHolder.getVehicleType().getID() != DEFAULT_VTYPE_ID ? myHolder.getVehicleType().getID() : "";
    tmp.write(od, OptionsCont::getOptions(), SUMO_TAG_VEHICLE, typeID);
    if (hasArrived) {
        od.writeAttr("arrival", time2string(MSNet::getInstance()->getCurrentTimeStep()));
        if (myRouteLength) {
            const bool includeInternalLengths = MSGlobals::gUsingInternalLanes && MSNet::getInstance()->hasInternalLinks();
            const double routeLength = myHolder.getRoute().getDistanceBetween(myHolder.getDepartPos(), myHolder.getArrivalPos(),
                                       myHolder.getRoute().begin(), myHolder.getCurrentRouteEdge(), includeInternalLengths);
            od.writeAttr("routeLength", routeLength);
        }
    }
    if (myDUAStyle) {
        const RandomDistributor<const MSRoute*>* const routeDist = MSRoute::distDictionary("!" + myHolder.getID());
        if (routeDist != nullptr) {
            const std::vector<const MSRoute*>& routes = routeDist->getVals();
            unsigned index = 0;
            while (index < routes.size() && routes[index] != myCurrentRoute) {
                ++index;
            }
            od.openTag(SUMO_TAG_ROUTE_DISTRIBUTION).writeAttr(SUMO_ATTR_LAST, index);
            const std::vector<double>& probs = routeDist->getProbs();
            for (int i = 0; i < (int)routes.size(); ++i) {
                od.setPrecision();
                od.openTag(SUMO_TAG_ROUTE);
                od.writeAttr(SUMO_ATTR_COST, routes[i]->getCosts());
                if (myWriteCosts) {
                    od.writeAttr(SUMO_ATTR_SAVINGS, routes[i]->getSavings());
                }
                od.setPrecision(8);
                od.writeAttr(SUMO_ATTR_PROB, probs[i]);
                od.setPrecision();
                od << " edges=\"";
                routes[i]->writeEdgeIDs(od, *routes[i]->begin());
                (od << "\"").closeTag();
            }
            od.closeTag();
        } else {
            writeXMLRoute(od);
        }
    } else {
        const int routesToSkip = myHolder.getParameter().wasSet(VEHPARS_FORCE_REROUTE) && !myIncludeIncomplete ? 1 : 0;
        if ((int)myReplacedRoutes.size() > routesToSkip) {
            od.openTag(SUMO_TAG_ROUTE_DISTRIBUTION);
            for (int i = routesToSkip; i < (int)myReplacedRoutes.size(); ++i) {
                writeXMLRoute(od, i);
            }
            writeXMLRoute(od);
            od.closeTag();
        } else {
            writeXMLRoute(od);
        }
    }
    od << myStopOut.getString();
    myHolder.getParameter().writeParams(od);
    od.closeTag();
    od.lf();
    if (mySorted) {
        myRouteInfos[tmp.depart][myHolder.getID()] = od.getString();
        myDepartureCounts[tmp.depart]--;
        std::map<const SUMOTime, int>::iterator it = myDepartureCounts.begin();
        while (it != myDepartureCounts.end() && it->second == 0) {
            std::map<const std::string, std::string>& infos = myRouteInfos[it->first];
            for (std::map<const std::string, std::string>::const_iterator it2 = infos.begin(); it2 != infos.end(); ++it2) {
                routeOut << it2->second;
            }
            myRouteInfos.erase(it->first);
            myDepartureCounts.erase(it);
            it = myDepartureCounts.begin();
        }
    } else {
        routeOut << od.getString();
    }
}


const MSRoute*
MSDevice_Vehroutes::getRoute(int index) const {
    if (index < (int)myReplacedRoutes.size()) {
        return myReplacedRoutes[index].route;
    } else {
        return nullptr;
    }
}


void
MSDevice_Vehroutes::addRoute(const std::string& info) {
    if (myMaxRoutes > 0) {
        if (myHolder.hasDeparted()) {
            myReplacedRoutes.push_back(RouteReplaceInfo(myHolder.getEdge(), MSNet::getInstance()->getCurrentTimeStep(), myCurrentRoute, info));
        } else {
            myReplacedRoutes.push_back(RouteReplaceInfo(nullptr, MSNet::getInstance()->getCurrentTimeStep(), myCurrentRoute, info));
        }
        if ((int)myReplacedRoutes.size() > myMaxRoutes) {
            myReplacedRoutes.front().route->release();
            myReplacedRoutes.erase(myReplacedRoutes.begin());
        }
    } else {
        myCurrentRoute->release();
    }
    myCurrentRoute = &myHolder.getRoute();
    myCurrentRoute->addReference();
}


void
MSDevice_Vehroutes::generateOutputForUnfinished() {
    for (const auto& it : myStateListener.myDevices) {
        if (it.first->hasDeparted()) {
            it.second->writeOutput(false);
        }
    }
    // unfinished persons
    MSNet* net = MSNet::getInstance();
    if (net->hasPersons()) {
        MSTransportableControl& pc = net->getPersonControl();
        while (pc.loadedBegin() != pc.loadedEnd()) {
            pc.erase(pc.loadedBegin()->second);
        }
    }
}


void
MSDevice_Vehroutes::saveState(OutputDevice& out) const {
    out.openTag(SUMO_TAG_DEVICE);
    out.writeAttr(SUMO_ATTR_ID, getID());
    std::vector<std::string> internals;
    if (!MSGlobals::gUseMesoSim) {
        internals.push_back(toString(myDepartLane));
        internals.push_back(toString(myDepartPosLat));
    }
    internals.push_back(toString(myDepartSpeed));
    internals.push_back(toString(myDepartPos));
    internals.push_back(toString(myReplacedRoutes.size()));
    for (int i = 0; i < (int)myReplacedRoutes.size(); ++i) {
        const std::string replacedOnEdge = myReplacedRoutes[i].edge == nullptr ? "!NULL" : myReplacedRoutes[i].edge->getID();
        internals.push_back(replacedOnEdge);
        internals.push_back(toString(myReplacedRoutes[i].time));
        internals.push_back(myReplacedRoutes[i].route->getID());
        internals.push_back(myReplacedRoutes[i].info);
    }
    out.writeAttr(SUMO_ATTR_STATE, toString(internals));
    out.closeTag();
}


void
MSDevice_Vehroutes::loadState(const SUMOSAXAttributes& attrs) {
    std::istringstream bis(attrs.getString(SUMO_ATTR_STATE));
    if (!MSGlobals::gUseMesoSim) {
        bis >> myDepartLane;
        bis >> myDepartPosLat;
    }
    bis >> myDepartSpeed;
    bis >> myDepartPos;
    int size;
    bis >> size;
    for (int i = 0; i < size; ++i) {
        std::string edgeID;
        SUMOTime time;
        std::string routeID;
        std::string info;
        bis >> edgeID;
        bis >> time;
        bis >> routeID;
        bis >> info;
        const MSRoute* route = MSRoute::dictionary(routeID);
        route->addReference();
        myReplacedRoutes.push_back(RouteReplaceInfo(MSEdge::dictionary(edgeID), time, route, info));
    }
}


/****************************************************************************/
