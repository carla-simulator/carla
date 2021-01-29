/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSPushButton.cpp
/// @author  Federico Caselli
/// @date    2015-07-15
///
// The class for a PushButton
/****************************************************************************/

#define SWARM_DEBUG
#include <utils/common/SwarmDebug.h>
#include "MSPushButton.h"
#include "MSPhaseDefinition.h"
#include "../MSEdge.h"
#include "../MSLane.h"
#include "../MSVehicle.h"
#include <microsim/transportables/MSPerson.h>

MSPushButton::MSPushButton(const MSEdge* edge, const MSEdge* crossingEdge) {
    m_edge = edge;
    m_crossingEdge = crossingEdge;
}

MSPushButton::~MSPushButton() {
    /// do not delete the MSEdge here
}

bool MSPushButton::anyActive(const std::vector<MSPushButton*>& pushButtons) {
    for (std::vector<MSPushButton*>::const_iterator it = pushButtons.begin(); it != pushButtons.end(); ++it) {
        if (it.operator * ()->isActivated()) {
            return true;
        }
    }
    return false;
}

std::map<std::string, std::vector<std::string> > MSPedestrianPushButton::m_crossingEdgeMap;
bool MSPedestrianPushButton::m_crossingEdgeMapLoaded = false;

MSPedestrianPushButton::MSPedestrianPushButton(const MSEdge* walkingEdge, const MSEdge* crossingEdge)
    : MSPushButton(walkingEdge, crossingEdge) {
    assert(walkingEdge->isWalkingArea() || ((walkingEdge->getPermissions() & SVC_PEDESTRIAN) != 0));
    assert(crossingEdge->isCrossing());
}

bool MSPedestrianPushButton::isActivated() const {
    return isActiveForEdge(m_edge, m_crossingEdge);
}

bool MSPedestrianPushButton::isActiveForEdge(const MSEdge* walkingEdge, const MSEdge* crossing) {
    const std::set<MSTransportable*> persons = walkingEdge->getPersons();
    if (persons.size() > 0) {
        for (std::set<MSTransportable*>::const_iterator pIt = persons.begin(); pIt != persons.end(); ++pIt) {
            const MSPerson* person = (MSPerson*)*pIt;
            const MSEdge* nextEdge = person->getNextEdgePtr();
            ///TODO keep using >= 1 or switch to ==1. Should change return value from always active to active only when pressed?
            ///TODO If changed the swarm logic must be changed since it relies on this behavior that keeps it active
            if (person->getWaitingSeconds() >= 1 && nextEdge && nextEdge->getID() == crossing->getID()) {
                DBG(
                    std::ostringstream oss;
                    oss << "MSPedestrianPushButton::isActiveForEdge Pushbutton active for edge " << walkingEdge->getID() << " crossing " << crossing->getID()
                    << " for " << person->getID() << " wait " << person->getWaitingSeconds();
                    WRITE_MESSAGE(oss.str());
                );
                return true;
            }
        }
    } else {
        //No person currently on the edge. But there may be some vehicles of class pedestrian
        for (std::vector<MSLane*>::const_iterator laneIt = walkingEdge->getLanes().begin();
                laneIt != walkingEdge->getLanes().end(); ++laneIt) {
            MSLane* lane = *laneIt;
            MSLane::VehCont vehicles = lane->getVehiclesSecure();
            for (MSLane::VehCont::const_iterator vehicleIt = vehicles.begin(); vehicleIt != vehicles.end(); ++vehicleIt) {
                MSVehicle* vehicle = *vehicleIt;
                if (vehicle->getVClass() == SVC_PEDESTRIAN) {
                    // It's a pedestrian
                    const MSEdge* nextEdge = vehicle->succEdge(1);
                    if (vehicle->getWaitingSeconds() >= 1 && nextEdge) {
                        // Next edge is not internal. Try to find if between the current vehicle edge and the next is the crossing.
                        // To do that check if between the successors (or predecessor) of crossing is the next edge and walking  precedes (or ensue) it.
                        if ((std::find(crossing->getPredecessors().begin(), crossing->getPredecessors().end(), walkingEdge) != crossing->getPredecessors().end()
                                && std::find(crossing->getSuccessors().begin(), crossing->getSuccessors().end(), nextEdge) != crossing->getSuccessors().end())
                                || (std::find(crossing->getSuccessors().begin(), crossing->getSuccessors().end(), walkingEdge) != crossing->getSuccessors().end()
                                    && std::find(crossing->getPredecessors().begin(), crossing->getPredecessors().end(), nextEdge) != crossing->getPredecessors().end())) {
                            DBG(
                                std::ostringstream oss;
                                oss << "MSPedestrianPushButton::isActiveForEdge Pushbutton active for edge " << walkingEdge->getID() << " crossing " << crossing->getID()
                                << " for " << vehicle->getID() << " wait " << vehicle->getWaitingSeconds(); WRITE_MESSAGE(oss.str()););
                            // Also release the vehicles here
                            lane->releaseVehicles();
                            return true;
                        }
                    }
                }
            }
            lane->releaseVehicles();
        }
    }
    DBG(
        std::ostringstream oss;
        oss << "MSPedestrianPushButton::isActiveForEdge Pushbutton not active for edge " << walkingEdge->getID() << " crossing " << crossing->getID()
        << " num Persons " << persons.size();
        WRITE_MESSAGE(oss.str());
    );
    return false;
}


