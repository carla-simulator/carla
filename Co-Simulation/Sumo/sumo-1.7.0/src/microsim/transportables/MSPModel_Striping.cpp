/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPModel_Striping.cpp
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model (prototype)
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <algorithm>
#include <utils/common/RandHelper.h>
#include <utils/geom/GeomHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/router/PedestrianRouter.h>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSJunction.h>
#include <microsim/MSGlobals.h>
#include <microsim/transportables/MSStage.h>
#include <microsim/transportables/MSTransportableControl.h>
#include "MSPModel_Striping.h"


// ===========================================================================
// DEBUGGING HELPERS
// ===========================================================================
//
#define DEBUGID1 ""
#define DEBUGID2 ""
//#define DEBUGCOND(PED) (false)
//#define DEBUGCOND(PED) ((PED).myPerson->getID() == DEBUGID1 || (PED).myPerson->getID() == DEBUGID2)
#define DEBUGCOND(PED) ((PED).myPerson->isSelected())
#define DEBUGCOND2(LANE) ((LANE)->isSelected())
//#define LOG_ALL 1
//#define DEBUG_MOVETOXY

void MSPModel_Striping::DEBUG_PRINT(const Obstacles& obs) {
    for (int i = 0; i < (int)obs.size(); ++i) {
        std::cout
                << "(" << obs[i].description
                << " x=(" << obs[i].xBack << "," << obs[i].xFwd
                << ") s=" << obs[i].speed
                << ")   ";
    }
    std::cout << "\n";
}

// ===========================================================================
// named (internal) constants
// ===========================================================================

// distances are comparable with lower values being "more important"
const double MSPModel_Striping::DIST_FAR_AWAY(10000);
const double MSPModel_Striping::DIST_BEHIND(1000);
const double MSPModel_Striping::DIST_OVERLAP(-1);

// ===========================================================================
// static members
// ===========================================================================

MSPModel_Striping::WalkingAreaPaths MSPModel_Striping::myWalkingAreaPaths;
std::map<const MSEdge*, std::vector<const MSLane*> >  MSPModel_Striping::myWalkingAreaFoes;
MSPModel_Striping::MinNextLengths MSPModel_Striping::myMinNextLengths;
MSPModel_Striping::Pedestrians MSPModel_Striping::noPedestrians;

// model parameters (static to simplify access from class PState
double MSPModel_Striping::stripeWidth;
double MSPModel_Striping::dawdling;
SUMOTime MSPModel_Striping::jamTime;
SUMOTime MSPModel_Striping::jamTimeCrossing;
const double MSPModel_Striping::LOOKAHEAD_SAMEDIR(4.0); // seconds
const double MSPModel_Striping::LOOKAHEAD_ONCOMING(10.0); // seconds
const double MSPModel_Striping::LOOKAROUND_VEHICLES(60.0); // meters
const double MSPModel_Striping::LATERAL_PENALTY(-1.); // meters
const double MSPModel_Striping::OBSTRUCTED_PENALTY(-300000.); // meters
const double MSPModel_Striping::INAPPROPRIATE_PENALTY(-20000.); // meters
const double MSPModel_Striping::ONCOMING_CONFLICT_PENALTY(-1000.); // meters
const double MSPModel_Striping::OBSTRUCTION_THRESHOLD(MSPModel_Striping::OBSTRUCTED_PENALTY * 0.5); // despite obstruction, additional utility may have been added
const double MSPModel_Striping::SQUEEZE(0.7);
double MSPModel_Striping::RESERVE_FOR_ONCOMING_FACTOR(0.0);
double MSPModel_Striping::RESERVE_FOR_ONCOMING_FACTOR_JUNCTIONS(0.34);
const double MSPModel_Striping::MAX_WAIT_TOLERANCE(120.); // seconds
const double MSPModel_Striping::LATERAL_SPEED_FACTOR(0.4);
const double MSPModel_Striping::MIN_STARTUP_DIST(0.4); // meters

#define MINGAP_TO_VEHICLE 0.25


// ===========================================================================
// MSPModel_Striping method definitions
// ===========================================================================

MSPModel_Striping::MSPModel_Striping(const OptionsCont& oc, MSNet* net) :
    myNumActivePedestrians(0),
    myAmActive(false) {
    initWalkingAreaPaths(net);
    // configurable parameters
    stripeWidth = oc.getFloat("pedestrian.striping.stripe-width");
    dawdling = oc.getFloat("pedestrian.striping.dawdling");
    RESERVE_FOR_ONCOMING_FACTOR = oc.getFloat("pedestrian.striping.reserve-oncoming");
    RESERVE_FOR_ONCOMING_FACTOR_JUNCTIONS = oc.getFloat("pedestrian.striping.reserve-oncoming.junctions");

    jamTime = string2time(oc.getString("pedestrian.striping.jamtime"));
    if (jamTime <= 0) {
        jamTime = SUMOTime_MAX;
    }
    jamTimeCrossing = string2time(oc.getString("pedestrian.striping.jamtime.crossing"));
    if (jamTimeCrossing <= 0) {
        jamTimeCrossing = SUMOTime_MAX;
    }
}


MSPModel_Striping::~MSPModel_Striping() {
    myActiveLanes.clear();
    myNumActivePedestrians = 0;
    myWalkingAreaPaths.clear(); // need to recompute when lane pointers change
    myWalkingAreaFoes.clear();
    myMinNextLengths.clear();
}


MSTransportableStateAdapter*
MSPModel_Striping::add(MSTransportable* transportable, MSStageMoving* stage, SUMOTime) {
    if (!transportable->isPerson()) {
        // containers are not supported (TODO add a warning here?)
        return nullptr;
    }
    MSPerson* person = static_cast<MSPerson*>(transportable);
    MSNet* net = MSNet::getInstance();
    if (!myAmActive) {
        net->getBeginOfTimestepEvents()->addEvent(new MovePedestrians(this), net->getCurrentTimeStep() + DELTA_T);
        myAmActive = true;
    }
    assert(person->getCurrentStageType() == MSStageType::WALKING);
    const MSLane* lane = getSidewalk<MSEdge, MSLane>(person->getEdge());
    if (lane == nullptr) {
        std::string error = "Person '" + person->getID() + "' could not find sidewalk on edge '" + person->getEdge()->getID() + "', time="
                            + time2string(net->getCurrentTimeStep()) + ".";
        if (OptionsCont::getOptions().getBool("ignore-route-errors")) {
            WRITE_WARNING(error);
            return nullptr;
        } else {
            throw ProcessError(error);
        }
    }
    PState* ped = new PState(person, stage, lane);
    myActiveLanes[lane].push_back(ped);
    myNumActivePedestrians++;
    return ped;
}


void
MSPModel_Striping::remove(MSTransportableStateAdapter* state) {
    const MSLane* lane = dynamic_cast<PState*>(state)->myLane;
    Pedestrians& pedestrians = myActiveLanes[lane];
    for (Pedestrians::iterator it = pedestrians.begin(); it != pedestrians.end(); ++it) {
        if (*it == state) {
            pedestrians.erase(it);
            myNumActivePedestrians--;
            return;
        }
    }
}


bool
MSPModel_Striping::blockedAtDist(const MSLane* lane, double vehSide, double vehWidth,
                                 double oncomingGap, std::vector<const MSPerson*>* collectBlockers) {
    const Pedestrians& pedestrians = getPedestrians(lane);
    for (Pedestrians::const_iterator it_ped = pedestrians.begin(); it_ped != pedestrians.end(); ++it_ped) {
        const PState& ped = **it_ped;
        const double leaderFrontDist = (ped.myDir == FORWARD ? vehSide - ped.myRelX : ped.myRelX - vehSide);
        const double leaderBackDist = leaderFrontDist + ped.getLength();
        if DEBUGCOND(ped) {
            std::cout << SIMTIME << " lane=" << lane->getID() << " dir=" << ped.myDir << " pX=" << ped.myRelX << " pL=" << ped.getLength()
                      << " vehSide=" << vehSide
                      << " vehWidth=" << vehWidth
                      << " lBD=" << leaderBackDist
                      << " lFD=" << leaderFrontDist
                      << "\n";
        }
        if (leaderBackDist >= -vehWidth
                && (leaderFrontDist < 0
                    // give right of way to (close) approaching pedestrians unless they are standing
                    || (leaderFrontDist <= oncomingGap && ped.myWaitingTime < TIME2STEPS(2.0)))) {
            // found one pedestrian that is not completely past the crossing point
            //std::cout << SIMTIME << " blocking pedestrian foeLane=" << lane->getID() << " ped=" << ped.myPerson->getID() << " dir=" << ped.myDir << " pX=" << ped.myRelX << " pL=" << ped.getLength() << " fDTC=" << distToCrossing << " lBD=" << leaderBackDist << "\n";
            if (collectBlockers == nullptr) {
                return true;
            } else {
                collectBlockers->push_back(ped.myPerson);
            }
        }
    }
    if (collectBlockers == nullptr) {
        return false;
    } else {
        return collectBlockers->size() > 0;
    }
}


bool
MSPModel_Striping::hasPedestrians(const MSLane* lane) {
    return getPedestrians(lane).size() > 0;
}


bool
MSPModel_Striping::usingInternalLanes() {
    return usingInternalLanesStatic();
}

bool
MSPModel_Striping::usingInternalLanesStatic() {
    return MSGlobals::gUsingInternalLanes && MSNet::getInstance()->hasInternalLinks();
}

PersonDist
MSPModel_Striping::nextBlocking(const MSLane* lane, double minPos, double minRight, double maxLeft, double stopTime) {
    PersonDist result((const MSPerson*)nullptr, -1);
    double closest = std::numeric_limits<double>::max();
    const Pedestrians& pedestrians = getPedestrians(lane);
    for (Pedestrians::const_iterator it_ped = pedestrians.begin(); it_ped != pedestrians.end(); ++it_ped) {
        const PState& ped = **it_ped;
        // account for distance covered by oncoming pedestrians
        double relX2 = ped.myRelX - (ped.myDir == FORWARD ? 0 : stopTime * ped.myPerson->getVehicleType().getMaxSpeed());
        if (ped.myRelX > minPos && (result.first == 0 || closest > relX2)) {
            const double center = lane->getWidth() - (ped.myRelY + stripeWidth * 0.5);
            const double halfWidth = 0.5 * ped.myPerson->getVehicleType().getWidth();
            const bool overlap = (center + halfWidth > minRight && center - halfWidth < maxLeft);
            if DEBUGCOND(ped) {
                std::cout << "  nextBlocking lane=" << lane->getID()
                          << " minPos=" << minPos << " minRight=" << minRight << " maxLeft=" << maxLeft
                          << " stopTime=" << stopTime
                          << " pedY=" << ped.myRelY
                          << " pedX=" << ped.myRelX
                          << " relX2=" << relX2
                          << " center=" << center
                          << " pedLeft=" << center + halfWidth
                          << " pedRight=" << center - halfWidth
                          << " overlap=" << overlap
                          << "\n";
            }
            if (overlap) {
                closest = relX2;
                result.first = ped.myPerson;
                result.second = relX2 - minPos - (ped.myDir == FORWARD ? ped.myPerson->getVehicleType().getLength() : 0);
            }
        }
    }
    return result;
}


MSPModel_Striping::Pedestrians&
MSPModel_Striping::getPedestrians(const MSLane* lane) {
    ActiveLanes::iterator it = myActiveLanes.find(lane);
    if (it != myActiveLanes.end()) {
        //std::cout << " found lane=" << lane->getID() << " n=" << it->second.size() << "\n";
        return (it->second);
    } else {
        return noPedestrians;
    }
}


int
MSPModel_Striping::numStripes(const MSLane* lane) {
    return MAX2(1, (int)floor(lane->getWidth() / stripeWidth));
}

int
MSPModel_Striping::connectedDirection(const MSLane* from, const MSLane* to) {
    if (from == nullptr || to == nullptr) {
        return UNDEFINED_DIRECTION;
    } else if (MSLinkContHelper::getConnectingLink(*from, *to)) {
        return FORWARD;
    } else if (MSLinkContHelper::getConnectingLink(*to, *from)) {
        return BACKWARD;
    } else {
        return UNDEFINED_DIRECTION;
    }
}


