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
/// @file    MSEdgeControl.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 09 Apr 2001
///
// Stores edges and lanes, performs moving of vehicle
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <queue>
#include <vector>
#include "MSEdgeControl.h"
#include "MSVehicleControl.h"
#include "MSGlobals.h"
#include "MSEdge.h"
#include "MSLane.h"
#include "MSVehicle.h"

#define PARALLEL_PLAN_MOVE
#define PARALLEL_EXEC_MOVE
//#define PARALLEL_CHANGE_LANES
//#define LOAD_BALANCING

// ===========================================================================
// member method definitions
// ===========================================================================
MSEdgeControl::MSEdgeControl(const std::vector< MSEdge* >& edges)
    : myEdges(edges),
      myLanes(MSLane::dictSize()),
      myWithVehicles2Integrate(MSGlobals::gNumSimThreads > 1),
      myLastLaneChange(MSEdge::dictSize()),
      myInactiveCheckCollisions(MSGlobals::gNumSimThreads > 1),
      myMinLengthGeometryFactor(1.) {
    // build the usage definitions for lanes
    for (MSEdge* const edge : myEdges) {
        const std::vector<MSLane*>& lanes = edge->getLanes();
        if (!edge->hasLaneChanger()) {
            const int pos = lanes.front()->getNumericalID();
            myLanes[pos].lane = lanes.front();
            myLanes[pos].amActive = false;
            myLanes[pos].haveNeighbors = false;
            myMinLengthGeometryFactor = MIN2(edge->getLengthGeometryFactor(), myMinLengthGeometryFactor);
        } else {
            for (MSLane* const l : lanes) {
                const int pos = l->getNumericalID();
                myLanes[pos].lane = l;
                myLanes[pos].amActive = false;
                myLanes[pos].haveNeighbors = true;
                myMinLengthGeometryFactor = MIN2(l->getLengthGeometryFactor(), myMinLengthGeometryFactor);
            }
            myLastLaneChange[edge->getNumericalID()] = -1;
        }
    }
#ifdef HAVE_FOX
    if (MSGlobals::gNumThreads > 1) {
        while (myThreadPool.size() < MSGlobals::gNumThreads) {
            new WorkerThread(myThreadPool);
        }
    }
#endif
}


MSEdgeControl::~MSEdgeControl() {
#ifdef HAVE_FOX
    myThreadPool.clear();
#endif
}


void
MSEdgeControl::patchActiveLanes() {
    for (std::set<MSLane*, ComparatorNumericalIdLess>::iterator i = myChangedStateLanes.begin(); i != myChangedStateLanes.end(); ++i) {
        LaneUsage& lu = myLanes[(*i)->getNumericalID()];
        // if the lane was inactive but is now...
        if (!lu.amActive && (*i)->getVehicleNumber() > 0) {
            // ... add to active lanes and mark as such
            if (lu.haveNeighbors) {
                myActiveLanes.push_front(*i);
            } else {
                myActiveLanes.push_back(*i);
            }
            lu.amActive = true;
        }
    }
    myChangedStateLanes.clear();
}


void
MSEdgeControl::planMovements(SUMOTime t) {
#ifdef LOAD_BALANCING
    myRNGLoad = std::priority_queue<std::pair<int, int> >();
    for (int i = 0; i < MSLane::getNumRNGs(); i++) {
        myRNGLoad.emplace(0, i);
    }
#endif
    for (std::list<MSLane*>::iterator i = myActiveLanes.begin(); i != myActiveLanes.end();) {
        const int vehNum = (*i)->getVehicleNumber();
        if (vehNum == 0) {
            myLanes[(*i)->getNumericalID()].amActive = false;
            i = myActiveLanes.erase(i);
        } else {
#ifdef LOAD_BALANCING
            std::pair<int, int> minRNG = myRNGLoad.top();
            (*i)->setRNGIndex(minRNG.second);
            myRNGLoad.pop();
            minRNG.first -= vehNum;
            myRNGLoad.push(minRNG);
#endif
#ifdef HAVE_FOX
            if (MSGlobals::gNumSimThreads > 1) {
                myThreadPool.add((*i)->getPlanMoveTask(t), (*i)->getRNGIndex() % myThreadPool.size());
                ++i;
                continue;
            }
#endif
            (*i)->planMovements(t);
            ++i;
        }
    }
#ifdef HAVE_FOX
    if (MSGlobals::gNumSimThreads > 1) {
        myThreadPool.waitAll(false);
    }
#endif
}


void
MSEdgeControl::setJunctionApproaches(SUMOTime t) {
    for (MSLane* const lane : myActiveLanes) {
        lane->setJunctionApproaches(t);
    }
}


