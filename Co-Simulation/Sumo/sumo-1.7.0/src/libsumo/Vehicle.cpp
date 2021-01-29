/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    Vehicle.cpp
/// @author  Jakob Erdmann
/// @date    15.03.2017
///
// C++ Vehicle API
/****************************************************************************/
#include <config.h>

#include <utils/geom/GeomHelper.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringTokenizer.h>
#include <utils/common/StringUtils.h>
#include <utils/gui/globjects/GUIGlObjectTypes.h>
#include <utils/emissions/PollutantsInterface.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/devices/MSDevice.h>
#include <microsim/MSEdgeWeightsStorage.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/MSVehicleType.h>
#include <microsim/MSInsertionControl.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSParkingArea.h>
#include <microsim/devices/MSDevice_Taxi.h>
#include <microsim/devices/MSDispatch_TraCI.h>
#include <mesosim/MEVehicle.h>
#include <libsumo/TraCIDefs.h>
#include <libsumo/TraCIConstants.h>
#include "Helper.h"
#include "Route.h"
#include "Polygon.h"
#include "Vehicle.h"

#define CALL_MICRO_FUN(veh, fun, mesoResult) ((dynamic_cast<MSVehicle*>(veh) == nullptr ? (mesoResult) : dynamic_cast<MSVehicle*>(veh)->fun))


// ===========================================================================
// debug defines
// ===========================================================================
//#define DEBUG_NEIGHBORS
//#define DEBUG_DYNAMIC_SHAPES
//#define DEBUG_MOVEXY
#define DEBUG_COND (veh->isSelected())



namespace libsumo {
// ===========================================================================
// static member initializations
// ===========================================================================
SubscriptionResults Vehicle::mySubscriptionResults;
ContextSubscriptionResults Vehicle::myContextSubscriptionResults;


// ===========================================================================
// static member definitions
// ===========================================================================
bool
Vehicle::isVisible(const SUMOVehicle* veh) {
    return veh->isOnRoad() || veh->isParking() || veh->wasRemoteControlled();
}


bool
Vehicle::isOnInit(const std::string& vehicleID) {
    SUMOVehicle* sumoVehicle = MSNet::getInstance()->getVehicleControl().getVehicle(vehicleID);
    return sumoVehicle == nullptr || sumoVehicle->getLane() == nullptr;
}


std::vector<std::string>
Vehicle::getIDList() {
    std::vector<std::string> ids;
    MSVehicleControl& c = MSNet::getInstance()->getVehicleControl();
    for (MSVehicleControl::constVehIt i = c.loadedVehBegin(); i != c.loadedVehEnd(); ++i) {
        if (isVisible((*i).second)) {
            ids.push_back((*i).first);
        }
    }
    return ids;
}

int
Vehicle::getIDCount() {
    return (int)getIDList().size();
}


double
Vehicle::getSpeed(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getSpeed() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getLateralSpeed(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? CALL_MICRO_FUN(veh, getLaneChangeModel().getSpeedLat(), 0) : INVALID_DOUBLE_VALUE;
}


double
Vehicle::getAcceleration(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? CALL_MICRO_FUN(veh, getAcceleration(), 0) : INVALID_DOUBLE_VALUE;
}


double
Vehicle::getSpeedWithoutTraCI(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? CALL_MICRO_FUN(veh, getSpeedWithoutTraciInfluence(), veh->getSpeed()) : INVALID_DOUBLE_VALUE;
}


TraCIPosition
Vehicle::getPosition(const std::string& vehicleID, const bool includeZ) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    if (isVisible(veh)) {
        return Helper::makeTraCIPosition(veh->getPosition(), includeZ);
    }
    return TraCIPosition();
}


TraCIPosition
Vehicle::getPosition3D(const std::string& vehicleID) {
    return getPosition(vehicleID, true);
}


double
Vehicle::getAngle(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? GeomHelper::naviDegree(veh->getAngle()) : INVALID_DOUBLE_VALUE;
}


double
Vehicle::getSlope(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? veh->getSlope() : INVALID_DOUBLE_VALUE;
}


std::string
Vehicle::getRoadID(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? CALL_MICRO_FUN(veh, getLane()->getEdge().getID(), veh->getEdge()->getID()) : "";
}


std::string
Vehicle::getLaneID(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? CALL_MICRO_FUN(veh, getLane()->getID(), "") : "";
}


int
Vehicle::getLaneIndex(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? CALL_MICRO_FUN(veh, getLane()->getIndex(), INVALID_INT_VALUE) : INVALID_INT_VALUE;
}


std::string
Vehicle::getTypeID(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getID();
}


std::string
Vehicle::getRouteID(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getRoute().getID();
}


int
Vehicle::getRouteIndex(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->hasDeparted() ? veh->getRoutePosition() : INVALID_INT_VALUE;
}


TraCIColor
Vehicle::getColor(const std::string& vehicleID) {
    return Helper::makeTraCIColor(Helper::getVehicle(vehicleID)->getParameter().color);
}

double
Vehicle::getLanePosition(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? veh->getPositionOnLane() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getLateralLanePosition(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? CALL_MICRO_FUN(veh, getLateralPositionOnLane(), 0) : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getCO2Emission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getCO2Emissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getCOEmission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getCOEmissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getHCEmission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getHCEmissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getPMxEmission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getPMxEmissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getNOxEmission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getNOxEmissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getFuelConsumption(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getFuelConsumption() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getNoiseEmission(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getHarmonoise_NoiseEmissions() : INVALID_DOUBLE_VALUE;
}

double
Vehicle::getElectricityConsumption(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return isVisible(veh) ? veh->getElectricityConsumption() : INVALID_DOUBLE_VALUE;
}

int
Vehicle::getPersonNumber(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getPersonNumber();
}

int
Vehicle::getPersonCapacity(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getPersonCapacity();
}

std::vector<std::string>
Vehicle::getPersonIDList(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getPersonIDList();
}

std::pair<std::string, double>
Vehicle::getLeader(const std::string& vehicleID, double dist) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    if (veh->isOnRoad()) {
        std::pair<const MSVehicle* const, double> leaderInfo = veh->getLeader(dist);
        return std::make_pair(
                   leaderInfo.first != nullptr ? leaderInfo.first->getID() : "",
                   leaderInfo.second);
    } else {
        return std::make_pair("", -1);
    }
}


std::pair<std::string, double>
Vehicle::getFollower(const std::string& vehicleID, double dist) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    if (veh->isOnRoad()) {
        std::pair<const MSVehicle* const, double> leaderInfo = veh->getFollower(dist);
        return std::make_pair(
                   leaderInfo.first != nullptr ? leaderInfo.first->getID() : "",
                   leaderInfo.second);
    } else {
        return std::make_pair("", -1);
    }
}


double
Vehicle::getWaitingTime(const std::string& vehicleID) {
    return STEPS2TIME(Helper::getVehicle(vehicleID)->getWaitingTime());
}


double
Vehicle::getAccumulatedWaitingTime(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return CALL_MICRO_FUN(veh, getAccumulatedWaitingSeconds(), INVALID_DOUBLE_VALUE);
}


double
Vehicle::getAdaptedTraveltime(const std::string& vehicleID, double time, const std::string& edgeID) {
    MSEdge* edge = Helper::getEdge(edgeID);
    double value = INVALID_DOUBLE_VALUE;
    Helper::getVehicle(vehicleID)->getWeightsStorage().retrieveExistingTravelTime(edge, time, value);
    return value;
}


double
Vehicle::getEffort(const std::string& vehicleID, double time, const std::string& edgeID) {
    MSEdge* edge = Helper::getEdge(edgeID);
    double value = INVALID_DOUBLE_VALUE;
    Helper::getVehicle(vehicleID)->getWeightsStorage().retrieveExistingEffort(edge, time, value);
    return value;
}


bool
Vehicle::isRouteValid(const std::string& vehicleID) {
    std::string msg;
    return Helper::getVehicle(vehicleID)->hasValidRoute(msg);
}


std::vector<std::string>
Vehicle::getRoute(const std::string& vehicleID) {
    std::vector<std::string> result;
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    const MSRoute& r = veh->getRoute();
    for (MSRouteIterator i = r.begin(); i != r.end(); ++i) {
        result.push_back((*i)->getID());
    }
    return result;
}


int
Vehicle::getSignals(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return CALL_MICRO_FUN(veh, getSignals(), MSVehicle::VEH_SIGNAL_NONE);
}


std::vector<TraCIBestLanesData>
Vehicle::getBestLanes(const std::string& vehicleID) {
    std::vector<TraCIBestLanesData> result;
    MSVehicle* veh = dynamic_cast<MSVehicle*>(Helper::getVehicle(vehicleID));
    if (veh != nullptr && veh->isOnRoad()) {
        for (const MSVehicle::LaneQ& lq : veh->getBestLanes()) {
            TraCIBestLanesData bld;
            bld.laneID = lq.lane->getID();
            bld.length = lq.length;
            bld.occupation = lq.nextOccupation;
            bld.bestLaneOffset = lq.bestLaneOffset;
            bld.allowsContinuation = lq.allowsContinuation;
            for (const MSLane* const lane : lq.bestContinuations) {
                if (lane != nullptr) {
                    bld.continuationLanes.push_back(lane->getID());
                }
            }
            result.emplace_back(bld);
        }
    }
    return result;
}


std::vector<TraCINextTLSData>
Vehicle::getNextTLS(const std::string& vehicleID) {
    std::vector<TraCINextTLSData> result;
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (!vehicle->isOnRoad()) {
        return result;
    }
    if (veh != nullptr) {
        const MSLane* lane = veh->getLane();
        const std::vector<MSLane*>& bestLaneConts = veh->getBestLanesContinuation(lane);
        double seen = lane->getLength() - veh->getPositionOnLane();
        int view = 1;
        MSLinkCont::const_iterator linkIt = MSLane::succLinkSec(*veh, view, *lane, bestLaneConts);
        while (!lane->isLinkEnd(linkIt)) {
            if (!lane->getEdge().isInternal()) {
                if ((*linkIt)->isTLSControlled()) {
                    TraCINextTLSData ntd;
                    ntd.id = (*linkIt)->getTLLogic()->getID();
                    ntd.tlIndex = (*linkIt)->getTLIndex();
                    ntd.dist = seen;
                    ntd.state = (char)(*linkIt)->getState();
                    result.push_back(ntd);
                }
            }
            lane = (*linkIt)->getViaLaneOrLane();
            if (!lane->getEdge().isInternal()) {
                view++;
            }
            seen += lane->getLength();
            linkIt = MSLane::succLinkSec(*veh, view, *lane, bestLaneConts);
        }
        // consider edges beyond bestLanes
        const int remainingEdges = (int)(veh->getRoute().end() - veh->getCurrentRouteEdge()) - view;
        //std::cout << SIMTIME << "remainingEdges=" << remainingEdges << " seen=" << seen << " view=" << view << " best=" << toString(bestLaneConts) << "\n";
        for (int i = 0; i < remainingEdges; i++) {
            const MSEdge* prev = *(veh->getCurrentRouteEdge() + view + i - 1);
            const MSEdge* next = *(veh->getCurrentRouteEdge() + view + i);
            const std::vector<MSLane*>* allowed = prev->allowedLanes(*next, veh->getVClass());
            if (allowed != nullptr && allowed->size() != 0) {
                for (const MSLink* const link : allowed->front()->getLinkCont()) {
                    if (&link->getLane()->getEdge() == next) {
                        if (link->isTLSControlled()) {
                            TraCINextTLSData ntd;
                            ntd.id = link->getTLLogic()->getID();
                            ntd.tlIndex = link->getTLIndex();
                            ntd.dist = seen;
                            ntd.state = (char)link->getState();
                            result.push_back(ntd);
                        }
                        seen += allowed->front()->getLength();
                    }
                }
            } else {
                // invalid route, cannot determine nextTLS
                break;
            }
        }
    } else {
        WRITE_WARNING("getNextTLS not yet implemented for meso");
    }
    return result;
}

std::vector<TraCINextStopData>
Vehicle::getNextStops(const std::string& vehicleID) {
    return getStops(vehicleID, 0);
}

std::vector<TraCINextStopData>
Vehicle::getStops(const std::string& vehicleID, int limit) {
    std::vector<TraCINextStopData> result;
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("getNextStops not yet implemented for meso");
        return result;
    }
    if (limit < 0) {
        // return past stops up to the given limit
        const std::vector<SUMOVehicleParameter::Stop>& pastStops = veh->getPastStops();
        const int n = (int)pastStops.size();
        for (int i = MAX2(0, n + limit); i < n; i++) {
            result.push_back(buildStopData(pastStops[i]));
        }
    } else {
        for (const MSVehicle::Stop& stop : veh->getStops()) {
            if (!stop.collision) {
                TraCINextStopData nsd = buildStopData(stop.pars);
                if (stop.reached) {
                    nsd.duration = STEPS2TIME(stop.duration);
                }
                result.push_back(nsd);
                if (limit > 0 && (int)result.size() >= limit) {
                    break;
                }
            }
        }
    }
    return result;
}


int
Vehicle::getStopState(const std::string& vehicleID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("getStopState not yet implemented for meso");
        return 0;
    }
    int result = 0;
    if (veh->isStopped()) {
        const MSVehicle::Stop& stop = veh->getNextStop();
        result = ((stop.reached ? 1 : 0) +
                  (stop.pars.parking ? 2 : 0) +
                  (stop.pars.triggered ? 4 : 0) +
                  (stop.pars.containerTriggered ? 8 : 0) +
                  (stop.busstop != nullptr ? 16 : 0) +
                  (stop.containerstop != nullptr ? 32 : 0) +
                  (stop.chargingStation != nullptr ? 64 : 0) +
                  (stop.parkingarea != nullptr ? 128 : 0));
    }
    return result;
}


