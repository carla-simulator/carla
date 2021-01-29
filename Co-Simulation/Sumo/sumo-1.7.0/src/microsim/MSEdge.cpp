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
/// @file    MSEdge.cpp
/// @author  Christian Roessel
/// @author  Jakob Erdmann
/// @author  Christoph Sommer
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @author  Sascha Krieg
/// @date    Tue, 06 Mar 2001
///
// A road/street connecting two junctions
/****************************************************************************/
#include <config.h>

#include <algorithm>
#include <iostream>
#include <cassert>
#include <utils/common/StringTokenizer.h>
#include <utils/options/OptionsCont.h>
#include <microsim/devices/MSRoutingEngine.h>
#include <mesosim/MELoop.h>
#include <mesosim/MESegment.h>
#include <mesosim/MEVehicle.h>
#include "MSInsertionControl.h"
#include "MSJunction.h"
#include "MSLane.h"
#include "MSLaneChanger.h"
#include "MSLaneChangerSublane.h"
#include "MSGlobals.h"
#include "MSNet.h"
#include "MSVehicle.h"
#include "MSLeaderInfo.h"
#include <microsim/transportables/MSTransportable.h>
#include "MSEdgeWeightsStorage.h"
#include "MSEdge.h"

#define BEST_LANE_LOOKAHEAD 3000.0

// ===========================================================================
// static member definitions
// ===========================================================================
MSEdge::DictType MSEdge::myDict;
MSEdgeVector MSEdge::myEdges;


// ===========================================================================
// member method definitions
// ===========================================================================
MSEdge::MSEdge(const std::string& id, int numericalID,
               const SumoXMLEdgeFunc function,
               const std::string& streetName,
               const std::string& edgeType,
               int priority,
               double distance) :
    Named(id), myNumericalID(numericalID), myLanes(nullptr),
    myLaneChanger(nullptr), myFunction(function), myVaporizationRequests(0),
    myLastFailedInsertionTime(-1),
    myFromJunction(nullptr), myToJunction(nullptr),
    myOtherTazConnector(nullptr),
    myStreetName(streetName),
    myEdgeType(edgeType),
    myPriority(priority),
    myDistance(distance),
    myWidth(0.),
    myLength(0.),
    myEmptyTraveltime(0.),
    myTimePenalty(0.),
    myAmDelayed(false),
    myAmRoundabout(false),
    myAmFringe(true),
    myBidiEdge(nullptr)
{ }


MSEdge::~MSEdge() {
    delete myLaneChanger;
}


void
MSEdge::initialize(const std::vector<MSLane*>* lanes) {
    assert(lanes != 0);
    myLanes = std::shared_ptr<const std::vector<MSLane*> >(lanes);
    if (myFunction == SumoXMLEdgeFunc::CONNECTOR) {
        myCombinedPermissions = SVCAll;
    }
    for (MSLane* const lane : *lanes) {
        lane->setRightSideOnEdge(myWidth, (int)mySublaneSides.size());
        MSLeaderInfo ahead(lane);
        for (int j = 0; j < ahead.numSublanes(); ++j) {
            mySublaneSides.push_back(myWidth + j * MSGlobals::gLateralResolution);
        }
        myWidth += lane->getWidth();
    }
}


void MSEdge::recalcCache() {
    if (myLanes->empty()) {
        return;
    }
    myLength = myLanes->front()->getLength();
    myEmptyTraveltime = myLength / MAX2(getSpeedLimit(), NUMERICAL_EPS);

    if (MSGlobals::gMesoTLSPenalty > 0 || MSGlobals::gMesoMinorPenalty > 0) {
        // add tls penalties to the minimum travel time
        SUMOTime minPenalty = -1;
        for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
            MSLane* l = *i;
            const MSLinkCont& lc = l->getLinkCont();
            for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); ++j) {
                MSLink* link = *j;
                SUMOTime linkPenalty = link->getMesoTLSPenalty() + (link->havePriority() ? 0 : MSGlobals::gMesoMinorPenalty);
                if (minPenalty == -1) {
                    minPenalty = linkPenalty;
                } else {
                    minPenalty = MIN2(minPenalty, linkPenalty);
                }
            }
        }
        if (minPenalty > 0) {
            myEmptyTraveltime += STEPS2TIME(minPenalty);
        }
    } else if (isInternal() && MSGlobals::gUsingInternalLanes) {
        const MSLink* link = myLanes->front()->getIncomingLanes()[0].viaLink;
        if (!link->isTLSControlled() && !link->havePriority()) {
            myEmptyTraveltime += MSGlobals::gMinorPenalty;
            myTimePenalty = MSGlobals::gMinorPenalty;
        }
    }
}


void
MSEdge::closeBuilding() {
    for (MSLane* const lane : *myLanes) {
        for (MSLink* const link : lane->getLinkCont()) {
            link->initParallelLinks();
            MSLane* const toL = link->getLane();
            MSLane* const viaL = link->getViaLane();
            if (toL != nullptr) {
                MSEdge& to = toL->getEdge();
                if (std::find(mySuccessors.begin(), mySuccessors.end(), &to) == mySuccessors.end()) {
                    mySuccessors.push_back(&to);
                    myViaSuccessors.push_back(std::make_pair(&to, (viaL == nullptr ? nullptr : &viaL->getEdge())));
                }
                if (std::find(to.myPredecessors.begin(), to.myPredecessors.end(), this) == to.myPredecessors.end()) {
                    to.myPredecessors.push_back(this);
                }
                if (link->getDirection() != LinkDirection::TURN) {
                    myAmFringe = false;
                }
            }
            if (viaL != nullptr) {
                MSEdge& to = viaL->getEdge();
                if (std::find(to.myPredecessors.begin(), to.myPredecessors.end(), this) == to.myPredecessors.end()) {
                    to.myPredecessors.push_back(this);
                }
            }
        }
        lane->checkBufferType();
    }
    std::sort(mySuccessors.begin(), mySuccessors.end(), by_id_sorter());
    rebuildAllowedLanes();
    recalcCache();
    // segment building depends on the finished list of successors (for multi-queue)
    if (MSGlobals::gUseMesoSim && !myLanes->empty()) {
        MSGlobals::gMesoNet->buildSegmentsFor(*this, OptionsCont::getOptions());
    }
}