void
MSPModel_Striping::initWalkingAreaPaths(const MSNet*) {
    if (myWalkingAreaPaths.size() > 0) {
        return;
    }
    // collect vehicle lanes that cross walkingareas
    for (MSEdgeVector::const_iterator i = MSEdge::getAllEdges().begin(); i != MSEdge::getAllEdges().end(); ++i) {
        const MSEdge* edge = *i;
        if (!edge->isWalkingArea() && !edge->isCrossing()) {
            for (MSLane* lane : edge->getLanes()) {
                for (MSLink* link : lane->getLinkCont()) {
                    if (link->getWalkingAreaFoe() != nullptr) {
                        // link is an exit link
                        myWalkingAreaFoes[&link->getWalkingAreaFoe()->getEdge()].push_back(link->getLaneBefore());
                        //std::cout << " wa=" << link->getWalkingAreaFoe()->getEdge().getID() << " foe=" << link->getLaneBefore()->getID() << "\n";
                    }
                    if (link->getWalkingAreaFoeExit() != nullptr) {
                        // link is an exit link
                        myWalkingAreaFoes[&link->getWalkingAreaFoeExit()->getEdge()].push_back(link->getLaneBefore());
                        //std::cout << " wa=" << link->getWalkingAreaFoeExit()->getEdge().getID() << " foe=" << link->getLaneBefore()->getID() << "\n";
                    }
                }
            }
        }
    }

    // build walkingareaPaths
    for (MSEdgeVector::const_iterator i = MSEdge::getAllEdges().begin(); i != MSEdge::getAllEdges().end(); ++i) {
        const MSEdge* edge = *i;
        if (edge->isWalkingArea()) {
            const MSLane* walkingArea = getSidewalk<MSEdge, MSLane>(edge);
            myMinNextLengths[walkingArea] = walkingArea->getLength();
            // build all possible paths across this walkingArea
            // gather all incident lanes
            std::vector<const MSLane*> lanes;
            for (const MSEdge* in : edge->getPredecessors()) {
                if (!in->isTazConnector()) {
                    lanes.push_back(getSidewalk<MSEdge, MSLane>(in));
                }
            }
            for (const MSEdge* out : edge->getSuccessors()) {
                if (!out->isTazConnector()) {
                    lanes.push_back(getSidewalk<MSEdge, MSLane>(out));
                }
            }
            // build all combinations
            for (int j = 0; j < (int)lanes.size(); ++j) {
                for (int k = 0; k < (int)lanes.size(); ++k) {
                    if (j != k) {
                        // build the walkingArea
                        const MSLane* from = lanes[j];
                        const MSLane* to = lanes[k];
                        const int fromDir = MSLinkContHelper::getConnectingLink(*from, *walkingArea) != nullptr ? FORWARD : BACKWARD;
                        const int toDir = MSLinkContHelper::getConnectingLink(*walkingArea, *to) != nullptr ? FORWARD : BACKWARD;
                        PositionVector shape;
                        Position fromPos = from->getShape()[fromDir == FORWARD ? -1 : 0];
                        Position toPos = to->getShape()[toDir == FORWARD ? 0 : -1];
                        const double maxExtent = fromPos.distanceTo2D(toPos) / 4; // prevent sharp corners
                        const double extrapolateBy = MIN2(maxExtent, walkingArea->getWidth() / 2);
                        // assemble shape
                        shape.push_back(fromPos);
                        if (extrapolateBy > POSITION_EPS) {
                            PositionVector fromShp = from->getShape();
                            fromShp.extrapolate(extrapolateBy);
                            shape.push_back_noDoublePos(fromDir == FORWARD ? fromShp.back() : fromShp.front());
                            PositionVector nextShp = to->getShape();
                            nextShp.extrapolate(extrapolateBy);
                            shape.push_back_noDoublePos(toDir == FORWARD ? nextShp.front() : nextShp.back());
                        }
                        shape.push_back_noDoublePos(toPos);
                        if (shape.size() < 2) {
                            PositionVector fromShp = from->getShape();
                            fromShp.extrapolate(1.5 * POSITION_EPS); // noDoublePos requires a difference of POSITION_EPS in at least one coordinate
                            shape.push_back_noDoublePos(fromDir == FORWARD ? fromShp.back() : fromShp.front());
                            assert(shape.size() == 2);
                        }
                        if (fromDir == BACKWARD) {
                            // will be walking backward on walkingArea
                            shape = shape.reverse();
                        }
                        WalkingAreaPath wap = WalkingAreaPath(from, walkingArea, to, shape, fromDir);
                        myWalkingAreaPaths.insert(std::make_pair(std::make_pair(from, to), wap));
                        myMinNextLengths[walkingArea] = MIN2(myMinNextLengths[walkingArea], wap.length);
                    }
                }
            }
        }
    }
}


const MSPModel_Striping::WalkingAreaPath*
MSPModel_Striping::getArbitraryPath(const MSEdge* walkingArea) {
    assert(walkingArea->isWalkingArea());
    std::vector<const MSLane*> lanes;
    for (const MSEdge* const pred : walkingArea->getPredecessors()) {
        lanes.push_back(getSidewalk<MSEdge, MSLane>(pred));
    }
    for (const MSEdge* const succ : walkingArea->getSuccessors()) {
        lanes.push_back(getSidewalk<MSEdge, MSLane>(succ));
    }
    if (lanes.size() < 1) {
        throw ProcessError("Invalid walkingarea '" + walkingArea->getID() + "' does not allow continuation.");
    }
    return &myWalkingAreaPaths.find(std::make_pair(lanes.front(), lanes.back()))->second;
}

const MSPModel_Striping::WalkingAreaPath*
MSPModel_Striping::guessPath(const MSEdge* walkingArea, const MSEdge* before, const MSEdge* after) {
    assert(walkingArea->isWalkingArea());
    const MSLane* swBefore = getSidewalk<MSEdge, MSLane>(before);
    const MSLane* swAfter = getSidewalk<MSEdge, MSLane>(after);
    const MSEdgeVector& preds = walkingArea->getPredecessors();
    bool useBefore = swBefore != nullptr && std::find(preds.begin(), preds.end(), before) != preds.end();
    const MSEdgeVector& succs = walkingArea->getSuccessors();
    bool useAfter = swAfter != nullptr && std::find(succs.begin(), succs.end(), after) != succs.end();
    if (useBefore) {
        if (useAfter) {
            return getWalkingAreaPath(walkingArea, swBefore, swAfter);
        } else {
            // could also try to exploit direction
            return getWalkingAreaPath(walkingArea, swBefore, getSidewalk<MSEdge, MSLane>(succs.front()));
        }
    } else if (useAfter) {
        // could also try to exploit direction
        return getWalkingAreaPath(walkingArea, getSidewalk<MSEdge, MSLane>(preds.front()), swAfter);
    } else {
        return getArbitraryPath(walkingArea);
    }
}


const MSPModel_Striping::WalkingAreaPath*
MSPModel_Striping::getWalkingAreaPath(const MSEdge* walkingArea, const MSLane* before, const MSLane* after) {
    assert(walkingArea->isWalkingArea());
    const auto pathIt = myWalkingAreaPaths.find(std::make_pair(before, after));
    if (pathIt != myWalkingAreaPaths.end()) {
        return &pathIt->second;
    } else {
        // this can happen in case of moveToXY where before can point anywhere
        const MSEdge* const pred = walkingArea->getPredecessors().front();
        const auto pathIt2 = myWalkingAreaPaths.find(std::make_pair(getSidewalk<MSEdge, MSLane>(pred), after));
        assert(pathIt2 != myWalkingAreaPaths.end());
        return &pathIt2->second;
    }
}



MSPModel_Striping::NextLaneInfo
MSPModel_Striping::getNextLane(const PState& ped, const MSLane* currentLane, const MSLane* prevLane) {
    const MSEdge* currentEdge = &currentLane->getEdge();
    const MSJunction* junction = ped.myDir == FORWARD ? currentEdge->getToJunction() : currentEdge->getFromJunction();
    const MSEdge* nextRouteEdge = ped.myStage->getNextRouteEdge();
    const MSLane* nextRouteLane = getSidewalk<MSEdge, MSLane>(nextRouteEdge);
    // result values
    const MSLane* nextLane = nextRouteLane;
    MSLink* link = nullptr;
    int nextDir = UNDEFINED_DIRECTION;

    //if DEBUGCOND(ped) {
    //    std::cout << "  nextRouteLane=" << Named::getIDSecure(nextRouteLane) << " junction=" << junction->getID() << "\n";
    //}
    if (nextRouteLane == nullptr && nextRouteEdge != nullptr) {
        std::string error = "Person '" + ped.myPerson->getID() + "' could not find sidewalk on edge '" + nextRouteEdge->getID() + "', time="
                            + time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".";
        if (OptionsCont::getOptions().getBool("ignore-route-errors")) {
            WRITE_WARNING(error);
            nextRouteLane = nextRouteEdge->getLanes().front();
        } else {
            throw ProcessError(error);
        }
    }

    if (nextRouteLane != nullptr) {
        if (currentEdge->isInternal()) {
            assert(junction == currentEdge->getFromJunction());
            nextDir = junction == nextRouteEdge->getFromJunction() ? FORWARD : BACKWARD;
            if (nextDir == FORWARD) {
                nextLane = currentLane->getLinkCont()[0]->getViaLaneOrLane();
            } else {
                nextLane = currentLane->getLogicalPredecessorLane();
            }
            if DEBUGCOND(ped) {
                std::cout << "  internal\n";
            }
        } else if (currentEdge->isCrossing()) {
            nextDir = ped.myDir;
            if (ped.myDir == FORWARD) {
                nextLane = currentLane->getLinkCont()[0]->getLane();
            } else {
                nextLane = currentLane->getLogicalPredecessorLane();
            }
            if DEBUGCOND(ped) {
                std::cout << "  crossing\n";
            }
        } else if (currentEdge->isWalkingArea())  {
            ConstMSEdgeVector crossingRoute;
            // departPos can be 0 because the direction of the walkingArea does not matter
            // for the arrivalPos, we need to make sure that the route does not deviate across other junctions
            const int nextRouteEdgeDir = nextRouteEdge->getFromJunction() == junction ? FORWARD : BACKWARD;
            const double arrivalPos = (nextRouteEdge == ped.myStage->getRoute().back()
                                       ? ped.myStage->getArrivalPos()
                                       : (nextRouteEdgeDir == FORWARD ? 0 : nextRouteEdge->getLength()));
            MSEdgeVector prohibited;
            if (prevLane != nullptr) {
                prohibited.push_back(&prevLane->getEdge());
            }
            MSNet::getInstance()->getPedestrianRouter(0, prohibited).compute(currentEdge, nextRouteEdge, 0, arrivalPos, ped.myStage->getMaxSpeed(ped.myPerson), 0, junction, crossingRoute, true);
            if DEBUGCOND(ped) {
                std::cout
                        << "   nre=" << nextRouteEdge->getID()
                        << "   nreDir=" << nextRouteEdgeDir
                        << "   aPos=" << arrivalPos
                        << " crossingRoute=" << toString(crossingRoute)
                        << "\n";
            }
            if (crossingRoute.size() > 1) {
                const MSEdge* nextEdge = crossingRoute[1];
                nextLane = getSidewalk<MSEdge, MSLane>(crossingRoute[1]);
                assert((nextEdge->getFromJunction() == junction || nextEdge->getToJunction() == junction));
                assert(nextLane != prevLane);
                nextDir = connectedDirection(currentLane, nextLane);
                if DEBUGCOND(ped) {
                    std::cout << " nextDir=" << nextDir << "\n";
                }
                assert(nextDir != UNDEFINED_DIRECTION);
                if (nextDir == FORWARD) {
                    link = MSLinkContHelper::getConnectingLink(*currentLane, *nextLane);
                } else {
                    link = MSLinkContHelper::getConnectingLink(*nextLane, *currentLane);
                    if (nextEdge->isCrossing() && link->getTLLogic() == nullptr) {
                        const MSLane* oppositeWalkingArea = nextLane->getLogicalPredecessorLane();
                        link = MSLinkContHelper::getConnectingLink(*oppositeWalkingArea, *nextLane);
                    }
                }
                assert(link != 0);
            } else {
                if DEBUGCOND(ped) {
                    std::cout << SIMTIME
                              << " no route from '" << (currentEdge == nullptr ? "NULL" : currentEdge->getID())
                              << "' to '" << (nextRouteEdge == nullptr ? "NULL" : nextRouteEdge->getID())
                              << "\n";
                }
                WRITE_WARNING("Person '" + ped.myPerson->getID() + "' could not find route across junction '" + junction->getID()
                              + "' from walkingArea '" + currentEdge->getID()
                              + "' to edge '" + nextRouteEdge->getID() + "', time=" +
                              time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".");
                // error indicated by nextDir == UNDEFINED_DIRECTION
                nextLane = nextRouteLane;
            }
        } else if (currentEdge == nextRouteEdge) {
            // strange loop in this route. No need to use walkingArea
            nextDir = -ped.myDir;
        } else {
            // normal edge. by default use next / previous walking area
            nextDir = ped.myDir;
            nextLane = getNextWalkingArea(currentLane, ped.myDir, link);
            if (nextLane != nullptr) {
                // walking area found
                if DEBUGCOND(ped) {
                    std::cout << "  next walkingArea " << (nextDir == FORWARD ? "forward" : "backward") << "\n";
                }
            } else {
                // walk forward by default
                nextDir = junction == nextRouteEdge->getToJunction() ? BACKWARD : FORWARD;
                // try to use a direct link as fallback
                // direct links only exist if built explicitly. They are used to model tl-controlled links if there are no crossings
                if (ped.myDir == FORWARD) {
                    link = MSLinkContHelper::getConnectingLink(*currentLane, *nextRouteLane);
                    if (link != nullptr) {
                        if DEBUGCOND(ped) {
                            std::cout << "  direct forward\n";
                        }
                        nextLane = MSLinkContHelper::getInternalFollowingLane(currentLane, nextRouteLane);
                    }
                } else {
                    link = MSLinkContHelper::getConnectingLink(*nextRouteLane, *currentLane);
                    if (link != nullptr) {
                        if DEBUGCOND(ped) {
                            std::cout << "  direct backward\n";
                        }
                        nextLane = MSLinkContHelper::getInternalFollowingLane(nextRouteLane, currentLane);
                        if (nextLane != nullptr) {
                            // advance to the end of consecutive internal lanes
                            while (nextLane->getLinkCont()[0]->getViaLaneOrLane()->isInternal()) {
                                nextLane = nextLane->getLinkCont()[0]->getViaLaneOrLane();
                            }
                        }
                    }
                }
            }
            if (nextLane == nullptr) {
                // no internal lane found
                nextLane = nextRouteLane;
                if DEBUGCOND(ped) {
                    std::cout << SIMTIME << " no next lane found for " << currentLane->getID() << " dir=" << ped.myDir << "\n";
                }
                if (usingInternalLanesStatic() && currentLane->getLinkCont().size() > 0 && MSNet::getInstance()->hasPedestrianNetwork()) {
                    WRITE_WARNING("Person '" + ped.myPerson->getID() + "' could not find route across junction '" + junction->getID()
                                  + "' from edge '" + currentEdge->getID()
                                  + "' to edge '" + nextRouteEdge->getID() + "', time=" +
                                  time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".");
                }
            } else if (nextLane->getLength() <= POSITION_EPS) {
                // internal lane too short
                // most often this is due to a zero-size junction. However, if
                // the person needs to pass a crossing we cannot skip ahead
                if ((nextLane->getCanonicalSuccessorLane() == nullptr
                        || !nextLane->getCanonicalSuccessorLane()->getEdge().isCrossing())
                        && (nextLane->getLogicalPredecessorLane() == nullptr ||
                            !nextLane->getLogicalPredecessorLane()->getEdge().isCrossing())) {
                    //WRITE_WARNING("Person '" + ped.getID()
                    //        + "' skips short lane '" + nextLane->getID()
                    //        + "' length=" + toString(nextLane->getLength())
                    //        + " time=" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".");
                    nextLane = nextRouteLane;
                    nextDir = nextRouteEdge->getFromJunction() == junction ? FORWARD : BACKWARD;
                }
            }
        }
    }
    if DEBUGCOND(ped) {
        std::cout << SIMTIME
                  << " p=" << ped.myPerson->getID()
                  << " l=" << currentLane->getID()
                  << " nl=" << (nextLane == nullptr ? "NULL" : nextLane->getID())
                  << " nrl=" << (nextRouteLane == nullptr ? "NULL" : nextRouteLane->getID())
                  << " d=" << nextDir
                  << " link=" << (link == nullptr ? "NULL" : link->getViaLaneOrLane()->getID())
                  << " pedDir=" << ped.myDir
                  << "\n";
    }
    assert(nextLane != 0 || nextRouteLane == 0);
    return NextLaneInfo(nextLane, link, nextDir);
}