double
Vehicle::getDistance(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    if (veh->isOnRoad()) {
        return veh->getOdometer();
    } else {
        return INVALID_DOUBLE_VALUE;
    }
}


double
Vehicle::getDrivingDistance(const std::string& vehicleID, const std::string& edgeID, double position, int /* laneIndex */) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSVehicle* microVeh = dynamic_cast<MSVehicle*>(veh);
    if (veh->isOnRoad()) {
        const MSEdge* edge = microVeh != nullptr ? &veh->getLane()->getEdge() : veh->getEdge();
        double distance = veh->getRoute().getDistanceBetween(veh->getPositionOnLane(), position,
                          edge, Helper::getEdge(edgeID), true, veh->getRoutePosition());
        if (distance == std::numeric_limits<double>::max()) {
            return INVALID_DOUBLE_VALUE;
        }
        return distance;
    } else {
        return INVALID_DOUBLE_VALUE;
    }
}


double
Vehicle::getDrivingDistance2D(const std::string& vehicleID, double x, double y) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    if (veh == nullptr) {
        return INVALID_DOUBLE_VALUE;
    }
    if (veh->isOnRoad()) {
        std::pair<MSLane*, double> roadPos = Helper::convertCartesianToRoadMap(Position(x, y), veh->getVehicleType().getVehicleClass());
        double distance = veh->getRoute().getDistanceBetween(veh->getPositionOnLane(), roadPos.second,
                          veh->getEdge(), &roadPos.first->getEdge(), true, veh->getRoutePosition());
        if (distance == std::numeric_limits<double>::max()) {
            return INVALID_DOUBLE_VALUE;
        }
        return distance;
    } else {
        return INVALID_DOUBLE_VALUE;
    }
}


double
Vehicle::getAllowedSpeed(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return veh->isOnRoad() ? CALL_MICRO_FUN(veh, getLane()->getVehicleMaxSpeed(veh), veh->getEdge()->getVehicleMaxSpeed(veh)) : INVALID_DOUBLE_VALUE;
}


double
Vehicle::getSpeedFactor(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getChosenSpeedFactor();
}


int
Vehicle::getSpeedMode(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return CALL_MICRO_FUN(veh, getInfluencer().getSpeedMode(), INVALID_INT_VALUE);
}


int
Vehicle::getLaneChangeMode(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    return CALL_MICRO_FUN(veh, getInfluencer().getLaneChangeMode(), INVALID_INT_VALUE);
}


int
Vehicle::getRoutingMode(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getBaseInfluencer().getRoutingMode();
}


std::string
Vehicle::getLine(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getParameter().line;
}


std::vector<std::string>
Vehicle::getVia(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getParameter().via;
}


std::pair<int, int>
Vehicle::getLaneChangeState(const std::string& vehicleID, int direction) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    auto undefined = std::make_pair((int)LCA_UNKNOWN, (int)LCA_UNKNOWN);
    return veh->isOnRoad() ? CALL_MICRO_FUN(veh, getLaneChangeModel().getSavedState(direction), undefined) : undefined;
}


std::string
Vehicle::getParameter(const std::string& vehicleID, const std::string& key) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    std::string error;
    std::string result = veh->getPrefixedParameter(key, error);
    if (error != "") {
        throw TraCIException(error);
    }
    return result;
}


LIBSUMO_GET_PARAMETER_WITH_KEY_IMPLEMENTATION(Vehicle)


std::vector<std::pair<std::string, double> >
Vehicle::getNeighbors(const std::string& vehicleID, const int mode) {
    int dir = (1 & mode) != 0 ? -1 : 1;
    bool queryLeaders = (2 & mode) != 0;
    bool blockersOnly = (4 & mode) != 0;

    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    std::vector<std::pair<std::string, double> > neighs;
    if (veh == nullptr) {
        return neighs;
    }
    auto& lcm = veh->getLaneChangeModel();

#ifdef DEBUG_NEIGHBORS
    if (DEBUG_COND) {
        std::cout << "getNeighbors() for veh '" << vehicleID << "': dir=" << dir
                  << ", queryLeaders=" << queryLeaders
                  << ", blockersOnly=" << blockersOnly << std::endl;
    }
#endif



    if (blockersOnly) {
        // Check if a blocking neigh exists in the given direction
        bool blocked = false;
        if (dir == -1) {
            if (queryLeaders) {
                blocked = (lcm.getOwnState() & LCA_BLOCKED_BY_RIGHT_LEADER) != 0;
            } else {
                blocked = (lcm.getOwnState() & LCA_BLOCKED_BY_RIGHT_FOLLOWER) != 0;
            }
        } else {
            if (queryLeaders) {
                blocked = (lcm.getOwnState() & LCA_BLOCKED_BY_LEFT_LEADER) != 0;
            } else {
                blocked = (lcm.getOwnState() & LCA_BLOCKED_BY_LEFT_FOLLOWER) != 0;
            }
        }

#ifdef DEBUG_NEIGHBORS
        if (DEBUG_COND) {
            std::cout << " blocked=" << blocked << std::endl;
        }
#endif

        if (!blocked) {
            // Not blocked => return empty vector
            return neighs;
        }
    }

    const std::shared_ptr<MSLeaderDistanceInfo> res = queryLeaders ? lcm.getLeaders(dir) : lcm.getFollowers(dir);
    if (res != nullptr && res->hasVehicles()) {
        auto distIt = begin(res->getDistances());
        auto vehIt = begin(res->getVehicles());
        while (distIt != end(res->getDistances())) {
            if (*vehIt != nullptr) {
                if (neighs.size() == 0 || neighs.back().first != (*vehIt)->getID()) {
                    neighs.push_back(std::make_pair((*vehIt)->getID(), *distIt));
                }
            }
            ++vehIt;
            ++distIt;
        }
    }
    return neighs;
}


double
Vehicle::getFollowSpeed(const std::string& vehicleID, double speed, double gap, double leaderSpeed, double leaderMaxDecel, const std::string& leaderID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("getFollowSpeed not applicable for meso");
        return INVALID_DOUBLE_VALUE;
    }
    MSVehicle* leader = dynamic_cast<MSVehicle*>(MSNet::getInstance()->getVehicleControl().getVehicle(leaderID));
    return veh->getCarFollowModel().followSpeed(veh, speed, gap, leaderSpeed, leaderMaxDecel, leader);
}


double
Vehicle::getSecureGap(const std::string& vehicleID, double speed, double leaderSpeed, double leaderMaxDecel, const std::string& leaderID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("getSecureGap not applicable for meso");
        return INVALID_DOUBLE_VALUE;
    }
    MSVehicle* leader = dynamic_cast<MSVehicle*>(MSNet::getInstance()->getVehicleControl().getVehicle(leaderID));
    return veh->getCarFollowModel().getSecureGap(veh, leader, speed, leaderSpeed, leaderMaxDecel);
}