void
MSEdge::buildLaneChanger() {
    if (!myLanes->empty()) {
        const bool allowChanging = allowsLaneChanging();
        if (MSGlobals::gLateralResolution > 0) {
            // may always initiate sublane-change
            myLaneChanger = new MSLaneChangerSublane(myLanes.get(), allowChanging);
        } else {
            if (MSGlobals::gLaneChangeDuration > 0) {
                myLaneChanger = new MSLaneChanger(myLanes.get(), allowChanging);
            } else if (myLanes->size() > 1 || canChangeToOpposite()) {
                myLaneChanger = new MSLaneChanger(myLanes.get(), allowChanging);
            }
        }
    }
}


bool
MSEdge::allowsLaneChanging() const {
    if (isInternal() && MSGlobals::gUsingInternalLanes) {
        // allow changing only if all links leading to this internal lane have priority
        // or they are controlled by a traffic light
        for (std::vector<MSLane*>::const_iterator it = myLanes->begin(); it != myLanes->end(); ++it) {
            MSLane* pred = (*it)->getLogicalPredecessorLane();
            MSLink* link = MSLinkContHelper::getConnectingLink(*pred, **it);
            assert(link != 0);
            LinkState state = link->getState();
            if (state == LINKSTATE_MINOR
                    || state == LINKSTATE_EQUAL
                    || state == LINKSTATE_STOP
                    || state == LINKSTATE_ALLWAY_STOP
                    || state == LINKSTATE_DEADEND) {
                return false;
            }
        }
    }
    return true;
}


void
MSEdge::addToAllowed(const SVCPermissions permissions, std::shared_ptr<const std::vector<MSLane*> > allowedLanes, AllowedLanesCont& laneCont) const {
    if (!allowedLanes->empty()) {
        // recheck whether we had this list to save memory
        for (auto& allowed : laneCont) {
            if (*allowed.second == *allowedLanes) {
                allowed.first |= permissions;
                return;
            }
        }
        laneCont.push_back(std::make_pair(permissions, allowedLanes));
    }
}


void
MSEdge::rebuildAllowedLanes() {
    // rebuild myMinimumPermissions and myCombinedPermissions
    myMinimumPermissions = SVCAll;
    myCombinedPermissions = 0;
    for (MSLane* const lane : *myLanes) {
        myMinimumPermissions &= lane->getPermissions();
        myCombinedPermissions |= lane->getPermissions();
    }
    // rebuild myAllowed
    myAllowed.clear();
    if (myCombinedPermissions != myMinimumPermissions) {
        myAllowed.push_back(std::make_pair(SVC_IGNORING, myLanes));
        for (int vclass = SVC_PRIVATE; vclass <= SUMOVehicleClass_MAX; vclass *= 2) {
            if ((myCombinedPermissions & vclass) == vclass) {
                std::shared_ptr<std::vector<MSLane*> > allowedLanes = std::make_shared<std::vector<MSLane*> >();
                for (MSLane* const lane : *myLanes) {
                    if (lane->allowsVehicleClass((SUMOVehicleClass)vclass)) {
                        allowedLanes->push_back(lane);
                    }
                }
                addToAllowed(vclass, allowedLanes, myAllowed);
            }
        }
    }
    rebuildAllowedTargets(false);
}


void
MSEdge::rebuildAllowedTargets(const bool updateVehicles) {
    myAllowedTargets.clear();
    for (const MSEdge* target : mySuccessors) {
        bool universalMap = true; // whether the mapping for SVC_IGNORING is also valid for all vehicle classes
        std::shared_ptr<std::vector<MSLane*> > allLanes = std::make_shared<std::vector<MSLane*> >();
        // compute the mapping for SVC_IGNORING
        for (MSLane* const lane : *myLanes) {
            SVCPermissions combinedTargetPermissions = 0;
            for (const MSLink* const link : lane->getLinkCont()) {
                if (&link->getLane()->getEdge() == target) {
                    allLanes->push_back(lane);
                    combinedTargetPermissions |= link->getLane()->getPermissions();
                }
            }
            if (combinedTargetPermissions == 0 || (lane->getPermissions() & combinedTargetPermissions) != lane->getPermissions()) {
                universalMap = false;
            }
        }
        if (universalMap) {
            if (myAllowed.empty()) {
                // we have no lane specific permissions
                myAllowedTargets[target].push_back(std::make_pair(myMinimumPermissions, myLanes));
            } else {
                for (const auto& i : myAllowed) {
                    addToAllowed(i.first, i.second, myAllowedTargets[target]);
                }
            }
        } else {
            addToAllowed(SVC_IGNORING, allLanes, myAllowedTargets[target]);
            // compute the vclass specific mapping
            for (int vclass = SVC_PRIVATE; vclass <= SUMOVehicleClass_MAX; vclass *= 2) {
                if ((myCombinedPermissions & vclass) == vclass) {
                    std::shared_ptr<std::vector<MSLane*> > allowedLanes = std::make_shared<std::vector<MSLane*> >();
                    for (MSLane* const lane : *myLanes) {
                        if (lane->allowsVehicleClass((SUMOVehicleClass)vclass)) {
                            for (const MSLink* const link : lane->getLinkCont()) {
                                if (link->getLane()->allowsVehicleClass((SUMOVehicleClass)vclass) && &link->getLane()->getEdge() == target && (link->getViaLane() == nullptr || link->getViaLane()->allowsVehicleClass((SUMOVehicleClass)vclass))) {
                                    allowedLanes->push_back(lane);
                                }
                            }
                        }
                    }
                    addToAllowed(vclass, allowedLanes, myAllowedTargets[target]);
                }
            }
        }
    }
    if (updateVehicles) {
        for (const MSLane* const lane : *myLanes) {
            const MSLane::VehCont& vehs = lane->getVehiclesSecure();
            for (MSVehicle* veh : vehs) {
                veh->updateBestLanes(true);
            }
            lane->releaseVehicles();
        }
    }
    myClassesSuccessorMap.clear();
}


