/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    Helper.cpp
/// @author  Laura Bieker-Walz
/// @author  Robert Hilbrich
/// @author  Leonhard Luecken
/// @date    15.09.2017
///
// C++ TraCI client API implementation
/****************************************************************************/
#include <config.h>

#include <cstring>
#include <utils/geom/GeomHelper.h>
#include <utils/geom/GeoConvHelper.h>
#include <microsim/MSNet.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/transportables/MSTransportable.h>
#include <microsim/transportables/MSPerson.h>
#include <libsumo/TraCIDefs.h>
#include <libsumo/Edge.h>
#include <libsumo/InductionLoop.h>
#include <libsumo/Junction.h>
#include <libsumo/Lane.h>
#include <libsumo/LaneArea.h>
#include <libsumo/MultiEntryExit.h>
#include <libsumo/Person.h>
#include <libsumo/POI.h>
#include <libsumo/Polygon.h>
#include <libsumo/Route.h>
#include <libsumo/Simulation.h>
#include <libsumo/TrafficLight.h>
#include <libsumo/Vehicle.h>
#include <libsumo/VehicleType.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"

#define FAR_AWAY 1000.0

//#define DEBUG_MOVEXY
//#define DEBUG_MOVEXY_ANGLE
//#define DEBUG_SURROUNDING

void
LaneStoringVisitor::add(const MSLane* const l) const {
    switch (myDomain) {
        case libsumo::CMD_GET_VEHICLE_VARIABLE: {
            for (const MSVehicle* veh : l->getVehiclesSecure()) {
                if (myShape.distance2D(veh->getPosition()) <= myRange) {
                    myObjects.insert(veh);
                }
            }
            for (const MSVehicle* veh : l->getParkingVehicles()) {
                if (myShape.distance2D(veh->getPosition()) <= myRange) {
                    myObjects.insert(veh);
                }
            }
            l->releaseVehicles();
        }
        break;
        case libsumo::CMD_GET_PERSON_VARIABLE: {
            l->getVehiclesSecure();
            std::vector<MSTransportable*> persons = l->getEdge().getSortedPersons(MSNet::getInstance()->getCurrentTimeStep(), true);
            for (auto p : persons) {
                if (myShape.distance2D(p->getPosition()) <= myRange) {
                    myObjects.insert(p);
                }
            }
            l->releaseVehicles();
        }
        break;
        case libsumo::CMD_GET_EDGE_VARIABLE: {
            if (myShape.size() != 1 || l->getShape().distance2D(myShape[0]) <= myRange) {
                myObjects.insert(&l->getEdge());
            }
        }
        break;
        case libsumo::CMD_GET_LANE_VARIABLE: {
            if (myShape.size() != 1 || l->getShape().distance2D(myShape[0]) <= myRange) {
                myObjects.insert(l);
            }
        }
        break;
        default:
            break;

    }
}

namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
std::vector<Subscription> Helper::mySubscriptions;
Subscription* Helper::myLastContextSubscription = nullptr;
std::map<int, std::shared_ptr<VariableWrapper> > Helper::myWrapper;
Helper::VehicleStateListener Helper::myVehicleStateListener;
LANE_RTREE_QUAL* Helper::myLaneTree;
std::map<std::string, MSVehicle*> Helper::myRemoteControlledVehicles;
std::map<std::string, MSPerson*> Helper::myRemoteControlledPersons;


// ===========================================================================
// static member definitions
// ===========================================================================

void
Helper::debugPrint(const SUMOTrafficObject* veh) {
    if (veh != nullptr) {
        if (veh->isVehicle()) {
            std::cout << "  '" << veh->getID() << "' on lane '" << ((SUMOVehicle*)veh)->getLane()->getID() << "'\n";
        } else {
            std::cout << "  '" << veh->getID() << "' on edge '" << veh->getEdge()->getID() << "'\n";
        }
    }
}


void
Helper::subscribe(const int commandId, const std::string& id, const std::vector<int>& variables,
                  const double beginTime, const double endTime, const int contextDomain, const double range) {
    myLastContextSubscription = nullptr;
    if (variables.empty()) {
        for (std::vector<libsumo::Subscription>::iterator j = mySubscriptions.begin(); j != mySubscriptions.end();) {
            if (j->id == id && j->commandId == commandId && j->contextDomain == contextDomain) {
                j = mySubscriptions.erase(j);
            } else {
                ++j;
            }
        }
        return;
    }
    std::vector<std::vector<unsigned char> > parameters(variables.size());
    const SUMOTime begin = beginTime == INVALID_DOUBLE_VALUE ? 0 : TIME2STEPS(beginTime);
    const SUMOTime end = endTime == INVALID_DOUBLE_VALUE || endTime > STEPS2TIME(SUMOTime_MAX) ? SUMOTime_MAX : TIME2STEPS(endTime);
    libsumo::Subscription s(commandId, id, variables, parameters, begin, end, contextDomain, range);
    if (s.variables.size() == 1 && s.variables.front() == -1) {
        s.variables.clear();
    }
    handleSingleSubscription(s);
    libsumo::Subscription* modifiedSubscription = nullptr;
    needNewSubscription(s, mySubscriptions, modifiedSubscription);
    if (modifiedSubscription->isVehicleToVehicleContextSubscription()
            || modifiedSubscription->isVehicleToPersonContextSubscription()) {
        // Set last modified vehicle context subscription active for filter modifications
        myLastContextSubscription = modifiedSubscription;
    }
}


void
Helper::addSubscriptionParam(double param) {
    std::vector<unsigned char> dest(sizeof(param));
    std::memcpy(dest.data(), &param, sizeof(param));
    mySubscriptions.back().parameters.pop_back();
    mySubscriptions.back().parameters.emplace_back(dest);
}


void
Helper::handleSubscriptions(const SUMOTime t) {
    for (auto& wrapper : myWrapper) {
        wrapper.second->clear();
    }
    for (std::vector<libsumo::Subscription>::iterator i = mySubscriptions.begin(); i != mySubscriptions.end();) {
        const libsumo::Subscription& s = *i;
        const bool isArrivedVehicle = (s.commandId == CMD_SUBSCRIBE_VEHICLE_VARIABLE || s.commandId == CMD_SUBSCRIBE_VEHICLE_CONTEXT)
                                      && (find(getVehicleStateChanges(MSNet::VEHICLE_STATE_ARRIVED).begin(), getVehicleStateChanges(MSNet::VEHICLE_STATE_ARRIVED).end(), s.id) != getVehicleStateChanges(MSNet::VEHICLE_STATE_ARRIVED).end());
        const bool isArrivedPerson = (s.commandId == libsumo::CMD_SUBSCRIBE_PERSON_VARIABLE || s.commandId == libsumo::CMD_SUBSCRIBE_PERSON_CONTEXT)
                                     && MSNet::getInstance()->getPersonControl().get(s.id) == nullptr;
        if (s.endTime < t || isArrivedVehicle || isArrivedPerson) {
            i = mySubscriptions.erase(i);
            continue;
        }
        ++i;
    }
    for (const libsumo::Subscription& s : mySubscriptions) {
        if (s.beginTime <= t) {
            handleSingleSubscription(s);
        }
    }
}


bool
Helper::needNewSubscription(libsumo::Subscription& s, std::vector<Subscription>& subscriptions, libsumo::Subscription*& modifiedSubscription) {
    for (libsumo::Subscription& o : subscriptions) {
        if (s.commandId == o.commandId && s.id == o.id &&
                s.beginTime == o.beginTime && s.endTime == o.endTime &&
                s.contextDomain == o.contextDomain && s.range == o.range) {
            std::vector<std::vector<unsigned char> >::const_iterator k = s.parameters.begin();
            for (const int v : s.variables) {
                const int offset = (int)(std::find(o.variables.begin(), o.variables.end(), v) - o.variables.begin());
                if (offset == (int)o.variables.size() || o.parameters[offset] != *k) {
                    o.variables.push_back(v);
                    o.parameters.push_back(*k);
                }
                ++k;
            }
            modifiedSubscription = &o;
            return false;
        }
    }
    subscriptions.push_back(s);
    modifiedSubscription = &subscriptions.back();
    return true;
}


void
Helper::clearSubscriptions() {
    mySubscriptions.clear();
    myLastContextSubscription = nullptr;
}


