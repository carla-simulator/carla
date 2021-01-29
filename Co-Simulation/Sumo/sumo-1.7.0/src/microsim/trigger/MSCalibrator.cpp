/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSCalibrator.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Tue, May 2005
///
// Calibrates the flow on an edge by removing an inserting vehicles
/****************************************************************************/
#include <config.h>

#include <string>
#include <algorithm>
#include <cmath>
#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSVehicleControl.h>
#include <microsim/output/MSRouteProbe.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include <utils/common/ToString.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringTokenizer.h>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/StringUtils.h>
#include <utils/options/OptionsCont.h>
#include <utils/vehicle/SUMOVehicleParserHelper.h>
#include <utils/distribution/RandomDistributor.h>
#include <utils/vehicle/SUMOVehicleParameter.h>
#include "MSCalibrator.h"

//#define MSCalibrator_DEBUG

#define DEBUGID ""
#define DEBUGCOND (getID() == DEBUGID)
#define DEBUGCOND2(id) ((id) == DEBUGID)

// ===========================================================================
// static members
// ===========================================================================
std::vector<MSMoveReminder*> MSCalibrator::LeftoverReminders;
std::vector<SUMOVehicleParameter*> MSCalibrator::LeftoverVehicleParameters;
std::map<std::string, MSCalibrator*> MSCalibrator::myInstances;

// ===========================================================================
// method definitions
// ===========================================================================
MSCalibrator::MSCalibrator(const std::string& id,
                           const MSEdge* const edge,
                           MSLane* lane,
                           const double pos,
                           const std::string& aXMLFilename,
                           const std::string& outputFilename,
                           const SUMOTime freq, const double length,
                           const MSRouteProbe* probe,
                           const double invalidJamThreshold,
                           const std::string& vTypes,
                           bool addLaneMeanData) :
    MSTrigger(id),
    MSRouteHandler(aXMLFilename, true),
    MSDetectorFileOutput(id, vTypes, false), // detecting persons not yet supported
    myEdge(edge),
    myLane(lane),
    myPos(pos), myProbe(probe),
    myMeanDataParent(id + "_dummyMeanData", 0, 0, false, false, false, false, false, false, 1, 0, 0, vTypes, ""),
    myEdgeMeanData(nullptr, length, false, &myMeanDataParent),
    myCurrentStateInterval(myIntervals.begin()),
    myOutput(nullptr), myFrequency(freq), myRemoved(0),
    myInserted(0), myClearedInJam(0),
    mySpeedIsDefault(true), myDidSpeedAdaption(false), myDidInit(false),
    myDefaultSpeed(myLane == nullptr ? myEdge->getSpeedLimit() : myLane->getSpeedLimit()),
    myHaveWarnedAboutClearingJam(false),
    myAmActive(false),
    myInvalidJamThreshold(invalidJamThreshold),
    myHaveInvalidJam(false) {
    myInstances[id] = this;
    if (outputFilename != "") {
        myOutput = &OutputDevice::getDevice(outputFilename);
        writeXMLDetectorProlog(*myOutput);
    }
    if (aXMLFilename != "") {
        XMLSubSys::runParser(*this, aXMLFilename);
        if (!myDidInit) {
            init();
        }
    }
    if (addLaneMeanData) {
        // disabled for METriggeredCalibrator
        for (MSLane* const eLane : myEdge->getLanes()) {
            if (myLane == nullptr || myLane == eLane) {
                //std::cout << " cali=" << getID() << " myLane=" << Named::getIDSecure(myLane) << " checkLane=" << i << "\n";
                MSMeanData_Net::MSLaneMeanDataValues* laneData = new MSMeanData_Net::MSLaneMeanDataValues(eLane, eLane->getLength(), true, &myMeanDataParent);
                laneData->setDescription("meandata_calibrator_" + eLane->getID());
                LeftoverReminders.push_back(laneData);
                myLaneMeanData.push_back(laneData);
                VehicleRemover* remover = new VehicleRemover(eLane, this);
                LeftoverReminders.push_back(remover);
                myVehicleRemovers.push_back(remover);
            }
        }
    }
}


