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
/// @file    MSTransportable.cpp
/// @author  Melanie Weber
/// @author  Andreas Kendziorra
/// @author  Michael Behrisch
/// @date    Thu, 12 Jun 2014
///
// The common superclass for modelling transportable objects like persons and containers
/****************************************************************************/
#include <config.h>

#include <utils/common/StringTokenizer.h>
#include <utils/geom/GeomHelper.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include <utils/router/PedestrianRouter.h>
#include <utils/router/IntermodalRouter.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSNet.h>
#include <microsim/MSStoppingPlace.h>
#include <microsim/transportables/MSPerson.h>
#include <microsim/transportables/MSStageDriving.h>
#include <microsim/devices/MSTransportableDevice.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/transportables/MSTransportableControl.h>
#include <microsim/transportables/MSTransportable.h>


// ===========================================================================
// method definitions
// ===========================================================================
MSTransportable::MSTransportable(const SUMOVehicleParameter* pars, MSVehicleType* vtype, MSTransportablePlan* plan, const bool isPerson) :
    SUMOTrafficObject(pars->id),
    myParameter(pars), myVType(vtype), myPlan(plan), myAmPerson(isPerson) {
    myStep = myPlan->begin();
    // init devices
    MSDevice::buildTransportableDevices(*this, myDevices);
}


MSTransportable::~MSTransportable() {
    if (myStep != myPlan->end() && getCurrentStageType() == MSStageType::DRIVING) {
        MSStageDriving* const stage = dynamic_cast<MSStageDriving*>(*myStep);
        if (stage->getVehicle() != nullptr) {
            stage->getVehicle()->removeTransportable(this);
        }
    }
    if (myPlan != nullptr) {
        for (MSTransportablePlan::const_iterator i = myPlan->begin(); i != myPlan->end(); ++i) {
            delete *i;
        }
        delete myPlan;
        myPlan = nullptr;
    }
    for (MSTransportableDevice* dev : myDevices) {
        delete dev;
    }
    delete myParameter;
    if (myVType->isVehicleSpecific()) {
        MSNet::getInstance()->getVehicleControl().removeVType(myVType);
    }
}

std::mt19937*
MSTransportable::getRNG() const {
    return getEdge()->getLanes()[0]->getRNG();
}

bool
MSTransportable::proceed(MSNet* net, SUMOTime time, const bool vehicleArrived) {
    MSStage* prior = *myStep;
    const std::string& error = prior->setArrived(net, this, time, vehicleArrived);
    // must be done before increasing myStep to avoid invalid state for rendering
    if (myAmPerson) {
        prior->getEdge()->removePerson(this);
    } else {
        prior->getEdge()->removeContainer(this);
    }
    myStep++;
    if (error != "") {
        throw ProcessError(error);
    }
    if (prior->getStageType() == MSStageType::WALKING) {
        checkAccess(prior);
    }
    if (myStep != myPlan->end()) {
        if ((*myStep)->getStageType() == MSStageType::WALKING && (prior->getStageType() != MSStageType::ACCESS || prior->getDestination() != (*myStep)->getFromEdge())) {
            checkAccess(prior, false);
        }
        (*myStep)->proceed(net, this, time, prior);
        return true;
    } else {
        // cleanup
        if (prior->getDestinationStop() != nullptr) {
            prior->getDestinationStop()->removeTransportable(this);
        }
        MSNet::getInstance()->getPersonControl().addArrived();
        return false;
    }
}


void
MSTransportable::setID(const std::string& /*newID*/) {
    throw ProcessError("Changing a transportable ID is not permitted");
}

SUMOTime
MSTransportable::getDesiredDepart() const {
    return myParameter->depart;
}

void
MSTransportable::setDeparted(SUMOTime now) {
    (*myStep)->setDeparted(now);
}

double
MSTransportable::getEdgePos() const {
    return (*myStep)->getEdgePos(MSNet::getInstance()->getCurrentTimeStep());
}

Position
MSTransportable::getPosition() const {
    return (*myStep)->getPosition(MSNet::getInstance()->getCurrentTimeStep());
}

double
MSTransportable::getAngle() const {
    return (*myStep)->getAngle(MSNet::getInstance()->getCurrentTimeStep());
}

double
MSTransportable::getWaitingSeconds() const {
    return STEPS2TIME((*myStep)->getWaitingTime(MSNet::getInstance()->getCurrentTimeStep()));
}

double
MSTransportable::getSpeed() const {
    return (*myStep)->getSpeed();
}


int
MSTransportable::getNumRemainingStages() const {
    return (int)(myPlan->end() - myStep);
}


int
MSTransportable::getNumStages() const {
    return (int)myPlan->size();
}