Subscription*
Helper::addSubscriptionFilter(SubscriptionFilterType filter) {
    if (myLastContextSubscription != nullptr) {
        myLastContextSubscription->activeFilters |= filter;
    } else {
        WRITE_WARNING("addSubscriptionFilter: No previous vehicle context subscription exists to apply the context filter.");
    }
    return myLastContextSubscription;
}


void
Helper::handleSingleSubscription(const Subscription& s) {
    const int getCommandId = s.contextDomain > 0 ? s.contextDomain : s.commandId - 0x30;
    std::set<std::string> objIDs;
    if (s.contextDomain > 0) {
        if ((s.activeFilters & SUBS_FILTER_NO_RTREE) == 0) {
            PositionVector shape;
            findObjectShape(s.commandId, s.id, shape);
            collectObjectIDsInRange(s.contextDomain, shape, s.range, objIDs);
        }
        applySubscriptionFilters(s, objIDs);
    } else {
        objIDs.insert(s.id);
    }
    if (myWrapper.empty()) {
        myWrapper[libsumo::CMD_GET_EDGE_VARIABLE] = Edge::makeWrapper();
        myWrapper[libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE] = InductionLoop::makeWrapper();
        myWrapper[libsumo::CMD_GET_JUNCTION_VARIABLE] = Junction::makeWrapper();
        myWrapper[libsumo::CMD_GET_LANE_VARIABLE] = Lane::makeWrapper();
        myWrapper[libsumo::CMD_GET_LANEAREA_VARIABLE] = LaneArea::makeWrapper();
        myWrapper[libsumo::CMD_GET_MULTIENTRYEXIT_VARIABLE] = MultiEntryExit::makeWrapper();
        myWrapper[libsumo::CMD_GET_PERSON_VARIABLE] = Person::makeWrapper();
        myWrapper[libsumo::CMD_GET_POI_VARIABLE] = POI::makeWrapper();
        myWrapper[libsumo::CMD_GET_POLYGON_VARIABLE] = Polygon::makeWrapper();
        myWrapper[libsumo::CMD_GET_ROUTE_VARIABLE] = Route::makeWrapper();
        myWrapper[libsumo::CMD_GET_SIM_VARIABLE] = Simulation::makeWrapper();
        myWrapper[libsumo::CMD_GET_TL_VARIABLE] = TrafficLight::makeWrapper();
        myWrapper[libsumo::CMD_GET_VEHICLE_VARIABLE] = Vehicle::makeWrapper();
        myWrapper[libsumo::CMD_GET_VEHICLETYPE_VARIABLE] = VehicleType::makeWrapper();
    }
    auto wrapper = myWrapper.find(getCommandId);
    if (wrapper == myWrapper.end()) {
        throw TraCIException("Unsupported command specified");
    }
    std::shared_ptr<VariableWrapper> handler = wrapper->second;
    VariableWrapper* container = handler.get();
    if (s.contextDomain > 0) {
        auto containerWrapper = myWrapper.find(s.commandId + 0x20);
        if (containerWrapper == myWrapper.end()) {
            throw TraCIException("Unsupported domain specified");
        }
        container = containerWrapper->second.get();
        container->setContext(s.id);
    } else {
        container->setContext("");
    }
    for (const std::string& objID : objIDs) {
        if (!s.variables.empty()) {
            std::vector<std::vector<unsigned char> >::const_iterator k = s.parameters.begin();
            for (const int variable : s.variables) {
                if (!k->empty()) {
                    container->setParams(&*k);
                }
                handler->handle(objID, variable, container);
                if (!k->empty()) {
                    container->setParams(nullptr);
                }
                ++k;
            }
        } else {
            if (s.contextDomain == 0 && getCommandId == libsumo::CMD_GET_VEHICLE_VARIABLE) {
                // default for vehicles is edge id and lane position
                handler->handle(objID, VAR_ROAD_ID, container);
                handler->handle(objID, VAR_LANEPOSITION, container);
            } else if (s.contextDomain > 0 || !handler->handle(objID, libsumo::LAST_STEP_VEHICLE_NUMBER, container)) {
                // default for detectors is vehicle number, for all others (and contexts) id list
                handler->handle(objID, libsumo::TRACI_ID_LIST, container);
            }
        }
    }
}



void
Helper::fuseLaneCoverage(std::shared_ptr<LaneCoverageInfo> aggregatedLaneCoverage, const std::shared_ptr<LaneCoverageInfo> newLaneCoverage) {
    for (auto& p : *newLaneCoverage) {
        const MSLane* lane = p.first;
        if (aggregatedLaneCoverage->find(lane) == aggregatedLaneCoverage->end()) {
            // Lane has no coverage in aggregatedLaneCoverage, yet
            (*aggregatedLaneCoverage)[lane] = (*newLaneCoverage)[lane];
        } else {
            // Lane is covered in aggregatedLaneCoverage as well
            std::pair<double, double>& range1 = (*aggregatedLaneCoverage)[lane];
            std::pair<double, double>& range2 = (*newLaneCoverage)[lane];
            std::pair<double, double> hull = std::make_pair(MIN2(range1.first, range2.first), MAX2(range1.second, range2.second));
            (*aggregatedLaneCoverage)[lane] = hull;
        }
    }
}


TraCIPositionVector
Helper::makeTraCIPositionVector(const PositionVector& positionVector) {
    TraCIPositionVector tp;
    for (int i = 0; i < (int)positionVector.size(); ++i) {
        tp.push_back(makeTraCIPosition(positionVector[i]));
    }
    return tp;
}


PositionVector
Helper::makePositionVector(const TraCIPositionVector& vector) {
    PositionVector pv;
    for (const TraCIPosition& pos : vector) {
        if (std::isnan(pos.x) || std::isnan(pos.y)) {
            throw libsumo::TraCIException("NaN-Value in shape.");
        }
        pv.push_back(Position(pos.x, pos.y));
    }
    return pv;
}


TraCIColor
Helper::makeTraCIColor(const RGBColor& color) {
    TraCIColor tc;
    tc.a = color.alpha();
    tc.b = color.blue();
    tc.g = color.green();
    tc.r = color.red();
    return tc;
}


RGBColor
Helper::makeRGBColor(const TraCIColor& c) {
    return RGBColor((unsigned char)c.r, (unsigned char)c.g, (unsigned char)c.b, (unsigned char)c.a);
}


TraCIPosition
Helper::makeTraCIPosition(const Position& position, const bool includeZ) {
    TraCIPosition p;
    p.x = position.x();
    p.y = position.y();
    p.z = includeZ ? position.z() : INVALID_DOUBLE_VALUE;
    return p;
}


Position
Helper::makePosition(const TraCIPosition& tpos) {
    return Position(tpos.x, tpos.y, tpos.z);
}


MSEdge*
Helper::getEdge(const std::string& edgeID) {
    MSEdge* edge = MSEdge::dictionary(edgeID);
    if (edge == nullptr) {
        throw TraCIException("Referenced edge '" + edgeID + "' is not known.");
    }
    return edge;
}


const MSLane*
Helper::getLaneChecking(const std::string& edgeID, int laneIndex, double pos) {
    const MSEdge* edge = MSEdge::dictionary(edgeID);
    if (edge == nullptr) {
        throw TraCIException("Unknown edge " + edgeID);
    }
    if (laneIndex < 0 || laneIndex >= (int)edge->getLanes().size()) {
        throw TraCIException("Invalid lane index for " + edgeID);
    }
    const MSLane* lane = edge->getLanes()[laneIndex];
    if (pos < 0 || pos > lane->getLength()) {
        throw TraCIException("Position on lane invalid");
    }
    return lane;
}


std::pair<MSLane*, double>
Helper::convertCartesianToRoadMap(const Position& pos, const SUMOVehicleClass vClass) {
    const PositionVector shape({ pos });
    std::pair<MSLane*, double> result;
    double range = 1000.;
    const Boundary& netBounds = GeoConvHelper::getFinal().getConvBoundary();
    const double maxRange = MAX2(1001., netBounds.getWidth() + netBounds.getHeight() + netBounds.distanceTo2D(pos));
    while (range < maxRange) {
        std::set<const Named*> lanes;
        collectObjectsInRange(libsumo::CMD_GET_LANE_VARIABLE, shape, range, lanes);
        double minDistance = std::numeric_limits<double>::max();
        for (const Named* named : lanes) {
            MSLane* lane = const_cast<MSLane*>(dynamic_cast<const MSLane*>(named));
            if (lane->allowsVehicleClass(vClass)) {
                // @todo this may be a place where 3D is required but 2D is used
                const double newDistance = lane->getShape().distance2D(pos);
                if (newDistance < minDistance) {
                    minDistance = newDistance;
                    result.first = lane;
                }
            }
        }
        if (minDistance < std::numeric_limits<double>::max()) {
            result.second = result.first->interpolateGeometryPosToLanePos(result.first->getShape().nearest_offset_to_point2D(pos, false));
            break;
        }
        range *= 2;
    }
    return result;
}