double
Vehicle::getStopSpeed(const std::string& vehicleID, const double speed, double gap) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("getStopSpeed not applicable for meso");
        return INVALID_DOUBLE_VALUE;
    }
    return veh->getCarFollowModel().stopSpeed(veh, speed, gap);
}

double
Vehicle::getStopDelay(const std::string& vehicleID) {
    return Helper::getVehicle(vehicleID)->getStopDelay();
}

std::vector<std::string>
Vehicle::getTaxiFleet(int taxiState) {
    std::vector<std::string> result;
    for (MSDevice_Taxi* taxi : MSDevice_Taxi::getFleet()) {
        if (taxi->getHolder().hasDeparted()) {
            if (taxiState == -1
                    || (taxiState == 0 && taxi->getState() == 0)
                    || (taxiState != 0 && (taxi->getState() & taxiState) == taxiState)) {
                result.push_back(taxi->getHolder().getID());
            }
        }
    }
    return result;
}

std::string
Vehicle::getEmissionClass(const std::string& vehicleID) {
    return PollutantsInterface::getName(Helper::getVehicleType(vehicleID).getEmissionClass());
}

std::string
Vehicle::getShapeClass(const std::string& vehicleID) {
    return getVehicleShapeName(Helper::getVehicleType(vehicleID).getGuiShape());
}


double
Vehicle::getLength(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getLength();
}


double
Vehicle::getAccel(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getMaxAccel();
}


double
Vehicle::getDecel(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getMaxDecel();
}


double Vehicle::getEmergencyDecel(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getEmergencyDecel();
}


double Vehicle::getApparentDecel(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getApparentDecel();
}


double Vehicle::getActionStepLength(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getActionStepLengthSecs();
}


double Vehicle::getLastActionTime(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSVehicle* microVeh = dynamic_cast<MSVehicle*>(veh);
    if (microVeh != nullptr) {
        return STEPS2TIME(microVeh->getLastActionTime());
    } else {
        MEVehicle* mesoVeh = dynamic_cast<MEVehicle*>(veh);
        return STEPS2TIME(mesoVeh->getEventTime());
    }
}


double
Vehicle::getTau(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getHeadwayTime();
}


double
Vehicle::getImperfection(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getCarFollowModel().getImperfection();
}


double
Vehicle::getSpeedDeviation(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getSpeedFactor().getParameter()[1];
}


std::string
Vehicle::getVehicleClass(const std::string& vehicleID) {
    return toString(Helper::getVehicleType(vehicleID).getVehicleClass());
}


double
Vehicle::getMinGap(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getMinGap();
}


double
Vehicle::getMinGapLat(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getMinGapLat();
}


double
Vehicle::getMaxSpeed(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getMaxSpeed();
}


double
Vehicle::getMaxSpeedLat(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getMaxSpeedLat();
}


std::string
Vehicle::getLateralAlignment(const std::string& vehicleID) {
    return toString(Helper::getVehicleType(vehicleID).getPreferredLateralAlignment());
}


double
Vehicle::getWidth(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getWidth();
}


double
Vehicle::getHeight(const std::string& vehicleID) {
    return Helper::getVehicleType(vehicleID).getHeight();
}


void
Vehicle::setStop(const std::string& vehicleID,
                 const std::string& edgeID,
                 double pos,
                 int laneIndex,
                 double duration,
                 int flags,
                 double startPos,
                 double until) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("setStop not yet implemented for meso");
        return;
    }
    SUMOVehicleParameter::Stop stopPars = buildStopParameters(edgeID,
                                          pos, laneIndex, startPos, flags, duration, until);
    std::string error;
    if (!veh->addTraciStop(stopPars, error)) {
        throw TraCIException(error);
    }
}


void
Vehicle::replaceStop(const std::string& vehicleID,
                     int nextStopIndex,
                     const std::string& edgeID,
                     double pos,
                     int laneIndex,
                     double duration,
                     int flags,
                     double startPos,
                     double until) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("replaceStop not yet implemented for meso");
        return;
    }
    SUMOVehicleParameter::Stop stopPars = buildStopParameters(edgeID,
                                          pos, laneIndex, startPos, flags, duration, until);

    std::string error;
    if (!veh->replaceStop(nextStopIndex, stopPars, "traci:replaceStop", error)) {
        throw TraCIException("Stop replacement failed for vehicle '" + vehicleID + "' (" + error + ").");
    }
}


void
Vehicle::rerouteParkingArea(const std::string& vehicleID, const std::string& parkingAreaID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("rerouteParkingArea not yet implemented for meso");
        return;
    }
    std::string error;
    // Forward command to vehicle
    if (!veh->rerouteParkingArea(parkingAreaID, error)) {
        throw TraCIException(error);
    }
}

void
Vehicle::resume(const std::string& vehicleID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("resume not yet implemented for meso");
        return;
    }
    if (!veh->hasStops()) {
        throw TraCIException("Failed to resume vehicle '" + veh->getID() + "', it has no stops.");
    }
    if (!veh->resumeFromStopping()) {
        MSVehicle::Stop& sto = veh->getNextStop();
        std::ostringstream strs;
        strs << "reached: " << sto.reached;
        strs << ", duration:" << sto.duration;
        strs << ", edge:" << (*sto.edge)->getID();
        strs << ", startPos: " << sto.pars.startPos;
        std::string posStr = strs.str();
        throw TraCIException("Failed to resume from stopping for vehicle '" + veh->getID() + "', " + posStr);
    }
}


void
Vehicle::changeTarget(const std::string& vehicleID, const std::string& edgeID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    const MSEdge* destEdge = MSEdge::dictionary(edgeID);
    const bool onInit = isOnInit(vehicleID);
    if (destEdge == nullptr) {
        throw TraCIException("Destination edge '" + edgeID + "' is not known.");
    }
    // build a new route between the vehicle's current edge and destination edge
    ConstMSEdgeVector newRoute;
    const MSEdge* currentEdge = veh->getRerouteOrigin();
    veh->getBaseInfluencer().getRouterTT(veh->getRNGIndex(), veh->getVClass()).compute(
        currentEdge, destEdge, veh, MSNet::getInstance()->getCurrentTimeStep(), newRoute);
    // replace the vehicle's route by the new one (cost is updated by call to reroute())
    if (!veh->replaceRouteEdges(newRoute, -1, 0, "traci:changeTarget", onInit)) {
        throw TraCIException("Route replacement failed for vehicle '" + veh->getID() + "'.");
    }
    // route again to ensure usage of via/stops
    try {
        veh->reroute(MSNet::getInstance()->getCurrentTimeStep(), "traci:changeTarget",
                     veh->getBaseInfluencer().getRouterTT(veh->getRNGIndex(), veh->getVClass()), onInit);
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
}


void
Vehicle::changeLane(const std::string& vehicleID, int laneIndex, double duration) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("changeLane not applicable for meso");
        return;
    }

    std::vector<std::pair<SUMOTime, int> > laneTimeLine;
    laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), laneIndex));
    laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep() + TIME2STEPS(duration), laneIndex));
    veh->getInfluencer().setLaneTimeLine(laneTimeLine);
}

void
Vehicle::changeLaneRelative(const std::string& vehicleID, int indexOffset, double duration) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("changeLaneRelative not applicable for meso");
        return;
    }

    std::vector<std::pair<SUMOTime, int> > laneTimeLine;
    int laneIndex = veh->getLaneIndex() + indexOffset;
    if (laneIndex < 0 && !veh->getLaneChangeModel().isOpposite()) {
        WRITE_WARNING("Ignoring indexOffset -1 for vehicle '" + vehicleID + "' which is already on laneIndex 0");
    } else {
        laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), laneIndex));
        laneTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep() + TIME2STEPS(duration), laneIndex));
        veh->getInfluencer().setLaneTimeLine(laneTimeLine);
    }
}


void
Vehicle::changeSublane(const std::string& vehicleID, double latDist) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("changeSublane not applicable for meso");
        return;
    }

    veh->getInfluencer().setSublaneChange(latDist);
}