const MSLane*
MSPModel_Striping::getNextWalkingArea(const MSLane* currentLane, const int dir, MSLink*& link) {
    if (dir == FORWARD) {
        const MSLinkCont& links = currentLane->getLinkCont();
        for (MSLinkCont::const_iterator it = links.begin(); it != links.end(); ++it) {
            if ((*it)->getLane()->getEdge().isWalkingArea()) {
                link = *it;
                return (*it)->getLane();
            }
        }
    } else {
        const std::vector<MSLane::IncomingLaneInfo>& laneInfos = currentLane->getIncomingLanes();
        for (std::vector<MSLane::IncomingLaneInfo>::const_iterator it = laneInfos.begin(); it != laneInfos.end(); ++it) {
            if ((*it).lane->getEdge().isWalkingArea()) {
                link = (*it).viaLink;
                return (*it).lane;
            }
        }
    }
    return nullptr;
}


MSPModel_Striping::Obstacles
MSPModel_Striping::getNeighboringObstacles(const Pedestrians& pedestrians, int egoIndex, int stripes) {
    const PState& ego = *pedestrians[egoIndex];
    const int egoStripe = ego.stripe();
    Obstacles obs(stripes, Obstacle(ego.myDir));
    std::vector<bool> haveBlocker(stripes, false);
    for (int index = egoIndex + 1; index < (int)pedestrians.size(); index++) {
        const PState& p = *pedestrians[index];
        if DEBUGCOND(ego) {
            std::cout << SIMTIME << " ped=" << ego.getID() << " cur=" << egoStripe << " checking neighbor " << p.getID()
                      << " nCur=" << p.stripe() << " nOth=" << p.otherStripe();
        }
        if (!p.myWaitingToEnter && !p.myAmJammed) {
            const Obstacle o(p);
            if DEBUGCOND(ego) {
                std::cout << " dist=" << ego.distanceTo(o) << std::endl;
            }
            if (ego.distanceTo(o) == DIST_BEHIND) {
                break;
            }
            if (ego.distanceTo(o) == DIST_OVERLAP) {
                if (p.stripe() != egoStripe || p.myDir != ego.myDir) {
                    obs[p.stripe()] = o;
                    haveBlocker[p.stripe()] = true;
                } else {
                    //std::cout << SIMTIME << "   ignoring overlap between " << ego.getID() << " and " << p.getID() << " on stripe=" << egoStripe << "\n";
                }
                if (p.otherStripe() != egoStripe || p.myDir != ego.myDir) {
                    obs[p.otherStripe()] = o;
                    haveBlocker[p.otherStripe()] = true;
                } else {
                    //std::cout << SIMTIME << "   ignoring overlap between " << ego.getID() << " and " << p.getID() << " on stripe2=" << egoStripe << "\n";
                }
            } else {
                if (!haveBlocker[p.stripe()]) {
                    obs[p.stripe()] = o;
                }
                if (!haveBlocker[p.otherStripe()]) {
                    obs[p.otherStripe()] = o;
                }
            }
        }
    }
    if DEBUGCOND(ego) {
        std::cout << SIMTIME << " ped=" << ego.myPerson->getID() << "  neighObs=";
        DEBUG_PRINT(obs);
    }
    return obs;
}


int
MSPModel_Striping::getStripeOffset(int origStripes, int destStripes, bool addRemainder) {
    int offset = (destStripes - origStripes) / 2;
    if (addRemainder) {
        offset += (destStripes - origStripes) % 2;
    }
    return offset;
}


const MSPModel_Striping::Obstacles&
MSPModel_Striping::getNextLaneObstacles(NextLanesObstacles& nextLanesObs, const
                                        MSLane* lane, const MSLane* nextLane, int stripes, int nextDir,
                                        double currentLength, int currentDir) {
    if (nextLanesObs.count(nextLane) == 0) {
        const double nextLength = nextLane->getEdge().isWalkingArea() ? myMinNextLengths[nextLane] : nextLane->getLength();
        // figure out the which pedestrians are ahead on the next lane
        const int nextStripes = numStripes(nextLane);
        // do not move past the end of the next lane in a single step
        Obstacles obs(stripes, Obstacle(nextDir == FORWARD ? nextLength : 0, 0, OBSTACLE_NEXTEND, "nextEnd", 0));

        const int offset = getStripeOffset(nextStripes, stripes, currentDir != nextDir && nextStripes > stripes);
        //std::cout << SIMTIME << " getNextLaneObstacles"
        //    << " nextLane=" << nextLane->getID()
        //    << " nextLength=" << nextLength
        //    << " nextDir=" << nextDir
        //    << " currentLength=" << currentLength
        //    << " currentDir=" << currentDir
        //    << " stripes=" << stripes
        //    << " nextStripes=" << nextStripes
        //    << " offset=" << offset
        //    << "\n";
        if (nextStripes < stripes) {
            // some stripes do not continue
            for (int ii = 0; ii < stripes; ++ii) {
                if (ii < offset || ii >= nextStripes + offset) {
                    obs[ii] = Obstacle(nextDir == FORWARD ? 0 : nextLength, 0, OBSTACLE_END, "stripeEnd", 0);
                }
            }
        }
        Pedestrians& pedestrians = getPedestrians(nextLane);
        if (nextLane->getEdge().isWalkingArea()) {
            transformToCurrentLanePositions(obs, currentDir, nextDir, currentLength, nextLength);
            // complex transformation into the coordinate system of the current lane
            // (pedestrians on next lane may walk at arbitrary angles relative to the current lane)
            double lateral_offset = (lane->getWidth() - stripeWidth) * 0.5;
            if ((stripes - nextStripes) % 2 != 0) {
                lateral_offset += 0.5 * stripeWidth;
            }
            nextDir = currentDir;
            // transform pedestrians into the current coordinate system
            for (int ii = 0; ii < (int)pedestrians.size(); ++ii) {
                PState& p = *pedestrians[ii];
                if (p.myWaitingToEnter || p.myAmJammed) {
                    continue;
                }
                Position relPos =  lane->getShape().transformToVectorCoordinates(p.getPosition(*p.myStage, -1), true);
                const double newY = relPos.y() + lateral_offset;
                //if (p.myPerson->getID() == "ped200") std::cout << "    ped=" << p.myPerson->getID() << "  relX=" << relPos.x() << " relY=" << newY << " latOff=" << lateral_offset << " s=" << p.stripe(newY) << " os=" << p.otherStripe(newY) << "\n";
                if ((currentDir == FORWARD && relPos.x() >= lane->getLength()) || (currentDir == BACKWARD && relPos.x() < 0)) {
                    addCloserObstacle(obs, relPos.x(), p.stripe(newY), stripes, p.myPerson->getID(), p.myPerson->getVehicleType().getWidth(), currentDir, OBSTACLE_PED);
                    addCloserObstacle(obs, relPos.x(), p.otherStripe(newY), stripes, p.myPerson->getID(), p.myPerson->getVehicleType().getWidth(), currentDir, OBSTACLE_PED);
                }
            }
        } else {
            // simple transformation into the coordinate system of the current lane
            // (only need to worry about currentDir and nextDir)
            // XXX consider waitingToEnter on nextLane
            sort(pedestrians.begin(), pedestrians.end(), by_xpos_sorter(nextDir));
            for (int ii = 0; ii < (int)pedestrians.size(); ++ii) {
                const PState& p = *pedestrians[ii];
                if (p.myWaitingToEnter || p.myAmJammed) {
                    continue;
                }
                double newY = p.myRelY;
                Obstacle pObs(p);
                if (nextDir != currentDir) {
                    newY = (nextStripes - 1) * stripeWidth - newY;
                    pObs.speed *= -1;
                }
                newY += offset * stripeWidth;
                const int stripe = p.stripe(newY);
                if (stripe >= 0 && stripe < stripes) {
                    obs[stripe] = pObs;
                }
                const int otherStripe = p.otherStripe(newY);
                if (otherStripe >= 0 && otherStripe < stripes) {
                    obs[otherStripe] = pObs;
                }
            }
            if (nextLane->getEdge().isCrossing()) {
                // add vehicle obstacles
                const MSLink* crossingEntryLink = nextLane->getIncomingLanes().front().viaLink;
                const bool prio = crossingEntryLink->havePriority() || crossingEntryLink->getTLLogic() != nullptr;
                addCrossingVehs(nextLane, stripes, offset, nextDir, obs, prio);
            }
            if (nextLane->getVehicleNumberWithPartials() > 0) {
                Obstacles vehObs = getVehicleObstacles(nextLane, nextDir);
                PState::mergeObstacles(obs, vehObs, nextDir, offset);
            }
            transformToCurrentLanePositions(obs, currentDir, nextDir, currentLength, nextLength);
        }
        nextLanesObs[nextLane] = obs;
    }
    return nextLanesObs[nextLane];
}