double
Helper::getDrivingDistance(std::pair<const MSLane*, double>& roadPos1, std::pair<const MSLane*, double>& roadPos2) {
    if (roadPos1.first == roadPos2.first && roadPos1.second <= roadPos2.second) {
        // same edge
        return roadPos2.second - roadPos1.second;
    }
    double distance = 0.0;
    ConstMSEdgeVector newRoute;
    while (roadPos2.first->isInternal() && roadPos2.first != roadPos1.first) {
        distance += roadPos2.second;
        roadPos2.first = roadPos2.first->getLogicalPredecessorLane();
        roadPos2.second = roadPos2.first->getLength();
    }
    MSNet::getInstance()->getRouterTT(0).compute(&roadPos1.first->getEdge(), &roadPos2.first->getEdge(), nullptr, SIMSTEP, newRoute, true);
    if (newRoute.empty()) {
        return libsumo::INVALID_DOUBLE_VALUE;
    }
    MSRoute route("", newRoute, false, nullptr, std::vector<SUMOVehicleParameter::Stop>());
    return distance + route.getDistanceBetween(roadPos1.second, roadPos2.second, &roadPos1.first->getEdge(), &roadPos2.first->getEdge());
}


MSBaseVehicle*
Helper::getVehicle(const std::string& id) {
    SUMOVehicle* sumoVehicle = MSNet::getInstance()->getVehicleControl().getVehicle(id);
    if (sumoVehicle == nullptr) {
        throw TraCIException("Vehicle '" + id + "' is not known.");
    }
    MSBaseVehicle* v = dynamic_cast<MSBaseVehicle*>(sumoVehicle);
    if (v == nullptr) {
        throw TraCIException("Vehicle '" + id + "' is not a proper vehicle.");
    }
    return v;
}


MSPerson*
Helper::getPerson(const std::string& personID) {
    MSTransportableControl& c = MSNet::getInstance()->getPersonControl();
    MSPerson* p = dynamic_cast<MSPerson*>(c.get(personID));
    if (p == nullptr) {
        throw TraCIException("Person '" + personID + "' is not known");
    }
    return p;
}

SUMOTrafficObject*
Helper::getTrafficObject(int domain, const std::string& id) {
    if (domain == CMD_GET_VEHICLE_VARIABLE) {
        return getVehicle(id);
    } else if (domain == CMD_GET_PERSON_VARIABLE) {
        return getPerson(id);
    } else {
        throw TraCIException("Cannot retrieve traffic object for domain " + toString(domain));
    }
}

const MSVehicleType&
Helper::getVehicleType(const std::string& vehicleID) {
    return getVehicle(vehicleID)->getVehicleType();
}


void
Helper::cleanup() {
    // clean up NamedRTrees
    Polygon::cleanup();
    POI::cleanup();
    InductionLoop::cleanup();
    Junction::cleanup();
    delete myLaneTree;
    myLaneTree = nullptr;
}


void
Helper::registerVehicleStateListener() {
    if (MSNet::hasInstance()) {
        MSNet::getInstance()->addVehicleStateListener(&myVehicleStateListener);
    }
}


const std::vector<std::string>&
Helper::getVehicleStateChanges(const MSNet::VehicleState state) {
    return myVehicleStateListener.myVehicleStateChanges[state];
}


void
Helper::clearVehicleStates() {
    for (auto& i : myVehicleStateListener.myVehicleStateChanges) {
        i.second.clear();
    }
}


void
Helper::findObjectShape(int domain, const std::string& id, PositionVector& shape) {
    switch (domain) {
        case libsumo::CMD_SUBSCRIBE_INDUCTIONLOOP_CONTEXT:
            InductionLoop::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_LANE_CONTEXT:
            Lane::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_VEHICLE_CONTEXT:
            Vehicle::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_PERSON_CONTEXT:
            Person::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_POI_CONTEXT:
            POI::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_POLYGON_CONTEXT:
            Polygon::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_JUNCTION_CONTEXT:
            Junction::storeShape(id, shape);
            break;
        case libsumo::CMD_SUBSCRIBE_EDGE_CONTEXT:
            Edge::storeShape(id, shape);
            break;
        default:
            break;
    }
}


void
Helper::collectObjectIDsInRange(int domain, const PositionVector& shape, double range, std::set<std::string>& into) {
    std::set<const Named*> objects;
    collectObjectsInRange(domain, shape, range, objects);
    for (const Named* obj : objects) {
        into.insert(obj->getID());
    }
}


void
Helper::collectObjectsInRange(int domain, const PositionVector& shape, double range, std::set<const Named*>& into) {
    const Boundary b = shape.getBoxBoundary().grow(range);
    const float cmin[2] = {(float) b.xmin(), (float) b.ymin()};
    const float cmax[2] = {(float) b.xmax(), (float) b.ymax()};
    Named::StoringVisitor sv(into);
    switch (domain) {
        case libsumo::CMD_GET_INDUCTIONLOOP_VARIABLE:
            InductionLoop::getTree()->Search(cmin, cmax, sv);
            break;
        case libsumo::CMD_GET_POI_VARIABLE:
            POI::getTree()->Search(cmin, cmax, sv);
            break;
        case libsumo::CMD_GET_POLYGON_VARIABLE:
            Polygon::getTree()->Search(cmin, cmax, sv);
            break;
        case libsumo::CMD_GET_JUNCTION_VARIABLE:
            Junction::getTree()->Search(cmin, cmax, sv);
            break;
        case libsumo::CMD_GET_EDGE_VARIABLE:
        case libsumo::CMD_GET_LANE_VARIABLE:
        case libsumo::CMD_GET_PERSON_VARIABLE:
        case libsumo::CMD_GET_VEHICLE_VARIABLE: {
            if (myLaneTree == nullptr) {
                myLaneTree = new LANE_RTREE_QUAL(&MSLane::visit);
                MSLane::fill(*myLaneTree);
            }
            LaneStoringVisitor lsv(into, shape, range, domain);
            myLaneTree->Search(cmin, cmax, lsv);
        }
        break;
        default:
            break;
    }
}