void
Vehicle::add(const std::string& vehicleID,
             const std::string& routeID,
             const std::string& typeID,
             const std::string& depart,
             const std::string& departLane,
             const std::string& departPos,
             const std::string& departSpeed,
             const std::string& arrivalLane,
             const std::string& arrivalPos,
             const std::string& arrivalSpeed,
             const std::string& fromTaz,
             const std::string& toTaz,
             const std::string& line,
             int /*personCapacity*/,
             int personNumber) {
    SUMOVehicle* veh = MSNet::getInstance()->getVehicleControl().getVehicle(vehicleID);
    if (veh != nullptr) {
        throw TraCIException("The vehicle '" + vehicleID + "' to add already exists.");
    }

    SUMOVehicleParameter vehicleParams;
    vehicleParams.id = vehicleID;
    MSVehicleType* vehicleType = MSNet::getInstance()->getVehicleControl().getVType(typeID);
    if (!vehicleType) {
        throw TraCIException("Invalid type '" + typeID + "' for vehicle '" + vehicleID + "'.");
    }
    const MSRoute* route = MSRoute::dictionary(routeID);
    if (!route) {
        if (routeID == "") {
            // assume, route was intentionally left blank because the caller
            // intends to control the vehicle remotely
            SUMOVehicleClass vclass = vehicleType->getVehicleClass();
            const std::string dummyRouteID = "DUMMY_ROUTE_" + SumoVehicleClassStrings.getString(vclass);
            route = MSRoute::dictionary(dummyRouteID);
            if (route == nullptr) {
                for (MSEdge* e : MSEdge::getAllEdges()) {
                    if (e->getFunction() == SumoXMLEdgeFunc::NORMAL && (e->getPermissions() & vclass) == vclass) {
                        std::vector<std::string>  edges;
                        edges.push_back(e->getID());
                        libsumo::Route::add(dummyRouteID, edges);
                        break;
                    }
                }
            }
            route = MSRoute::dictionary(dummyRouteID);
            if (!route) {
                throw TraCIException("Could not build dummy route for vehicle class: '" + SumoVehicleClassStrings.getString(vehicleType->getVehicleClass()) + "'");
            }
        } else {
            throw TraCIException("Invalid route '" + routeID + "' for vehicle '" + vehicleID + "'.");
        }
    }
    // check if the route implies a trip
    if (route->getEdges().size() == 2) {
        const MSEdgeVector& succ = route->getEdges().front()->getSuccessors();
        if (std::find(succ.begin(), succ.end(), route->getEdges().back()) == succ.end()) {
            vehicleParams.parametersSet |= VEHPARS_FORCE_REROUTE;
        }
    }
    if (fromTaz != "" || toTaz != "") {
        vehicleParams.parametersSet |= VEHPARS_FORCE_REROUTE;
    }
    std::string error;
    if (!SUMOVehicleParameter::parseDepart(depart, "vehicle", vehicleID, vehicleParams.depart, vehicleParams.departProcedure, error)) {
        throw TraCIException(error);
    }
    if (vehicleParams.departProcedure == DEPART_GIVEN && vehicleParams.depart < MSNet::getInstance()->getCurrentTimeStep()) {
        vehicleParams.depart = MSNet::getInstance()->getCurrentTimeStep();
        WRITE_WARNING("Departure time for vehicle '" + vehicleID + "' is in the past; using current time instead.");
    } else if (vehicleParams.departProcedure == DEPART_NOW) {
        vehicleParams.depart = MSNet::getInstance()->getCurrentTimeStep();
    }
    if (!SUMOVehicleParameter::parseDepartLane(departLane, "vehicle", vehicleID, vehicleParams.departLane, vehicleParams.departLaneProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseDepartPos(departPos, "vehicle", vehicleID, vehicleParams.departPos, vehicleParams.departPosProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseDepartSpeed(departSpeed, "vehicle", vehicleID, vehicleParams.departSpeed, vehicleParams.departSpeedProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseArrivalLane(arrivalLane, "vehicle", vehicleID, vehicleParams.arrivalLane, vehicleParams.arrivalLaneProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseArrivalPos(arrivalPos, "vehicle", vehicleID, vehicleParams.arrivalPos, vehicleParams.arrivalPosProcedure, error)) {
        throw TraCIException(error);
    }
    if (!SUMOVehicleParameter::parseArrivalSpeed(arrivalSpeed, "vehicle", vehicleID, vehicleParams.arrivalSpeed, vehicleParams.arrivalSpeedProcedure, error)) {
        throw TraCIException(error);
    }
    vehicleParams.fromTaz = fromTaz;
    vehicleParams.toTaz = toTaz;
    vehicleParams.line = line;
    //vehicleParams.personCapacity = personCapacity;
    vehicleParams.personNumber = personNumber;

    SUMOVehicleParameter* params = new SUMOVehicleParameter(vehicleParams);
    try {
        SUMOVehicle* vehicle = MSNet::getInstance()->getVehicleControl().buildVehicle(params, route, vehicleType, true, false);
        if (fromTaz == "" && !route->getEdges().front()->validateDepartSpeed(*vehicle)) {
            MSNet::getInstance()->getVehicleControl().deleteVehicle(vehicle, true);
            throw TraCIException("Departure speed for vehicle '" + vehicleID + "' is too high for the departure edge '" + route->getEdges().front()->getID() + "'.");
        }
        std::string msg;
        if (vehicle->getRouteValidity(true, true) != MSBaseVehicle::ROUTE_VALID) {
            MSNet::getInstance()->getVehicleControl().deleteVehicle(vehicle, true);
            throw TraCIException("Vehicle '" + vehicleID + "' has no valid route. ");
        }
        MSNet::getInstance()->getVehicleControl().addVehicle(vehicleParams.id, vehicle);
        if (vehicleParams.departProcedure != DEPART_TRIGGERED && vehicleParams.departProcedure != DEPART_CONTAINER_TRIGGERED) {
            MSNet::getInstance()->getInsertionControl().add(vehicle);
        }
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
}


void
Vehicle::moveToXY(const std::string& vehicleID, const std::string& edgeID, const int laneIndex,
                  const double x, const double y, double angle, const int keepRoute) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("moveToXY not yet implemented for meso");
        return;
    }
    const bool doKeepRoute = (keepRoute & 1) != 0 && veh->getID() != "VTD_EGO";
    const bool mayLeaveNetwork = (keepRoute & 2) != 0;
    const bool ignorePermissions = (keepRoute & 4) != 0;
    const bool setLateralPos = (MSGlobals::gLateralResolution > 0 || mayLeaveNetwork);
    SUMOVehicleClass vClass = ignorePermissions ? SVC_IGNORING : veh->getVClass();
    // process
    const std::string origID = edgeID + "_" + toString(laneIndex);
    // @todo add an interpretation layer for OSM derived origID values (without lane index)
    Position pos(x, y);
#ifdef DEBUG_MOVEXY
    const double origAngle = angle;
#endif
    // angle must be in [0,360] because it will be compared against those returned by naviDegree()
    // angle set to INVALID_DOUBLE_VALUE is ignored in the evaluated and later set to the angle of the matched lane
    if (angle != INVALID_DOUBLE_VALUE) {
        while (angle >= 360.) {
            angle -= 360.;
        }
        while (angle < 0.) {
            angle += 360.;
        }
    }

    Position vehPos = veh->getPosition();
#ifdef DEBUG_MOVEXY
    std::cout << std::endl << SIMTIME << " moveToXY veh=" << veh->getID() << " vehPos=" << vehPos << " lane=" << Named::getIDSecure(veh->getLane()) << std::endl;
    std::cout << " wantedPos=" << pos << " origID=" << origID << " laneIndex=" << laneIndex << " origAngle=" << origAngle << " angle=" << angle << " keepRoute=" << keepRoute << std::endl;
#endif

    ConstMSEdgeVector edges;
    MSLane* lane = nullptr;
    double lanePos;
    double lanePosLat = 0;
    double bestDistance = std::numeric_limits<double>::max();
    int routeOffset = 0;
    bool found;
    double maxRouteDistance = 100;
    /* EGO vehicle is known to have a fixed route. @todo make this into a parameter of the TraCI call */
    if (doKeepRoute) {
        // case a): vehicle is on its earlier route
        //  we additionally assume it is moving forward (SUMO-limit);
        //  note that the route ("edges") is not changed in this case

        found = Helper::moveToXYMap_matchingRoutePosition(pos, origID,
                veh->getRoute().getEdges(), (int)(veh->getCurrentRouteEdge() - veh->getRoute().begin()),
                vClass, setLateralPos,
                bestDistance, &lane, lanePos, routeOffset);
        // @note silenty ignoring mapping failure
    } else {
        double speed = pos.distanceTo2D(veh->getPosition()); // !!!veh->getSpeed();
        found = Helper::moveToXYMap(pos, maxRouteDistance, mayLeaveNetwork, origID, angle,
                                    speed, veh->getRoute().getEdges(), veh->getRoutePosition(), veh->getLane(), veh->getPositionOnLane(), veh->isOnRoad(),
                                    vClass, setLateralPos,
                                    bestDistance, &lane, lanePos, routeOffset, edges);
    }
    if ((found && bestDistance <= maxRouteDistance) || mayLeaveNetwork) {
        // optionally compute lateral offset
        pos.setz(veh->getPosition().z());
        if (found && setLateralPos) {
            const double perpDist = lane->getShape().distance2D(pos, false);
            if (perpDist != GeomHelper::INVALID_OFFSET) {
                lanePosLat = perpDist;
                if (!mayLeaveNetwork) {
                    lanePosLat = MIN2(lanePosLat, 0.5 * (lane->getWidth() + veh->getVehicleType().getWidth() - MSGlobals::gLateralResolution));
                }
                // figure out whether the offset is to the left or to the right
                PositionVector tmp = lane->getShape();
                try {
                    tmp.move2side(-lanePosLat); // moved to left
                } catch (ProcessError&) {
                    WRITE_WARNING("Could not determine position on lane '" + lane->getID() + "' at lateral position " + toString(-lanePosLat) + ".");
                }
                //std::cout << " lane=" << lane->getID() << " posLat=" << lanePosLat << " shape=" << lane->getShape() << " tmp=" << tmp << " tmpDist=" << tmp.distance2D(pos) << "\n";
                if (tmp.distance2D(pos) > perpDist) {
                    lanePosLat = -lanePosLat;
                }
            }
            pos.setz(lane->geometryPositionAtOffset(lanePos).z());
        }
        if (found && !mayLeaveNetwork && MSGlobals::gLateralResolution < 0) {
            // mapped position may differ from pos
            pos = lane->geometryPositionAtOffset(lanePos, -lanePosLat);
        }
        assert((found && lane != 0) || (!found && lane == 0));
        assert(!ISNAN(lanePos));
        if (angle == INVALID_DOUBLE_VALUE) {
            if (lane != nullptr) {
                angle = GeomHelper::naviDegree(lane->getShape().rotationAtOffset(lanePos));
            } else {
                // compute angle outside road network from old and new position
                angle = GeomHelper::naviDegree(veh->getPosition().angleTo2D(pos));
            }
        }
        // use the best we have
#ifdef DEBUG_MOVEXY
        std::cout << SIMTIME << " veh=" << vehicleID + " moveToXYResult lane='" << Named::getIDSecure(lane) << "' lanePos=" << lanePos << " lanePosLat=" << lanePosLat << "\n";
#endif
        Helper::setRemoteControlled(veh, pos, lane, lanePos, lanePosLat, angle, routeOffset, edges, MSNet::getInstance()->getCurrentTimeStep());
        if (!veh->isOnRoad()) {
            MSNet::getInstance()->getInsertionControl().alreadyDeparted(veh);
        }
    } else {
        if (lane == nullptr) {
            throw TraCIException("Could not map vehicle '" + vehicleID + "', no road found within " + toString(maxRouteDistance) + "m.");
        } else {
            throw TraCIException("Could not map vehicle '" + vehicleID + "', distance to road is " + toString(bestDistance) + ".");
        }
    }
}

void
Vehicle::slowDown(const std::string& vehicleID, double speed, double duration) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("slowDown not applicable for meso");
        return;
    }

    std::vector<std::pair<SUMOTime, double> > speedTimeLine;
    speedTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), veh->getSpeed()));
    speedTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep() + TIME2STEPS(duration), speed));
    veh->getInfluencer().setSpeedTimeLine(speedTimeLine);
}