// ------------ Access to the edge's lanes
MSLane*
MSEdge::leftLane(const MSLane* const lane) const {
    return parallelLane(lane, 1);
}


MSLane*
MSEdge::rightLane(const MSLane* const lane) const {
    return parallelLane(lane, -1);
}


MSLane*
MSEdge::parallelLane(const MSLane* const lane, int offset, bool includeOpposite) const {
    const int resultIndex = lane->getIndex() + offset;
    if (resultIndex == (int)myLanes->size() && includeOpposite) {
        return lane->getOpposite();
    } else if (resultIndex >= (int)myLanes->size() || resultIndex < 0) {
        return nullptr;
    } else {
        return (*myLanes)[resultIndex];
    }
}


const std::vector<MSLane*>*
MSEdge::allowedLanes(const MSEdge& destination, SUMOVehicleClass vclass) const {
    AllowedLanesByTarget::const_iterator i = myAllowedTargets.find(&destination);
    if (i != myAllowedTargets.end()) {
        for (const auto& allowed : i->second) {
            if ((allowed.first & vclass) == vclass) {
                return allowed.second.get();
            }
        }
    }
    return nullptr;
}


const std::vector<MSLane*>*
MSEdge::allowedLanes(SUMOVehicleClass vclass) const {
    if ((myMinimumPermissions & vclass) == vclass) {
        return myLanes.get();
    } else {
        if ((myCombinedPermissions & vclass) == vclass) {
            for (const auto& allowed : myAllowed) {
                if ((allowed.first & vclass) == vclass) {
                    return allowed.second.get();
                }
            }
        }
        return nullptr;
    }
}


// ------------
SUMOTime
MSEdge::incVaporization(SUMOTime) {
    ++myVaporizationRequests;
    return 0;
}


SUMOTime
MSEdge::decVaporization(SUMOTime) {
    --myVaporizationRequests;
    return 0;
}


MSLane*
MSEdge::getFreeLane(const std::vector<MSLane*>* allowed, const SUMOVehicleClass vclass, double departPos) const {
    if (allowed == nullptr) {
        allowed = allowedLanes(vclass);
    }
    MSLane* res = nullptr;
    if (allowed != nullptr) {
        double largestGap = 0;
        MSLane* resByGap = nullptr;
        double leastOccupancy = std::numeric_limits<double>::max();
        for (std::vector<MSLane*>::const_iterator i = allowed->begin(); i != allowed->end(); ++i) {
            const double occupancy = (*i)->getBruttoOccupancy();
            if (occupancy < leastOccupancy) {
                res = (*i);
                leastOccupancy = occupancy;
            }
            const MSVehicle* last = (*i)->getLastFullVehicle();
            const double lastGap = (last != nullptr ? last->getPositionOnLane() : myLength) - departPos;
            if (lastGap > largestGap) {
                largestGap = lastGap;
                resByGap = (*i);
            }
        }
        if (resByGap != nullptr) {
            //if (res != resByGap) std::cout << SIMTIME << " edge=" << getID() << " departPos=" << departPos << " res=" << Named::getIDSecure(res) << " resByGap=" << Named::getIDSecure(resByGap) << " largestGap=" << largestGap << "\n";
            res = resByGap;
        }
    }
    return res;
}


double
MSEdge::getDepartPosBound(const MSVehicle& veh, bool upper) const {
    const SUMOVehicleParameter& pars = veh.getParameter();
    double pos = getLength();
    // determine the position
    switch (pars.departPosProcedure) {
        case DepartPosDefinition::GIVEN:
            pos = pars.departPos;
            if (pos < 0.) {
                pos += myLength;
            }
            break;
        case DepartPosDefinition::RANDOM:
            // could be any position on the edge
            break;
        case DepartPosDefinition::RANDOM_FREE:
            // could be any position on the edge due to multiple random attempts
            break;
        case DepartPosDefinition::FREE:
            // many candidate positions, upper bound could be computed exactly
            // with much effort
            break;
        case DepartPosDefinition::LAST:
            if (upper) {
                for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
                    MSVehicle* last = (*i)->getLastFullVehicle();
                    if (last != nullptr) {
                        pos = MIN2(pos, last->getPositionOnLane());
                    }
                }
            } else {
                pos = 0;
            }
        case DepartPosDefinition::BASE:
        case DepartPosDefinition::DEFAULT:
            break;
        default:
            pos = MIN2(pos, veh.getVehicleType().getLength());
            break;
    }
    return pos;
}