void
Helper::applySubscriptionFilters(const Subscription& s, std::set<std::string>& objIDs) {
#ifdef DEBUG_SURROUNDING
    MSBaseVehicle* _veh = getVehicle(s.id);
    std::cout << SIMTIME << " applySubscriptionFilters for vehicle '" << _veh->getID() << "' on lane '" << _veh->getLane()->getID() << "'"
              << "\n       on edge '" << _veh->getLane()->getEdge().getID() << "' (" << toString(_veh->getLane()->getEdge().getLanes()) << ")\n"
              << "objIDs = " << toString(objIDs) << std::endl;
#endif

    if (s.activeFilters == 0) {
        // No filters set
        return;
    }

    // Whether vehicles on opposite lanes shall be taken into account
    const bool disregardOppositeDirection = (s.activeFilters & SUBS_FILTER_NOOPPOSITE) != 0;

    // Check filter specification consistency
    // TODO: Warn only once
    if (disregardOppositeDirection && (s.activeFilters & SUBS_FILTER_NO_RTREE) == 0) {
        WRITE_WARNING("Ignoring no-opposite subscription filter for geographic range object collection. Consider using the 'lanes' filter.")
    }
    if ((s.activeFilters & SUBS_FILTER_FIELD_OF_VISION) != 0 && (s.activeFilters & SUBS_FILTER_NO_RTREE) != 0) {
        WRITE_WARNING("Ignoring field of vision subscription filter due to incompatibility with other filter(s).")
    }

    // TODO: Treat case, where ego vehicle is currently on opposite lane

    std::set<const SUMOTrafficObject*> vehs;
    if (s.activeFilters & SUBS_FILTER_NO_RTREE) {
        // Set defaults for upstream/downstream/lateral distances
        double downstreamDist = s.range, upstreamDist = s.range, lateralDist = s.range;
        if (s.activeFilters & SUBS_FILTER_DOWNSTREAM_DIST) {
            // Specifies maximal downstream distance for vehicles in context subscription result
            downstreamDist = s.filterDownstreamDist;
        }
        if (s.activeFilters & SUBS_FILTER_UPSTREAM_DIST) {
            // Specifies maximal downstream distance for vehicles in context subscription result
            upstreamDist = s.filterUpstreamDist;
        }
        if (s.activeFilters & SUBS_FILTER_LATERAL_DIST) {
            // Specifies maximal lateral distance for vehicles in context subscription result
            lateralDist = s.filterLateralDist;
        }
        MSVehicle* v = dynamic_cast<MSVehicle*>(getVehicle(s.id));
        if (v == nullptr) {
            throw TraCIException("Subscription filter not yet implemented for meso vehicle");
        }
        if (!v->isOnRoad()) {
            return;
        }
        MSLane* vehLane = v->getLane();
        if (vehLane == nullptr) {
            return;
        }
        MSEdge* vehEdge = &vehLane->getEdge();
        std::vector<int> filterLanes;
        if ((s.activeFilters & SUBS_FILTER_LANES) == 0) {
            // No lane indices are specified (but downstream and/or upstream distance)
            //   -> use only vehicle's current lane as origin for the searches
            filterLanes = {0};
            // Lane indices must be specified when leader/follower information is requested.
            assert((s.activeFilters & SUBS_FILTER_LEAD_FOLLOW) == 0);
        } else {
            filterLanes = s.filterLanes;
        }

#ifdef DEBUG_SURROUNDING
        std::cout << "Filter lanes: " << toString(filterLanes) << std::endl;
        std::cout << "Downstream distance: " << downstreamDist << std::endl;
        std::cout << "Upstream distance: " << upstreamDist << std::endl;
        std::cout << "Lateral distance: " << lateralDist << std::endl;
#endif

        if ((s.activeFilters & SUBS_FILTER_MANEUVER) != 0) {
            // Maneuver filters disables road net search for all surrounding vehicles
            if ((s.activeFilters & SUBS_FILTER_LEAD_FOLLOW) != 0) {
                // Return leader and follower on the specified lanes in context subscription result.
                for (int offset : filterLanes) {
                    MSLane* lane = v->getLane()->getParallelLane(offset, false);
                    if (lane != nullptr) {
                        // this is a non-opposite lane
                        MSVehicle* leader = lane->getLeader(v, v->getPositionOnLane(), v->getBestLanesContinuation(lane), downstreamDist).first;
                        MSVehicle* follower = lane->getFollower(v, v->getPositionOnLane(), upstreamDist, false).first;
                        vehs.insert(vehs.end(), leader);
                        vehs.insert(vehs.end(), follower);

#ifdef DEBUG_SURROUNDING
                        std::cout << "Lane at index " << offset << ": '" << lane->getID() << std::endl;
                        std::cout << "Leader: '" << (leader != nullptr ? leader->getID() : "NULL") << "'" << std::endl;
                        std::cout << "Follower: '" << (follower != nullptr ? follower->getID() : "NULL") << "'" << std::endl;
#endif

                    } else if (!disregardOppositeDirection && offset > 0) { // TODO: offset<0 may indicate opposite query when vehicle is on opposite itself
                        // check whether ix points to an opposite lane
                        const MSEdge* opposite = vehEdge->getOppositeEdge();
                        if (opposite == nullptr) {
#ifdef DEBUG_SURROUNDING
                            std::cout << "No lane at index " << offset << std::endl;
#endif
                            // no opposite edge
                            continue;
                        }
                        // Index of opposite lane at relative offset
                        const int ix_opposite = (int)opposite->getLanes().size() - 1 - (vehLane->getIndex() + offset - (int)vehEdge->getLanes().size());
                        if (ix_opposite < 0) {
#ifdef DEBUG_SURROUNDING
                            std::cout << "No lane on opposite at index " << ix_opposite << std::endl;
#endif
                            // no opposite edge
                            continue;
                        }
                        lane = opposite->getLanes()[ix_opposite];
                        // Search vehs along opposite lanes (swap upstream and downstream distance)
                        // XXX transformations for curved geometries
                        double posOnOpposite = MAX2(0., opposite->getLength() - v->getPositionOnLane());
                        // Get leader on opposite
                        vehs.insert(vehs.end(), lane->getFollower(v, posOnOpposite, downstreamDist, true).first);
                        // Get follower (no search on consecutive lanes
                        vehs.insert(vehs.end(), lane->getLeader(v, posOnOpposite - v->getLength(), std::vector<MSLane*>()).first);
                    }
                }
            }

            if (s.activeFilters & SUBS_FILTER_TURN) {
                // Get upcoming junctions and vialanes within downstream distance, where foe links exist or at least the link direction is not straight
                MSLane* lane = v->getLane();
                std::vector<const MSLink*> links = lane->getUpcomingLinks(v->getPositionOnLane(), downstreamDist, v->getBestLanesContinuation());
#ifdef DEBUG_SURROUNDING
                std::cout << "Applying turn filter for vehicle '" << v->getID() << "'\n Gathering foes ..." << std::endl;
#endif
                // Iterate through junctions and find approaching foes within upstreamDist.
                for (auto& l : links) {
#ifdef DEBUG_SURROUNDING
                    std::cout << "  On junction '" << l->getJunction()->getID() << "' (no. foe links = " << l->getFoeLinks().size() << "):" << std::endl;
#endif
                    for (auto& foeLane : l->getFoeLanes()) {
                        // Check vehicles approaching the entry link corresponding to this lane
                        const MSLink* foeLink = foeLane->getEntryLink();
                        for (auto& vi : foeLink->getApproaching()) {
                            if (vi.second.dist <= upstreamDist) {
#ifdef DEBUG_SURROUNDING
                                std::cout << "    Approaching from foe-lane '" << vi.first->getID() << "'" << std::endl;
#endif
                                vehs.insert(vehs.end(), dynamic_cast<const MSVehicle*>(vi.first));
                            }
                        }
                        // add vehicles currently on the junction
                        for (const MSVehicle* foe : foeLane->getVehiclesSecure()) {
                            vehs.insert(vehs.end(), foe);
                        }
                        foeLane->releaseVehicles();
                    }
                }
            }
#ifdef DEBUG_SURROUNDING
            std::cout << SIMTIME << " applySubscriptionFilters() for veh '" << v->getID() << "'. Found the following vehicles:\n";
            for (auto veh : vehs) {
                debugPrint(veh);
            }
#endif
        } else if (s.activeFilters & SUBS_FILTER_LATERAL_DIST) {
            assert(vehs.size() == 0);
            assert(objIDs.size() == 0);

            // collect all vehicles within maximum range of interest to get an upper bound
            PositionVector vehShape;
            findObjectShape(s.commandId, s.id, vehShape);
            double range = MAX3(downstreamDist, upstreamDist, lateralDist);
            collectObjectIDsInRange(s.contextDomain, vehShape, range, objIDs);

#ifdef DEBUG_SURROUNDING
            std::cout << "FILTER_LATERAL_DIST: collected object IDs (range " << range << "):" << std::endl;
            for (std::string i : objIDs) {
                std::cout << i << std::endl;
            }
#endif

#ifdef DEBUG_SURROUNDING
            std::cout << "FILTER_LATERAL_DIST: myLane is '" << v->getLane()->getID() << "'" << std::endl;
#endif
            // 1st pass: downstream (make sure that the whole length of the vehicle is included in the match)
            const double backPosOnLane = MAX2(0.0, v->getPositionOnLane() - v->getVehicleType().getLength());
            applySubscriptionFilterLateralDistanceSinglePass(s, objIDs, vehs, v->getUpcomingLanesUntil(downstreamDist),
                    backPosOnLane, v->getLateralPositionOnLane(), true);
            // 2nd pass: upstream
            applySubscriptionFilterLateralDistanceSinglePass(s, objIDs, vehs, v->getPastLanesUntil(upstreamDist),
                    v->getPositionOnLane(), v->getLateralPositionOnLane(), false);

            objIDs.clear();
        } else {
            // No maneuver or lateral distance filters requested, but only lanes filter (directly, or indirectly by specifying downstream or upstream distance)
            assert(filterLanes.size() > 0);
            // This is to remember the lanes checked in the driving direction of the vehicle (their opposites can be added in a second pass)
            auto checkedLanesInDrivingDir = std::make_shared<LaneCoverageInfo>();
            for (int offset : filterLanes) {
                MSLane* lane = vehLane->getParallelLane(offset, false);
                if (lane != nullptr) {
#ifdef DEBUG_SURROUNDING
                    std::cout << "Checking for surrounding vehicles starting on lane '" << lane->getID() << "' at index " << offset << std::endl;
#endif
                    // Search vehs along this lane
                    // (Coverage info is collected per origin lane since lanes reached from neighboring lanes may have different distances
                    // and aborting at previously scanned when coming from a closer origin may prevent scanning of parts that should be included.)
                    std::shared_ptr<LaneCoverageInfo> checkedLanes = std::make_shared<LaneCoverageInfo>();
                    const std::set<MSVehicle*> new_vehs = lane->getSurroundingVehicles(v->getPositionOnLane(), downstreamDist, upstreamDist + v->getLength(), checkedLanes);
                    vehs.insert(new_vehs.begin(), new_vehs.end());
                    fuseLaneCoverage(checkedLanesInDrivingDir, checkedLanes);
                } else if (!disregardOppositeDirection && offset > 0) {
                    // Check opposite edge, too
                    assert(vehLane->getIndex() + offset >= (int)vehEdge->getLanes().size()); // index points beyond this edge
                    const MSEdge* opposite = vehEdge->getOppositeEdge();
                    if (opposite == nullptr) {
#ifdef DEBUG_SURROUNDING
                        std::cout << "No opposite edge, thus no lane at index " << offset << std::endl;
#endif
                        // no opposite edge
                        continue;
                    }
                    // Index of opposite lane at relative offset
                    const int ix_opposite = (int)opposite->getLanes().size() - 1 - (vehLane->getIndex() + offset - (int)vehEdge->getLanes().size());
                    if (ix_opposite < 0) {
#ifdef DEBUG_SURROUNDING
                        std::cout << "No lane on opposite at index " << ix_opposite << std::endl;
#endif
                        // no opposite edge
                        continue;
                    }
                    lane = opposite->getLanes()[ix_opposite];
                    // Search vehs along opposite lanes (swap upstream and downstream distance)
                    const std::set<MSVehicle*> new_vehs = lane->getSurroundingVehicles(lane->getLength() - v->getPositionOnLane(), upstreamDist + v->getLength(), downstreamDist, std::make_shared<LaneCoverageInfo>());
                    vehs.insert(new_vehs.begin(), new_vehs.end());
                }
#ifdef DEBUG_SURROUNDING
                else {
                    std::cout << "No lane at index " << offset << std::endl;
                }
#endif

                if (!disregardOppositeDirection) {
                    // If opposite should be checked, do this for each lane of the search tree in checkedLanesInDrivingDir
                    // (For instance, some opposite lanes of these would not be obtained if the ego lane does not have an opposite.)

                    // Number of opposite lanes to be checked (assumes filterLanes.size()>0, see assertion above) determined as hypothetical offset
                    // overlap into opposing edge from the vehicle's current lane.
                    // TODO: offset<0 may indicate opposite query when vehicle is on opposite itself (-> use min_element(filterLanes...) instead, etc)
                    const int nOpp = MAX2(0, (*std::max_element(filterLanes.begin(), filterLanes.end())) - ((int)vehEdge->getLanes().size() - 1 - vehLane->getIndex()));
                    // Collect vehicles from opposite lanes
                    if (nOpp > 0) {
                        for (auto& laneCov : *checkedLanesInDrivingDir) {
                            const MSLane* const l = laneCov.first;
                            if (l == nullptr || l->getEdge().getOppositeEdge() == nullptr) {
                                continue;
                            }
                            const MSEdge* opposite = l->getEdge().getOppositeEdge();
                            const std::pair<double, double>& range = laneCov.second;
                            auto leftMostOppositeLaneIt = opposite->getLanes().rbegin();
                            for (auto oppositeLaneIt = leftMostOppositeLaneIt;
                                    oppositeLaneIt != opposite->getLanes().rend(); ++oppositeLaneIt) {
                                if ((int)(oppositeLaneIt - leftMostOppositeLaneIt) == nOpp) {
                                    break;
                                }
                                // Add vehicles from corresponding range on opposite direction
                                const MSLane* oppositeLane = *oppositeLaneIt;
                                auto new_vehs = oppositeLane->getVehiclesInRange(l->getLength() - range.second, l->getLength() - range.first);
                                vehs.insert(new_vehs.begin(), new_vehs.end());
                            }
                        }
                    }
                }
#ifdef DEBUG_SURROUNDING
                std::cout << SIMTIME << " applySubscriptionFilters() for veh '" << v->getID() << "'. Found the following vehicles:\n";
                for (auto veh : vehs) {
                    debugPrint(veh);
                }
#endif
            }

            // filter vehicles in vehs by class and/or type if requested
            if (s.activeFilters & SUBS_FILTER_VCLASS) {
                // Only return vehicles of the given vClass in context subscription result
                auto i = vehs.begin();
                while (i != vehs.end()) {
                    if (((*i)->getVehicleType().getVehicleClass() & s.filterVClasses) == 0) {
                        i = vehs.erase(i);
                    } else {
                        ++i;
                    }
                }
            }
            if (s.activeFilters & SUBS_FILTER_VTYPE) {
                // Only return vehicles of the given vType in context subscription result
                auto i = vehs.begin();
                while (i != vehs.end()) {
                    if (s.filterVTypes.find((*i)->getVehicleType().getID()) == s.filterVTypes.end()) {
                        i = vehs.erase(i);
                    } else {
                        ++i;
                    }
                }
            }
        }
        // Write vehs IDs in objIDs
        for (const SUMOTrafficObject* veh : vehs) {
            if (veh != nullptr) {
                objIDs.insert(objIDs.end(), veh->getID());
            }
        }
    } else { // apply rTree-based filters
        if (s.activeFilters & SUBS_FILTER_VCLASS) {
            // Only return vehicles of the given vClass in context subscription result
            auto i = objIDs.begin();
            while (i != objIDs.end()) {
                MSBaseVehicle* veh = getVehicle(*i);
                if ((veh->getVehicleType().getVehicleClass() & s.filterVClasses) == 0) {
                    i = objIDs.erase(i);
                } else {
                    ++i;
                }
            }
        }
        if (s.activeFilters & SUBS_FILTER_VTYPE) {
            // Only return vehicles of the given vType in context subscription result
            auto i = objIDs.begin();
            while (i != objIDs.end()) {
                MSBaseVehicle* veh = getVehicle(*i);
                if (s.filterVTypes.find(veh->getVehicleType().getID()) == s.filterVTypes.end()) {
                    i = objIDs.erase(i);
                } else {
                    ++i;
                }
            }
        }
        if (s.activeFilters & SUBS_FILTER_FIELD_OF_VISION) {
            // Only return vehicles within field of vision in context subscription result
            applySubscriptionFilterFieldOfVision(s, objIDs);
        }
    }
}