void
MSTransportable::tripInfoOutput(OutputDevice& os) const {
    os.openTag(isPerson() ? "personinfo" : "containerinfo");
    os.writeAttr("id", getID());
    os.writeAttr("depart", time2string(getDesiredDepart()));
    os.writeAttr("type", getVehicleType().getID());
    for (MSStage* const i : *myPlan) {
        i->tripInfoOutput(os, this);
    }
    os.closeTag();
}


void
MSTransportable::routeOutput(OutputDevice& os, const bool withRouteLength) const {
    const std::string typeID = (
                                   (isPerson() && getVehicleType().getID() == DEFAULT_PEDTYPE_ID)
                                   || (isContainer() && getVehicleType().getID() == DEFAULT_CONTAINERTYPE_ID)) ? "" : getVehicleType().getID();
    myParameter->write(os, OptionsCont::getOptions(), isPerson() ? SUMO_TAG_PERSON : SUMO_TAG_CONTAINER, typeID);
    if (hasArrived()) {
        os.writeAttr("arrival", time2string(MSNet::getInstance()->getCurrentTimeStep()));
    }
    const MSStage* previous = nullptr;
    for (const MSStage* const stage : *myPlan) {
        stage->routeOutput(myAmPerson, os, withRouteLength, previous);
        previous = stage;
    }
    os.closeTag();
    os.lf();
}


void
MSTransportable::appendStage(MSStage* stage, int next) {
    // myStep is invalidated upon modifying myPlan
    const int stepIndex = (int)(myStep - myPlan->begin());
    if (next < 0) {
        myPlan->push_back(stage);
    } else {
        if (stepIndex + next > (int)myPlan->size()) {
            throw ProcessError("invalid index '" + toString(next) + "' for inserting new stage into plan of '" + getID() + "'");
        }
        myPlan->insert(myPlan->begin() + stepIndex + next, stage);
    }
    myStep = myPlan->begin() + stepIndex;
}


void
MSTransportable::removeStage(int next) {
    assert(myStep + next < myPlan->end());
    assert(next >= 0);
    if (next > 0) {
        // myStep is invalidated upon modifying myPlan
        int stepIndex = (int)(myStep - myPlan->begin());
        delete *(myStep + next);
        myPlan->erase(myStep + next);
        myStep = myPlan->begin() + stepIndex;
    } else {
        if (myStep + 1 == myPlan->end()) {
            // stay in the simulation until the start of simStep to allow appending new stages (at the correct position)
            appendStage(new MSStageWaiting(getEdge(), nullptr, 0, 0, getEdgePos(), "last stage removed", false));
        }
        (*myStep)->abort(this);
        proceed(MSNet::getInstance(), MSNet::getInstance()->getCurrentTimeStep());
    }
}


void
MSTransportable::setSpeed(double speed) {
    for (MSTransportablePlan::const_iterator i = myPlan->begin(); i != myPlan->end(); ++i) {
        (*i)->setSpeed(speed);
    }
}


void
MSTransportable::replaceVehicleType(MSVehicleType* type) {
    if (myVType->isVehicleSpecific()) {
        MSNet::getInstance()->getVehicleControl().removeVType(myVType);
    }
    myVType = type;
}


MSVehicleType&
MSTransportable::getSingularType() {
    if (myVType->isVehicleSpecific()) {
        return *myVType;
    }
    MSVehicleType* type = myVType->buildSingularType(myVType->getID() + "@" + getID());
    replaceVehicleType(type);
    return *type;
}


PositionVector
MSTransportable::getBoundingBox() const {
    PositionVector centerLine;
    const Position p = getPosition();
    const double angle = getAngle();
    const double length = getVehicleType().getLength();
    const Position back = p + Position(-cos(angle) * length, -sin(angle) * length);
    centerLine.push_back(p);
    centerLine.push_back(back);
    centerLine.move2side(0.5 * getVehicleType().getWidth());
    PositionVector result = centerLine;
    centerLine.move2side(-getVehicleType().getWidth());
    result.append(centerLine.reverse(), POSITION_EPS);
    //std::cout << " transp=" << getID() << " p=" << p << " angle=" << GeomHelper::naviDegree(angle) << " back=" << back << " result=" << result << "\n";
    return result;
}


std::string
MSTransportable::getStageSummary(int stageIndex) const {
    assert(stageIndex < (int)myPlan->size());
    assert(stageIndex >= 0);
    return (*myPlan)[stageIndex]->getStageSummary(myAmPerson);
}


bool
MSTransportable::hasArrived() const {
    return myStep == myPlan->end();
}

bool
MSTransportable::hasDeparted() const {
    return myPlan->size() > 0 && myPlan->front()->getDeparted() >= 0;
}