void
MSCalibrator::init() {
    if (myIntervals.size() > 0) {
        if (myIntervals.back().end == -1) {
            myIntervals.back().end = SUMOTime_MAX;
        }
        // calibration should happen after regular insertions have taken place
        MSNet::getInstance()->getEndOfTimestepEvents()->addEvent(new CalibratorCommand(this));
    } else {
        WRITE_WARNING("No flow intervals in calibrator '" + getID() + "'.");
    }
    myDidInit = true;
}


MSCalibrator::~MSCalibrator() {
    if (myCurrentStateInterval != myIntervals.end()) {
        intervalEnd();
    }
    for (VehicleRemover* const remover : myVehicleRemovers) {
        remover->disable();
    }
    myInstances.erase(getID());
}

MSCalibrator::AspiredState
MSCalibrator::getCurrentStateInterval() const {
    if (myCurrentStateInterval == myIntervals.end()) {
        throw ProcessError("Calibrator '" + getID() + "' has no active or upcoming interval");
    }
    return *myCurrentStateInterval;
}

void
MSCalibrator::myStartElement(int element,
                             const SUMOSAXAttributes& attrs) {
    if (element == SUMO_TAG_FLOW) {
        AspiredState state;
        SUMOTime lastEnd = -1;
        if (myIntervals.size() > 0) {
            lastEnd = myIntervals.back().end;
            if (lastEnd == -1) {
                lastEnd = myIntervals.back().begin;
            }
        }
        try {
            bool ok = true;
            state.q = attrs.getOpt<double>(SUMO_ATTR_VEHSPERHOUR, nullptr, ok, -1.);
            state.v = attrs.getOpt<double>(SUMO_ATTR_SPEED, nullptr, ok, -1.);
            state.begin = attrs.getSUMOTimeReporting(SUMO_ATTR_BEGIN, getID().c_str(), ok);
            if (state.begin < lastEnd) {
                WRITE_ERROR("Overlapping or unsorted intervals in calibrator '" + getID() + "'.");
            }
            state.end = attrs.getOptSUMOTimeReporting(SUMO_ATTR_END, getID().c_str(), ok, -1);
            state.vehicleParameter = SUMOVehicleParserHelper::parseVehicleAttributes(element, attrs, true, true, true);
            LeftoverVehicleParameters.push_back(state.vehicleParameter);
            // vehicles should be inserted with max speed unless stated otherwise
            if (state.vehicleParameter->departSpeedProcedure == DepartSpeedDefinition::DEFAULT) {
                state.vehicleParameter->departSpeedProcedure = DepartSpeedDefinition::MAX;
            }
            // vehicles should be inserted on any lane unless stated otherwise
            if (state.vehicleParameter->departLaneProcedure == DepartLaneDefinition::DEFAULT) {
                if (myLane == nullptr) {
                    state.vehicleParameter->departLaneProcedure = DepartLaneDefinition::ALLOWED_FREE;
                } else {
                    state.vehicleParameter->departLaneProcedure = DepartLaneDefinition::GIVEN;
                    state.vehicleParameter->departLane = myLane->getIndex();
                }
            } else if (myLane != nullptr && (
                           state.vehicleParameter->departLaneProcedure != DepartLaneDefinition::GIVEN
                           || state.vehicleParameter->departLane != myLane->getIndex())) {
                WRITE_WARNING("Insertion lane may differ from calibrator lane for calibrator '" + getID() + "'.");
            }
            if (state.vehicleParameter->vtypeid != DEFAULT_VTYPE_ID &&
                    MSNet::getInstance()->getVehicleControl().getVType(state.vehicleParameter->vtypeid) == nullptr) {
                WRITE_ERROR("Unknown vehicle type '" + state.vehicleParameter->vtypeid + "' in calibrator '" + getID() + "'.");
            }
        } catch (EmptyData&) {
            WRITE_ERROR("Mandatory attribute missing in definition of calibrator '" + getID() + "'.");
        } catch (NumberFormatException&) {
            WRITE_ERROR("Non-numeric value for numeric attribute in definition of calibrator '" + getID() + "'.");
        }
        if (state.q < 0 && state.v < 0 && state.vehicleParameter->vtypeid == DEFAULT_VTYPE_ID) {
            WRITE_ERROR("Either 'vehsPerHour',  'speed' or 'type' has to be set in flow definition of calibrator '" + getID() + "'.");
        }
        if (MSGlobals::gUseMesoSim && state.q < 0 && state.vehicleParameter->vtypeid != DEFAULT_VTYPE_ID) {
            WRITE_ERROR("Type calibration is not supported in meso for calibrator '" + getID() + "'.");
        }
        if (myIntervals.size() > 0 && myIntervals.back().end == -1) {
            myIntervals.back().end = state.begin;
        }
        myIntervals.push_back(state);
        myCurrentStateInterval = myIntervals.begin();
    } else {
        MSRouteHandler::myStartElement(element, attrs);
    }
}