void
MSPModel_Striping::transformToCurrentLanePositions(Obstacles& obs, int currentDir, int nextDir, double currentLength, double nextLength) {
    for (int ii = 0; ii < (int)obs.size(); ++ii) {
        Obstacle& o = obs[ii];
        if (currentDir == FORWARD) {
            if (nextDir == FORWARD) {
                o.xFwd += currentLength;
                o.xBack += currentLength;
            } else {
                const double tmp = o.xFwd;
                o.xFwd = currentLength + nextLength - o.xBack;
                o.xBack = currentLength + nextLength - tmp;
            }
        } else {
            if (nextDir == FORWARD) {
                const double tmp = o.xFwd;
                o.xFwd = -o.xBack;
                o.xBack = -tmp;
            } else {
                o.xFwd -= nextLength;
                o.xBack -= nextLength;
            }
        }
    }
}


void
MSPModel_Striping::addCloserObstacle(Obstacles& obs, double x, int stripe, int numStripes, const std::string& id, double width, int dir, ObstacleType type) {
    if (stripe >= 0 && stripe < numStripes) {
        if ((dir == FORWARD && x - width / 2. < obs[stripe].xBack) || (dir == BACKWARD && x + width / 2. > obs[stripe].xFwd)) {
            obs[stripe] = Obstacle(x, 0, type, id, width);
        }
    }
}

void
MSPModel_Striping::moveInDirection(SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir) {
    for (ActiveLanes::iterator it_lane = myActiveLanes.begin(); it_lane != myActiveLanes.end(); ++it_lane) {
        const MSLane* lane = it_lane->first;
        Pedestrians& pedestrians = it_lane->second;
        if (pedestrians.size() == 0) {
            continue;
        }
        //std::cout << SIMTIME << ">>> lane=" << lane->getID() << " numPeds=" << pedestrians.size() << "\n";
        if (lane->getEdge().isWalkingArea()) {
            const double lateral_offset = (lane->getWidth() - stripeWidth) * 0.5;
            const double minY = stripeWidth * - 0.5 + NUMERICAL_EPS;
            const double maxY = stripeWidth * (numStripes(lane) - 0.5) - NUMERICAL_EPS;
            const WalkingAreaPath* debugPath = nullptr;
            // need to handle each walkingAreaPath separately and transform
            // coordinates beforehand
            std::set<const WalkingAreaPath*, walkingarea_path_sorter> paths;
            for (Pedestrians::iterator it = pedestrians.begin(); it != pedestrians.end(); ++it) {
                const PState* p = *it;
                assert(p->myWalkingAreaPath != 0);
                if (p->myDir == dir) {
                    paths.insert(p->myWalkingAreaPath);
                    if DEBUGCOND(*p) {
                        debugPath = p->myWalkingAreaPath;
                        std::cout << SIMTIME << " debugging WalkingAreaPath from=" << debugPath->from->getID() << " to=" << debugPath->to->getID() << "\n";
                    }
                }
            }
            const double usableWidth = (numStripes(lane) - 1) * stripeWidth;
            for (std::set<const WalkingAreaPath*, walkingarea_path_sorter>::iterator it = paths.begin(); it != paths.end(); ++it) {
                const WalkingAreaPath* path = *it;
                Pedestrians toDelete;
                Pedestrians transformedPeds;
                transformedPeds.reserve(pedestrians.size());
                for (Pedestrians::iterator it_p = pedestrians.begin(); it_p != pedestrians.end(); ++it_p) {
                    PState* p = *it_p;
                    if (p->myWalkingAreaPath == path) {
                        transformedPeds.push_back(p);
                        if (path == debugPath) std::cout << "  ped=" << p->myPerson->getID() << "  relX=" << p->myRelX << " relY=" << p->myRelY << " (untransformed), vecCoord="
                                                             << path->shape.transformToVectorCoordinates(p->getPosition(*p->myStage, -1)) << "\n";
                    } else if (p->myWalkingAreaPath->from == path->to && p->myWalkingAreaPath->to == path->from) {
                        if (p->myWalkingAreaPath->dir != path->dir) {
                            // opposite direction is already in the correct coordinate system
                            transformedPeds.push_back(p);
                            if (path == debugPath) std::cout << "  ped=" << p->myPerson->getID() << "  relX=" << p->myRelX << " relY=" << p->myRelY << " (untransformed), vecCoord="
                                                                 << path->shape.transformToVectorCoordinates(p->getPosition(*p->myStage, -1)) << "\n";
                        } else {
                            // x position must be reversed
                            PState* tp = new PState(*p);
                            tp->myRelX = path->length - p->myRelX;
                            tp->myRelY = usableWidth - p->myRelY;
                            tp->myDir = !path->dir;
                            tp->mySpeed = -p->mySpeed;
                            toDelete.push_back(tp);
                            transformedPeds.push_back(tp);
                            if (path == debugPath) std::cout << "  ped=" << p->myPerson->getID() << "  relX=" << p->myRelX << " relY=" << p->myRelY << " (semi-transformed), vecCoord="
                                                                 << path->shape.transformToVectorCoordinates(p->getPosition(*p->myStage, -1)) << "\n";
                        }
                    } else {
                        const Position relPos = path->shape.transformToVectorCoordinates(p->getPosition(*p->myStage, -1));
                        const double newY = relPos.y() + lateral_offset;
                        if (relPos != Position::INVALID && newY >= minY && newY <= maxY) {
                            PState* tp = new PState(*p);
                            tp->myRelX = relPos.x();
                            tp->myRelY = newY;
                            // only an obstacle, speed may be orthogonal to dir
                            tp->myDir = !dir;
                            tp->mySpeed = 0;
                            toDelete.push_back(tp);
                            transformedPeds.push_back(tp);
                            if (path == debugPath) {
                                std::cout << "  ped=" << p->myPerson->getID() << "  relX=" << relPos.x() << " relY=" << newY << " (transformed), vecCoord=" << relPos << "\n";
                            }
                        } else {
                            if (path == debugPath) {
                                std::cout << "  ped=" << p->myPerson->getID() << "  relX=" << relPos.x() << " relY=" << newY << " (invalid), vecCoord=" << relPos << "\n";
                            }
                        }
                    }
                }
                auto itFoe = myWalkingAreaFoes.find(&lane->getEdge());
                if (itFoe != myWalkingAreaFoes.end()) {
                    // add vehicle foes on paths which cross this walkingarea
                    // translate the vehicle into a number of dummy-pedestrians
                    // that occupy the same space
                    for (const MSLane* foeLane : itFoe->second) {
                        for (auto itVeh = foeLane->anyVehiclesBegin(); itVeh != foeLane->anyVehiclesEnd(); ++itVeh) {
                            const MSVehicle* veh = *itVeh;
                            const Position relPos = path->shape.transformToVectorCoordinates(veh->getPosition());
                            const Position relPos2 = path->shape.transformToVectorCoordinates(veh->getBackPosition());
                            //std::cout << " pos=" << veh->getPosition() << " back=" << veh->getBackPosition() << " relPos=" << relPos << " relPos2=" << relPos2 << " shape=" << path->shape << "\n";
                            if (addVehicleFoe(veh, lane, relPos, lateral_offset, minY, maxY, toDelete, transformedPeds)
                                    && addVehicleFoe(veh, lane, relPos2, lateral_offset, minY, maxY, toDelete, transformedPeds)) {
                                // add in-between positions
                                const double length = veh->getVehicleType().getLength();
                                for (double dist = stripeWidth; dist < length; dist += stripeWidth) {
                                    const double relDist = dist / length;
                                    Position between = (relPos * relDist) + (relPos2 * (1 - relDist));
                                    addVehicleFoe(veh, lane, between, lateral_offset, minY, maxY, toDelete, transformedPeds);
                                }
                            }
                        }
                    }
                }
                moveInDirectionOnLane(transformedPeds, lane, currentTime, changedLane, dir);
                arriveAndAdvance(pedestrians, currentTime, changedLane, dir);
                // clean up
                for (Pedestrians::iterator it_p = toDelete.begin(); it_p != toDelete.end(); ++it_p) {
                    delete *it_p;
                }
            }
        } else {
            moveInDirectionOnLane(pedestrians, lane, currentTime, changedLane, dir);
            arriveAndAdvance(pedestrians, currentTime, changedLane, dir);
        }
    }
}


bool
MSPModel_Striping::addVehicleFoe(const MSVehicle* veh, const MSLane* walkingarea, const Position& relPos, double lateral_offset,
                                 double minY, double maxY, Pedestrians& toDelete, Pedestrians& transformedPeds) {
    if (relPos != Position::INVALID) {
        const double newY = relPos.y() + lateral_offset;
        if (newY >= minY && newY <= maxY) {
            PState* tp = new PStateVehicle(veh, walkingarea, relPos.x(), newY);
            //std::cout << SIMTIME << " addVehicleFoe=" << veh->getID() << " rx=" << relPos.x() << " ry=" << newY << " s=" << tp->stripe() << " o=" << tp->otherStripe() << "\n";
            toDelete.push_back(tp);
            transformedPeds.push_back(tp);
        }
        return true;
    } else {
        return false;
    }
}

void
MSPModel_Striping::arriveAndAdvance(Pedestrians& pedestrians, SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir) {
    // advance to the next lane / arrive at destination
    sort(pedestrians.begin(), pedestrians.end(), by_xpos_sorter(dir));
    // can't use iterators because we do concurrent modification
    for (int i = 0; i < (int)pedestrians.size(); i++) {
        PState* const p = pedestrians[i];
        if (p->isRemoteControlled()) {
            continue;
        }
        if (p->myDir == dir && p->distToLaneEnd() < 0) {
            // moveToNextLane may trigger re-insertion (for consecutive
            // walks) so erase must be called first
            pedestrians.erase(pedestrians.begin() + i);
            i--;
            p->moveToNextLane(currentTime);
            if (p->myLane != nullptr) {
                changedLane.insert(p->myPerson);
                myActiveLanes[p->myLane].push_back(p);
            } else {
                // end walking stage and destroy PState
                p->myStage->moveToNextEdge(p->myPerson, currentTime);
                myNumActivePedestrians--;
            }
        }
    }
}