void
MSEdgeControl::executeMovements(SUMOTime t) {
    std::vector<MSLane*> wasActive(myActiveLanes.begin(), myActiveLanes.end());
    myWithVehicles2Integrate.clear();
#ifdef HAVE_FOX
#ifdef PARALLEL_EXEC_MOVE
    if (MSGlobals::gNumSimThreads > 1) {
#ifdef LOAD_BALANCING
        myRNGLoad = std::priority_queue<std::pair<int, int> >();
        for (int i = 0; i < MSLane::getNumRNGs(); i++) {
            myRNGLoad.emplace(0, i);
        }
#endif
        for (MSLane* const lane : myActiveLanes) {
#ifdef LOAD_BALANCING
            std::pair<int, int> minRNG = myRNGLoad.top();
            lane->setRNGIndex(minRNG.second);
            myRNGLoad.pop();
            minRNG.first -= lane->getVehicleNumber();
            myRNGLoad.push(minRNG);
#endif
            myThreadPool.add(lane->getExecuteMoveTask(t), lane->getRNGIndex() % myThreadPool.size());
        }
        myThreadPool.waitAll(false);
    }
#endif
#endif
    for (std::list<MSLane*>::iterator i = myActiveLanes.begin(); i != myActiveLanes.end();) {
        if (
#ifdef PARALLEL_EXEC_MOVE
            MSGlobals::gNumSimThreads <= 1 &&
#endif
            (*i)->getVehicleNumber() > 0) {
            (*i)->executeMovements(t);
        }
        if ((*i)->getVehicleNumber() == 0) {
            myLanes[(*i)->getNumericalID()].amActive = false;
            i = myActiveLanes.erase(i);
        } else {
            ++i;
        }
    }
    for (MSLane* lane : wasActive) {
        lane->updateLengthSum();
    }
    MSNet::getInstance()->getVehicleControl().removePending();
    std::vector<MSLane*>& toIntegrate = myWithVehicles2Integrate.getContainer();
    std::sort(toIntegrate.begin(), toIntegrate.end(), ComparatorIdLess());
    /// @todo: sorting only needed to account for lane-ordering dependencies.
    //This should disappear when parallelization is working. Until then it would
    //be better to use ComparatorNumericalIdLess instead of ComparatorIdLess
    myWithVehicles2Integrate.unlock();
    for (MSLane* const lane : toIntegrate) {
        const bool wasInactive = lane->getVehicleNumber() == 0;
        lane->integrateNewVehicles();
        if (wasInactive && lane->getVehicleNumber() > 0) {
            LaneUsage& lu = myLanes[lane->getNumericalID()];
            if (!lu.amActive) {
                if (lu.haveNeighbors) {
                    myActiveLanes.push_front(lane);
                } else {
                    myActiveLanes.push_back(lane);
                }
                lu.amActive = true;
            }
        }
    }
}


void
MSEdgeControl::changeLanes(const SUMOTime t) {
    std::vector<MSLane*> toAdd;
#ifdef PARALLEL_CHANGE_LANES
    std::vector<const MSEdge*> recheckLaneUsage;
#endif
    MSGlobals::gComputeLC = true;
    for (const MSLane* const l : myActiveLanes) {
        if (myLanes[l->getNumericalID()].haveNeighbors) {
            const MSEdge& edge = l->getEdge();
            if (myLastLaneChange[edge.getNumericalID()] != t) {
                myLastLaneChange[edge.getNumericalID()] = t;
#ifdef PARALLEL_CHANGE_LANES
                if (MSGlobals::gNumSimThreads > 1) {
                    MSLane* lane = edge.getLanes()[0];
                    myThreadPool.add(lane->getLaneChangeTask(t), lane->getRNGIndex() % myThreadPool.size());
                    recheckLaneUsage.push_back(&edge);
                } else {
#endif
                    edge.changeLanes(t);
                    for (MSLane* const lane : edge.getLanes()) {
                        LaneUsage& lu = myLanes[lane->getNumericalID()];
                        //if ((*i)->getID() == "disabled") {
                        //    std::cout << SIMTIME << " vehicles=" << toString((*i)->getVehiclesSecure()) << "\n";
                        //    (*i)->releaseVehicles();
                        //}
                        if (lane->getVehicleNumber() > 0 && !lu.amActive) {
                            toAdd.push_back(lane);
                            lu.amActive = true;
                        }
                    }
#ifdef PARALLEL_CHANGE_LANES
                }
#endif
            }
        } else {
            break;
        }
    }

#ifdef PARALLEL_CHANGE_LANES
    if (MSGlobals::gNumSimThreads > 1) {
        myThreadPool.waitAll(false);
        for (const MSEdge* e : recheckLaneUsage) {
            for (MSLane* const l : e->getLanes()) {
                LaneUsage& lu = myLanes[l->getNumericalID()];
                if (l->getVehicleNumber() > 0 && !lu.amActive) {
                    toAdd.push_back(l);
                    lu.amActive = true;
                }
            }
        }
    }
#endif

    MSGlobals::gComputeLC = false;
    for (std::vector<MSLane*>::iterator i = toAdd.begin(); i != toAdd.end(); ++i) {
        myActiveLanes.push_front(*i);
    }
    if (MSGlobals::gLateralResolution > 0) {
        // sort maneuver reservations
        for (LaneUsageVector::iterator it = myLanes.begin(); it != myLanes.end(); ++it) {
            (*it).lane->sortManeuverReservations();
        }
    }
}


void
MSEdgeControl::detectCollisions(SUMOTime timestep, const std::string& stage) {
    // Detections is made by the edge's lanes, therefore hand over.
    for (MSLane* lane : myActiveLanes) {
        if (lane->needsCollisionCheck()) {
            lane->detectCollisions(timestep, stage);
        }
    }
    if (myInactiveCheckCollisions.size() > 0) {
        for (MSLane* lane : myInactiveCheckCollisions.getContainer()) {
            lane->detectCollisions(timestep, stage);
        }
        myInactiveCheckCollisions.clear();
        myInactiveCheckCollisions.unlock();
    }
}


void
MSEdgeControl::gotActive(MSLane* l) {
    myChangedStateLanes.insert(l);
}

void
MSEdgeControl::checkCollisionForInactive(MSLane* l) {
    myInactiveCheckCollisions.insert(l);
}

void
MSEdgeControl::setAdditionalRestrictions() {
    for (MSEdgeVector::const_iterator i = myEdges.begin(); i != myEdges.end(); ++i) {
        const std::vector<MSLane*>& lanes = (*i)->getLanes();
        for (std::vector<MSLane*>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
            (*j)->initRestrictions();
        }
    }
}


/****************************************************************************/