void
MSCalibrator::myEndElement(int element) {
    if (element == SUMO_TAG_CALIBRATOR) {
        if (!myDidInit) {
            init();
        }
    } else if (element != SUMO_TAG_FLOW) {
        MSRouteHandler::myEndElement(element);
    }
}


void
MSCalibrator::intervalEnd() {
    if (myOutput != nullptr) {
        writeXMLOutput(*myOutput, myCurrentStateInterval->begin, myCurrentStateInterval->end);
    }
    myDidSpeedAdaption = false;
    myInserted = 0;
    myRemoved = 0;
    myClearedInJam = 0;
    myHaveWarnedAboutClearingJam = false;
    reset();
}


bool
MSCalibrator::isCurrentStateActive(SUMOTime time) {
    while (myCurrentStateInterval != myIntervals.end() && myCurrentStateInterval->end <= time) {
        // XXX what about skipped intervals?
        myCurrentStateInterval++;
    }
    return myCurrentStateInterval != myIntervals.end() &&
           myCurrentStateInterval->begin <= time && myCurrentStateInterval->end > time;
}

int
MSCalibrator::totalWished() const {
    if (myCurrentStateInterval != myIntervals.end()) {
        const double totalHourFraction = STEPS2TIME(myCurrentStateInterval->end - myCurrentStateInterval->begin) / (double) 3600.;
        return (int)std::floor(myCurrentStateInterval->q * totalHourFraction + 0.5); // round to closest int
    } else {
        return -1;
    }
}


double
MSCalibrator::currentFlow() const {
    const double totalHourFraction = STEPS2TIME(MSNet::getInstance()->getCurrentTimeStep() - myCurrentStateInterval->begin) / (double) 3600.;
    return passed() / totalHourFraction;
}

double
MSCalibrator::currentSpeed() const {
    if (myEdgeMeanData.getSamples() > 0) {
        return myEdgeMeanData.getTravelledDistance() / myEdgeMeanData.getSamples();
    } else {
        return -1;
    }
}


bool
MSCalibrator::removePending() {
    if (myToRemove.size() > 0) {
        MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
        // it is not save to remove the vehicles inside
        // VehicleRemover::notifyEnter so we do it here
        for (std::set<std::string>::iterator it = myToRemove.begin(); it != myToRemove.end(); ++it) {
            MSVehicle* vehicle = dynamic_cast<MSVehicle*>(vc.getVehicle(*it));
            if (vehicle != nullptr) {
                vehicle->onRemovalFromNet(MSMoveReminder::NOTIFICATION_VAPORIZED_CALIBRATOR);
                vehicle->getLane()->removeVehicle(vehicle, MSMoveReminder::NOTIFICATION_VAPORIZED_CALIBRATOR);
                vc.scheduleVehicleRemoval(vehicle, true);
            } else {
                WRITE_WARNING("Calibrator '" + getID() + "' could not remove vehicle '" + *it + "' time=" + time2string(MSNet::getInstance()->getCurrentTimeStep()) + ".");
            }
        }
        myToRemove.clear();
        return true;
    }
    return false;
}