void
MSPModel_Striping::moveInDirectionOnLane(Pedestrians& pedestrians, const MSLane* lane, SUMOTime currentTime, std::set<MSPerson*>& changedLane, int dir) {
    const int stripes = numStripes(lane);
    //std::cout << " laneWidth=" << lane->getWidth() << " stripeWidth=" << stripeWidth << " stripes=" << stripes << "\n";
    Obstacles obs(stripes, Obstacle(dir)); // continously updated
    NextLanesObstacles nextLanesObs; // continously updated
    sort(pedestrians.begin(), pedestrians.end(), by_xpos_sorter(dir));

    Obstacles crossingVehs(stripes, Obstacle(dir));
    bool hasCrossingVehObs = false;
    if (lane->getEdge().isCrossing()) {
        // assume that vehicles will brake when already on the crossing
        hasCrossingVehObs = addCrossingVehs(lane, stripes, 0, dir, crossingVehs, true);
    }

    for (int ii = 0; ii < (int)pedestrians.size(); ++ii) {
        PState& p = *pedestrians[ii];
        //std::cout << SIMTIME << "CHECKING" << p.myPerson->getID() << "\n";
        Obstacles currentObs = obs;
        if (p.myDir != dir || changedLane.count(p.myPerson) != 0 || p.myRemoteXYPos != Position::INVALID) {
            if (!p.myWaitingToEnter && !p.myAmJammed) {
                //if DEBUGCOND(p) {
                //    std::cout << "   obs=" << p.myPerson->getID() << "  y=" << p.myRelY << "  stripe=" << p.stripe() << " oStripe=" << p.otherStripe() << "\n";
                //}
                Obstacle o(p);
                if (p.myDir != dir && p.mySpeed == 0) {
                    // ensure recognition of oncoming
                    o.speed = (p.myDir == FORWARD ? 0.1 : -0.1);
                }
                obs[p.stripe()] = o;
                obs[p.otherStripe()] = o;
            }
            continue;
        }
        if DEBUGCOND(p) {
            std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  currentObs=";
            gDebugFlag1 = true;
            DEBUG_PRINT(currentObs);
        }
        const MSLane* nextLane = p.myNLI.lane;
        const MSLink* link = p.myNLI.link;
        const double dist = p.distToLaneEnd();
        const double speed = p.myStage->getMaxSpeed(p.myPerson);
        if (nextLane != nullptr && dist <= LOOKAHEAD_ONCOMING) {
            const double currentLength = (p.myWalkingAreaPath == nullptr ? lane->getLength() : p.myWalkingAreaPath->length);
            const Obstacles& nextObs = getNextLaneObstacles(
                                           nextLanesObs, lane, nextLane, stripes,
                                           p.myNLI.dir, currentLength, dir);

            if DEBUGCOND(p) {
                std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  nextObs=";
                DEBUG_PRINT(nextObs);
            }
            p.mergeObstacles(currentObs, nextObs);
        }
        if DEBUGCOND(p) {
            std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  obsWithNext=";
            DEBUG_PRINT(currentObs);
        }
        p.mergeObstacles(currentObs, getNeighboringObstacles(pedestrians, ii, stripes));
        if DEBUGCOND(p) {
            std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  obsWithNeigh=";
            DEBUG_PRINT(currentObs);
        }
        // time gap to pass the intersection ahead of a vehicle.
        const double passingClearanceTime = 2;
        const double passingLength = p.getLength() + passingClearanceTime * speed;
        // check link state
        if DEBUGCOND(p) {
            gDebugFlag1 = true; // get debug output from MSLink
            std::cout << "   link=" << (link == nullptr ? "NULL" : link->getViaLaneOrLane()->getID())
                      << " dist=" << dist << " d2=" << dist - p.getMinGap() << " la=" << LOOKAHEAD_SAMEDIR* speed << "\n";
        }
        if (link != nullptr
                // only check close before junction, @todo we should take deceleration into account here
                && dist - p.getMinGap() < LOOKAHEAD_SAMEDIR * speed
                // persons move before vehicles so we subtract DELTA_TO because they cannot rely on vehicles having passed the intersection in the current time step
                && !link->opened(currentTime - DELTA_T, speed, speed, passingLength, p.getImpatience(currentTime), speed, 0, 0, nullptr, p.ignoreRed(link), p.myPerson)) {
            // prevent movement passed a closed link
            Obstacles closedLink(stripes, Obstacle(p.myRelX + dir * (dist - NUMERICAL_EPS), 0, OBSTACLE_LINKCLOSED, "closedLink_" + link->getViaLaneOrLane()->getID(), 0));
            p.mergeObstacles(currentObs, closedLink);
            if DEBUGCOND(p) {
                std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  obsWithTLS=";
                DEBUG_PRINT(currentObs);
            }
            // consider rerouting over another crossing
            if (p.myWalkingAreaPath != nullptr) {
                // @todo actually another path would be needed starting at the current position
                p.myNLI = getNextLane(p, p.myLane, p.myWalkingAreaPath->from);
            }
        }
        if DEBUGCOND(p) {
            gDebugFlag1 = false;
        }
        if (&lane->getEdge() == p.myStage->getDestination() && p.myStage->getDestinationStop() != nullptr) {
            Obstacles arrival(stripes, Obstacle(p.myStage->getArrivalPos() + dir * p.getMinGap(), 0, OBSTACLE_ARRIVALPOS, "arrival", 0));
            p.mergeObstacles(currentObs, arrival);
        }

        if (lane->getVehicleNumberWithPartials() > 0) {
            // react to vehicles on the same lane
            // @todo: improve efficiency by using the same iterator for all pedestrians on this lane
            Obstacles vehObs = getVehicleObstacles(lane, dir, &p);
            p.mergeObstacles(currentObs, vehObs);
            if DEBUGCOND(p) {
                std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  obsWithVehs=";
                DEBUG_PRINT(currentObs);
            }
        }
        if (hasCrossingVehObs) {
            p.mergeObstacles(currentObs, crossingVehs);
            if DEBUGCOND(p) {
                std::cout << SIMTIME << " ped=" << p.myPerson->getID() << "  obsWithVehs2=";
                DEBUG_PRINT(currentObs);
            }
        }

        // walk, taking into account all obstacles
        p.walk(currentObs, currentTime);
        gDebugFlag1 = false;
        if (!p.myWaitingToEnter && !p.myAmJammed) {
            Obstacle o(p);
            obs[p.stripe()] = o;
            obs[p.otherStripe()] = o;
            if (MSGlobals::gCheck4Accidents && p.myWalkingAreaPath == nullptr && !p.myAmJammed) {
                for (int coll = 0; coll < ii; ++coll) {
                    PState& c = *pedestrians[coll];
                    if (!c.myWaitingToEnter && c.myWalkingAreaPath == nullptr && !c.myAmJammed) {
                        if (c.stripe() == p.stripe() || p.stripe() == c.otherStripe() || p.otherStripe() == c.stripe() || p.otherStripe() == c.otherStripe()) {
                            Obstacle cObs(c);
                            // we check only for real collisions, no min gap violations
                            if (p.distanceTo(cObs, false) == DIST_OVERLAP) {
                                WRITE_WARNING("Collision of person '" + p.myPerson->getID() + "' and person '" + c.myPerson->getID()
                                              + "', lane='" + lane->getID() + "', time=" + time2string(currentTime) + ".");
                            }
                        }
                    }
                }
            }
        }
        //std::cout << SIMTIME << p.myPerson->getID() << " lane=" << lane->getID() << " x=" << p.myRelX << "\n";
    }
}

bool
MSPModel_Striping::addCrossingVehs(const MSLane* crossing, int stripes, double lateral_offset, int dir, Obstacles& obs, bool prio) {
    bool hasCrossingVehObs = false;
    const MSLink* crossingExitLink = crossing->getLinkCont().front();
    gDebugFlag1 = DEBUGCOND2(crossing);
    const MSLink::LinkLeaders linkLeaders = crossingExitLink->getLeaderInfo(nullptr, crossing->getLength());
    gDebugFlag1 = false;
    if (linkLeaders.size() > 0) {
        for (MSLink::LinkLeaders::const_iterator it = linkLeaders.begin(); it != linkLeaders.end(); ++it) {
            // the vehicle to enter the junction first has priority
            const MSVehicle* veh = (*it).vehAndGap.first;
            if (veh != nullptr) {
                Obstacle vo((*it).distToCrossing, 0, OBSTACLE_VEHICLE, veh->getID(), veh->getVehicleType().getWidth() + 2 * MINGAP_TO_VEHICLE);
                // block entry to the crossing in walking direction but allow leaving it
                Obstacle voBlock = vo;
                if (dir == FORWARD) {
                    voBlock.xBack = NUMERICAL_EPS;
                } else {
                    voBlock.xFwd = crossing->getLength() - NUMERICAL_EPS;
                }
                // when approaching a priority crossings, vehicles must be able
                // to brake, otherwise the person must be able to cross in time
                const double distToCrossBeforeVeh = (dir == FORWARD ? vo.xFwd : crossing->getLength() - vo.xBack);
                const double bGap = (prio
                                     ? veh->getCarFollowModel().brakeGap(veh->getSpeed(), veh->getCarFollowModel().getMaxDecel(), 0)
                                     : veh->getSpeed() * distToCrossBeforeVeh); // walking 1m/s
                double vehYmin;
                double vehYmax;
                // relY increases from left to right (the other way around from vehicles)
                if ((*it).fromLeft) {
                    vehYmin = -(*it).vehAndGap.second + lateral_offset; // vehicle back
                    vehYmax = vehYmin + veh->getVehicleType().getLength() + bGap + MINGAP_TO_VEHICLE;
                    vehYmin -= MINGAP_TO_VEHICLE;
                } else {
                    vehYmax = crossing->getWidth() + (*it).vehAndGap.second - lateral_offset; // vehicle back
                    vehYmin = vehYmax - veh->getVehicleType().getLength() - bGap - MINGAP_TO_VEHICLE;
                    vehYmax += MINGAP_TO_VEHICLE;

                }
                for (int s = MAX2(0, PState::stripe(vehYmin)); s < MIN2(PState::stripe(vehYmax), stripes); ++s) {
                    if ((dir == FORWARD && obs[s].xBack > vo.xBack)
                            || (dir == BACKWARD && obs[s].xFwd < vo.xFwd)) {
                        if (!prio && veh->getSpeed() > SUMO_const_haltingSpeed) {
                            // do not enter the crossing
                            obs[s] = voBlock;
                        } else {
                            obs[s] = vo;
                        }
                        hasCrossingVehObs = true;
                    }
                }
                if (DEBUGCOND2(crossing)) {
                    std::cout << SIMTIME
                              << " crossingVeh=" << veh->getID()
                              << " lane=" << crossing->getID()
                              << " prio=" << prio
                              << " latOffset=" << lateral_offset
                              << " dir=" << dir
                              << " stripes=" << stripes
                              << " dist=" << (*it).distToCrossing
                              << " gap=" << (*it).vehAndGap.second
                              << " brakeGap=" << bGap
                              << " fromLeft=" << (*it).fromLeft
                              << " distToCrossBefore=" << distToCrossBeforeVeh
                              << " ymin=" << vehYmin
                              << " ymax=" << vehYmax
                              << " smin=" << PState::stripe(vehYmin)
                              << " smax=" << PState::stripe(vehYmax)
                              << "\n";
                    DEBUG_PRINT(obs);
                }
            }
        }
    }
    return hasCrossingVehObs;
}


MSPModel_Striping::Obstacles
MSPModel_Striping::getVehicleObstacles(const MSLane* lane, int dir, PState* ped) {
    const int stripes = numStripes(lane);
    Obstacles vehObs(stripes, Obstacle(dir));
    int current = -1;
    double minX = 0.;
    double maxX = 0.;
    double pRelY = -1.;
    double pWidth = 0.;
    std::string pID;
    bool debug = DEBUGCOND2(lane);
    if (ped != nullptr) {
        current = ped->stripe();
        minX = ped->getMinX();
        maxX = ped->getMaxX();
        pRelY = ped->myRelY;
        pWidth = ped->myPerson->getVehicleType().getWidth();
        pID = ped->myPerson->getID();
        debug = DEBUGCOND(*ped);
    } else if (dir == BACKWARD) {
        // checking vehicles on the next lane. Use entry point as reference
        minX = lane->getLength();
        maxX = lane->getLength();
    }
    MSLane::AnyVehicleIterator begin = (dir == FORWARD ? lane->anyVehiclesUpstreamBegin() : lane->anyVehiclesBegin());
    MSLane::AnyVehicleIterator end = (dir == FORWARD ? lane->anyVehiclesUpstreamEnd() : lane->anyVehiclesEnd());
    for (MSLane::AnyVehicleIterator it = begin; it != end; ++it) {
        const MSVehicle* veh = *it;
        const double vehBack = veh->getBackPositionOnLane(lane);
        const double vehFront = vehBack + veh->getVehicleType().getLength();
        // ensure that vehicles are not blocked
        const double vehNextSpeed = MAX2(veh->getSpeed(), 1.0);
        const double clearance = SAFETY_GAP + vehNextSpeed * LOOKAHEAD_SAMEDIR;
        if ((dir == FORWARD && vehFront + clearance > minX && vehBack <= maxX + LOOKAHEAD_SAMEDIR)
                || (dir == BACKWARD && vehBack < maxX && vehFront >= minX - LOOKAROUND_VEHICLES)) {
            Obstacle vo(vehBack, veh->getSpeed(), OBSTACLE_VEHICLE, veh->getID(), 0);
            // moving vehicles block space along their path
            vo.xFwd += veh->getVehicleType().getLength() + clearance;
            vo.xBack -= SAFETY_GAP;
            // relY increases from left to right (the other way around from vehicles)
            // XXX lateral offset for partial vehicles
            const double vehYmax = 0.5 * (lane->getWidth() + veh->getVehicleType().getWidth() - stripeWidth) - veh->getLateralPositionOnLane();
            const double vehYmin = vehYmax - veh->getVehicleType().getWidth();
            for (int s = MAX2(0, PState::stripe(vehYmin)); s < MIN2(PState::stripe(vehYmax) + 1, stripes); ++s) {
                Obstacle prior = vehObs[s];
                vehObs[s] = vo;
                if (s == current && vehFront + SAFETY_GAP < minX) {
                    // ignore if aleady overlapping while vehicle is still behind
                    if (pRelY - pWidth < vehYmax &&
                            pRelY + pWidth > vehYmin && dir == FORWARD) {
                        if (debug) {
                            std::cout << "   ignoring vehicle '" << veh->getID() << " on stripe " << s << " vehFrontSG=" << vehFront + SAFETY_GAP << " minX=" << minX << "\n";
                        }
                        if (dir == FORWARD) {
                            vehObs[s] = prior;
                        } else {
                            vehObs[s].xFwd = MIN2(vo.xFwd, vehFront + SAFETY_GAP);
                        }
                    }
                }
            }
            if (debug) {
                std::cout << SIMTIME << " ped=" << pID << " veh=" << veh->getID() << " obstacle on lane=" << lane->getID()
                          << "\n"
                          << "     ymin=" << vehYmin
                          << " ymax=" << vehYmax
                          << " smin=" << PState::stripe(vehYmin)
                          << " smax=" << PState::stripe(vehYmax)
                          << " relY=" << pRelY
                          << " current=" << current
                          << " vo.xFwd=" << vo.xFwd
                          << " vo.xBack=" << vo.xBack
                          << "\n";
            }
        }
    }
    return vehObs;
}