void
Helper::applySubscriptionFilterFieldOfVision(const Subscription& s, std::set<std::string>& objIDs) {
    if (s.filterFieldOfVisionOpeningAngle <= 0. || s.filterFieldOfVisionOpeningAngle >= 360.) {
        WRITE_WARNINGF("Field of vision opening angle ('%') should be within interval (0, 360), ignoring filter...", s.filterFieldOfVisionOpeningAngle);
        return;
    }

    MSBaseVehicle* egoVehicle = getVehicle(s.id);
    Position egoPosition = egoVehicle->getPosition();
    double openingAngle = DEG2RAD(s.filterFieldOfVisionOpeningAngle);

#ifdef DEBUG_SURROUNDING
    std::cout << "FOVFILTER: ego direction = " << toString(RAD2DEG(egoVehicle->getAngle())) << " (deg)" << std::endl;
#endif

    auto i = objIDs.begin();
    while (i != objIDs.end()) {
        if (s.id.compare(*i) == 0) { // skip if this is the ego vehicle
            ++i;
            continue;
        }
        SUMOTrafficObject* obj = getTrafficObject(s.contextDomain, *i);
        double angleEgoToVeh = egoPosition.angleTo2D(obj->getPosition());
        double alpha = GeomHelper::angleDiff(egoVehicle->getAngle(), angleEgoToVeh);

#ifdef DEBUG_SURROUNDING
        const std::string objType = s.isVehicleToPersonContextSubscription() ? "person" : "veh";
        std::cout << "FOVFILTER: " << objType << " '" << *i << "' dist  = " << toString(egoPosition.distanceTo2D(obj->getPosition())) << std::endl;
        std::cout << "FOVFILTER: " << objType << " '" << *i << "' alpha = " << toString(RAD2DEG(alpha)) << " (deg)" << std::endl;
#endif

        if (abs(alpha) > openingAngle * 0.5) {
            i = objIDs.erase(i);
        } else {
            ++i;
        }
    }
}