SUMOTime
MSCalibrator::execute(SUMOTime currentTime) {
    const bool calibrateFlow = myCurrentStateInterval->q >= 0;
    const bool calibrateSpeed = myCurrentStateInterval->v >= 0;
    // get current simulation values (valid for the last simulation second)
    myHaveInvalidJam = (calibrateFlow || calibrateSpeed) && invalidJam(myLane == 0 ? -1 : myLane->getIndex());
    // XXX could we miss vehicle movements if this is called less often than every DELTA_T (default) ?
    updateMeanData();
    const bool hadRemovals = removePending();
    // check whether an adaptation value exists
    if (isCurrentStateActive(currentTime)) {
        myAmActive = true;
        // all happens in isCurrentStateActive()
    } else {
        myAmActive = false;
        reset();
        if (!mySpeedIsDefault) {
            // reset speed to default
            if (myLane == nullptr) {
                myEdge->setMaxSpeed(myDefaultSpeed);
            } else {
                myLane->setMaxSpeed(myDefaultSpeed);
            }
            mySpeedIsDefault = true;
        }
        if (myCurrentStateInterval == myIntervals.end()) {
            // keep calibrator alive for gui but do not call again
            return TIME2STEPS(86400);
        }
        return myFrequency;
    }
    // we are active
    if (!myDidSpeedAdaption && calibrateSpeed) {
        if (myLane == nullptr) {
            myEdge->setMaxSpeed(myCurrentStateInterval->v);
        } else {
            myLane->setMaxSpeed(myCurrentStateInterval->v);
        }
        mySpeedIsDefault = false;
        myDidSpeedAdaption = true;
    }

    const int totalWishedNum = totalWished();
    int adaptedNum = passed() + myClearedInJam;
#ifdef MSCalibrator_DEBUG
    if (DEBUGCOND) {
        std::cout << time2string(currentTime) << " " << getID()
                  << " q=" << myCurrentStateInterval->q
                  << " totalWished=" << totalWishedNum
                  << " adapted=" << adaptedNum
                  << " jam=" << myHaveInvalidJam
                  << " entered=" << myEdgeMeanData.nVehEntered
                  << " departed=" << myEdgeMeanData.nVehDeparted
                  << " arrived=" << myEdgeMeanData.nVehArrived
                  << " left=" << myEdgeMeanData.nVehLeft
                  << " waitSecs=" << myEdgeMeanData.waitSeconds
                  << " vaporized=" << myEdgeMeanData.nVehVaporized
                  << "\n";
    }
#endif
    if (calibrateFlow && adaptedNum < totalWishedNum && !hadRemovals) {
        // we need to insert some vehicles
        const double hourFraction = STEPS2TIME(currentTime - myCurrentStateInterval->begin + DELTA_T) / (double) 3600.;
        const int wishedNum = (int)std::floor(myCurrentStateInterval->q * hourFraction + 0.5); // round to closest int
        // only the difference between inflow and aspiredFlow should be added, thus
        // we should not count vehicles vaporized from a jam here
        // if we have enough time left we can add missing vehicles later
        const int relaxedInsertion = (int)std::floor(STEPS2TIME(myCurrentStateInterval->end - currentTime) / 3);
        const int insertionSlack = MAX2(0, adaptedNum + relaxedInsertion - totalWishedNum);
        // increase number of vehicles
#ifdef MSCalibrator_DEBUG
        if (DEBUGCOND) {
            std::cout << "   wished:" << wishedNum
                      << " slack:" << insertionSlack
                      << " before:" << adaptedNum
                      << "\n";
        }
#endif
        while (wishedNum > adaptedNum + insertionSlack) {
            SUMOVehicleParameter* pars = myCurrentStateInterval->vehicleParameter;
            const MSRoute* route = myProbe != nullptr ? myProbe->sampleRoute() : nullptr;
            if (route == nullptr) {
                route = MSRoute::dictionary(pars->routeid);
            }
            if (route == nullptr) {
                WRITE_WARNING("No valid routes in calibrator '" + getID() + "'.");
                break;
            }
            if (!route->contains(myEdge)) {
                WRITE_WARNING("Route '" + route->getID() + "' in calibrator '" + getID() + "' does not contain edge '" + myEdge->getID() + "'.");
                break;
            }
            const int routeIndex = (int)std::distance(route->begin(),
                                   std::find(route->begin(), route->end(), myEdge));
            MSVehicleType* vtype = MSNet::getInstance()->getVehicleControl().getVType(pars->vtypeid);
            assert(route != 0 && vtype != 0);
            // build the vehicle
            SUMOVehicleParameter* newPars = new SUMOVehicleParameter(*pars);
            newPars->id = getNewVehicleID();
            newPars->depart = currentTime;
            newPars->routeid = route->getID();
            newPars->departLaneProcedure = DepartLaneDefinition::FIRST_ALLOWED; // ensure successful vehicle creation
            MSVehicle* vehicle;
            try {
                vehicle = dynamic_cast<MSVehicle*>(MSNet::getInstance()->getVehicleControl().buildVehicle(
                                                       newPars, route, vtype, true, false));
            } catch (const ProcessError& e) {
                if (!MSGlobals::gCheckRoutes) {
                    WRITE_WARNING(e.what());
                    vehicle = nullptr;
                    break;
                } else {
                    throw e;
                }
            }
#ifdef MSCalibrator_DEBUG
            if (DEBUGCOND) {
                std::cout << " resetting route pos: " << routeIndex << "\n";
            }
#endif
            vehicle->resetRoutePosition(routeIndex, pars->departLaneProcedure);
            if (myEdge->insertVehicle(*vehicle, currentTime)) {
                if (!MSNet::getInstance()->getVehicleControl().addVehicle(vehicle->getID(), vehicle)) {
                    throw ProcessError("Emission of vehicle '" + vehicle->getID() + "' in calibrator '" + getID() + "'failed!");
                }
                myInserted++;
                adaptedNum++;
#ifdef MSCalibrator_DEBUG
                if (DEBUGCOND) {
                    std::cout << "I ";
                }
#endif
            } else {
                // could not insert vehicle
#ifdef MSCalibrator_DEBUG
                if (DEBUGCOND) {
                    std::cout << "F ";
                }
#endif
                MSNet::getInstance()->getVehicleControl().deleteVehicle(vehicle, true);
                break;
            }
        }
    }
    if (myCurrentStateInterval->end <= currentTime + myFrequency) {
        intervalEnd();
    }
    return myFrequency;
}