// ===========================================================================
// MSPModel_Striping::Obstacle method definitions
// ===========================================================================
MSPModel_Striping::Obstacle::Obstacle(int dir, double dist) :
    xFwd(dir * dist),  // by default, far away when seen in dir
    xBack(dir * dist),  // by default, far away when seen in dir
    speed(0),
    type(OBSTACLE_NONE),
    description("") {
}


MSPModel_Striping::Obstacle::Obstacle(const PState& ped) :
    xFwd(ped.getMaxX()),
    xBack(ped.getMinX()),
    speed(ped.myDir * ped.mySpeed),
    type(OBSTACLE_PED),
    description(ped.getID()) {
    assert(!ped.myWaitingToEnter);
}


// ===========================================================================
// MSPModel_Striping::PState method definitions
// ===========================================================================
MSPModel_Striping::PState::PState(MSPerson* person, MSStageMoving* stage, const MSLane* lane):
    myPerson(person),
    myStage(stage),
    myLane(lane),
    myRelX(stage->getDepartPos()),
    myRelY(stage->getDepartPosLat()),
    myDir(FORWARD),
    mySpeed(0),
    myWaitingToEnter(true),
    myWaitingTime(0),
    myWalkingAreaPath(nullptr),
    myAmJammed(false),
    myRemoteXYPos(Position::INVALID),
    myAngle(std::numeric_limits<double>::max()) {
    const MSEdge* currentEdge = &lane->getEdge();
    const ConstMSEdgeVector& route = myStage->getRoute();
    assert(!route.empty());
    if (route.size() == 1) {
        // only a single edge, move towards end pos
        myDir = (myRelX <= myStage->getArrivalPos()) ? FORWARD : BACKWARD;
    } else if (route.front()->getFunction() != SumoXMLEdgeFunc::NORMAL) {
        // start on an intersection
        myDir = FORWARD;
        if (route.front()->isWalkingArea()) {
            myWalkingAreaPath = getArbitraryPath(route.front());
        }
    } else {
        const bool mayStartForward = canTraverse(FORWARD, route) != UNDEFINED_DIRECTION;
        const bool mayStartBackward = canTraverse(BACKWARD, route) != UNDEFINED_DIRECTION;
        if DEBUGCOND(*this) {
            std::cout << "  initialize dir for " << myPerson->getID() << " forward=" << mayStartForward << " backward=" << mayStartBackward << "\n";
        }
        if (mayStartForward && mayStartBackward) {
            // figure out the best direction via routing
            ConstMSEdgeVector crossingRoute;
            MSNet::getInstance()->getPedestrianRouter(0).compute(currentEdge, route.back(), myRelX, myStage->getArrivalPos(), myStage->getMaxSpeed(person), 0, nullptr, crossingRoute, true);
            if (crossingRoute.size() > 1) {
                // route found
                const MSEdge* nextEdge = crossingRoute[1];
                if (nextEdge->getFromJunction() == currentEdge->getFromJunction() || nextEdge->getToJunction() == currentEdge->getFromJunction()) {
                    myDir = BACKWARD;
                }
            }
            if DEBUGCOND(*this) {
                std::cout << " crossingRoute=" << toString(crossingRoute) << "\n";
            }
        } else {
            myDir = !mayStartBackward ? FORWARD : BACKWARD;
        }
    }
    if (lane->getVehicleNumberWithPartials() > 0 && myRelY == 0) {
        // better start next to the road if nothing was specified
        myRelY -= stripeWidth;
    }
    if (myDir == FORWARD) {
        // start at the right side of the sidewalk
        myRelY = stripeWidth * (numStripes(lane) - 1) - myRelY;
    }
    if DEBUGCOND(*this) {
        std::cout << "  added new pedestrian " << myPerson->getID() << " on " << lane->getID() << " myRelX=" << myRelX << " myRelY=" << myRelY << " dir=" << myDir << " route=" << toString(myStage->getRoute()) << "\n";
    }

    myNLI = getNextLane(*this, lane, nullptr);
}

MSPModel_Striping::PState::PState():
    myPerson(nullptr),
    myStage(nullptr),
    myLane(nullptr),
    myRelX(0),
    myRelY(0),
    myDir(UNDEFINED_DIRECTION),
    mySpeed(0),
    myWaitingToEnter(false),
    myWaitingTime(0),
    myWalkingAreaPath(nullptr),
    myAmJammed(false),
    myRemoteXYPos(Position::INVALID),
    myAngle(std::numeric_limits<double>::max()) {
}


double
MSPModel_Striping::PState::getMinX(const bool includeMinGap) const {
    // @todo speed should have an influence here because faster persons need more space
    if (myDir == FORWARD) {
        return myRelX - getLength();
    }
    return myRelX - (includeMinGap ? getMinGap() : 0.);
}


double
MSPModel_Striping::PState::getMaxX(const bool includeMinGap) const {
    // @todo speed should have an influence here because faster persons need more space
    if (myDir == FORWARD) {
        return myRelX + (includeMinGap ? getMinGap() : 0.);
    }
    return myRelX + getLength();
}


double
MSPModel_Striping::PState::getLength() const {
    return myPerson->getVehicleType().getLength();
}


double
MSPModel_Striping::PState::getMinGap() const {
    return myPerson->getVehicleType().getMinGap();
}


int
MSPModel_Striping::PState::stripe(double relY) {
    return (int)floor(relY / stripeWidth + 0.5);
}


int
MSPModel_Striping::PState::otherStripe(double relY) const {
    const int s = stripe(relY);
    const double offset = relY - s * stripeWidth;
    const double threshold = MAX2(NUMERICAL_EPS, stripeWidth - SQUEEZE * getWidth());
    int result;
    if (offset > threshold) {
        result = s + 1;
    } else if (offset < -threshold) {
        result = s - 1;
    } else {
        result = s;
    }
    //std::cout.setf(std::ios::fixed , std::ios::floatfield);
    //std::cout << std::setprecision(5);
    //if DEBUGCOND(*this) std::cout << "  otherStripe " << myPerson->getID() << " offset=" << offset << " threshold=" << threshold << " rawResult=" << result << "\n";
    return result;
}

int
MSPModel_Striping::PState::stripe() const {
    return MIN2(MAX2(0, stripe(myRelY)), numStripes(myLane) - 1);
}


int
MSPModel_Striping::PState::otherStripe() const {
    return MIN2(MAX2(0, otherStripe(myRelY)), numStripes(myLane) - 1);
}


double
MSPModel_Striping::PState::distToLaneEnd() const {
    if (myStage->getNextRouteEdge() == nullptr) {
        return myDir * (myStage->getArrivalPos() - myRelX) - POSITION_EPS;
    } else {
        const double length = myWalkingAreaPath == nullptr ? myLane->getLength() : myWalkingAreaPath->length;
        return myDir == FORWARD ? length - myRelX : myRelX;
    }
}


bool
MSPModel_Striping::PState::moveToNextLane(SUMOTime currentTime) {
    double dist = distToLaneEnd();
    if (DEBUGCOND(*this)) {
        std::cout << SIMTIME << " ped=" << myPerson->getID() << " myRelX=" << myRelX << " dist=" << dist << "\n";
    }
    if (dist <= 0) {
        //if (ped.myPerson->getID() == DEBUG1) {
        //    std::cout << SIMTIME << " addToLane x=" << ped.myRelX << " newDir=" << newDir << " newLane=" << newLane->getID() << " walkingAreaShape=" << walkingAreaShape << "\n";
        //}
        //std::cout << " changing to " << newLane->getID() << " myRelY=" << ped.myRelY << " oldStripes=" << numStripes(myLane) << " newStripes=" << numStripes(newLane);
        //std::cout << " newY=" << ped.myRelY << " myDir=" << ped.myDir << " newDir=" << newDir;
        const int oldDir = myDir;
        const MSLane* oldLane = myLane;
        myLane = myNLI.lane;
        myDir = myNLI.dir;
        const bool normalLane = (myLane == nullptr || myLane->getEdge().getFunction() == SumoXMLEdgeFunc::NORMAL || &myLane->getEdge() == myStage->getNextRouteEdge());
        if DEBUGCOND(*this) {
            std::cout << SIMTIME
                      << " ped=" << myPerson->getID()
                      << " moveToNextLane old=" << oldLane->getID()
                      << " new=" << (myLane == nullptr ? "NULL" : myLane->getID())
                      << " oldDir=" << oldDir
                      << " newDir=" << myDir
                      << " myRelX=" << myRelX
                      << " dist=" << dist
                      << "\n";
        }
        if (myLane == nullptr) {
            myRelX = myStage->getArrivalPos();
        }
        // moveToNextEdge might destroy the person and thus mess up the heap. Better check first
        if (myStage->getRouteStep() == myStage->getRoute().end() - 1) {
            myLane = nullptr;
        } else {
            const bool arrived = myStage->moveToNextEdge(myPerson, currentTime, normalLane ? nullptr : &myLane->getEdge());
            UNUSED_PARAMETER(arrived);
            assert(!arrived);
            assert(myDir != UNDEFINED_DIRECTION);
            myNLI = getNextLane(*this, myLane, oldLane);
            assert(myNLI.lane != oldLane); // do not turn around
            if DEBUGCOND(*this) {
                std::cout << "    nextLane=" << (myNLI.lane == nullptr ? "NULL" : myNLI.lane->getID()) << "\n";
            }
            if (myLane->getEdge().isWalkingArea()) {
                if (myNLI.dir != UNDEFINED_DIRECTION) {
                    myWalkingAreaPath = getWalkingAreaPath(&myLane->getEdge(), oldLane, myNLI.lane);
                    assert(myWalkingAreaPath->shape.size() >= 2);
                    if DEBUGCOND(*this) {
                        std::cout << "  mWAPath shape=" << myWalkingAreaPath->shape << " length=" << myWalkingAreaPath->length << "\n";
                    }
                } else {
                    // disconnnected route. move to the next edge
                    if (OptionsCont::getOptions().getBool("ignore-route-errors")) {
                        // try to determine direction from topology, otherwise maintain current direction
                        const MSEdge* currRouteEdge = *myStage->getRouteStep();
                        const MSEdge* nextRouteEdge = myStage->getNextRouteEdge();
                        if ((nextRouteEdge->getToJunction() == currRouteEdge->getFromJunction())
                                || nextRouteEdge->getToJunction() == currRouteEdge->getToJunction()) {
                            myDir = BACKWARD;
                        } else if ((nextRouteEdge->getFromJunction() == currRouteEdge->getFromJunction())
                                   || nextRouteEdge->getFromJunction() == currRouteEdge->getToJunction()) {
                            myDir = FORWARD;
                        }
                        myStage->moveToNextEdge(myPerson, currentTime, nullptr);
                        myLane = myNLI.lane;
                        assert(myLane != 0);
                        assert(myLane->getEdge().getFunction() == SumoXMLEdgeFunc::NORMAL);
                        myNLI = getNextLane(*this, myLane, oldLane);
                        myWalkingAreaPath = nullptr;
                    } else {
                        throw ProcessError("Disconnected walk for person '" + myPerson->getID() + "'.");
                    }
                }
            } else {
                myWalkingAreaPath = nullptr;
            }
            // adapt x to fit onto the new lane
            // (make sure we do not move past the end of the new lane since that
            // lane was not checked for obstacles)
            const double newLength = (myWalkingAreaPath == nullptr ? myLane->getLength() : myWalkingAreaPath->length);
            if (-dist > newLength) {
                assert(OptionsCont::getOptions().getBool("ignore-route-errors"));
                // should not happen because the end of myLane should have been an obstacle as well
                // (only when the route is broken)
                dist = -newLength;
            }
            if (myDir == BACKWARD) {
                myRelX = newLength + dist;
            } else {
                myRelX = -dist;
            }
            if DEBUGCOND(*this) {
                std::cout << SIMTIME << " update myRelX ped=" << myPerson->getID()
                          << " newLength=" << newLength
                          << " dist=" << dist
                          << " myRelX=" << myRelX
                          << "\n";
            }
            // adjust to change in direction
            if (myDir != oldDir) {
                myRelY = (numStripes(oldLane) - 1) * stripeWidth - myRelY;
            }
            // adjust to differences in sidewalk width
            const int offset = getStripeOffset(numStripes(oldLane), numStripes(myLane), oldDir != myDir && numStripes(myLane) < numStripes(oldLane));
            myRelY += offset * stripeWidth;
            if DEBUGCOND(*this) {
                std::cout << SIMTIME << " transformY ped=" << myPerson->getID()
                          << " newLane=" << Named::getIDSecure(myLane)
                          << " newY=" << myRelY
                          << " os=" << numStripes(oldLane) << " ns=" << numStripes(myLane)
                          << " od=" << oldDir << " nd=" << myDir
                          << " offset=" << offset << "\n";
            }
        }
        return true;
    } else {
        return false;
    }
}