void
Helper::applySubscriptionFilterLateralDistanceSinglePass(const Subscription& s, std::set<std::string>& objIDs,
        std::set<const SUMOTrafficObject*>& vehs,
        const std::vector<const MSLane*>& lanes, double posOnLane, double posLat, bool isDownstream) {
    const double streamDist = isDownstream ? s.filterDownstreamDist : s.filterUpstreamDist;
    double distRemaining = streamDist;
    bool isFirstLane = true;
    PositionVector combinedShape;
    for (const MSLane* lane : lanes) {
#ifdef DEBUG_SURROUNDING
        std::cout << "FILTER_LATERAL_DIST: current lane " << (isDownstream ? "down" : "up") << " is '" << lane->getID() << "', length " << lane->getLength()
                  << ", pos " << posOnLane << ", distRemaining " << distRemaining << std::endl;
#endif
        PositionVector laneShape = lane->getShape();
        if (isFirstLane) {
            isFirstLane = false;
            if (posOnLane == 0) {
                if (!isDownstream) {
                    continue;
                }
            } else {
                double geometryPos = lane->interpolateLanePosToGeometryPos(posOnLane);
                if (geometryPos >= laneShape.length()) {
                    laneShape = isDownstream ? PositionVector() : laneShape;
                } else {
                    auto pair = laneShape.splitAt(geometryPos, false);
                    laneShape = isDownstream ? pair.second : pair.first;
                }
            }
        }
        double laneLength = lane->interpolateGeometryPosToLanePos(laneShape.length());
        if (distRemaining - laneLength < 0.) {
            double geometryPos = lane->interpolateLanePosToGeometryPos(isDownstream ? distRemaining : laneLength - distRemaining);
            auto pair = laneShape.splitAt(geometryPos, false);
            laneShape = isDownstream ? pair.first : pair.second;
        }
        distRemaining -= laneLength;
        try {
            laneShape.move2side(-posLat);
        } catch (ProcessError&) {
            WRITE_WARNING("addSubscriptionFilterLateralDistance could not determine shape of lane '" + lane->getID() + "' with lateral shift of " + toString(posLat));
        }
#ifdef DEBUG_SURROUNDING
        std::cout << "   posLat=" << posLat << " laneShape=" << laneShape << "\n";
#endif
        combinedShape.append(laneShape);
        if (distRemaining <= 0) {
            break;
        }
    }
    // check remaining objects' distances to the combined shape
    auto i = objIDs.begin();
    while (i != objIDs.end()) {
        SUMOTrafficObject* obj = getTrafficObject(s.contextDomain, *i);
        double minPerpendicularDist = combinedShape.distance2D(obj->getPosition(), true);
#ifdef DEBUG_SURROUNDING
        std::cout << " obj " << obj->getID() << " dist=" << minPerpendicularDist << " filterDist=" << s.filterLateralDist << "\n";
#endif
        if ((minPerpendicularDist != GeomHelper::INVALID_OFFSET) && (minPerpendicularDist <= s.filterLateralDist)) {
            vehs.insert(obj);
            i = objIDs.erase(i);
        } else {
            ++i;
        }
    }
}

void
Helper::setRemoteControlled(MSVehicle* v, Position xyPos, MSLane* l, double pos, double posLat, double angle,
                            int edgeOffset, ConstMSEdgeVector route, SUMOTime t) {
    myRemoteControlledVehicles[v->getID()] = v;
    v->getInfluencer().setRemoteControlled(xyPos, l, pos, posLat, angle, edgeOffset, route, t);
}

void
Helper::setRemoteControlled(MSPerson* p, Position xyPos, MSLane* l, double pos, double posLat, double angle,
                            int edgeOffset, ConstMSEdgeVector route, SUMOTime t) {
    myRemoteControlledPersons[p->getID()] = p;
    p->getInfluencer().setRemoteControlled(xyPos, l, pos, posLat, angle, edgeOffset, route, t);
}


void
Helper::postProcessRemoteControl() {
    for (auto& controlled : myRemoteControlledVehicles) {
        if (MSNet::getInstance()->getVehicleControl().getVehicle(controlled.first) != nullptr) {
            controlled.second->getInfluencer().postProcessRemoteControl(controlled.second);
        } else {
            WRITE_WARNING("Vehicle '" + controlled.first + "' was removed though being controlled by TraCI");
        }
    }
    myRemoteControlledVehicles.clear();
    for (auto& controlled : myRemoteControlledPersons) {
        if (MSNet::getInstance()->getPersonControl().get(controlled.first) != nullptr) {
            controlled.second->getInfluencer().postProcessRemoteControl(controlled.second);
        } else {
            WRITE_WARNING("Person '" + controlled.first + "' was removed though being controlled by TraCI");
        }
    }
    myRemoteControlledPersons.clear();
}