void
MSCalibrator::reset() {
    myEdgeMeanData.reset();
    for (std::vector<MSMeanData_Net::MSLaneMeanDataValues*>::iterator it = myLaneMeanData.begin(); it != myLaneMeanData.end(); ++it) {
        (*it)->reset();
    }
}


bool
MSCalibrator::invalidJam(int laneIndex) const {
    if (laneIndex < 0) {
        const int numLanes = (int)myEdge->getLanes().size();
        for (int i = 0; i < numLanes; ++i) {
            if (invalidJam(i)) {
                return true;
            }
        }
        return false;
    }
    assert(laneIndex < (int)myEdge->getLanes().size());
    const MSLane* const lane = myEdge->getLanes()[laneIndex];
    if (lane->getVehicleNumber() < 4) {
        // cannot reliably detect invalid jams
        return false;
    }
    // maxSpeed reflects the calibration target
    const bool toSlow = lane->getMeanSpeed() < myInvalidJamThreshold * myEdge->getSpeedLimit();
    return toSlow && remainingVehicleCapacity(laneIndex) < 1;
}


int
MSCalibrator::remainingVehicleCapacity(int laneIndex) const {
    if (laneIndex < 0) {
        const int numLanes = (int)myEdge->getLanes().size();
        int result = 0;
        for (int i = 0; i < numLanes; ++i) {
            result = MAX2(result, remainingVehicleCapacity(i));
        }
        return result;
    }
    assert(laneIndex < (int)myEdge->getLanes().size());
    MSLane* lane = myEdge->getLanes()[laneIndex];
    MSVehicle* last = lane->getLastFullVehicle();
    const SUMOVehicleParameter* pars = myCurrentStateInterval->vehicleParameter;
    const MSVehicleType* vtype = MSNet::getInstance()->getVehicleControl().getVType(pars->vtypeid);
    const double spacePerVehicle = vtype->getLengthWithGap() + myEdge->getSpeedLimit() * vtype->getCarFollowModel().getHeadwayTime();
    int overallSpaceLeft = (int)ceil(lane->getLength() / spacePerVehicle) - lane->getVehicleNumber();
    if (last != nullptr) {
        int entrySpaceLeft = (int)(last->getPositionOnLane() / spacePerVehicle);
        return MAX2(overallSpaceLeft, entrySpaceLeft);
    } else {
        return overallSpaceLeft;
    }
}