MSLane*
MSEdge::getDepartLane(MSVehicle& veh) const {
    switch (veh.getParameter().departLaneProcedure) {
        case DepartLaneDefinition::GIVEN:
            if ((int) myLanes->size() <= veh.getParameter().departLane || !(*myLanes)[veh.getParameter().departLane]->allowsVehicleClass(veh.getVehicleType().getVehicleClass())) {
                return nullptr;
            }
            return (*myLanes)[veh.getParameter().departLane];
        case DepartLaneDefinition::RANDOM:
            return RandHelper::getRandomFrom(*allowedLanes(veh.getVehicleType().getVehicleClass()));
        case DepartLaneDefinition::FREE:
            return getFreeLane(nullptr, veh.getVehicleType().getVehicleClass(), getDepartPosBound(veh, false));
        case DepartLaneDefinition::ALLOWED_FREE:
            if (veh.getRoute().size() == 1) {
                return getFreeLane(nullptr, veh.getVehicleType().getVehicleClass(), getDepartPosBound(veh, false));
            } else {
                return getFreeLane(allowedLanes(**(veh.getRoute().begin() + 1), veh.getVehicleType().getVehicleClass()), veh.getVehicleType().getVehicleClass(), getDepartPosBound(veh, false));
            }
        case DepartLaneDefinition::BEST_FREE: {
            veh.updateBestLanes(false, myLanes->front());
            const std::vector<MSVehicle::LaneQ>& bl = veh.getBestLanes();
            double bestLength = -1;
            for (std::vector<MSVehicle::LaneQ>::const_iterator i = bl.begin(); i != bl.end(); ++i) {
                if ((*i).length > bestLength) {
                    bestLength = (*i).length;
                }
            }
            // beyond a certain length, all lanes are suitable
            // however, we still need to check departPos to avoid unsuitable insertion
            // (this is only possible in some cases)
            double departPos = 0;
            if (bestLength > BEST_LANE_LOOKAHEAD) {
                departPos = getDepartPosBound(veh);
                bestLength = MIN2(bestLength - departPos, BEST_LANE_LOOKAHEAD);
            }
            std::vector<MSLane*>* bestLanes = new std::vector<MSLane*>();
            for (std::vector<MSVehicle::LaneQ>::const_iterator i = bl.begin(); i != bl.end(); ++i) {
                if (((*i).length - departPos) >= bestLength) {
                    bestLanes->push_back((*i).lane);
                }
            }
            MSLane* ret = getFreeLane(bestLanes, veh.getVehicleType().getVehicleClass(), getDepartPosBound(veh, false));
            delete bestLanes;
            return ret;
        }
        case DepartLaneDefinition::DEFAULT:
        case DepartLaneDefinition::FIRST_ALLOWED:
            for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
                if ((*i)->allowsVehicleClass(veh.getVehicleType().getVehicleClass())) {
                    return *i;
                }
            }
            return nullptr;
        default:
            break;
    }
    if (!(*myLanes)[0]->allowsVehicleClass(veh.getVehicleType().getVehicleClass())) {
        return nullptr;
    }
    return (*myLanes)[0];
}

bool
MSEdge::validateDepartSpeed(SUMOVehicle& v) const {
    const SUMOVehicleParameter& pars = v.getParameter();
    const MSVehicleType& type = v.getVehicleType();
    if (pars.departSpeedProcedure == DepartSpeedDefinition::GIVEN && pars.departSpeed > getVehicleMaxSpeed(&v) + NUMERICAL_EPS) {
        const std::vector<double>& speedFactorParams = type.getSpeedFactor().getParameter();
        if (speedFactorParams[1] > 0.) {
            v.setChosenSpeedFactor(type.computeChosenSpeedDeviation(nullptr, pars.departSpeed / getSpeedLimit()));
            if (v.getChosenSpeedFactor() > speedFactorParams[0] + 2 * speedFactorParams[1]) {
                // only warn for significant deviation
                WRITE_WARNING("Choosing new speed factor " + toString(v.getChosenSpeedFactor()) + " for vehicle '" + pars.id + "' to match departure speed.");
            }
        } else {
            return false;
        }
    }
    return true;
}