bool
Helper::moveToXYMap(const Position& pos, double maxRouteDistance, bool mayLeaveNetwork, const std::string& origID, const double angle,
                    double speed, const ConstMSEdgeVector& currentRoute, const int routePosition, MSLane* currentLane, double currentLanePos, bool onRoad,
                    SUMOVehicleClass vClass, bool setLateralPos,
                    double& bestDistance, MSLane** lane, double& lanePos, int& routeOffset, ConstMSEdgeVector& edges) {
    // collect edges around the vehicle/person
#ifdef DEBUG_MOVEXY
    std::cout << SIMTIME << " moveToXYMap pos=" << pos << " angle=" << angle << " vClass=" << toString(vClass) << "\n";
#endif
    const MSEdge* const currentRouteEdge = currentRoute[routePosition];
    std::set<const Named*> into;
    PositionVector shape;
    shape.push_back(pos);
    collectObjectsInRange(libsumo::CMD_GET_EDGE_VARIABLE, shape, maxRouteDistance, into);
    double maxDist = 0;
    std::map<MSLane*, LaneUtility> lane2utility;
    // compute utility for all candidate edges
    for (const Named* namedEdge : into) {
        const MSEdge* e = dynamic_cast<const MSEdge*>(namedEdge);
        const MSEdge* prevEdge = nullptr;
        const MSEdge* nextEdge = nullptr;
        bool onRoute = false;
        // the next if/the clause sets "onRoute", "prevEdge", and "nextEdge", depending on
        //  whether the currently seen edge is an internal one or a normal one
        if (e->isWalkingArea() || e->isCrossing()) {
            // find current intersection along the route
            const MSJunction* junction = e->getFromJunction();
            for (int i = routePosition; i < (int)currentRoute.size(); i++) {
                const MSEdge* cand = currentRoute[i];
                if (cand->getToJunction() == junction) {
                    onRoute = true;
                    prevEdge = cand;
                    if (i + 1 < (int)currentRoute.size()) {
                        nextEdge = currentRoute[i + 1];
                    }
                    break;
                }
            }
            if (onRoute == false) {
                // search backward
                for (int i = routePosition - 1; i >= 0; i--) {
                    const MSEdge* cand = currentRoute[i];
                    if (cand->getToJunction() == junction) {
                        onRoute = true;
                        prevEdge = cand;
                        nextEdge = currentRoute[i + 1];
                        break;
                    }
                }
            }
            if (prevEdge == nullptr) {
                // use arbitrary predecessor
                prevEdge = e->getPredecessors().front();
            }
            if (nextEdge == nullptr) {
                nextEdge = e->getSuccessors().front();
            }
#ifdef DEBUG_MOVEXY_ANGLE
            std::cout << "walkingarea/crossing:" << e->getID() << " prev:" << Named::getIDSecure(prevEdge) << " next:" << Named::getIDSecure(nextEdge) << "\n";
#endif
        } else if (e->isNormal()) {
            // a normal edge
            //
            // check whether the currently seen edge is in the vehicle's route
            //  - either the one it's on or one of the next edges
            ConstMSEdgeVector::const_iterator searchStart = currentRoute.begin() + routePosition;
            if (onRoad && currentLane->getEdge().isInternal()) {
                ++searchStart;
            }
            ConstMSEdgeVector::const_iterator edgePos = std::find(searchStart, currentRoute.end(), e);
            onRoute = edgePos != currentRoute.end(); // no? -> onRoute is false
            if (edgePos == currentRoute.end() - 1 && currentRouteEdge == e) {
                // onRoute is false as well if the vehicle is beyond the edge
                onRoute &= currentRouteEdge->getLength() > currentLanePos + SPEED2DIST(speed);
            }
            // save prior and next edges
            prevEdge = e;
            nextEdge = !onRoute || edgePos == currentRoute.end() - 1 ? nullptr : *(edgePos + 1);
#ifdef DEBUG_MOVEXY_ANGLE
            std::cout << "normal:" << e->getID() << " prev:" << Named::getIDSecure(prevEdge) << " next:" << Named::getIDSecure(nextEdge) << "\n";
#endif
        } else if (e->isInternal()) {
            // an internal edge
            // get the previous edge
            prevEdge = e;
            while (prevEdge != nullptr && prevEdge->isInternal()) {
                MSLane* l = prevEdge->getLanes()[0];
                l = l->getLogicalPredecessorLane();
                prevEdge = l == nullptr ? nullptr : &l->getEdge();
            }
            // check whether the previous edge is on the route (was on the route)
            ConstMSEdgeVector::const_iterator prevEdgePos = std::find(currentRoute.begin() + routePosition, currentRoute.end(), prevEdge);
            nextEdge = e;
            while (nextEdge != nullptr && nextEdge->isInternal()) {
                nextEdge = nextEdge->getSuccessors()[0]; // should be only one for an internal edge
            }
            if (prevEdgePos != currentRoute.end() && (prevEdgePos + 1) != currentRoute.end()) {
                onRoute = *(prevEdgePos + 1) == nextEdge;
            }
#ifdef DEBUG_MOVEXY_ANGLE
            std::cout << "internal:" << e->getID() << " prev:" << Named::getIDSecure(prevEdge) << " next:" << Named::getIDSecure(nextEdge) << "\n";
#endif
        }


        // weight the lanes...
        const bool perpendicular = false;
        for (MSLane* const l : e->getLanes()) {
            if (!l->allowsVehicleClass(vClass)) {
                continue;
            }
            if (l->getShape().length() == 0) {
                // mapping to shapeless lanes is a bad idea
                continue;
            }
            double langle = 180.;
            double dist = FAR_AWAY;
            double perpendicularDist = FAR_AWAY;
            // add some slack to avoid issues from tiny gaps between consecutive lanes
            const double slack = POSITION_EPS;
            PositionVector laneShape = l->getShape();
            laneShape.extrapolate2D(slack);
            double off = laneShape.nearest_offset_to_point2D(pos, true);
            if (off != GeomHelper::INVALID_OFFSET) {
                perpendicularDist = laneShape.distance2D(pos, true);
            }
            off = l->getShape().nearest_offset_to_point2D(pos, perpendicular);
            if (off != GeomHelper::INVALID_OFFSET) {
                dist = l->getShape().distance2D(pos, perpendicular);
                langle = GeomHelper::naviDegree(l->getShape().rotationAtOffset(off));
            }
            // cannot trust lanePos on walkingArea
            bool sameEdge = onRoad && e == &currentLane->getEdge() && currentRouteEdge->getLength() > currentLanePos + SPEED2DIST(speed) && !e->isWalkingArea();
            /*
            const MSEdge* rNextEdge = nextEdge;
            while(rNextEdge==0&&lane->getEdge().getPurpose()==MSEdge::EDGEFUNCTION_INTERNAL) {
                MSLane* next = lane->getLinkCont()[0]->getLane();
                rNextEdge = next == 0 ? 0 : &next->getEdge();
            }
            */
            double dist2 = dist;
            if (mayLeaveNetwork && fabs(dist - perpendicularDist) > slack) {
                // ambiguous mapping. Don't trust this
                dist2 = FAR_AWAY;
            }
            const double angleDiff = (angle == INVALID_DOUBLE_VALUE ? 0 : GeomHelper::getMinAngleDiff(angle, langle));
#ifdef DEBUG_MOVEXY_ANGLE
            std::cout << std::setprecision(gPrecision)
                      << " candLane=" << l->getID() << " lAngle=" << langle << " lLength=" << l->getLength()
                      << " angleDiff=" << angleDiff
                      << " off=" << off
                      << " pDist=" << perpendicularDist
                      << " dist=" << dist
                      << " dist2=" << dist2
                      << "\n";
            std::cout << l->getID() << " param=" << l->getParameter(SUMO_PARAM_ORIGID, "") << " origID='" << origID << "\n";
#endif

            bool origIDMatch = l->getParameter(SUMO_PARAM_ORIGID, l->getID()) == origID;
            if (origIDMatch && setLateralPos
                    && perpendicularDist > l->getWidth() / 2) {
                origIDMatch = false;
            }
            lane2utility.emplace(l, LaneUtility(
                                     dist2, perpendicularDist, off, angleDiff,
                                     origIDMatch,
                                     onRoute, sameEdge, prevEdge, nextEdge));
            // update scaling value
            maxDist = MAX2(maxDist, MIN2(dist, SUMO_const_laneWidth));

        }
    }

    // get the best lane given the previously computed values
    double bestValue = 0;
    MSLane* bestLane = nullptr;
    for (const auto& it : lane2utility) {
        MSLane* const l = it.first;
        const LaneUtility& u = it.second;
        double distN = u.dist > 999 ? -10 : 1. - (u.dist / maxDist);
        double angleDiffN = 1. - (u.angleDiff / 180.);
        double idN = u.ID ? 1 : 0;
        double onRouteN = u.onRoute ? 1 : 0;
        double sameEdgeN = u.sameEdge ? MIN2(currentRouteEdge->getLength() / speed, (double)1.) : 0;
        double value = (distN * .5 // distance is more important than angle because the vehicle might be driving in the opposite direction
                        + angleDiffN * 0.35 /*.5 */
                        + idN * 1
                        + onRouteN * 0.1
                        + sameEdgeN * 0.1);
#ifdef DEBUG_MOVEXY
        std::cout << " x; l:" << l->getID() << " d:" << u.dist << " dN:" << distN << " aD:" << angleDiffN <<
                  " ID:" << idN << " oRN:" << onRouteN << " sEN:" << sameEdgeN << " value:" << value << std::endl;
#endif
        if (value > bestValue || bestLane == nullptr) {
            bestValue = value;
            if (u.dist == FAR_AWAY) {
                bestLane = nullptr;
            } else {
                bestLane = l;
            }
        }
    }
    // no best lane found, return
    if (bestLane == nullptr) {
        return false;
    }
    const LaneUtility& u = lane2utility.find(bestLane)->second;
    bestDistance = u.dist;
    *lane = bestLane;
    lanePos = MAX2(0., MIN2(double((*lane)->getLength() - POSITION_EPS),
                            bestLane->interpolateGeometryPosToLanePos(
                                bestLane->getShape().nearest_offset_to_point25D(pos, false))));
    const MSEdge* prevEdge = u.prevEdge;
    if (u.onRoute) {
        ConstMSEdgeVector::const_iterator prevEdgePos = std::find(currentRoute.begin(), currentRoute.end(), prevEdge);
        routeOffset = (int)std::distance(currentRoute.begin(), prevEdgePos);
        //std::cout << SIMTIME << "moveToXYMap vehicle=" << veh.getID() << " currLane=" << veh.getLane()->getID() << " routeOffset=" << routeOffset << " edges=" << toString(ev) << " bestLane=" << bestLane->getID() << " prevEdge=" << prevEdge->getID() << "\n";
    } else {
        edges.push_back(u.prevEdge);
        /*
           if(bestLane->getEdge().getPurpose()!=MSEdge::EDGEFUNCTION_INTERNAL) {
           edges.push_back(&bestLane->getEdge());
           }
        */
        if (u.nextEdge != nullptr) {
            edges.push_back(u.nextEdge);
        }
        routeOffset = 0;
#ifdef DEBUG_MOVEXY_ANGLE
        std::cout << SIMTIME << " internal2: lane=" << bestLane->getID() << " prev=" << Named::getIDSecure(u.prevEdge) << " next=" << Named::getIDSecure(u.nextEdge) << "\n";
#endif
    }
    return true;
}


