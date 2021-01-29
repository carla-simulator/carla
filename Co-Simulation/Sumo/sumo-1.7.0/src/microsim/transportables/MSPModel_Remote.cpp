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
/// @file    MSPModel_Remote.cpp
/// @author  Gregor Laemmel
/// @date    Mon, 13 Jan 2014
///
// The pedestrian following model for remote controlled pedestrian movement
/****************************************************************************/

#include <jps.h>
#include "microsim/MSEdge.h"
#include "microsim/MSLane.h"
#include "microsim/MSEdgeControl.h"
#include "microsim/MSEventControl.h"
#include "microsim/MSGlobals.h"
#include "utils/geom/Position.h"
#include "utils/geom/PositionVector.h"
#include "MSPModel_Remote.h"


MSPModel_Remote::MSPModel_Remote(const OptionsCont& oc, MSNet* net) : myNet(net) {
    initialize();
    Event* e = new Event(this);
    net->getBeginOfTimestepEvents()->addEvent(e, net->getCurrentTimeStep() + DELTA_T);
}


MSTransportableStateAdapter*
MSPModel_Remote::add(MSTransportable* person, MSStageMoving* stage, SUMOTime now) {
    assert(person->getCurrentStageType() == MSTransportable::MOVING_WITHOUT_VEHICLE);

    PState* state = new PState(static_cast<MSPerson*>(person), stage);

    JPS_Agent req;
    int id = myLastId++;
    remoteIdPStateMapping[id] = state;
    /*    req.set_id(id);

        MSLane* departureLane = getFirstPedestrianLane(*(stage->getRoute().begin()));
        double departureOffsetAlongLane = stage->getDepartPos();

        //TODO fix this on casim side [GL]
        double offset = departureOffsetAlongLane == 0 ? 0.4 : -0.4;
        departureOffsetAlongLane += offset;

        Position departurePos = departureLane->getShape().positionAtOffset(departureOffsetAlongLane);
        hybridsim::Coordinate* departureCoordinate = req.mutable_enterlocation();
        departureCoordinate->set_x(departurePos.x());
        departureCoordinate->set_y(departurePos.y());

        MSLane* arrivalLane = getFirstPedestrianLane(*(stage->getRoute().end() - 1));
        double arrivalOffsetAlongLange = stage->getArrivalPos();
        Position arrivalPos = arrivalLane->getShape().positionAtOffset(arrivalOffsetAlongLange);
        hybridsim::Coordinate* arrivalCoordinate = req.mutable_leavelocation();
        arrivalCoordinate->set_x(arrivalPos.x());
        arrivalCoordinate->set_y(arrivalPos.y());


        const MSEdge* prv = 0;
        for (ConstMSEdgeVector::const_iterator it = stage->getRoute().begin(); it != stage->getRoute().end(); it++) {
            const MSEdge* edge = *it;
            int dir = FORWARD;//default
            if (prv == 0) {
                if (stage->getRoute().size() > 1) {
                    const MSEdge* nxt = *(it + 1);
                    dir = (edge->getFromJunction() == nxt->getFromJunction()
                           || edge->getFromJunction() == nxt->getToJunction()) ? BACKWARD : FORWARD;
                } else {
                    dir = stage->getDepartPos() == 0 ? FORWARD : BACKWARD;
                }
            } else {
                dir = (edge->getFromJunction() == prv->getToJunction()
                       || edge->getFromJunction() == prv->getFromJunction()) ? FORWARD : BACKWARD;
            }
            if (edgesTransitionsMapping.find(edge) == edgesTransitionsMapping.end()) {
                throw ProcessError("Cannot map edge : " + edge->getID() + " to remote simulation");
            };
            std::tuple<int, int> transitions = edgesTransitionsMapping[edge];

            int frId = dir == FORWARD ? std::get<0>(transitions) : std::get<1>(transitions);
            int toId = dir == FORWARD ? std::get<1>(transitions) : std::get<0>(transitions);
            hybridsim::Destination* destFr = req.add_dests();
            destFr->set_id(frId);
            hybridsim::Destination* destTo = req.add_dests();
            destTo->set_id(toId);
            prv = edge;
        }

        hybridsim::Boolean rpl;
        ClientContext context;
        Status st = myHybridsimStub->transferAgent(&context, req, &rpl);
        if (!st.ok()) {
            throw ProcessError("Person: " + person->getID() + " could not be transferred to remote simulation");
        }
        if (!rpl.val()) {
            //TODO not yet implemented
            throw ProcessError("Remote simulation declined to accept person: " + person->getID() + ".");
        }
    */
    return state;
}

MSPModel_Remote::~MSPModel_Remote() {
    /*
        hybridsim::Empty req;
        hybridsim::Empty rpl;
        ClientContext context1;
        Status st = myHybridsimStub->shutdown(&context1, req, &rpl);
        if (!st.ok()) {
            throw ProcessError("Could not shutdown remote server");
        }

    */
}