///@brief Checks if any of the edges is a walking area
void getWalking(const std::vector<MSEdge*>& edges, std::vector< MSEdge*>& walkingEdges) {
    for (std::vector<MSEdge*>::const_iterator it = edges.begin(); it != edges.end(); ++it) {
        MSEdge* edge = *it;
        if (edge->isWalkingArea() || ((edge->getPermissions() & SVC_PEDESTRIAN) != 0)) {
            walkingEdges.push_back(edge);
        }
    }
}

///@brief Get the walking areas adjacent to the crossing
const std::vector<MSEdge*> getWalkingAreas(const MSEdge* crossing) {
    std::vector<MSEdge*> walkingEdges;
    getWalking(crossing->getSuccessors(), walkingEdges);
    getWalking(crossing->getPredecessors(), walkingEdges);
    return walkingEdges;

}

bool MSPedestrianPushButton::isActiveOnAnySideOfTheRoad(const MSEdge* crossing) {
    const std::vector<MSEdge*> walkingList = getWalkingAreas(crossing);
    for (std::vector<MSEdge*>::const_iterator wIt = walkingList.begin(); wIt != walkingList.end(); ++wIt) {
        MSEdge* walking = *wIt;
        if (isActiveForEdge(walking, crossing)) {
            DBG(WRITE_MESSAGE("MSPedestrianPushButton::isActiveOnAnySideOfTheRoad crossing edge " + crossing->getID() + " walking edge" + walking->getID()););
            return true;
        }
    }
    return false;
}

std::vector<MSPushButton*> MSPedestrianPushButton::loadPushButtons(const MSPhaseDefinition* phase) {
    loadCrossingEdgeMap();
    std::vector<MSPushButton*> pushButtons;
    const std::vector<std::string> lanes = phase->getTargetLaneSet();
//    Multiple lane can be of the same edge, so I avoid readding them
    std::set<std::string> controlledEdges;
    for (std::vector<std::string>::const_iterator lIt = lanes.begin(); lIt != lanes.end(); ++lIt) {
        MSLane* lane = MSLane::dictionary(*lIt);
        if (lane) {
            MSEdge* laneEdge = &lane->getEdge();
            if (controlledEdges.count(laneEdge->getID()) != 0) {
                continue;
            }
            controlledEdges.insert(laneEdge->getID());
            if (m_crossingEdgeMap.find(laneEdge->getID()) != m_crossingEdgeMap.end()) {
                //For every crossing edge that crosses this edge
                for (std::vector<std::string>::const_iterator cIt = m_crossingEdgeMap[laneEdge->getID()].begin();
                        cIt != m_crossingEdgeMap[laneEdge->getID()].end(); ++cIt) {
                    MSEdge* crossing = MSEdge::dictionary(*cIt);
                    const std::vector<MSEdge*> walkingList = getWalkingAreas(crossing);
                    for (std::vector<MSEdge*>::const_iterator wIt = walkingList.begin(); wIt != walkingList.end(); ++wIt) {
                        MSEdge* walking = *wIt;
                        DBG(WRITE_MESSAGE("MSPedestrianPushButton::loadPushButtons Added pushButton for walking edge " + walking->getID() + " crossing edge "
                                          + crossing->getID() + " crossed edge " + laneEdge->getID() + ". Phase state " + phase->getState()););
                        pushButtons.push_back(new MSPedestrianPushButton(walking, crossing));
                    }
                }
            }
        }
    }
    return pushButtons;
}

void MSPedestrianPushButton::loadCrossingEdgeMap() {
    if (!m_crossingEdgeMapLoaded) {
        m_crossingEdgeMapLoaded = true;
        for (MSEdgeVector::const_iterator eIt = MSEdge::getAllEdges().begin(); eIt != MSEdge::getAllEdges().end(); ++eIt) {
            const MSEdge* edge = *eIt;
            if (edge->isCrossing()) {
                for (std::vector<std::string>::const_iterator cIt = edge->getCrossingEdges().begin();
                        cIt != edge->getCrossingEdges().end(); ++cIt) {
                    m_crossingEdgeMap[*cIt].push_back(edge->getID());
                }
            }
        }
    }
}