void
MSCalibrator::cleanup() {
    for (std::vector<MSMoveReminder*>::iterator it = LeftoverReminders.begin(); it != LeftoverReminders.end(); ++it) {
        delete *it;
    }
    LeftoverReminders.clear();
    for (std::vector<SUMOVehicleParameter*>::iterator it = LeftoverVehicleParameters.begin();
            it != LeftoverVehicleParameters.end(); ++it) {
        delete *it;
    }
    LeftoverVehicleParameters.clear();
    myInstances.clear(); // deletion is performed in MSTrigger::cleanup()
}


void
MSCalibrator::updateMeanData() {
    myEdgeMeanData.reset();
    for (std::vector<MSMeanData_Net::MSLaneMeanDataValues*>::iterator it = myLaneMeanData.begin();
            it != myLaneMeanData.end(); ++it) {
        (*it)->addTo(myEdgeMeanData);
    }
}


bool
MSCalibrator::VehicleRemover::notifyEnter(SUMOTrafficObject& veh, Notification /* reason */, const MSLane* /* enteredLane */) {
    if (myParent == nullptr) {
        return false;
    }
    if (!myParent->vehicleApplies(veh)) {
        return false;
    }
    if (myParent->isActive()) {
        myParent->updateMeanData();
        const bool calibrateFlow = myParent->myCurrentStateInterval->q >= 0;
        const int totalWishedNum = myParent->totalWished();
        int adaptedNum = myParent->passed() + myParent->myClearedInJam;
        if (calibrateFlow && adaptedNum > totalWishedNum) {
#ifdef MSCalibrator_DEBUG
            if (DEBUGCOND2(myParent->getID())) std::cout << time2string(MSNet::getInstance()->getCurrentTimeStep()) << " " << myParent->getID()
                        << " vaporizing " << vehicle->getID() << " to reduce flow\n";
#endif
            if (myParent->scheduleRemoval(&veh)) {
                myParent->myRemoved++;
            }
        } else if (myParent->myHaveInvalidJam) {
#ifdef MSCalibrator_DEBUG
            if (DEBUGCOND2(myParent->getID())) std::cout << time2string(MSNet::getInstance()->getCurrentTimeStep()) << " " << myParent->getID()
                        << " vaporizing " << vehicle->getID() << " to clear jam\n";
#endif
            if (!myParent->myHaveWarnedAboutClearingJam) {
                WRITE_WARNING("Clearing jam at calibrator '" + myParent->getID() + "' at time "
                              + time2string(MSNet::getInstance()->getCurrentTimeStep()));
                myParent->myHaveWarnedAboutClearingJam = true;
            }
            if (myParent->scheduleRemoval(&veh)) {
                myParent->myClearedInJam++;
            }
        }
        const std::string typeID = myParent->myCurrentStateInterval->vehicleParameter->vtypeid;
        if (!calibrateFlow && typeID != DEFAULT_VTYPE_ID) {
            // calibrate type
            const std::string origType = veh.getParameter().vtypeid; // could by id of vTypeDistribution
            const MSVehicleControl& vc = MSNet::getInstance()->getVehicleControl();
            const RandomDistributor<MSVehicleType*>* oldDist = vc.getVTypeDistribution(origType);
            const RandomDistributor<MSVehicleType*>* newDist = vc.getVTypeDistribution(typeID);
            bool matchDistribution = false;
            if (oldDist != nullptr && newDist != nullptr &&  oldDist->getVals().size() == newDist->getVals().size()) {
                auto it = std::find(oldDist->getVals().begin(), oldDist->getVals().end(), &veh.getVehicleType());
                if (it != oldDist->getVals().end()) {
                    matchDistribution = true;
                    const int distIndex = (int)(it - oldDist->getVals().begin());
                    veh.replaceVehicleType(newDist->getVals()[distIndex]);
                }
            }
            if (!matchDistribution) {
                MSVehicleType* vehicleType = MSNet::getInstance()->getVehicleControl().getVType(typeID);
                if (vehicleType == nullptr) {
                    throw ProcessError("Unknown vehicle type '" + typeID + "' in calibrator '" + myParent->getID() + "'");
                }
                veh.replaceVehicleType(vehicleType);
            }
        }
    }
    return true;
}