void
MSPModel_Striping::PState::walk(const Obstacles& obs, SUMOTime currentTime) {
    myAngle = std::numeric_limits<double>::max(); // set on first access or via remote control
    const int stripes = (int)obs.size();
    const int sMax =  stripes - 1;
    assert(stripes == numStripes(myLane));
    const double vMax = myStage->getMaxSpeed(myPerson);
    // ultimate goal is to choose the prefered stripe (chosen)
    const int current = stripe();
    const int other = otherStripe();
    // compute distances
    std::vector<double> distance(stripes);
    for (int i = 0; i < stripes; ++i) {
        distance[i] = distanceTo(obs[i], obs[i].type == OBSTACLE_PED);
    }
    // compute utility for all stripes
    std::vector<double> utility(stripes, 0);
    // forbid stripes which are blocked and also all stripes behind them
    for (int i = 0; i < stripes; ++i) {
        if (distance[i] == DIST_OVERLAP) {
            if (i == current && (!myWaitingToEnter || stripe() != stripe(myRelY))) {
                utility[i] += OBSTRUCTED_PENALTY;
            }
            if (i < current) {
                for (int j = 0; j <= i; ++j) {
                    utility[j] += OBSTRUCTED_PENALTY;
                }
            }
            if (i > current) {
                for (int j = i; j < stripes; ++j) {
                    utility[j] += OBSTRUCTED_PENALTY;
                }
            }
        }
    }
    // forbid a portion of the leftmost stripes (in walking direction).
    // lanes with stripes less than 1 / RESERVE_FOR_ONCOMING_FACTOR
    // may still deadlock in heavy pedestrian traffic
    const bool onJunction = myLane->getEdge().isWalkingArea() || myLane->getEdge().isCrossing();
    const int reserved = (int)floor(stripes * (onJunction ? RESERVE_FOR_ONCOMING_FACTOR_JUNCTIONS : RESERVE_FOR_ONCOMING_FACTOR));
    if (myDir == FORWARD) {
        for (int i = 0; i < reserved; ++i) {
            utility[i] += INAPPROPRIATE_PENALTY * (i == current ? 0.5 : 1);
        }
    } else {
        for (int i = sMax; i > sMax - reserved; --i) {
            utility[i] += INAPPROPRIATE_PENALTY * (i == current ? 0.5 : 1);
        }
    }
    // adapt utility based on obstacles
    for (int i = 0; i < stripes; ++i) {
        if (obs[i].speed * myDir < 0) {
            // penalize evasion to the left
            if (myDir == FORWARD && i > 0) {
                utility[i - 1] -= 0.5;
            } else if (myDir == BACKWARD && i < sMax) {
                utility[i + 1] -= 0.5;
            }
        }
        // compute expected distance achievable by staying on this stripe for a time horizon
        const double walkDist = MAX2(0., distance[i]); // disregard special distance flags
        const double lookAhead = obs[i].speed * myDir >= 0 ? LOOKAHEAD_SAMEDIR : LOOKAHEAD_ONCOMING;
        const double expectedDist = MIN2(vMax * LOOKAHEAD_SAMEDIR, walkDist + obs[i].speed * myDir * lookAhead);
        if (DEBUGCOND(*this)) {
            std::cout << " util=" << utility[i] << " exp=" << expectedDist << " dist=" << distance[i] << "\n";
        }
        if (expectedDist >= 0) {
            utility[i] += expectedDist;
        } else {
            // let only the distance count
            utility[i] += ONCOMING_CONFLICT_PENALTY + distance[i];
        }
    }
    // discourage use of the leftmost lane (in walking direction) if there are oncoming
    if (myDir == FORWARD && obs[0].speed < 0) {
        utility[0] += ONCOMING_CONFLICT_PENALTY;
    } else if (myDir == BACKWARD && obs[sMax].speed > 0) {
        utility[sMax] += ONCOMING_CONFLICT_PENALTY;
    }
    // penalize lateral movement (if the current stripe permits walking)
    if (distance[current] > 0 && myWaitingTime == 0) {
        for (int i = 0; i < stripes; ++i) {
            utility[i] += abs(i - current) * LATERAL_PENALTY;
        }
    }

    // select best stripe
    int chosen = current;
    for (int i = 0; i < stripes; ++i) {
        if (utility[i] > utility[chosen] && utility[i] >= INAPPROPRIATE_PENALTY * 0.5) {
            chosen = i;
        }
    }
    // compute speed components along both axes
    const int next = (chosen == current ? current : (chosen < current ? current - 1 : current + 1));
    double xDist = MIN3(distance[current], distance[other], distance[next]);
    if (next != chosen) {
        // ensure that we do not collide with an obstacle in the stripe beyond
        // next as this might become the 'other' stripe in the next step
        const int nextOther = chosen < current ? current - 2 : current + 2;
        xDist = MIN2(xDist, distance[nextOther]);
    }
    // XXX preferred gap differs between approaching a standing obstacle or a moving obstacle
    const double preferredGap = NUMERICAL_EPS;
    double xSpeed = MIN2(vMax, MAX2(0., DIST2SPEED(xDist - preferredGap)));
    if (xSpeed < NUMERICAL_EPS) {
        xSpeed = 0.;
    }
    if (DEBUGCOND(*this)) {
        std::cout << " xSpeedPotential=" << xSpeed << "\n";
    }
    // avoid tiny steps
    // XXX pressure from behind?
    if (mySpeed == 0 && xDist < MIN_STARTUP_DIST &&
            // unless walking towards a short lane
            !(
                (xDist == distance[current] && obs[current].type >= OBSTACLE_END)
                || (xDist == distance[other] && obs[other].type >= OBSTACLE_END)
                || (xDist == distance[next] && obs[next].type >= OBSTACLE_END))
       ) {
        xSpeed = 0;
    }
    if (xSpeed == 0) {
        if (myWaitingTime > (myLane->getEdge().isCrossing() ? jamTimeCrossing : jamTime) || myAmJammed) {
            // squeeze slowly through the crowd ignoring others
            if (!myAmJammed) {
                MSNet::getInstance()->getPersonControl().registerJammed();
                WRITE_WARNING("Person '" + myPerson->getID()
                              + "' is jammed on edge '" + myStage->getEdge()->getID()
                              + "', time=" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".");
                myAmJammed = true;
            }
            xSpeed = vMax / 4;
        }
    } else if (stripe(myRelY) >= 0 && stripe(myRelY) <= sMax)  {
        myAmJammed = false;
    }
    // dawdling
    const double dawdle = MIN2(xSpeed, RandHelper::rand() * vMax * dawdling);
    xSpeed -= dawdle;

    // XXX ensure that diagonal speed <= vMax
    // avoid deadlocks on narrow sidewalks
    //if (oncoming && xSpeed == 0 && myStage->getWaitingTime(currentTime) > TIME2STEPS(ONCOMIN_PATIENCE)) {
    //    if DEBUGCOND(*this) std::cout << "  stepping asside to resolve oncoming deadlock\n";
    //    xSpeed = POSITION_EPS; // reset myWaitingTime
    //     if (myDir == FORWARD && chosen < sMax) {
    //         chosen += 1;
    //     } else if (myDir == BACKWARD && chosen > 0) {
    //         chosen -= 1;
    //     }
    //}
    const double maxYSpeed = MIN2(MAX2(vMax * LATERAL_SPEED_FACTOR, vMax - xSpeed), stripeWidth);
    double ySpeed = 0;
    double yDist = 0;
    if (utility[next] > OBSTRUCTION_THRESHOLD && utility[chosen] > OBSTRUCTION_THRESHOLD) {
        // don't move laterally if the stripes are blocked
        yDist = (chosen * stripeWidth) - myRelY;
        if (fabs(yDist) > NUMERICAL_EPS) {
            ySpeed = (yDist > 0 ?
                      MIN2(maxYSpeed, DIST2SPEED(yDist)) :
                      MAX2(-maxYSpeed, DIST2SPEED(yDist)));
        }
    } else if (utility[next] <= OBSTRUCTION_THRESHOLD && obs[next].type == OBSTACLE_VEHICLE
               // still on the road
               && stripe() == stripe(myRelY)
               // only when the vehicle is moving on the same lane
               && !myLane->getEdge().isCrossing()) {
        // step aside to let the vehicle pass
        myRelY += myDir * vMax;
    }
    // DEBUG
    if DEBUGCOND(*this) {
        std::cout << SIMTIME
                  << " ped=" << myPerson->getID()
                  << " edge=" << myStage->getEdge()->getID()
                  << " x=" << myRelX
                  << " y=" << myRelY
                  << " d=" << myDir
                  << " pvx=" << mySpeed
                  << " cur=" << current
                  << " cho=" << chosen
                  << " oth=" << other
                  << " nxt=" << next
                  << " vx=" << xSpeed
                  << " dawdle=" << dawdle
                  << " vy=" << ySpeed
                  << " xd=" << xDist
                  << " yd=" << yDist
                  << " vMax=" << myStage->getMaxSpeed(myPerson)
                  << " wTime=" << myStage->getWaitingTime(currentTime)
                  << " jammed=" << myAmJammed
                  << "\n   distance=" << toString(distance)
                  << "\n   utility=" << toString(utility)
                  << "\n";
        DEBUG_PRINT(obs);
    }
    myRelX += SPEED2DIST(xSpeed * myDir);
    myRelY += SPEED2DIST(ySpeed);
    mySpeed = xSpeed;
    if (xSpeed >= SUMO_const_haltingSpeed) {
        myWaitingToEnter = false;
        myWaitingTime = 0;
    } else {
        myWaitingTime += DELTA_T;
    }
}


double
MSPModel_Striping::PState::getImpatience(SUMOTime now) const {
    return MAX2(0., MIN2(1., myPerson->getVehicleType().getImpatience()
                         + STEPS2TIME(myStage->getWaitingTime(now)) / MAX_WAIT_TOLERANCE));
}


double
MSPModel_Striping::PState::getEdgePos(const MSStageMoving&, SUMOTime) const {
    return myRelX;
}


Position
MSPModel_Striping::PState::getPosition(const MSStageMoving& stage, SUMOTime) const {
    if (myRemoteXYPos != Position::INVALID) {
        return myRemoteXYPos;
    }
    if (myLane == nullptr) {
        // pedestrian has already finished
        return Position::INVALID;
    }
    const double lateral_offset = myRelY + (stripeWidth - myLane->getWidth()) * 0.5;
    if (myWalkingAreaPath == nullptr) {
        return stage.getLanePosition(myLane, myRelX, lateral_offset);
    } else {
        //if DEBUGCOND(*this) {
        //    std::cout << SIMTIME
        //        << " getPosition (walkingArea)"
        //        << " p=" << myPerson->getID()
        //        << " x=" << myRelX
        //        << " y=" << myRelY
        //        << " latOffset=" << lateral_offset
        //        << " shape=" << myWalkingAreaPath->shape
        //        << " pos=" << myWalkingAreaPath->shape.positionAtOffset(myRelX, lateral_offset)
        //        << "\n";
        //}
        return myWalkingAreaPath->shape.positionAtOffset(myRelX, lateral_offset);
    }
}