bool
MSEdge::insertVehicle(SUMOVehicle& v, SUMOTime time, const bool checkOnly, const bool forceCheck) const {
    // when vaporizing, no vehicles are inserted, but checking needs to be successful to trigger removal
    if (isVaporizing() || isTazConnector()
            || v.getRouteValidity(true, checkOnly) != MSBaseVehicle::ROUTE_VALID) {
        return checkOnly;
    }
    const SUMOVehicleParameter& pars = v.getParameter();
    if (!validateDepartSpeed(v)) {
        const std::string errorMsg = "Departure speed for vehicle '" + pars.id + "' is too high for the departure edge '" + getID() + "'.";
        if (MSGlobals::gCheckRoutes) {
            throw ProcessError(errorMsg);
        } else {
            WRITE_WARNING(errorMsg);
        }
    }
    if (MSGlobals::gUseMesoSim) {
        if (!forceCheck && myLastFailedInsertionTime == time) {
            return false;
        }
        double pos = 0.0;
        switch (pars.departPosProcedure) {
            case DepartPosDefinition::GIVEN:
                if (pars.departPos >= 0.) {
                    pos = pars.departPos;
                } else {
                    pos = pars.departPos + getLength();
                }
                if (pos < 0 || pos > getLength()) {
                    WRITE_WARNING("Invalid departPos " + toString(pos) + " given for vehicle '" +
                                  v.getID() + "'. Inserting at lane end instead.");
                    pos = getLength();
                }
                break;
            case DepartPosDefinition::RANDOM:
            case DepartPosDefinition::RANDOM_FREE:
                pos = RandHelper::rand(getLength());
                break;
            default:
                break;
        }
        bool result = false;
        MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this, pos);
        MEVehicle* veh = static_cast<MEVehicle*>(&v);
        int qIdx;
        if (pars.departPosProcedure == DepartPosDefinition::FREE) {
            while (segment != nullptr && !result) {
                if (checkOnly) {
                    result = segment->hasSpaceFor(veh, time, qIdx, true) == time;
                } else {
                    result = segment->initialise(veh, time);
                }
                segment = segment->getNextSegment();
            }
        } else {
            if (checkOnly) {
                result = segment->hasSpaceFor(veh, time, qIdx, true) == time;
            } else {
                result = segment->initialise(veh, time);
            }
        }
        return result;
    }
    if (checkOnly) {
        switch (v.getParameter().departLaneProcedure) {
            case DepartLaneDefinition::GIVEN:
            case DepartLaneDefinition::DEFAULT:
            case DepartLaneDefinition::FIRST_ALLOWED: {
                MSLane* insertionLane = getDepartLane(static_cast<MSVehicle&>(v));
                if (insertionLane == nullptr) {
                    WRITE_WARNING("could not insert vehicle '" + v.getID() + "' on any lane of edge '" + getID() + "', time=" + time2string(MSNet::getInstance()->getCurrentTimeStep()));
                    return false;
                }
                const double occupancy = insertionLane->getBruttoOccupancy();
                return occupancy == 0 || occupancy * myLength + v.getVehicleType().getLengthWithGap() <= myLength;
            }
            default:
                for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
                    const double occupancy = (*i)->getBruttoOccupancy();
                    if (occupancy == 0 || occupancy * myLength + v.getVehicleType().getLengthWithGap() <= myLength) {
                        return true;
                    }
                }
        }
        return false;
    }
    MSLane* insertionLane = getDepartLane(static_cast<MSVehicle&>(v));
    if (insertionLane == nullptr) {
        return false;
    }

    if (!forceCheck) {
        if (myLastFailedInsertionTime == time) {
            if (myFailedInsertionMemory.count(insertionLane->getIndex())) {
                // A vehicle was already rejected for the proposed insertionLane in this timestep
                return false;
            }
        } else {
            // last rejection occurred in a previous timestep, clear cache
            myFailedInsertionMemory.clear();
        }
    }

    bool success = insertionLane->insertVehicle(static_cast<MSVehicle&>(v));

    if (!success) {
        myFailedInsertionMemory.insert(insertionLane->getIndex());
    }
    return success;
}


void
MSEdge::changeLanes(SUMOTime t) const {
    if (myLaneChanger != nullptr) {
        myLaneChanger->laneChange(t);
    }
}


const MSEdge*
MSEdge::getInternalFollowingEdge(const MSEdge* followerAfterInternal) const {
    //@todo to be optimized
    for (const MSLane* const l : *myLanes) {
        for (const MSLink* const link : l->getLinkCont()) {
            if (&link->getLane()->getEdge() == followerAfterInternal) {
                if (link->getViaLane() != nullptr) {
                    return &link->getViaLane()->getEdge();
                } else {
                    return nullptr; // network without internal links
                }
            }
        }
    }
    return nullptr;
}


double
MSEdge::getInternalFollowingLengthTo(const MSEdge* followerAfterInternal) const {
    assert(followerAfterInternal != 0);
    assert(!followerAfterInternal->isInternal());
    double dist = 0.;
    const MSEdge* edge = getInternalFollowingEdge(followerAfterInternal);
    // Take into account non-internal lengths until next non-internal edge
    while (edge != nullptr && edge->isInternal()) {
        dist += edge->getLength();
        edge = edge->getInternalFollowingEdge(followerAfterInternal);
    }
    return dist;
}


const MSEdge*
MSEdge::getNormalBefore() const {
    const MSEdge* result = this;
    while (result->isInternal() && MSGlobals::gUsingInternalLanes) {
        assert(result->getPredecessors().size() == 1);
        result = result->getPredecessors().front();
    }
    return result;
}

double
MSEdge::getMeanSpeed() const {
    double v = 0;
    double no = 0;
    if (MSGlobals::gUseMesoSim) {
        for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this); segment != nullptr; segment = segment->getNextSegment()) {
            const int vehNo = segment->getCarNumber();
            if (vehNo > 0) {
                v += vehNo * segment->getMeanSpeed();
                no += vehNo;
            }
        }
        if (no == 0) {
            return getLength() / myEmptyTraveltime; // may include tls-penalty
        }
    } else {
        for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
            const double vehNo = (double)(*i)->getVehicleNumber();
            v += vehNo * (*i)->getMeanSpeed();
            no += vehNo;
        }
        if (myBidiEdge != nullptr) {
            for (const MSLane* lane : myBidiEdge->getLanes()) {
                if (lane->getVehicleNumber() > 0) {
                    // do not route across edges which are already occupied in reverse direction
                    return 0;
                }
            }
        }
        if (no == 0) {
            return getSpeedLimit();
        }
    }
    return v / no;
}