void
MSCalibrator::writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime) {
    updateMeanData();
    const int p = passed();
    // meandata will be off if vehicles are removed on the next edge instead of this one
    const int discrepancy = myEdgeMeanData.nVehEntered + myEdgeMeanData.nVehDeparted - myEdgeMeanData.nVehVaporized - passed();
    //assert(discrepancy >= 0); may go negative for lane calibrator when vehicles change lane before removal
    const std::string ds = (discrepancy > 0 ? "\" vaporizedOnNextEdge=\"" + toString(discrepancy) : "");
    const double durationSeconds = STEPS2TIME(stopTime - startTime);
    dev.openTag(SUMO_TAG_INTERVAL);
    dev.writeAttr(SUMO_ATTR_BEGIN, time2string(startTime));
    dev.writeAttr(SUMO_ATTR_END, time2string(stopTime));
    dev.writeAttr(SUMO_ATTR_ID, getID());
    dev.writeAttr("nVehContrib", p);
    dev.writeAttr("removed", myRemoved);
    dev.writeAttr("inserted", myInserted);
    dev.writeAttr("cleared", myClearedInJam);
    dev.writeAttr("flow", p * 3600.0 / durationSeconds);
    dev.writeAttr("aspiredFlow", myCurrentStateInterval->q);
    dev.writeAttr(SUMO_ATTR_SPEED, myEdgeMeanData.getSamples() != 0
                  ? myEdgeMeanData.getTravelledDistance() / myEdgeMeanData.getSamples() : -1);
    dev.writeAttr("aspiredSpeed", myCurrentStateInterval->v);
    if (discrepancy > 0) {
        dev.writeAttr("vaporizedOnNextEdge", discrepancy);
    }
    dev.closeTag();
}

void
MSCalibrator::writeXMLDetectorProlog(OutputDevice& dev) const {
    dev.writeXMLHeader("calibratorstats", "calibratorstats_file.xsd");
}

std::string
MSCalibrator::getNewVehicleID() {
    // avoid name clash for subsecond interval spacing
    const double beginS = STEPS2TIME(myCurrentStateInterval->begin);
    const int precision = beginS == int(beginS) ? 0 : 2;
    return getID() + "." + toString(beginS, precision) + "." + toString(myInserted);
}

void
MSCalibrator::setFlow(SUMOTime begin, SUMOTime end, double vehsPerHour, double speed, SUMOVehicleParameter vehicleParameter) {
    auto it = myCurrentStateInterval;
    while (it != myIntervals.end()) {
        if (it->begin > begin) {
            throw ProcessError("Cannot set flow for calibrator '" + getID() + "' with begin time " + time2string(begin) + " in the past.");
        } else if (it->begin == begin && it->end == end) {
            // update current interval
            AspiredState& state = const_cast<AspiredState&>(*it);
            state.q = vehsPerHour;
            state.v = speed;
            state.vehicleParameter->vtypeid = vehicleParameter.vtypeid;
            state.vehicleParameter->routeid = vehicleParameter.routeid;
            state.vehicleParameter->departLane = vehicleParameter.departLane;
            state.vehicleParameter->departLaneProcedure = vehicleParameter.departLaneProcedure;
            state.vehicleParameter->departSpeed = vehicleParameter.departSpeed;
            state.vehicleParameter->departSpeedProcedure = vehicleParameter.departSpeedProcedure;
            return;
        } else if (begin < it->end) {
            throw ProcessError("Cannot set flow for calibrator '" + getID() + "' with overlapping interval.");
        } else if (begin >= end) {
            throw ProcessError("Cannot set flow for calibrator '" + getID() + "' with negative interval.");
        }
        it++;
    }
    // add interval at the end of the known intervals
    const int intervalIndex = (int)(myCurrentStateInterval - myIntervals.begin());
    AspiredState state;
    state.begin = begin;
    state.end = end;
    state.q = vehsPerHour;
    state.v = speed;
    state.vehicleParameter = new SUMOVehicleParameter(vehicleParameter);
    myIntervals.push_back(state);
    // fix iterator
    myCurrentStateInterval = myIntervals.begin() + intervalIndex;
}

/****************************************************************************/