void
Vehicle::openGap(const std::string& vehicleID, double newTimeHeadway, double newSpaceHeadway, double duration, double changeRate, double maxDecel, const std::string& referenceVehID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("openGap not applicable for meso");
        return;
    }

    MSVehicle* refVeh = nullptr;
    if (referenceVehID != "") {
        refVeh = dynamic_cast<MSVehicle*>(Helper::getVehicle(referenceVehID));
    }
    const double originalTau = veh->getVehicleType().getCarFollowModel().getHeadwayTime();
    if (newTimeHeadway == -1) {
        newTimeHeadway = originalTau;
    }
    if (originalTau > newTimeHeadway) {
        WRITE_WARNING("Ignoring openGap(). New time headway must not be smaller than the original.");
        return;
    }
    veh->getInfluencer().activateGapController(originalTau, newTimeHeadway, newSpaceHeadway, duration, changeRate, maxDecel, refVeh);
}

void
Vehicle::deactivateGapControl(const std::string& vehicleID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("deactivateGapControl not applicable for meso");
        return;
    }

    if (veh->hasInfluencer()) {
        veh->getInfluencer().deactivateGapController();
    }
}

void
Vehicle::requestToC(const std::string& vehID, double leadTime) {
    setParameter(vehID, "device.toc.requestToC", toString(leadTime));
}

void
Vehicle::setSpeed(const std::string& vehicleID, double speed) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("setSpeed not yet implemented for meso");
        return;
    }

    std::vector<std::pair<SUMOTime, double> > speedTimeLine;
    if (speed >= 0) {
        speedTimeLine.push_back(std::make_pair(MSNet::getInstance()->getCurrentTimeStep(), speed));
        speedTimeLine.push_back(std::make_pair(SUMOTime_MAX - DELTA_T, speed));
    }
    veh->getInfluencer().setSpeedTimeLine(speedTimeLine);
}

void
Vehicle::setPreviousSpeed(const std::string& vehicleID, double prevspeed) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("setPreviousSpeed not yet implemented for meso");
        return;
    }

    veh->setPreviousSpeed(prevspeed);
}

void
Vehicle::setSpeedMode(const std::string& vehicleID, int speedMode) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("setSpeedMode not yet implemented for meso");
        return;
    }

    veh->getInfluencer().setSpeedMode(speedMode);
}

void
Vehicle::setLaneChangeMode(const std::string& vehicleID, int laneChangeMode) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("setLaneChangeMode not applicable for meso");
        return;
    }

    veh->getInfluencer().setLaneChangeMode(laneChangeMode);
}

void
Vehicle::setRoutingMode(const std::string& vehicleID, int routingMode) {
    Helper::getVehicle(vehicleID)->getBaseInfluencer().setRoutingMode(routingMode);
}

void
Vehicle::setType(const std::string& vehicleID, const std::string& typeID) {
    MSVehicleType* vehicleType = MSNet::getInstance()->getVehicleControl().getVType(typeID);
    if (vehicleType == nullptr) {
        throw TraCIException("Vehicle type '" + typeID + "' is not known");
    }
    Helper::getVehicle(vehicleID)->replaceVehicleType(vehicleType);
}

void
Vehicle::setRouteID(const std::string& vehicleID, const std::string& routeID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    const MSRoute* r = MSRoute::dictionary(routeID);
    if (r == nullptr) {
        throw TraCIException("The route '" + routeID + "' is not known.");
    }
    std::string msg;
    if (!veh->hasValidRoute(msg, r)) {
        WRITE_WARNING("Invalid route replacement for vehicle '" + veh->getID() + "'. " + msg);
        if (MSGlobals::gCheckRoutes) {
            throw TraCIException("Route replacement failed for " + veh->getID());
        }
    }

    if (!veh->replaceRoute(r, "traci:setRouteID", veh->getLane() == nullptr)) {
        throw TraCIException("Route replacement failed for " + veh->getID());
    }
}

void
Vehicle::setRoute(const std::string& vehicleID, const std::string& edgeID) {
    setRoute(vehicleID, std::vector<std::string>({edgeID}));
}

void
Vehicle::setRoute(const std::string& vehicleID, const std::vector<std::string>& edgeIDs) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    ConstMSEdgeVector edges;
    try {
        MSEdge::parseEdgesList(edgeIDs, edges, "<unknown>");
        if (edges.size() > 0 && edges.back()->isInternal()) {
            edges.push_back(edges.back()->getLanes()[0]->getNextNormal());
        }
    } catch (ProcessError& e) {
        throw TraCIException("Invalid edge list for vehicle '" + veh->getID() + "' (" + e.what() + ")");
    }
    if (!veh->replaceRouteEdges(edges, -1, 0, "traci:setRoute", veh->getLane() == nullptr, true)) {
        throw TraCIException("Route replacement failed for " + veh->getID());
    }
}

void
Vehicle::updateBestLanes(const std::string& vehicleID) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("updateBestLanes not applicable for meso");
        return;
    }

    veh->updateBestLanes(true);
}


void
Vehicle::setAdaptedTraveltime(const std::string& vehicleID, const std::string& edgeID,
                              double time, double begSeconds, double endSeconds) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSEdge* edge = MSEdge::dictionary(edgeID);
    if (edge == nullptr) {
        throw TraCIException("Edge '" + edgeID + "' is not known.");
    }
    if (time != INVALID_DOUBLE_VALUE) {
        // add time
        if (begSeconds == 0 && endSeconds == std::numeric_limits<double>::max()) {
            // clean up old values before setting whole range
            while (veh->getWeightsStorage().knowsTravelTime(edge)) {
                veh->getWeightsStorage().removeTravelTime(edge);
            }
        }
        veh->getWeightsStorage().addTravelTime(edge, begSeconds, endSeconds, time);
    } else {
        // remove time
        while (veh->getWeightsStorage().knowsTravelTime(edge)) {
            veh->getWeightsStorage().removeTravelTime(edge);
        }
    }
}


void
Vehicle::setEffort(const std::string& vehicleID, const std::string& edgeID,
                   double effort, double begSeconds, double endSeconds) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSEdge* edge = MSEdge::dictionary(edgeID);
    if (edge == nullptr) {
        throw TraCIException("Edge '" + edgeID + "' is not known.");
    }
    if (effort != INVALID_DOUBLE_VALUE) {
        // add effort
        if (begSeconds == 0 && endSeconds == std::numeric_limits<double>::max()) {
            // clean up old values before setting whole range
            while (veh->getWeightsStorage().knowsEffort(edge)) {
                veh->getWeightsStorage().removeEffort(edge);
            }
        }
        veh->getWeightsStorage().addEffort(edge, begSeconds, endSeconds, effort);
    } else {
        // remove effort
        while (veh->getWeightsStorage().knowsEffort(edge)) {
            veh->getWeightsStorage().removeEffort(edge);
        }
    }
}


void
Vehicle::rerouteTraveltime(const std::string& vehicleID, const bool currentTravelTimes) {
    UNUSED_PARAMETER(currentTravelTimes); // !!! see #5943
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    veh->reroute(MSNet::getInstance()->getCurrentTimeStep(), "traci:rerouteTraveltime",
                 veh->getBaseInfluencer().getRouterTT(veh->getRNGIndex(), veh->getVClass()), isOnInit(vehicleID));
}


void
Vehicle::rerouteEffort(const std::string& vehicleID) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    veh->reroute(MSNet::getInstance()->getCurrentTimeStep(), "traci:rerouteEffort",
                 MSNet::getInstance()->getRouterEffort(veh->getRNGIndex()), isOnInit(vehicleID));
}


void
Vehicle::setSignals(const std::string& vehicleID, int signals) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("setSignals not applicable for meso");
        return;
    }

    // set influencer to make the change persistent
    veh->getInfluencer().setSignals(signals);
    // set them now so that getSignals returns the correct value
    veh->switchOffSignal(0x0fffffff);
    if (signals >= 0) {
        veh->switchOnSignal(signals);
    }
}


void
Vehicle::moveTo(const std::string& vehicleID, const std::string& laneID, double position, int reason) {
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_WARNING("moveTo not yet implemented for meso");
        return;
    }

    MSLane* l = MSLane::dictionary(laneID);
    if (l == nullptr) {
        throw TraCIException("Unknown lane '" + laneID + "'.");
    }
    MSEdge* destinationEdge = &l->getEdge();
    const MSEdge* destinationRouteEdge = destinationEdge->getNormalBefore();
    // find edge in the remaining route
    MSRouteIterator it = std::find(veh->getCurrentRouteEdge(), veh->getRoute().end(), destinationRouteEdge);
    if (it == veh->getRoute().end()) {
        // find edge in the edges that were already passed
        it = std::find(veh->getRoute().begin(), veh->getRoute().end(), destinationRouteEdge);
    }
    if (it == veh->getRoute().end() ||
            // internal edge must continue the route
            (destinationEdge->isInternal() &&
             ((it + 1) == veh->getRoute().end()
              || l->getNextNormal() != *(it + 1)))) {
        throw TraCIException("Lane '" + laneID + "' is not on the route of vehicle '" + vehicleID + "'.");
    }
    Position oldPos = vehicle->getPosition();
    veh->onRemovalFromNet(MSMoveReminder::NOTIFICATION_TELEPORT);
    if (veh->getLane() != nullptr) {
        veh->getLane()->removeVehicle(veh, MSMoveReminder::NOTIFICATION_TELEPORT);
    } else {
        veh->setTentativeLaneAndPosition(l, position);
    }
    const int newRouteIndex = (int)(it - veh->getRoute().begin());
    veh->resetRoutePosition(newRouteIndex, veh->getParameter().departLaneProcedure);
    if (!veh->isOnRoad()) {
        MSNet::getInstance()->getInsertionControl().alreadyDeparted(veh);
    }
    MSMoveReminder::Notification moveReminderReason;
    if (veh->hasDeparted()) {
        if (reason == MOVE_TELEPORT) {
            moveReminderReason = MSMoveReminder::NOTIFICATION_TELEPORT;
        } else if (reason == MOVE_NORMAL) {
            moveReminderReason = MSMoveReminder::NOTIFICATION_JUNCTION;
        } else if (reason == MOVE_AUTOMATIC) {
            Position newPos = l->geometryPositionAtOffset(position);
            const double dist = newPos.distanceTo2D(oldPos);
            if (dist < SPEED2DIST(veh->getMaxSpeed())) {
                moveReminderReason = MSMoveReminder::NOTIFICATION_JUNCTION;
            } else {
                moveReminderReason = MSMoveReminder::NOTIFICATION_TELEPORT;
            }
        } else {
            throw TraCIException("Invalid moveTo reason '" + toString(reason) + "' for vehicle '" + vehicleID + "'.");
        }
    } else {
        moveReminderReason = MSMoveReminder::NOTIFICATION_DEPARTED;
    }

    l->forceVehicleInsertion(veh, position, moveReminderReason);
}