double
MSEdge::getMeanSpeedBike() const {
    if (MSGlobals::gUseMesoSim) {
        // no separate bicycle speeds in meso
        return getMeanSpeed();
    }
    double v = 0;
    double no = 0;
    for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
        const double vehNo = (double)(*i)->getVehicleNumber();
        v += vehNo * (*i)->getMeanSpeedBike();
        no += vehNo;
    }
    if (no == 0) {
        return getSpeedLimit();
    }
    return v / no;
}

double
MSEdge::getCurrentTravelTime(double minSpeed) const {
    assert(minSpeed > 0);
    if (!myAmDelayed) {
        return myEmptyTraveltime;
    }
    return getLength() / MAX2(minSpeed, getMeanSpeed());
}


double
MSEdge::getRoutingSpeed() const {
    return MSRoutingEngine::getAssumedSpeed(this, nullptr);
}


bool
MSEdge::dictionary(const std::string& id, MSEdge* ptr) {
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        myDict[id] = ptr;
        while ((int)myEdges.size() < ptr->getNumericalID() + 1) {
            myEdges.push_back(0);
        }
        myEdges[ptr->getNumericalID()] = ptr;
        return true;
    }
    return false;
}


MSEdge*
MSEdge::dictionary(const std::string& id) {
    DictType::iterator it = myDict.find(id);
    if (it == myDict.end()) {
        // id not in myDict.
        return nullptr;
    }
    return it->second;
}


int
MSEdge::dictSize() {
    return (int)myDict.size();
}


const MSEdgeVector&
MSEdge::getAllEdges() {
    return myEdges;
}


void
MSEdge::clear() {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); ++i) {
        delete (*i).second;
    }
    myDict.clear();
    myEdges.clear();
}


void
MSEdge::insertIDs(std::vector<std::string>& into) {
    for (DictType::iterator i = myDict.begin(); i != myDict.end(); ++i) {
        into.push_back((*i).first);
    }
}


void
MSEdge::parseEdgesList(const std::string& desc, ConstMSEdgeVector& into,
                       const std::string& rid) {
    StringTokenizer st(desc);
    parseEdgesList(st.getVector(), into, rid);
}


void
MSEdge::parseEdgesList(const std::vector<std::string>& desc, ConstMSEdgeVector& into,
                       const std::string& rid) {
    for (std::vector<std::string>::const_iterator i = desc.begin(); i != desc.end(); ++i) {
        const MSEdge* edge = MSEdge::dictionary(*i);
        // check whether the edge exists
        if (edge == nullptr) {
            throw ProcessError("The edge '" + *i + "' within the route " + rid + " is not known."
                               + "\n The route can not be build.");
        }
        into.push_back(edge);
    }
}


double
MSEdge::getDistanceTo(const MSEdge* other, const bool doBoundaryEstimate) const {
    assert(this != other);
    if (doBoundaryEstimate) {
        return myBoundary.distanceTo2D(other->myBoundary);
    }
    if (isTazConnector()) {
        if (other->isTazConnector()) {
            return myBoundary.distanceTo2D(other->myBoundary);
        }
        return myBoundary.distanceTo2D(other->getLanes()[0]->getShape()[0]);
    }
    if (other->isTazConnector()) {
        return other->myBoundary.distanceTo2D(getLanes()[0]->getShape()[-1]);
    }
    return getLanes()[0]->getShape()[-1].distanceTo2D(other->getLanes()[0]->getShape()[0]);
}


const Position
MSEdge::getStopPosition(const SUMOVehicleParameter::Stop& stop) {
    return MSLane::dictionary(stop.lane)->geometryPositionAtOffset((stop.endPos + stop.startPos) / 2.);
}


double
MSEdge::getSpeedLimit() const {
    // @note lanes might have different maximum speeds in theory
    return myLanes->empty() ? 1 : getLanes()[0]->getSpeedLimit();
}


double
MSEdge::getLengthGeometryFactor() const {
    return myLanes->empty() ? 1 : getLanes()[0]->getLengthGeometryFactor();
}

double
MSEdge::getVehicleMaxSpeed(const SUMOTrafficObject* const veh) const {
    // @note lanes might have different maximum speeds in theory
    return myLanes->empty() ? 1 : getLanes()[0]->getVehicleMaxSpeed(veh);
}


void
MSEdge::setMaxSpeed(double val) const {
    if (myLanes != nullptr) {
        for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
            (*i)->setMaxSpeed(val);
        }
    }
}


void
MSEdge::addPerson(MSTransportable* p) const {
    myPersons.insert(p);
}

void
MSEdge::removePerson(MSTransportable* p) const {
    std::set<MSTransportable*>::iterator i = myPersons.find(p);
    if (i != myPersons.end()) {
        myPersons.erase(i);
    }
}

std::vector<MSTransportable*>
MSEdge::getSortedPersons(SUMOTime timestep, bool includeRiding) const {
    std::vector<MSTransportable*> result(myPersons.begin(), myPersons.end());
    if (includeRiding) {
        for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
            const MSLane::VehCont& vehs = (*i)->getVehiclesSecure();
            for (MSLane::VehCont::const_iterator j = vehs.begin(); j != vehs.end(); ++j) {
                const std::vector<MSTransportable*>& persons = (*j)->getPersons();
                result.insert(result.end(), persons.begin(), persons.end());
            }
            (*i)->releaseVehicles();
        }
    }
    sort(result.begin(), result.end(), transportable_by_position_sorter(timestep));
    return result;
}