void
MSTransportable::rerouteParkingArea(MSStoppingPlace* orig, MSStoppingPlace* replacement) {
    // check whether the transportable was riding to the orignal stop
    // @note: parkingArea can currently not be set as myDestinationStop so we
    // check for stops on the edge instead
    assert(getCurrentStageType() == MSStageType::DRIVING);
    if (!myAmPerson) {
        WRITE_WARNING("parkingAreaReroute not support for containers");
        return;
    }
    if (getDestination() == &orig->getLane().getEdge()) {
        MSStageDriving* const stage = dynamic_cast<MSStageDriving*>(*myStep);
        assert(stage != 0);
        assert(stage->getVehicle() != 0);
        // adapt plan
        stage->setDestination(&replacement->getLane().getEdge(), replacement);
        if (myStep + 1 == myPlan->end()) {
            return;
        }
        // if the next step is a walk, adapt the route
        MSStage* nextStage = *(myStep + 1);
        if (nextStage->getStageType() == MSStageType::TRIP) {
            dynamic_cast<MSStageTrip*>(nextStage)->setOrigin(stage->getDestination());
        } else if (nextStage->getStageType() == MSStageType::WALKING) {
            MSStageTrip* newStage = new MSStageTrip(stage->getDestination(), nullptr, nextStage->getDestination(),
                                                    nextStage->getDestinationStop(), -1, 0, "", -1, 1, 0, true, nextStage->getArrivalPos());
            removeStage(1);
            appendStage(newStage, 1);
        }
        // if the plan contains another ride with the same vehicle from the same
        // parking area, adapt the preceeding walk to end at the replacement
        // (ride origin is set implicitly from the walk destination)
        for (auto it = myStep + 2; it != myPlan->end(); it++) {
            const MSStage* const futureStage = *it;
            MSStage* const prevStage = *(it - 1);
            if (futureStage->getStageType() == MSStageType::DRIVING) {
                const MSStageDriving* const ds = static_cast<const MSStageDriving*>(futureStage);
                if (ds->getLines() == stage->getLines()
                        && prevStage->getDestination() == &orig->getLane().getEdge()) {
                    if (prevStage->getStageType() == MSStageType::TRIP) {
                        dynamic_cast<MSStageTrip*>(prevStage)->setDestination(stage->getDestination(), replacement);
                    } else if (prevStage->getStageType() == MSStageType::WALKING) {
                        MSStageTrip* newStage = new MSStageTrip(prevStage->getFromEdge(), nullptr, stage->getDestination(),
                                                                replacement, -1, 0, "", -1, 1, 0, true, stage->getArrivalPos());
                        int prevStageRelIndex = (int)(it - 1 - myStep);
                        removeStage(prevStageRelIndex);
                        appendStage(newStage, prevStageRelIndex);
                    }
                    break;
                }
            }
        }
    }
}

MSTransportableDevice*
MSTransportable::getDevice(const std::type_info& type) const {
    for (MSTransportableDevice* const dev : myDevices) {
        if (typeid(*dev) == type) {
            return dev;
        }
    }
    return nullptr;
}

double
MSTransportable::getSlope() const {
    const MSEdge* edge = getEdge();
    const double ep = getEdgePos();
    const double gp = edge->getLanes()[0]->interpolateLanePosToGeometryPos(ep);
    return edge->getLanes()[0]->getShape().slopeDegreeAtOffset(gp);
}

SUMOTime
MSTransportable::getWaitingTime() const {
    return (*myStep)->getWaitingTime(MSNet::getInstance()->getCurrentTimeStep());
}

double
MSTransportable::getMaxSpeed() const {
    return getVehicleType().getMaxSpeed() * getSpeedFactor();
}

SUMOVehicleClass
MSTransportable::getVClass() const {
    return getVehicleType().getVehicleClass();
}


void
MSTransportable::saveState(OutputDevice& out) {
    // this saves lots of departParameters which are only needed for transportables that did not yet depart
    // the parameters may hold the name of a vTypeDistribution but we are interested in the actual type
    myParameter->write(out, OptionsCont::getOptions(), myAmPerson ? SUMO_TAG_PERSON : SUMO_TAG_CONTAINER, getVehicleType().getID());
    std::ostringstream state;
    state << (myStep - myPlan->begin());
    (*myStep)->saveState(state);
    out.writeAttr(SUMO_ATTR_STATE, state.str());
    const MSStage* previous = nullptr;
    for (const MSStage* const stage : *myPlan) {
        stage->routeOutput(myAmPerson, out, false, previous);
        previous = stage;
    }
    out.closeTag();
}


void
MSTransportable::loadState(const std::string& state) {
    std::istringstream iss(state);
    int step;
    iss >> step;
    myStep = myPlan->begin() + step;
    (*myStep)->loadState(this, iss);
}


/****************************************************************************/