void
Vehicle::setActionStepLength(const std::string& vehicleID, double actionStepLength, bool resetActionOffset) {
    if (actionStepLength < 0.0) {
        WRITE_ERROR("Invalid action step length (<0). Ignoring command setActionStepLength().");
        return;
    }
    MSBaseVehicle* vehicle = Helper::getVehicle(vehicleID);
    MSVehicle* veh = dynamic_cast<MSVehicle*>(vehicle);
    if (veh == nullptr) {
        WRITE_ERROR("setActionStepLength not applicable for meso");
        return;
    }

    if (actionStepLength == 0.) {
        veh->resetActionOffset();
    } else {
        veh->setActionStepLength(actionStepLength, resetActionOffset);
    }
}


void
Vehicle::remove(const std::string& vehicleID, char reason) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSMoveReminder::Notification n = MSMoveReminder::NOTIFICATION_ARRIVED;
    switch (reason) {
        case REMOVE_TELEPORT:
            // XXX semantics unclear
            // n = MSMoveReminder::NOTIFICATION_TELEPORT;
            n = MSMoveReminder::NOTIFICATION_TELEPORT_ARRIVED;
            break;
        case REMOVE_PARKING:
            // XXX semantics unclear
            // n = MSMoveReminder::NOTIFICATION_PARKING;
            n = MSMoveReminder::NOTIFICATION_ARRIVED;
            break;
        case REMOVE_ARRIVED:
            n = MSMoveReminder::NOTIFICATION_ARRIVED;
            break;
        case REMOVE_VAPORIZED:
            n = MSMoveReminder::NOTIFICATION_VAPORIZED_TRACI;
            break;
        case REMOVE_TELEPORT_ARRIVED:
            n = MSMoveReminder::NOTIFICATION_TELEPORT_ARRIVED;
            break;
        default:
            throw TraCIException("Unknown removal status.");
    }
    if (veh->hasDeparted()) {
        veh->onRemovalFromNet(n);
        if (veh->getLane() != nullptr) {
            veh->getLane()->removeVehicle(dynamic_cast<MSVehicle*>(veh), n);
        }
        MSVehicle* microVeh = dynamic_cast<MSVehicle*>(veh);
        if (microVeh != nullptr) {
            MSNet::getInstance()->getVehicleControl().scheduleVehicleRemoval(veh);
        }
    } else {
        MSNet::getInstance()->getInsertionControl().alreadyDeparted(veh);
        MSNet::getInstance()->getVehicleControl().deleteVehicle(veh, true);
    }
}


void
Vehicle::setColor(const std::string& vehicleID, const TraCIColor& col) {
    const SUMOVehicleParameter& p = Helper::getVehicle(vehicleID)->getParameter();
    p.color.set((unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)col.a);
    p.parametersSet |= VEHPARS_COLOR_SET;
}


void
Vehicle::setSpeedFactor(const std::string& vehicleID, double factor) {
    Helper::getVehicle(vehicleID)->setChosenSpeedFactor(factor);
}


void
Vehicle::setLine(const std::string& vehicleID, const std::string& line) {
    Helper::getVehicle(vehicleID)->getParameter().line = line;
}


void
Vehicle::setVia(const std::string& vehicleID, const std::vector<std::string>& via) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    try {
        // ensure edges exist
        ConstMSEdgeVector edges;
        MSEdge::parseEdgesList(via, edges, "<via-edges>");
    } catch (ProcessError& e) {
        throw TraCIException(e.what());
    }
    veh->getParameter().via = via;
}


void
Vehicle::setLength(const std::string& vehicleID, double length) {
    Helper::getVehicle(vehicleID)->getSingularType().setLength(length);
}


void
Vehicle::setMaxSpeed(const std::string& vehicleID, double speed) {
    Helper::getVehicle(vehicleID)->getSingularType().setMaxSpeed(speed);
}


void
Vehicle::setVehicleClass(const std::string& vehicleID, const std::string& clazz) {
    Helper::getVehicle(vehicleID)->getSingularType().setVClass(getVehicleClassID(clazz));
}


void
Vehicle::setShapeClass(const std::string& vehicleID, const std::string& clazz) {
    Helper::getVehicle(vehicleID)->getSingularType().setShape(getVehicleShapeID(clazz));
}


void
Vehicle::setEmissionClass(const std::string& vehicleID, const std::string& clazz) {
    Helper::getVehicle(vehicleID)->getSingularType().setEmissionClass(PollutantsInterface::getClassByName(clazz));
}


void
Vehicle::setWidth(const std::string& vehicleID, double width) {
    Helper::getVehicle(vehicleID)->getSingularType().setWidth(width);
}


void
Vehicle::setHeight(const std::string& vehicleID, double height) {
    Helper::getVehicle(vehicleID)->getSingularType().setHeight(height);
}


void
Vehicle::setMinGap(const std::string& vehicleID, double minGap) {
    Helper::getVehicle(vehicleID)->getSingularType().setMinGap(minGap);
}


void
Vehicle::setAccel(const std::string& vehicleID, double accel) {
    Helper::getVehicle(vehicleID)->getSingularType().setAccel(accel);
}


void
Vehicle::setDecel(const std::string& vehicleID, double decel) {
    VehicleType::setDecel(Helper::getVehicle(vehicleID)->getSingularType().getID(), decel);
}


void
Vehicle::setEmergencyDecel(const std::string& vehicleID, double decel) {
    VehicleType::setEmergencyDecel(Helper::getVehicle(vehicleID)->getSingularType().getID(), decel);
}


void
Vehicle::setApparentDecel(const std::string& vehicleID, double decel) {
    Helper::getVehicle(vehicleID)->getSingularType().setApparentDecel(decel);
}


void
Vehicle::setImperfection(const std::string& vehicleID, double imperfection) {
    Helper::getVehicle(vehicleID)->getSingularType().setImperfection(imperfection);
}


void
Vehicle::setTau(const std::string& vehicleID, double tau) {
    Helper::getVehicle(vehicleID)->getSingularType().setTau(tau);
}


void
Vehicle::setMinGapLat(const std::string& vehicleID, double minGapLat) {
    Helper::getVehicle(vehicleID)->getSingularType().setMinGapLat(minGapLat);
}


void
Vehicle::setMaxSpeedLat(const std::string& vehicleID, double speed) {
    Helper::getVehicle(vehicleID)->getSingularType().setMaxSpeedLat(speed);
}


void
Vehicle::setLateralAlignment(const std::string& vehicleID, const std::string& latAlignment) {
    Helper::getVehicle(vehicleID)->getSingularType().setPreferredLateralAlignment(SUMOXMLDefinitions::LateralAlignments.get(latAlignment));
}


void
Vehicle::setParameter(const std::string& vehicleID, const std::string& key, const std::string& value) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSVehicle* microVeh = dynamic_cast<MSVehicle*>(veh);
    if (StringUtils::startsWith(key, "device.")) {
        StringTokenizer tok(key, ".");
        if (tok.size() < 3) {
            throw TraCIException("Invalid device parameter '" + key + "' for vehicle '" + vehicleID + "'");
        }
        try {
            veh->setDeviceParameter(tok.get(1), key.substr(tok.get(0).size() + tok.get(1).size() + 2), value);
        } catch (InvalidArgument& e) {
            throw TraCIException("Vehicle '" + vehicleID + "' does not support device parameter '" + key + "' (" + e.what() + ").");
        }
    } else if (StringUtils::startsWith(key, "laneChangeModel.")) {
        if (microVeh == nullptr) {
            throw TraCIException("Meso Vehicle '" + vehicleID + "' does not support laneChangeModel parameters.");
        }
        const std::string attrName = key.substr(16);
        try {
            microVeh->getLaneChangeModel().setParameter(attrName, value);
        } catch (InvalidArgument& e) {
            throw TraCIException("Vehicle '" + vehicleID + "' does not support laneChangeModel parameter '" + key + "' (" + e.what() + ").");
        }
    } else if (StringUtils::startsWith(key, "carFollowModel.")) {
        if (microVeh == nullptr) {
            throw TraCIException("Meso Vehicle '" + vehicleID + "' does not support carFollowModel parameters.");
        }
        const std::string attrName = key.substr(15);
        try {
            microVeh->getCarFollowModel().setParameter(microVeh, attrName, value);
        } catch (InvalidArgument& e) {
            throw TraCIException("Vehicle '" + vehicleID + "' does not support carFollowModel parameter '" + key + "' (" + e.what() + ").");
        }
    } else if (StringUtils::startsWith(key, "has.") && StringUtils::endsWith(key, ".device")) {
        StringTokenizer tok(key, ".");
        if (tok.size() != 3) {
            throw TraCIException("Invalid request for device status change. Expected format is 'has.DEVICENAME.device'");
        }
        const std::string deviceName = tok.get(1);
        bool create;
        try {
            create = StringUtils::toBool(value);
        } catch (BoolFormatException&) {
            throw TraCIException("Changing device status requires a 'true' or 'false'");
        }
        if (!create) {
            throw TraCIException("Device removal is not supported for device of type '" + deviceName + "'");
        }
        try {
            veh->createDevice(deviceName);
        } catch (InvalidArgument& e) {
            throw TraCIException("Cannot create vehicle device (" + std::string(e.what()) + ").");
        }
    } else {
        ((SUMOVehicleParameter&)veh->getParameter()).setParameter(key, value);
    }
}


void
Vehicle::highlight(const std::string& vehicleID, const TraCIColor& col, double size, const int alphaMax, const double duration, const int type) {
    // NOTE: Code is duplicated in large parts in POI.cpp
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);

    // Center of the highlight circle
    Position center = veh->getPosition();
    const double l2 = veh->getLength() * 0.5;
    center.sub(cos(veh->getAngle())*l2, sin(veh->getAngle())*l2);
    // Size of the highlight circle
    if (size <= 0) {
        size = veh->getLength() * 0.7;
    }
    // Make polygon shape
    const unsigned int nPoints = 34;
    const PositionVector circlePV = GeomHelper::makeRing(size, size + 1., center, nPoints);
    TraCIPositionVector circle = Helper::makeTraCIPositionVector(circlePV);