std::vector<MSTransportable*>
MSEdge::getSortedContainers(SUMOTime timestep, bool /* includeRiding */) const {
    std::vector<MSTransportable*> result(myContainers.begin(), myContainers.end());
    sort(result.begin(), result.end(), transportable_by_position_sorter(timestep));
    return result;
}


int
MSEdge::transportable_by_position_sorter::operator()(const MSTransportable* const c1, const MSTransportable* const c2) const {
    const double pos1 = c1->getCurrentStage()->getEdgePos(myTime);
    const double pos2 = c2->getCurrentStage()->getEdgePos(myTime);
    if (pos1 != pos2) {
        return pos1 < pos2;
    }
    return c1->getID() < c2->getID();
}


void
MSEdge::addSuccessor(MSEdge* edge, const MSEdge* via) {
    mySuccessors.push_back(edge);
    myViaSuccessors.push_back(std::make_pair(edge, via));
    if (isTazConnector() && edge->getFromJunction() != nullptr) {
        myBoundary.add(edge->getFromJunction()->getPosition());
    }

    edge->myPredecessors.push_back(this);
    if (edge->isTazConnector() && getToJunction() != nullptr) {
        edge->myBoundary.add(getToJunction()->getPosition());
    }
}


const MSEdgeVector&
MSEdge::getSuccessors(SUMOVehicleClass vClass) const {
    if (vClass == SVC_IGNORING || !MSNet::getInstance()->hasPermissions() || myFunction == SumoXMLEdgeFunc::CONNECTOR) {
        return mySuccessors;
    }
#ifdef HAVE_FOX
    FXConditionalLock lock(mySuccessorMutex, MSGlobals::gNumThreads > 1);
#endif
    std::map<SUMOVehicleClass, MSEdgeVector>::iterator i = myClassesSuccessorMap.find(vClass);
    if (i == myClassesSuccessorMap.end()) {
        // instantiate vector
        myClassesSuccessorMap[vClass];
        i = myClassesSuccessorMap.find(vClass);
        // this vClass is requested for the first time. rebuild all successors
        for (MSEdgeVector::const_iterator it = mySuccessors.begin(); it != mySuccessors.end(); ++it) {
            if ((*it)->isTazConnector()) {
                i->second.push_back(*it);
            } else {
                const std::vector<MSLane*>* allowed = allowedLanes(**it, vClass);
                if (allowed != nullptr && allowed->size() > 0) {
                    i->second.push_back(*it);
                }
            }
        }
    }
    // can use cached value
    return i->second;
}


const MSConstEdgePairVector&
MSEdge::getViaSuccessors(SUMOVehicleClass vClass) const {
    if (vClass == SVC_IGNORING || !MSNet::getInstance()->hasPermissions() || myFunction == SumoXMLEdgeFunc::CONNECTOR) {
        return myViaSuccessors;
    }
#ifdef HAVE_FOX
    FXConditionalLock lock(mySuccessorMutex, MSGlobals::gNumThreads > 1);
#endif
    auto i = myClassesViaSuccessorMap.find(vClass);
    if (i != myClassesViaSuccessorMap.end()) {
        // can use cached value
        return i->second;
    }
    // instantiate vector
    MSConstEdgePairVector& result = myClassesViaSuccessorMap[vClass];
    // this vClass is requested for the first time. rebuild all successors
    for (const auto& viaPair : myViaSuccessors) {
        if (viaPair.first->isTazConnector()) {
            result.push_back(viaPair);
        } else {
            const std::vector<MSLane*>* allowed = allowedLanes(*viaPair.first, vClass);
            if (allowed != nullptr && allowed->size() > 0) {
                result.push_back(viaPair);
            }
        }
    }
    return result;
}


void
MSEdge::setJunctions(MSJunction* from, MSJunction* to) {
    myFromJunction = from;
    myToJunction = to;
    if (!isTazConnector()) {
        myBoundary.add(from->getPosition());
        myBoundary.add(to->getPosition());
    }
}


bool
MSEdge::canChangeToOpposite() {
    return (!myLanes->empty() && myLanes->back()->getOpposite() != nullptr &&
            // do not change on curved internal lanes
            (!isInternal() || myLanes->back()->getIncomingLanes()[0].viaLink->getDirection() == LinkDirection::STRAIGHT));
}


const MSEdge*
MSEdge::getOppositeEdge() const {
    if (!myLanes->empty() && myLanes->back()->getOpposite() != nullptr) {
        return &(myLanes->back()->getOpposite()->getEdge());
    } else {
        return nullptr;
    }
}


bool
MSEdge::hasMinorLink() const {
    for (std::vector<MSLane*>::const_iterator i = myLanes->begin(); i != myLanes->end(); ++i) {
        const MSLinkCont& lc = (*i)->getLinkCont();
        for (MSLinkCont::const_iterator j = lc.begin(); j != lc.end(); ++j) {
            if (!(*j)->havePriority()) {
                return true;
            }
        }
    }
    return false;
}


void
MSEdge::checkAndRegisterBiDirEdge(const std::string& bidiID) {
    if (bidiID != "") {
        myBidiEdge = dictionary(bidiID);
        if (myBidiEdge == nullptr) {
            WRITE_ERROR("Bidi-edge '" + bidiID + "' does not exist");
        }
        return;
    }
    if (getFunction() != SumoXMLEdgeFunc::NORMAL) {
        return;
    }
    ConstMSEdgeVector candidates = myToJunction->getOutgoing();
    for (ConstMSEdgeVector::const_iterator it = candidates.begin(); it != candidates.end(); it++) {
        if ((*it)->getToJunction() == myFromJunction) { //reverse edge
            if (myBidiEdge != nullptr && isSuperposable(*it)) {
                WRITE_WARNING("Ambiguous superposable edges between junction '" + myToJunction->getID() + "' and '" + myFromJunction->getID() + "'.");
                break;
            }
            myBidiEdge = isSuperposable(*it) ? *it : nullptr;
        }
    }
}