SUMOTime
MSPModel_Remote::execute(SUMOTime time) {
    /*
        hybridsim::LeftClosedRightOpenTimeInterval interval;
        interval.set_fromtimeincluding(time / DELTA_T);
        interval.set_totimeexcluding((time + DELTA_T) / DELTA_T);


        //1. simulate time interval
        hybridsim::Empty rpl;
        ClientContext context1;
        Status st = myHybridsimStub->simulatedTimeInerval(&context1, interval, &rpl);
        if (!st.ok()) {
            throw ProcessError("Could not simulated time interval from: " + toString(time) + " to: " + toString(time + DELTA_T));
        }

        //2. receive trajectories
        hybridsim::Empty req2;
        hybridsim::Trajectories trajectories;
        ClientContext context2;
        Status st2 = myHybridsimStub->receiveTrajectories(&context2, req2, &trajectories);
        if (!st2.ok()) {
            throw ProcessError("Could not receive trajectories from remote simulation");
        }
        for (hybridsim::Trajectory trajectory : trajectories.trajectories()) {
            if (remoteIdPStateMapping.find(trajectory.id()) != remoteIdPStateMapping.end()) {
                PState* pState = remoteIdPStateMapping[trajectory.id()];
                pState->setPosition(trajectory.x(), trajectory.y());
                pState->setPhi(trajectory.phi());
                if (transitionsEdgesMapping.find(trajectory.currentdest().id()) != transitionsEdgesMapping.end()) {
                    const MSEdge* nextTargetEdge = transitionsEdgesMapping[trajectory.currentdest().id()];
                    const MSEdge* nextStageEdge = pState->getStage()->getNextRouteEdge();
    //                const MSEdge* currentStageEdge = pState->getStage()->getEdge();
                    if (nextTargetEdge == nextStageEdge) {
                        const bool arrived = pState->getStage()->moveToNextEdge(pState->getPerson(), time);
                        std::cout << "next edge" << std::endl;
                    }
                }
    //            pState.
            } else {
                throw ProcessError("Pedestrian with id: " + toString(trajectory.id()) + " is not known.");
            }
        }

        //3. retrieve agents that are ready to come back home to SUMO
        hybridsim::Empty req3;
        hybridsim::Agents agents;
        ClientContext context3;
        Status st3 = myHybridsimStub->queryRetrievableAgents(&context3, req3, &agents);
        if (!st3.ok()) {
            throw ProcessError("Could not query retrievable agents");
        }
        //TODO check whether agents can be retrieved
        for (hybridsim::Agent agent : agents.agents()) {
            if (remoteIdPStateMapping.find(agent.id()) != remoteIdPStateMapping.end()) {
                PState* pState = remoteIdPStateMapping[agent.id()];
                while (!pState->getStage()->moveToNextEdge(pState->getPerson(), time)) {
                    remoteIdPStateMapping.erase(agent.id());
                }
            }
        }

        //4. confirm transferred agents
        hybridsim::Empty rpl2;
        ClientContext context4;
        Status st4 = myHybridsimStub->confirmRetrievedAgents(&context4, agents, &rpl2);
        if (!st4.ok()) {
            throw ProcessError("Could not confirm retrieved agents");
        }
    */
    return DELTA_T;
}


MSLane*
MSPModel_Remote::getFirstPedestrianLane(const MSEdge* const& edge) {
    for (MSLane* lane : edge->getLanes()) {
        if (lane->getPermissions() == SVC_PEDESTRIAN) {
            return lane;
        }
    }
    throw ProcessError("Edge: " + edge->getID() + " does not allow pedestrians.");
}


void
MSPModel_Remote::remove(MSTransportableStateAdapter* state) {
    // XXX do something here

}


void
MSPModel_Remote::initialize() {
    JPS_SimulationContext* context = JPS_initialize("geometry_file");
}



// ===========================================================================
// MSPModel_Remote::PState method definitions
// ===========================================================================
MSPModel_Remote::PState::PState(MSPerson* person, MSStageMoving* stage)
    : myPerson(person), myPhi(0), myPosition(0, 0), myStage(stage) {
}


MSPModel_Remote::PState::~PState() {
}


double MSPModel_Remote::PState::getEdgePos(const MSStageMoving& stage, SUMOTime now) const {
    return 0;
}


Position MSPModel_Remote::PState::getPosition(const MSStageMoving& stage, SUMOTime now) const {
    return myPosition;
}


double MSPModel_Remote::PState::getAngle(const MSStageMoving& stage, SUMOTime now) const {
    return myPhi;
}


SUMOTime MSPModel_Remote::PState::getWaitingTime(const MSStageMoving& stage, SUMOTime now) const {
    return 0;
}


double MSPModel_Remote::PState::getSpeed(const MSStageMoving& stage) const {
    return 0;
}


const MSEdge* MSPModel_Remote::PState::getNextEdge(const MSStageMoving& stage) const {
    return nullptr;
}


void MSPModel_Remote::PState::setPosition(double x, double y) {
    myPosition.set(x, y);
}


void MSPModel_Remote::PState::setPhi(double phi) {
    myPhi = phi;
}


MSStageMoving* MSPModel_Remote::PState::getStage() {
    return myStage;
}


MSPerson* MSPModel_Remote::PState::getPerson() {
    return myPerson;
}


bool
MSPModel_Remote::usingInternalLanes() {
    return MSGlobals::gUsingInternalLanes && MSNet::getInstance()->hasInternalLinks();
}