#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << SIMTIME << " Vehicle::highlight() for vehicle '" << vehicleID << "'\n"
              << " circle: " << circlePV << std::endl;
#endif

    // Find a free polygon id
    int i = 0;
    std::string polyID = veh->getID() + "_hl" + toString(i);
    while (Polygon::exists(polyID)) {
        polyID = veh->getID() + "_hl" + toString(++i);
    }
    // Line width
    double lw = 0.;
    // Layer
    double lyr = 0.;
    if (MSNet::getInstance()->isGUINet()) {
        lyr = GLO_VEHICLE + 0.01;
        lyr += (type + 1) / 257.;
    }
    // Make Polygon
    Polygon::addHighlightPolygon(vehicleID, type, polyID, circle, col, true, "highlight", (int)lyr, lw);

    // Animation time line
    double maxAttack = 1.0; // maximal fade-in time
    std::vector<double> timeSpan;
    if (duration > 0.) {
        timeSpan = {0, MIN2(maxAttack, duration / 3.), 2.*duration / 3., duration};
    }
    // Alpha time line
    std::vector<double> alphaSpan;
    if (alphaMax > 0.) {
        alphaSpan = {0., (double) alphaMax, (double)(alphaMax) / 3., 0.};
    }
    // Attach dynamics
    Polygon::addDynamics(polyID, vehicleID, timeSpan, alphaSpan, false, true);
}

void
Vehicle::dispatchTaxi(const std::string& vehicleID,  const std::vector<std::string>& reservations) {
    MSBaseVehicle* veh = Helper::getVehicle(vehicleID);
    MSDevice_Taxi* taxi = static_cast<MSDevice_Taxi*>(veh->getDevice(typeid(MSDevice_Taxi)));
    if (taxi == nullptr) {
        throw TraCIException("Vehicle '" + vehicleID + "' is not a taxi");
    }
    MSDispatch* dispatcher = MSDevice_Taxi::getDispatchAlgorithm();
    if (dispatcher == nullptr) {
        throw TraCIException("Cannot dispatch taxi because no reservations have been made");
    }
    MSDispatch_TraCI* traciDispatcher = dynamic_cast<MSDispatch_TraCI*>(dispatcher);
    if (traciDispatcher == nullptr) {
        throw TraCIException("device.taxi.dispatch-algorithm 'traci' has not been loaded");
    }
    if (reservations.size() == 0) {
        throw TraCIException("No reservations have been specified for vehicle '" + vehicleID + "'");
    }
    try {
        traciDispatcher->interpretDispatch(taxi, reservations);
    } catch (InvalidArgument& e) {
        throw TraCIException("Could not interpret reserations for vehicle '" + vehicleID + "' (" + e.what() + ").");
    }
}

LIBSUMO_SUBSCRIPTION_IMPLEMENTATION(Vehicle, VEHICLE)


void
Vehicle::subscribeLeader(const std::string& vehicleID, double dist, double beginTime, double endTime) {
    Vehicle::subscribe(vehicleID, std::vector<int>({libsumo::VAR_LEADER}), beginTime, endTime);
    Helper::addSubscriptionParam(dist);
}


void
Vehicle::addSubscriptionFilterLanes(const std::vector<int>& lanes, bool noOpposite, double downstreamDist, double upstreamDist) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_LANES);
    if (s != nullptr) {
        s->filterLanes = lanes;
    }
    if (noOpposite) {
        addSubscriptionFilterNoOpposite();
    }
    if (downstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
Vehicle::addSubscriptionFilterNoOpposite() {
    Helper::addSubscriptionFilter(SUBS_FILTER_NOOPPOSITE);
}


void
Vehicle::addSubscriptionFilterDownstreamDistance(double dist) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_DOWNSTREAM_DIST);
    if (s != nullptr) {
        s->filterDownstreamDist = dist;
    }
}


void
Vehicle::addSubscriptionFilterUpstreamDistance(double dist) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_UPSTREAM_DIST);
    if (s != nullptr) {
        s->filterUpstreamDist = dist;
    }
}


void
Vehicle::addSubscriptionFilterCFManeuver(double downstreamDist, double upstreamDist) {
    addSubscriptionFilterLeadFollow(std::vector<int>({0}));
    if (downstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }

}


void
Vehicle::addSubscriptionFilterLCManeuver(int direction, bool noOpposite, double downstreamDist, double upstreamDist) {
    std::vector<int> lanes;
    if (direction == INVALID_INT_VALUE) {
        // Using default: both directions
        lanes = std::vector<int>({-1, 0, 1});
    } else if (direction != -1 && direction != 1) {
        WRITE_WARNING("Ignoring lane change subscription filter with non-neighboring lane offset direction=" +
                      toString(direction) + ".");
    } else {
        lanes = std::vector<int>({0, direction});
    }
    addSubscriptionFilterLeadFollow(lanes);
    if (noOpposite) {
        addSubscriptionFilterNoOpposite();
    }
    if (downstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
Vehicle::addSubscriptionFilterLeadFollow(const std::vector<int>& lanes) {
    Helper::addSubscriptionFilter(SUBS_FILTER_LEAD_FOLLOW);
    addSubscriptionFilterLanes(lanes);
}


void
Vehicle::addSubscriptionFilterTurn(double downstreamDist, double upstreamDist) {
    Helper::addSubscriptionFilter(SUBS_FILTER_TURN);
    if (downstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
Vehicle::addSubscriptionFilterVClass(const std::vector<std::string>& vClasses) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_VCLASS);
    if (s != nullptr) {
        s->filterVClasses = parseVehicleClasses(vClasses);
    }
}


void
Vehicle::addSubscriptionFilterVType(const std::vector<std::string>& vTypes) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_VTYPE);
    if (s != nullptr) {
        s->filterVTypes.insert(vTypes.begin(), vTypes.end());
    }
}


void
Vehicle::addSubscriptionFilterFieldOfVision(double openingAngle) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_FIELD_OF_VISION);
    if (s != nullptr) {
        s->filterFieldOfVisionOpeningAngle = openingAngle;
    }
}


void
Vehicle::addSubscriptionFilterLateralDistance(double lateralDist, double downstreamDist, double upstreamDist) {
    Subscription* s = Helper::addSubscriptionFilter(SUBS_FILTER_LATERAL_DIST);
    if (s != nullptr) {
        s->filterLateralDist = lateralDist;
    }
    if (downstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterDownstreamDistance(downstreamDist);
    }
    if (upstreamDist != INVALID_DOUBLE_VALUE) {
        addSubscriptionFilterUpstreamDistance(upstreamDist);
    }
}


void
Vehicle::storeShape(const std::string& id, PositionVector& shape) {
    shape.push_back(Helper::getVehicle(id)->getPosition());
}


std::shared_ptr<VariableWrapper>
Vehicle::makeWrapper() {
    return std::make_shared<Helper::SubscriptionWrapper>(handleVariable, mySubscriptionResults, myContextSubscriptionResults);
}