bool
Helper::findCloserLane(const MSEdge* edge, const Position& pos, SUMOVehicleClass vClass, double& bestDistance, MSLane** lane) {
    if (edge == nullptr) {
        return false;
    }
    const std::vector<MSLane*>& lanes = edge->getLanes();
    bool newBest = false;
    for (std::vector<MSLane*>::const_iterator k = lanes.begin(); k != lanes.end() && bestDistance > POSITION_EPS; ++k) {
        MSLane* candidateLane = *k;
        if (!candidateLane->allowsVehicleClass(vClass)) {
            continue;
        }
        if (candidateLane->getShape().length() == 0) {
            // mapping to shapeless lanes is a bad idea
            continue;
        }
        const double dist = candidateLane->getShape().distance2D(pos); // get distance
#ifdef DEBUG_MOVEXY
        std::cout << "   b at lane " << candidateLane->getID() << " dist:" << dist << " best:" << bestDistance << std::endl;
#endif
        if (dist < bestDistance) {
            // is the new distance the best one? keep then...
            bestDistance = dist;
            *lane = candidateLane;
            newBest = true;
        }
    }
    return newBest;
}


bool
Helper::moveToXYMap_matchingRoutePosition(const Position& pos, const std::string& origID,
        const ConstMSEdgeVector& currentRoute, int routeIndex,
        SUMOVehicleClass vClass, bool setLateralPos,
        double& bestDistance, MSLane** lane, double& lanePos, int& routeOffset) {
#ifdef DEBUG_MOVEXY
    std::cout << SIMTIME << " moveToXYMap_matchingRoutePosition pos=" << pos << " vClass=" << toString(vClass) << "\n";
#endif
    //std::cout << "moveToXYMap_matchingRoutePosition pos=" << pos << "\n";
    routeOffset = 0;
    // routes may be looped which makes routeOffset ambiguous. We first try to
    // find the closest upcoming edge on the route and then look for closer passed edges

    // look forward along the route
    const MSEdge* prev = nullptr;
    for (int i = routeIndex; i < (int)currentRoute.size(); ++i) {
        const MSEdge* cand = currentRoute[i];
        while (prev != nullptr) {
            // check internal edge(s)
            const MSEdge* internalCand = prev->getInternalFollowingEdge(cand);
            findCloserLane(internalCand, pos, vClass, bestDistance, lane);
            prev = internalCand;
        }
        if (findCloserLane(cand, pos, vClass, bestDistance, lane)) {
            routeOffset = i;
        }
        prev = cand;
    }
    // look backward along the route
    const MSEdge* next = currentRoute[routeIndex];
    for (int i = routeIndex; i >= 0; --i) {
        const MSEdge* cand = currentRoute[i];
        //std::cout << "  next=" << next->getID() << " cand=" << cand->getID() << " i=" << i << " routeIndex=" << routeIndex << "\n";
        prev = cand;
        while (prev != nullptr) {
            // check internal edge(s)
            const MSEdge* internalCand = prev->getInternalFollowingEdge(next);
            if (findCloserLane(internalCand, pos, vClass, bestDistance, lane)) {
                routeOffset = i;
            }
            prev = internalCand;
        }
        if (findCloserLane(cand, pos, vClass, bestDistance, lane)) {
            routeOffset = i;
        }
        next = cand;
    }
    if (vClass == SVC_PEDESTRIAN) {
        // consider all crossings and walkingareas along the route
        std::map<const MSJunction*, int> routeJunctions;
        for (int i = 0; i < (int)currentRoute.size() - 1; ++i) {
            routeJunctions[currentRoute[i]->getToJunction()] = i;
        }
        std::set<const Named*> into;
        PositionVector shape;
        shape.push_back(pos);
        collectObjectsInRange(libsumo::CMD_GET_LANE_VARIABLE, shape, 100, into);
        for (const Named* named : into) {
            const MSLane* cand = dynamic_cast<const MSLane*>(named);
            if ((cand->getEdge().isWalkingArea() || cand->getEdge().isCrossing())
                    && routeJunctions.count(cand->getEdge().getToJunction()) != 0) {
                if (findCloserLane(&cand->getEdge(), pos, vClass, bestDistance, lane)) {
                    routeOffset = routeJunctions[cand->getEdge().getToJunction()];
                }
            }
        }
    }

    assert(lane != 0);
    // quit if no solution was found, reporting a failure
    if (lane == nullptr) {
#ifdef DEBUG_MOVEXY
        std::cout << "  b failed - no best route lane" << std::endl;
#endif
        return false;
    }


    // position may be inaccurate; let's checkt the given index, too
    // @note: this is enabled for non-internal lanes only, as otherwise the position information may ambiguous
    if (!(*lane)->getEdge().isInternal()) {
        const std::vector<MSLane*>& lanes = (*lane)->getEdge().getLanes();
        for (std::vector<MSLane*>::const_iterator i = lanes.begin(); i != lanes.end(); ++i) {
            if ((*i)->getParameter(SUMO_PARAM_ORIGID, (*i)->getID()) == origID) {
                if (setLateralPos) {
                    // vehicle might end up on top of another lane with a big
                    // lateral offset to the lane with origID.
                    const double dist = (*i)->getShape().distance2D(pos); // get distance
                    if (dist < (*i)->getWidth() / 2) {
                        *lane = *i;
                        break;
                    }
                } else {
                    *lane = *i;
                    break;
                }
            }
        }
    }
    // check position, stuff, we should have the best lane along the route
    lanePos = MAX2(0., MIN2(double((*lane)->getLength() - POSITION_EPS),
                            (*lane)->interpolateGeometryPosToLanePos(
                                (*lane)->getShape().nearest_offset_to_point25D(pos, false))));
    //std::cout << SIMTIME << " moveToXYMap_matchingRoutePosition vehicle=" << veh.getID() << " currLane=" << veh.getLane()->getID() << " routeOffset=" << routeOffset << " edges=" << toString(edges) << " lane=" << (*lane)->getID() << "\n";
#ifdef DEBUG_MOVEXY
    std::cout << "  b ok lane " << (*lane)->getID() << " lanePos:" << lanePos << std::endl;
#endif
    return true;
}


Helper::SubscriptionWrapper::SubscriptionWrapper(VariableWrapper::SubscriptionHandler handler, SubscriptionResults& into, ContextSubscriptionResults& context)
    : VariableWrapper(handler), myResults(into), myContextResults(context), myActiveResults(&into) {

}


void
Helper::SubscriptionWrapper::setContext(const std::string& refID) {
    myActiveResults = refID == "" ? &myResults : &myContextResults[refID];
}


void
Helper::SubscriptionWrapper::setParams(const std::vector<unsigned char>* params) {
    myParams = params;
}


void
Helper::SubscriptionWrapper::clear() {
    myActiveResults = &myResults;
    myResults.clear();
    myContextResults.clear();
}


bool
Helper::SubscriptionWrapper::wrapDouble(const std::string& objID, const int variable, const double value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIDouble>(value);
    return true;
}


bool
Helper::SubscriptionWrapper::wrapInt(const std::string& objID, const int variable, const int value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIInt>(value);
    return true;
}


bool
Helper::SubscriptionWrapper::wrapString(const std::string& objID, const int variable, const std::string& value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIString>(value);
    return true;
}


bool
Helper::SubscriptionWrapper::wrapStringList(const std::string& objID, const int variable, const std::vector<std::string>& value) {
    auto sl = std::make_shared<TraCIStringList>();
    sl->value = value;
    (*myActiveResults)[objID][variable] = sl;
    return true;
}


bool
Helper::SubscriptionWrapper::wrapPosition(const std::string& objID, const int variable, const TraCIPosition& value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIPosition>(value);
    return true;
}


bool
Helper::SubscriptionWrapper::wrapColor(const std::string& objID, const int variable, const TraCIColor& value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIColor>(value);
    return true;
}


bool
Helper::SubscriptionWrapper::wrapRoadPosition(const std::string& objID, const int variable, const TraCIRoadPosition& value) {
    (*myActiveResults)[objID][variable] = std::make_shared<TraCIRoadPosition>(value);
    return true;
}


void
Helper::VehicleStateListener::vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& /*info*/) {
    myVehicleStateChanges[to].push_back(vehicle->getID());
}


}


/****************************************************************************/