double
MSPModel_Striping::PState::getAngle(const MSStageMoving&, SUMOTime) const {
    if (myAngle != std::numeric_limits<double>::max()) {
        return myAngle;
    }
    if (myLane == nullptr) {
        // pedestrian has already finished
        return 0;
    }
    const PositionVector& shp = myWalkingAreaPath == nullptr ? myLane->getShape() : myWalkingAreaPath->shape;
    double geomX = myWalkingAreaPath == nullptr ? myLane->interpolateLanePosToGeometryPos(myRelX) : myRelX;
    double angle = shp.rotationAtOffset(geomX) + (myDir == MSPModel::BACKWARD ? M_PI : 0);
    if (angle > M_PI) {
        angle -= 2 * M_PI;
    }
    myAngle = angle;
    return angle;
}


SUMOTime
MSPModel_Striping::PState::getWaitingTime(const MSStageMoving&, SUMOTime) const {
    return myWaitingTime;
}


double
MSPModel_Striping::PState::getSpeed(const MSStageMoving&) const {
    return mySpeed;
}


const MSEdge*
MSPModel_Striping::PState::getNextEdge(const MSStageMoving&) const {
    return myNLI.lane == nullptr ? nullptr : &myNLI.lane->getEdge();
}

void
MSPModel_Striping::PState::moveToXY(MSPerson* p, Position pos, MSLane* lane, double lanePos,
                                    double lanePosLat, double angle, int routeOffset,
                                    const ConstMSEdgeVector& edges, SUMOTime t) {
    MSPModel_Striping* pm = dynamic_cast<MSPModel_Striping*>(MSNet::getInstance()->getPersonControl().getMovementModel());
    assert(p == myPerson);
    assert(pm != nullptr);
    const double oldAngle = GeomHelper::naviDegree(getAngle(*myStage, t));
    myAngle = GeomHelper::fromNaviDegree(angle);
#ifdef DEBUG_MOVETOXY
    std::cout << SIMTIME << " ped=" << p->getID()
              << " moveToXY"
              << " pos=" << pos
              << " lane=" << lane->getID()
              << " lanePos=" << lanePos
              << " lanePosLat=" << lanePosLat
              << " angle=" << angle
              << " routeOffset=" << routeOffset
              << " edges=" << toString(edges)
              << " oldLane=" << Named::getIDSecure(myLane)
              << " path=" << (myWalkingAreaPath == nullptr ? "null" : (myWalkingAreaPath->from->getID() + "->" + myWalkingAreaPath->to->getID())) << "\n";
#endif

    if (lane != myLane && myLane != nullptr) {
        pm->remove(this);
        pm->registerActive();
    }
    if (lane != nullptr &&
            fabs(lanePosLat) < (0.5 * (lane->getWidth() + p->getVehicleType().getWidth()) + SIDEWALK_OFFSET)) {
        myRemoteXYPos = Position::INVALID;
        const MSEdge* old = myStage->getEdge();
        const MSLane* oldLane = myLane;
        if (lane != myLane) {
            // implicitly adds new active lane if necessary
            pm->myActiveLanes[lane].push_back(this);
        }
        if (edges.empty()) {
            // map within route
            myStage->setRouteIndex(myPerson, routeOffset);
        } else {
            myStage->replaceRoute(myPerson, edges, routeOffset);
        }
        if (!lane->getEdge().isNormal()) {
            myStage->moveToNextEdge(myPerson, t, &lane->getEdge());
        }

        myLane = lane;
        const double lateral_offset = (lane->getWidth() - stripeWidth) * 0.5;
        if (lane->getEdge().isWalkingArea()) {
            if (myWalkingAreaPath == nullptr || myWalkingAreaPath->lane != lane) {
                // entered new walkingarea. Determine path
                myWalkingAreaPath = guessPath(&lane->getEdge(), old, myStage->getNextRouteEdge());
#ifdef DEBUG_MOVETOXY
                std::cout << " guessPath old=" << old->getID() << " next=" << Named::getIDSecure(myStage->getNextRouteEdge())
                          << " path=" << myWalkingAreaPath->from->getID() << "->" << myWalkingAreaPath->to->getID() << "\n";
#endif
            }
            // lanePos and lanePosLat are matched onto the circumference of the
            // walkingarea. Use pos instead
            const Position relPos = myWalkingAreaPath->shape.transformToVectorCoordinates(pos);
            if (relPos == Position::INVALID) {
                WRITE_WARNING("Could not map position " + toString(pos) + " onto lane '" + myLane->getID() + "'");
                myRemoteXYPos = pos;
            } else {
                myRelX = relPos.x();
                myRelY = lateral_offset + relPos.y();
            }
        } else {
            myWalkingAreaPath = nullptr;
            myRelX = lanePos;
            myRelY = lateral_offset - lanePosLat;
        }
        // guess direction
        const double angleDiff = GeomHelper::getMinAngleDiff(angle, oldAngle);
        if (myStage->getNextRouteEdge() != nullptr) {
            if (myStage->getEdge()->getToJunction() == myStage->getNextRouteEdge()->getFromJunction() ||
                    myStage->getEdge()->getToJunction() == myStage->getNextRouteEdge()->getToJunction()) {
                myDir = FORWARD;
            } else {
                myDir = BACKWARD;
            }
        } else {
            // guess from angle
            if (angleDiff <= 90) {
                // keep direction
                if (myDir == UNDEFINED_DIRECTION) {
                    myDir = FORWARD;
                }
            } else {
                // change direction
                myDir = (myDir == BACKWARD) ? FORWARD : BACKWARD;
            }
        }
        // update next lane info (after guessing direction)
        if (oldLane == nullptr || &oldLane->getEdge() != &myLane->getEdge()) {
            const MSLane* sidewalk = getSidewalk<MSEdge, MSLane>(&myLane->getEdge());
            // assume that we will eventually move back onto the sidewalk if
            // there is one
            myNLI = getNextLane(*this, sidewalk == nullptr ? myLane : sidewalk, nullptr);
#ifdef DEBUG_MOVETOXY
            std::cout << " myNLI=" << Named::getIDSecure(myNLI.lane) << " link=" << (myNLI.link == nullptr ? "NULL" : myNLI.link->getDescription()) << " dir=" << myNLI.dir << "\n";
#endif
        }
#ifdef DEBUG_MOVETOXY
        std::cout << " newRelPos=" << Position(myRelX, myRelY) << " edge=" << myPerson->getEdge()->getID() << " newPos=" << myPerson->getPosition()
                  << " oldAngle=" << oldAngle << " angleDiff=" << angleDiff << " newDir=" << myDir << "\n";
#endif
    } else {
        // map outside the network
        myRemoteXYPos = pos;
    }

}


bool
MSPModel_Striping::PState::isJammed() const {
    return myAmJammed;
}

const MSLane*
MSPModel_Striping::PState::getLane() const {
    return myLane;
}

double
MSPModel_Striping::PState::distanceTo(const Obstacle& obs, const bool includeMinGap) const {
    // check for overlap
    const double maxX = getMaxX(includeMinGap);
    const double minX = getMinX(includeMinGap);
    //if (DEBUGCOND(*this)) {
    //    std::cout << std::setprecision(2) <<   "   distanceTo=" << obs.description << " maxX=" << maxX << " minX=" << minX << " obs.xFwd=" << obs.xFwd << " obs.xBack=" << obs.xBack << "\n";
    //}
    if ((obs.xFwd >= maxX && obs.xBack <= maxX) || (obs.xFwd <= maxX && obs.xFwd >= minX)) {
        // avoid blocking by itself on looped route
        return (obs.type == OBSTACLE_PED && obs.description == myPerson->getID()) ? DIST_FAR_AWAY : DIST_OVERLAP;
    }
    if (myDir == FORWARD) {
        return obs.xFwd < minX ? DIST_BEHIND : obs.xBack - maxX;
    } else {
        return obs.xBack > maxX ? DIST_BEHIND : minX - obs.xFwd;
    }
}


void
MSPModel_Striping::PState::mergeObstacles(Obstacles& into, const Obstacles& obs2) {
    for (int i = 0; i < (int)into.size(); ++i) {
        if (gDebugFlag1) {
            std::cout << "     i=" << i << " maxX=" << getMaxX(true) << " minX=" << getMinX(true)
                      << " into=" << into[i].description << " iDist=" << distanceTo(into[i], into[i].type == OBSTACLE_PED)
                      << " obs2=" << obs2[i].description << " oDist=" << distanceTo(obs2[i], obs2[i].type == OBSTACLE_PED) << "\n";
        }
        const double dO = distanceTo(obs2[i], obs2[i].type == OBSTACLE_PED);
        const double dI = distanceTo(into[i], into[i].type == OBSTACLE_PED);
        if (dO < dI) {
            into[i] = obs2[i];
        } else if (dO == dI
                   && into[i].type != OBSTACLE_PED
                   && into[i].type != OBSTACLE_VEHICLE
                   && (obs2[i].type == OBSTACLE_PED ||
                       obs2[i].type == OBSTACLE_VEHICLE)) {
            into[i] = obs2[i];
        }
    }
}

void
MSPModel_Striping::PState::mergeObstacles(Obstacles& into, const Obstacles& obs2, int dir, int offset) {
    for (int i = 0; i < (int)into.size(); ++i) {
        int i2 = i + offset;
        if (i2 >= 0 && i2 < (int)obs2.size()) {
            if (dir == FORWARD) {
                if (obs2[i2].xBack < into[i].xBack) {
                    into[i] = obs2[i2];
                }
            } else {
                if (obs2[i2].xFwd > into[i].xFwd) {
                    into[i] = obs2[i2];
                }
            }
        }
    }
}


bool
MSPModel_Striping::PState::ignoreRed(const MSLink* link) const {
    if (link->haveRed()) {
        const double ignoreRedTime = myPerson->getVehicleType().getParameter().getJMParam(SUMO_ATTR_JM_DRIVE_AFTER_RED_TIME, -1);
        if (ignoreRedTime >= 0) {
            const double redDuration = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - link->getLastStateChange());
            if (DEBUGCOND(*this)) {
                std::cout << SIMTIME << "  ignoreRedTime=" << ignoreRedTime << " redDuration=" << redDuration << "\n";
            }
            return ignoreRedTime > redDuration;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

const std::string&
MSPModel_Striping::PState::getID() const {
    return myPerson->getID();
}

double
MSPModel_Striping::PState::getWidth() const {
    return myPerson->getVehicleType().getWidth();
}


bool
MSPModel_Striping::PState::isRemoteControlled() const {
    return myPerson->hasInfluencer() && myPerson->getInfluencer().isRemoteControlled();
}

// ===========================================================================
// MSPModel_Striping::PStateVehicle method definitions
// ===========================================================================

MSPModel_Striping::PStateVehicle::PStateVehicle(const MSVehicle* veh, const MSLane* walkingarea, double relX, double relY):
    myVehicle(veh) {
    myLane = walkingarea; // to ensure correct limits when calling otherStripe()
    myRelX = relX;
    myRelY = relY;
}

const std::string&
MSPModel_Striping::PStateVehicle::getID() const {
    return myVehicle->getID();
}

double
MSPModel_Striping::PStateVehicle::getWidth() const {
    return myVehicle->getVehicleType().getWidth();
}

double
MSPModel_Striping::PStateVehicle::getMinX(const bool /*includeMinGap*/) const {
    return myRelX - myVehicle->getVehicleType().getWidth() / 2 - SAFETY_GAP ;
}

double
MSPModel_Striping::PStateVehicle::getMaxX(const bool /*includeMinGap*/) const {
    return myRelX + myVehicle->getVehicleType().getWidth() / 2 + SAFETY_GAP;
}

// ===========================================================================
// MSPModel_Striping::MovePedestrians method definitions
// ===========================================================================

SUMOTime
MSPModel_Striping::MovePedestrians::execute(SUMOTime currentTime) {
    std::set<MSPerson*> changedLane;
    myModel->moveInDirection(currentTime, changedLane, FORWARD);
    myModel->moveInDirection(currentTime, changedLane, BACKWARD);
    // DEBUG
#ifdef LOG_ALL
    for (ActiveLanes::const_iterator it_lane = myModel->getActiveLanes().begin(); it_lane != myModel->getActiveLanes().end(); ++it_lane) {
        const MSLane* lane = it_lane->first;
        Pedestrians pedestrians = it_lane->second;
        if (pedestrians.size() == 0) {
            continue;
        }
        sort(pedestrians.begin(), pedestrians.end(), by_xpos_sorter(FORWARD));
        std::cout << SIMTIME << " lane=" << lane->getID();
        for (int ii = 0; ii < (int)pedestrians.size(); ++ii) {
            const PState& p = *pedestrians[ii];
            std::cout << " (" << p.myPerson->getID() << " " << p.myRelX << "," << p.myRelY << " " << p.myDir << ")";
        }
        std::cout << "\n";
    }
#endif
    return DELTA_T;
}