bool
Vehicle::handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper) {
    switch (variable) {
        case TRACI_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getIDList());
        case ID_COUNT:
            return wrapper->wrapInt(objID, variable, getIDCount());
        case VAR_POSITION:
            return wrapper->wrapPosition(objID, variable, getPosition(objID));
        case VAR_POSITION3D:
            return wrapper->wrapPosition(objID, variable, getPosition(objID, true));
        case VAR_ANGLE:
            return wrapper->wrapDouble(objID, variable, getAngle(objID));
        case VAR_SPEED:
            return wrapper->wrapDouble(objID, variable, getSpeed(objID));
        case VAR_SPEED_LAT:
            return wrapper->wrapDouble(objID, variable, getLateralSpeed(objID));
        case VAR_ROAD_ID:
            return wrapper->wrapString(objID, variable, getRoadID(objID));
        case VAR_SPEED_WITHOUT_TRACI:
            return wrapper->wrapDouble(objID, variable, getSpeedWithoutTraCI(objID));
        case VAR_SLOPE:
            return wrapper->wrapDouble(objID, variable, getSlope(objID));
        case VAR_LANE_ID:
            return wrapper->wrapString(objID, variable, getLaneID(objID));
        case VAR_LANE_INDEX:
            return wrapper->wrapInt(objID, variable, getLaneIndex(objID));
        case VAR_TYPE:
            return wrapper->wrapString(objID, variable, getTypeID(objID));
        case VAR_ROUTE_ID:
            return wrapper->wrapString(objID, variable, getRouteID(objID));
        case VAR_ROUTE_INDEX:
            return wrapper->wrapInt(objID, variable, getRouteIndex(objID));
        case VAR_COLOR:
            return wrapper->wrapColor(objID, variable, getColor(objID));
        case VAR_LANEPOSITION:
            return wrapper->wrapDouble(objID, variable, getLanePosition(objID));
        case VAR_LANEPOSITION_LAT:
            return wrapper->wrapDouble(objID, variable, getLateralLanePosition(objID));
        case VAR_CO2EMISSION:
            return wrapper->wrapDouble(objID, variable, getCO2Emission(objID));
        case VAR_COEMISSION:
            return wrapper->wrapDouble(objID, variable, getCOEmission(objID));
        case VAR_HCEMISSION:
            return wrapper->wrapDouble(objID, variable, getHCEmission(objID));
        case VAR_PMXEMISSION:
            return wrapper->wrapDouble(objID, variable, getPMxEmission(objID));
        case VAR_NOXEMISSION:
            return wrapper->wrapDouble(objID, variable, getNOxEmission(objID));
        case VAR_FUELCONSUMPTION:
            return wrapper->wrapDouble(objID, variable, getFuelConsumption(objID));
        case VAR_NOISEEMISSION:
            return wrapper->wrapDouble(objID, variable, getNoiseEmission(objID));
        case VAR_ELECTRICITYCONSUMPTION:
            return wrapper->wrapDouble(objID, variable, getElectricityConsumption(objID));
        case VAR_PERSON_NUMBER:
            return wrapper->wrapInt(objID, variable, getPersonNumber(objID));
        case VAR_PERSON_CAPACITY:
            return wrapper->wrapInt(objID, variable, getPersonCapacity(objID));
        case LAST_STEP_PERSON_ID_LIST:
            return wrapper->wrapStringList(objID, variable, getPersonIDList(objID));
        case VAR_WAITING_TIME:
            return wrapper->wrapDouble(objID, variable, getWaitingTime(objID));
        case VAR_ACCUMULATED_WAITING_TIME:
            return wrapper->wrapDouble(objID, variable, getAccumulatedWaitingTime(objID));
        case VAR_ROUTE_VALID:
            return wrapper->wrapInt(objID, variable, isRouteValid(objID));
        case VAR_EDGES:
            return wrapper->wrapStringList(objID, variable, getRoute(objID));
        case VAR_SIGNALS:
            return wrapper->wrapInt(objID, variable, getSignals(objID));
        case VAR_STOPSTATE:
            return wrapper->wrapInt(objID, variable, getStopState(objID));
        case VAR_DISTANCE:
            return wrapper->wrapDouble(objID, variable, getDistance(objID));
        case VAR_ALLOWED_SPEED:
            return wrapper->wrapDouble(objID, variable, getAllowedSpeed(objID));
        case VAR_SPEED_FACTOR:
            return wrapper->wrapDouble(objID, variable, getSpeedFactor(objID));
        case VAR_SPEEDSETMODE:
            return wrapper->wrapInt(objID, variable, getSpeedMode(objID));
        case VAR_LANECHANGE_MODE:
            return wrapper->wrapInt(objID, variable, getLaneChangeMode(objID));
        case VAR_ROUTING_MODE:
            return wrapper->wrapInt(objID, variable, getRoutingMode(objID));
        case VAR_LINE:
            return wrapper->wrapString(objID, variable, getLine(objID));
        case VAR_VIA:
            return wrapper->wrapStringList(objID, variable, getVia(objID));
        case VAR_ACCELERATION:
            return wrapper->wrapDouble(objID, variable, getAcceleration(objID));
        case VAR_LASTACTIONTIME:
            return wrapper->wrapDouble(objID, variable, getLastActionTime(objID));
        case VAR_STOP_DELAY:
            return wrapper->wrapDouble(objID, variable, getStopDelay(objID));
        case VAR_LEADER: {
            double dist = 0.;
            // this fallback is needed since the very first call right on subscribing has no parameters set
            if (wrapper->getParams() != nullptr) {
                const std::vector<unsigned char>& param = *wrapper->getParams();
                memcpy(&dist, param.data(), sizeof(dist));
            }
            const auto& lead = getLeader(objID, dist);
            TraCIRoadPosition rp;
            rp.edgeID = lead.first;
            rp.pos = lead.second;
            return wrapper->wrapRoadPosition(objID, variable, rp);
        }
        case VAR_FOLLOWER: {
            double dist = 0.;
            // this fallback is needed since the very first call right on subscribing has no parameters set
            if (wrapper->getParams() != nullptr) {
                const std::vector<unsigned char>& param = *wrapper->getParams();
                memcpy(&dist, param.data(), sizeof(dist));
            }
            const auto& follower = getFollower(objID, dist);
            TraCIRoadPosition rp;
            rp.edgeID = follower.first;
            rp.pos = follower.second;
            return wrapper->wrapRoadPosition(objID, variable, rp);
        }
        case VAR_TAXI_FLEET:
            return false;
        default:
            return VehicleType::handleVariableWithID(objID, getTypeID(objID), variable, wrapper);
    }
}

SUMOVehicleParameter::Stop
Vehicle::buildStopParameters(const std::string& edgeOrStoppingPlaceID,
                             double pos, int laneIndex, double startPos, int flags, double duration, double until) {
    SUMOVehicleParameter::Stop newStop;
    newStop.duration = duration == INVALID_DOUBLE_VALUE ? SUMOTime_MAX : TIME2STEPS(duration);
    newStop.until = until == INVALID_DOUBLE_VALUE ? -1 : TIME2STEPS(until);
    newStop.index = STOP_INDEX_FIT;
    if (newStop.duration >= 0) {
        newStop.parametersSet |= STOP_DURATION_SET;
    }
    if (newStop.until >= 0) {
        newStop.parametersSet |= STOP_UNTIL_SET;
    }
    if ((flags & 1) != 0) {
        newStop.parking = true;
        newStop.parametersSet |= STOP_PARKING_SET;
    }
    if ((flags & 2) != 0) {
        newStop.triggered = true;
        newStop.parametersSet |= STOP_TRIGGER_SET;
    }
    if ((flags & 4) != 0) {
        newStop.containerTriggered = true;
        newStop.parametersSet |= STOP_CONTAINER_TRIGGER_SET;
    }

    SumoXMLTag stoppingPlaceType = SUMO_TAG_NOTHING;
    if ((flags & 8) != 0) {
        stoppingPlaceType = SUMO_TAG_BUS_STOP;
    }
    if ((flags & 16) != 0) {
        stoppingPlaceType = SUMO_TAG_CONTAINER_STOP;
    }
    if ((flags & 32) != 0) {
        stoppingPlaceType = SUMO_TAG_CHARGING_STATION;
    }
    if ((flags & 64) != 0) {
        stoppingPlaceType = SUMO_TAG_PARKING_AREA;
    }
    if ((flags & 128) != 0) {
        stoppingPlaceType = SUMO_TAG_OVERHEAD_WIRE_SEGMENT;
    }

    if (stoppingPlaceType != SUMO_TAG_NOTHING) {
        MSStoppingPlace* bs = MSNet::getInstance()->getStoppingPlace(edgeOrStoppingPlaceID, stoppingPlaceType);
        if (bs == nullptr) {
            throw TraCIException("The " + toString(stoppingPlaceType) + " '" + edgeOrStoppingPlaceID + "' is not known");
        }
        newStop.lane = bs->getLane().getID();
        newStop.endPos = bs->getEndLanePosition();
        newStop.startPos = bs->getBeginLanePosition();
        switch (stoppingPlaceType) {
            case SUMO_TAG_BUS_STOP:
                newStop.busstop = edgeOrStoppingPlaceID;
                break;
            case SUMO_TAG_CONTAINER_STOP:
                newStop.containerstop = edgeOrStoppingPlaceID;
                break;
            case SUMO_TAG_CHARGING_STATION:
                newStop.chargingStation = edgeOrStoppingPlaceID;
                break;
            case SUMO_TAG_PARKING_AREA:
                newStop.parkingarea = edgeOrStoppingPlaceID;
                break;
            case SUMO_TAG_OVERHEAD_WIRE_SEGMENT:
                newStop.overheadWireSegment = edgeOrStoppingPlaceID;
                break;
            default:
                throw TraCIException("Unknown stopping place type '" + toString(stoppingPlaceType) + "'.");
        }
    } else {
        if (startPos == INVALID_DOUBLE_VALUE) {
            startPos = pos - POSITION_EPS;
        }
        if (startPos < 0.) {
            throw TraCIException("Position on lane must not be negative.");
        }
        if (pos < startPos) {
            throw TraCIException("End position on lane must be after start position.");
        }
        // get the actual lane that is referenced by laneIndex
        MSEdge* road = MSEdge::dictionary(edgeOrStoppingPlaceID);
        if (road == nullptr) {
            throw TraCIException("Edge '" + edgeOrStoppingPlaceID + "' is not known.");
        }
        const std::vector<MSLane*>& allLanes = road->getLanes();
        if ((laneIndex < 0) || laneIndex >= (int)(allLanes.size())) {
            throw TraCIException("No lane with index '" + toString(laneIndex) + "' on edge '" + edgeOrStoppingPlaceID + "'.");
        }
        newStop.lane = allLanes[laneIndex]->getID();
        newStop.endPos = pos;
        newStop.startPos = startPos;
        newStop.parametersSet |= STOP_START_SET | STOP_END_SET;
    }
    return newStop;
}

TraCINextStopData
Vehicle::buildStopData(const SUMOVehicleParameter::Stop& stopPar) {
    std::string stoppingPlaceID = "";
    if (stopPar.busstop != "") {
        stoppingPlaceID = stopPar.busstop;
    }
    if (stopPar.containerstop != "") {
        stoppingPlaceID = stopPar.containerstop;
    }
    if (stopPar.parkingarea != "") {
        stoppingPlaceID = stopPar.parkingarea;
    }
    if (stopPar.chargingStation != "") {
        stoppingPlaceID = stopPar.chargingStation;
    }
    if (stopPar.overheadWireSegment != "") {
        stoppingPlaceID = stopPar.overheadWireSegment;
    }
    int stopFlags = (
                        (stopPar.parking ? 1 : 0) +
                        (stopPar.triggered ? 2 : 0) +
                        (stopPar.containerTriggered ? 4 : 0) +
                        (stopPar.busstop != "" ? 8 : 0) +
                        (stopPar.containerstop != "" ? 16 : 0) +
                        (stopPar.chargingStation != "" ? 32 : 0) +
                        (stopPar.parkingarea != "" ? 64 : 0) +
                        (stopPar.overheadWireSegment != "" ? 128 : 0));

    return TraCINextStopData(stopPar.lane,
                             stopPar.startPos,
                             stopPar.endPos,
                             stoppingPlaceID,
                             stopFlags,
                             // negative duration is permitted to indicate that a vehicle cannot
                             // re-enter traffic after parking
                             stopPar.duration != -1 ? STEPS2TIME(stopPar.duration) : INVALID_DOUBLE_VALUE,
                             stopPar.until >= 0 ? STEPS2TIME(stopPar.until) : INVALID_DOUBLE_VALUE,
                             stopPar.arrival >= 0 ? STEPS2TIME(stopPar.arrival) : INVALID_DOUBLE_VALUE,
                             stopPar.actualArrival >= 0 ? STEPS2TIME(stopPar.actualArrival) : INVALID_DOUBLE_VALUE,
                             stopPar.depart >= 0 ? STEPS2TIME(stopPar.depart) : INVALID_DOUBLE_VALUE,
                             stopPar.split,
                             stopPar.join,
                             stopPar.actType,
                             stopPar.tripId,
                             stopPar.line,
                             stopPar.speed);
}



}


/****************************************************************************/