bool
MSEdge::isSuperposable(const MSEdge* other) {
    if (other == nullptr || other->getLanes().size() != myLanes->size()) {
        return false;
    }
    std::vector<MSLane*>::const_iterator it1 = myLanes->begin();
    std::vector<MSLane*>::const_reverse_iterator it2 = other->getLanes().rbegin();
    do {
        if ((*it1)->getShape().reverse() != (*it2)->getShape()) {
            return false;
        }
        it1++;
        it2++;
    } while (it1 != myLanes->end());

    return true;
}


void
MSEdge::addWaiting(SUMOVehicle* vehicle) const {
#ifdef HAVE_FOX
    FXConditionalLock lock(myWaitingMutex, MSGlobals::gNumSimThreads > 1);
#endif
    myWaiting.push_back(vehicle);
}


void
MSEdge::removeWaiting(const SUMOVehicle* vehicle) const {
#ifdef HAVE_FOX
    FXConditionalLock lock(myWaitingMutex, MSGlobals::gNumSimThreads > 1);
#endif
    std::vector<SUMOVehicle*>::iterator it = std::find(myWaiting.begin(), myWaiting.end(), vehicle);
    if (it != myWaiting.end()) {
        myWaiting.erase(it);
    }
}


SUMOVehicle*
MSEdge::getWaitingVehicle(MSTransportable* transportable, const double position) const {
#ifdef HAVE_FOX
    FXConditionalLock lock(myWaitingMutex, MSGlobals::gNumSimThreads > 1);
#endif
    for (SUMOVehicle* const vehicle : myWaiting) {
        if (transportable->isWaitingFor(vehicle)) {
            if (vehicle->isStoppedInRange(position, MSGlobals::gStopTolerance) ||
                    (!vehicle->hasDeparted() &&
                     (vehicle->getParameter().departProcedure == DEPART_TRIGGERED ||
                      vehicle->getParameter().departProcedure == DEPART_CONTAINER_TRIGGERED))) {
                return vehicle;
            }
            if (!vehicle->isLineStop(position) && vehicle->allowsBoarding(transportable)) {
                WRITE_WARNING((transportable->isPerson() ? "Person '" : "Container '")
                              + transportable->getID() + "' at edge '" + getID() + "' position " + toString(position) + " cannot use waiting vehicle '"
                              + vehicle->getID() + "' at position " + toString(vehicle->getPositionOnLane()) + " because it is too far away.");
            }
        }
    }
    return nullptr;
}

std::vector<const SUMOVehicle*>
MSEdge::getVehicles() const {
    std::vector<const SUMOVehicle*> result;
    if (MSGlobals::gUseMesoSim) {
        for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this); segment != nullptr; segment = segment->getNextSegment()) {
            std::vector<const MEVehicle*> segmentVehs = segment->getVehicles();
            result.insert(result.end(), segmentVehs.begin(), segmentVehs.end());
        }
    } else {
        for (MSLane* lane : getLanes()) {
            for (auto veh : lane->getVehiclesSecure()) {
                result.push_back(veh);
            }
            lane->releaseVehicles();
        }
    }
    return result;
}


int
MSEdge::getVehicleNumber() const {
    return (int)getVehicles().size();
}


double
MSEdge::getWaitingSeconds() const {
    double wtime = 0;
    if (MSGlobals::gUseMesoSim) {
        for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this); segment != nullptr; segment = segment->getNextSegment()) {
            wtime += segment->getWaitingSeconds();
        }
    } else {
        for (MSLane* lane : getLanes()) {
            wtime += lane->getWaitingSeconds();
        }
    }
    return wtime;
}


double
MSEdge::getOccupancy() const {
    if (MSGlobals::gUseMesoSim) {
        /// @note MESegment only tracks brutto occupancy so we compute this from sratch
        double sum = 0;
        for (const SUMOVehicle* veh : getVehicles()) {
            sum += dynamic_cast<const MEVehicle*>(veh)->getVehicleType().getLength();
        }
        return sum / (myLength * myLanes->size());
    } else {
        double sum = 0;
        for (auto lane : getLanes()) {
            sum += lane->getNettoOccupancy();
        }
        return sum / myLanes->size();
    }
}


double
MSEdge::getFlow() const {
    double flow = 0;
    for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this); segment != nullptr; segment = segment->getNextSegment()) {
        flow += (double) segment->getCarNumber() * segment->getMeanSpeed();
    }
    return 3600 * flow / (*myLanes)[0]->getLength();
}


double
MSEdge::getBruttoOccupancy() const {
    double occ = 0;
    for (MESegment* segment = MSGlobals::gMesoNet->getSegmentForEdge(*this); segment != nullptr; segment = segment->getNextSegment()) {
        occ += segment->getBruttoOccupancy();
    }
    return occ / (*myLanes)[0]->getLength() / (double)(myLanes->size());
}

double
MSEdge::getTravelTimeAggregated(const MSEdge* const edge, const SUMOVehicle* const veh, double /*time*/) {
    return edge->getLength() / MIN2(MSRoutingEngine::getAssumedSpeed(edge, veh), veh->getMaxSpeed());
}

/****************************************************************************/
