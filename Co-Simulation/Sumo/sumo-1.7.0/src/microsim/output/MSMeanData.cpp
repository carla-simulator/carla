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
/// @file    MSMeanData.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @author  Leonhard Luecken
/// @date    Mon, 10.05.2004
///
// Data collector for edges/lanes
/****************************************************************************/
#include <config.h>

#include <limits>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/common/StringTokenizer.h>
#include <utils/iodevices/OutputDevice.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/cfmodels/MSCFModel.h>
#include <microsim/MSNet.h>
#include "MSMeanData_Amitran.h"
#include "MSMeanData.h"

#include <microsim/MSGlobals.h>
#include <mesosim/MESegment.h>
#include <mesosim/MELoop.h>


// ===========================================================================
// debug constants
// ===========================================================================
//#define DEBUG_NOTIFY_MOVE
//#define DEBUG_NOTIFY_ENTER

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// MSMeanData::MeanDataValues - methods
// ---------------------------------------------------------------------------
MSMeanData::MeanDataValues::MeanDataValues(
    MSLane* const lane, const double length, const bool doAdd,
    const MSMeanData* const parent) :
    MSMoveReminder("meandata_" + (lane == nullptr ? "NULL" :  lane->getID()), lane, doAdd),
    myParent(parent),
    myLaneLength(length),
    sampleSeconds(0),
    travelledDistance(0) {}


MSMeanData::MeanDataValues::~MeanDataValues() {
}


bool
MSMeanData::MeanDataValues::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
#ifdef DEBUG_NOTIFY_ENTER
    std::cout << "\n" << SIMTIME << " MSMeanData_Net::MSLaneMeanDataValues: veh '" << veh.getID() << "' enters lane '" << enteredLane->getID() << "'" << std::endl;
#else
    UNUSED_PARAMETER(enteredLane);
#endif
    UNUSED_PARAMETER(reason);
    return myParent == nullptr || myParent->vehicleApplies(veh);
}


bool
MSMeanData::MeanDataValues::notifyMove(SUMOTrafficObject& veh, double oldPos, double newPos, double newSpeed) {
    // if the vehicle has arrived, the reminder must be kept so it can be
    // notified of the arrival subsequently
    const double oldSpeed = veh.getPreviousSpeed();
    double enterSpeed = MSGlobals::gSemiImplicitEulerUpdate ? newSpeed : oldSpeed; // NOTE: For the euler update, the vehicle is assumed to travel at constant speed for the whole time step
    double leaveSpeed = newSpeed, leaveSpeedFront = newSpeed;

    // These values will be further decreased below
    double timeOnLane = TS;
    double frontOnLane = oldPos > myLaneLength ? 0. : TS;
    bool ret = true;

    // entry and exit times (will be modified below)
    double timeBeforeEnter = 0.;
    double timeBeforeEnterBack = 0.;
    double timeBeforeLeaveFront = newPos < myLaneLength ? TS : 0.;
    double timeBeforeLeave = TS;

    // Treat the case that the vehicle entered the lane in the last step
    if (oldPos < 0 && newPos >= 0) {
        // Vehicle was not on this lane in the last time step
        timeBeforeEnter = MSCFModel::passingTime(oldPos, 0, newPos, oldSpeed, newSpeed);
        timeOnLane = TS - timeBeforeEnter;
        frontOnLane = timeOnLane;
        enterSpeed = MSCFModel::speedAfterTime(timeBeforeEnter, oldSpeed, newPos - oldPos);
    }

    const double oldBackPos = oldPos - veh.getVehicleType().getLength();
    const double newBackPos = newPos - veh.getVehicleType().getLength();

    // Determine the time before the vehicle back enters
    if (oldBackPos < 0. && newBackPos > 0.) {
        timeBeforeEnterBack = MSCFModel::passingTime(oldBackPos, 0., newBackPos, oldSpeed, newSpeed);
    } else if (newBackPos <= 0) {
        timeBeforeEnterBack = TS;
    } else {
        timeBeforeEnterBack = 0.;
    }

    // Treat the case that the vehicle's back left the lane in the last step
    if (newBackPos > myLaneLength // vehicle's back has left the lane
            && oldBackPos <= myLaneLength) { // and hasn't left the lane before
        assert(!MSGlobals::gSemiImplicitEulerUpdate || newSpeed != 0); // how could it move across the lane boundary otherwise
        // (Leo) vehicle left this lane (it can also have skipped over it in one time step -> therefore we use "timeOnLane -= ..." and ( ... - timeOnLane) below)
        timeBeforeLeave = MSCFModel::passingTime(oldBackPos, myLaneLength, newBackPos, oldSpeed, newSpeed);
        const double timeAfterLeave = TS - timeBeforeLeave;
        timeOnLane -= timeAfterLeave;
        leaveSpeed = MSCFModel::speedAfterTime(timeBeforeLeave, oldSpeed, newPos - oldPos);
        // XXX: Do we really need this? Why would this "reduce rounding errors"? (Leo) Refs. #2579
        if (fabs(timeOnLane) < NUMERICAL_EPS) { // reduce rounding errors
            timeOnLane = 0.;
        }
        ret = veh.hasArrived();
    }

    // Treat the case that the vehicle's front left the lane in the last step
    if (newPos > myLaneLength && oldPos <= myLaneLength) {
        // vehicle's front has left the lane and has not left before
        assert(!MSGlobals::gSemiImplicitEulerUpdate || newSpeed != 0);
        timeBeforeLeaveFront = MSCFModel::passingTime(oldPos, myLaneLength, newPos, oldSpeed, newSpeed);
        const double timeAfterLeave = TS - timeBeforeLeaveFront;
        frontOnLane -= timeAfterLeave;
        // XXX: Do we really need this? Why would this "reduce rounding errors"? (Leo) Refs. #2579
        if (fabs(frontOnLane) < NUMERICAL_EPS) { // reduce rounding errors
            frontOnLane = 0.;
        }
        leaveSpeedFront = MSCFModel::speedAfterTime(timeBeforeLeaveFront, oldSpeed, newPos - oldPos);
    }

    assert(frontOnLane <= TS);
    assert(timeOnLane <= TS);

    if (timeOnLane < 0) {
        WRITE_ERROR("Negative vehicle step fraction for '" + veh.getID() + "' on lane '" + getLane()->getID() + "'.");
        return veh.hasArrived();
    }
    if (timeOnLane == 0) {
        return veh.hasArrived();
    }

#ifdef DEBUG_NOTIFY_MOVE
    std::stringstream ss;
    ss << "\n"
       << "lane length: " << myLaneLength
       << "\noldPos: " << oldPos
       << "\nnewPos: " << newPos
       << "\noldPosBack: " << oldBackPos
       << "\nnewPosBack: " << newBackPos
       << "\ntimeBeforeEnter: " << timeBeforeEnter
       << "\ntimeBeforeEnterBack: " << timeBeforeEnterBack
       << "\ntimeBeforeLeaveFront: " << timeBeforeLeaveFront
       << "\ntimeBeforeLeave: " << timeBeforeLeave;
    if (!(timeBeforeLeave >= MAX2(timeBeforeEnterBack, timeBeforeLeaveFront))
            || !(timeBeforeEnter <= MIN2(timeBeforeEnterBack, timeBeforeLeaveFront))) {
        WRITE_ERROR(ss.str());
    } else {
        std::cout << ss.str() << std::endl;
    }

#endif

    assert(timeBeforeEnter <= MIN2(timeBeforeEnterBack, timeBeforeLeaveFront));
    assert(timeBeforeLeave >= MAX2(timeBeforeEnterBack, timeBeforeLeaveFront));
    // compute average vehicle length on lane in last step
    double vehLength = veh.getVehicleType().getLength();
    // occupied lane length at timeBeforeEnter (resp. stepStart if already on lane)
    double lengthOnLaneAtStepStart = MAX2(0., MIN4(myLaneLength, vehLength, vehLength - (oldPos - myLaneLength), oldPos));
    // occupied lane length at timeBeforeLeave (resp. stepEnd if still on lane)
    double lengthOnLaneAtStepEnd = MAX2(0., MIN4(myLaneLength, vehLength, vehLength - (newPos - myLaneLength), newPos));
    double integratedLengthOnLane = 0.;
    if (timeBeforeEnterBack < timeBeforeLeaveFront) {
        // => timeBeforeLeaveFront>0, myLaneLength>vehLength
        // vehicle length on detector at timeBeforeEnterBack
        double lengthOnLaneAtBackEnter = MIN2(veh.getVehicleType().getLength(), newPos);
        // linear quadrature of occupancy between timeBeforeEnter and timeBeforeEnterBack
        integratedLengthOnLane += (timeBeforeEnterBack - timeBeforeEnter) * (lengthOnLaneAtBackEnter + lengthOnLaneAtStepStart) * 0.5;
        // linear quadrature of occupancy between timeBeforeEnterBack and timeBeforeLeaveFront
        // (vehicle is completely on the edge in between)
        integratedLengthOnLane += (timeBeforeLeaveFront - timeBeforeEnterBack) * vehLength;
        // and until vehicle leaves/stepEnd
        integratedLengthOnLane += (timeBeforeLeave - timeBeforeLeaveFront) * (vehLength + lengthOnLaneAtStepEnd) * 0.5;
    } else if (timeBeforeEnterBack >= timeBeforeLeaveFront) {
        // => myLaneLength <= vehLength or (timeBeforeLeaveFront == timeBeforeEnterBack == 0)
        // vehicle length on detector at timeBeforeLeaveFront
        double lengthOnLaneAtLeaveFront;
        if (timeBeforeLeaveFront == timeBeforeEnter) {
            // for the case that front already left
            lengthOnLaneAtLeaveFront = lengthOnLaneAtStepStart;
        } else if (timeBeforeLeaveFront == timeBeforeLeave) {
            // for the case that front doesn't leave in this step
            lengthOnLaneAtLeaveFront = lengthOnLaneAtStepEnd;
        } else {
            lengthOnLaneAtLeaveFront = myLaneLength;
        }
#ifdef DEBUG_NOTIFY_MOVE
        std::cout << "lengthOnLaneAtLeaveFront=" << lengthOnLaneAtLeaveFront << std::endl;
#endif
        // linear quadrature of occupancy between timeBeforeEnter and timeBeforeLeaveFront
        integratedLengthOnLane += (timeBeforeLeaveFront - timeBeforeEnter) * (lengthOnLaneAtLeaveFront + lengthOnLaneAtStepStart) * 0.5;
        // linear quadrature of occupancy between timeBeforeLeaveFront and timeBeforeEnterBack
        integratedLengthOnLane += (timeBeforeEnterBack - timeBeforeLeaveFront) * lengthOnLaneAtLeaveFront;
        // and until vehicle leaves/stepEnd
        integratedLengthOnLane += (timeBeforeLeave - timeBeforeEnterBack) * (lengthOnLaneAtLeaveFront + lengthOnLaneAtStepEnd) * 0.5;
    }

    double meanLengthOnLane = integratedLengthOnLane / TS;
#ifdef DEBUG_NOTIFY_MOVE
    std::cout << "Calculated mean length on lane '" << myLane->getID() << "' in last step as " << meanLengthOnLane
              << "\nlengthOnLaneAtStepStart=" << lengthOnLaneAtStepStart << ", lengthOnLaneAtStepEnd=" << lengthOnLaneAtStepEnd << ", integratedLengthOnLane=" << integratedLengthOnLane
              << std::endl;
#endif

//    // XXX: use this, when #2556 is fixed! Refs. #2575
//    const double travelledDistanceFrontOnLane = MAX2(0., MIN2(newPos, myLaneLength) - MAX2(oldPos, 0.));
//    const double travelledDistanceVehicleOnLane = MIN2(newPos, myLaneLength) - MAX2(oldPos, 0.) + MIN2(MAX2(0., newPos - myLaneLength), veh.getVehicleType().getLength());
//    // XXX: #2556 fixed for ballistic update
    const double travelledDistanceFrontOnLane = MSGlobals::gSemiImplicitEulerUpdate ? frontOnLane * newSpeed
            : MAX2(0., MIN2(newPos, myLaneLength) - MAX2(oldPos, 0.));
    const double travelledDistanceVehicleOnLane = MSGlobals::gSemiImplicitEulerUpdate ? timeOnLane * newSpeed
            : MIN2(newPos, myLaneLength) - MAX2(oldPos, 0.) + MIN2(MAX2(0., newPos - myLaneLength), veh.getVehicleType().getLength());
//    // XXX: no fix
//    const double travelledDistanceFrontOnLane = frontOnLane*newSpeed;
//    const double travelledDistanceVehicleOnLane = timeOnLane*newSpeed;

#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, MSGlobals::gNumSimThreads > 1);
#endif
    notifyMoveInternal(veh, frontOnLane, timeOnLane, (enterSpeed + leaveSpeedFront) / 2., (enterSpeed + leaveSpeed) / 2., travelledDistanceFrontOnLane, travelledDistanceVehicleOnLane, meanLengthOnLane);
    return ret;
}


bool
MSMeanData::MeanDataValues::notifyLeave(SUMOTrafficObject& /*veh*/, double /*lastPos*/, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (MSGlobals::gUseMesoSim) {
        return false; // reminder is re-added on every segment (@recheck for performance)
    }
    return reason == MSMoveReminder::NOTIFICATION_JUNCTION;
}


bool
MSMeanData::MeanDataValues::isEmpty() const {
    return sampleSeconds == 0;
}


void
MSMeanData::MeanDataValues::update() {
}


double
MSMeanData::MeanDataValues::getSamples() const {
    return sampleSeconds;
}


// ---------------------------------------------------------------------------
// MSMeanData::MeanDataValueTracker - methods
// ---------------------------------------------------------------------------
MSMeanData::MeanDataValueTracker::MeanDataValueTracker(MSLane* const lane,
        const double length,
        const MSMeanData* const parent)
    : MSMeanData::MeanDataValues(lane, length, true, parent) {
    myCurrentData.push_back(new TrackerEntry(parent->createValues(lane, length, false)));
}


MSMeanData::MeanDataValueTracker::~MeanDataValueTracker() {
    std::list<TrackerEntry*>::iterator i;
    for (i = myCurrentData.begin(); i != myCurrentData.end(); i++) {
        delete *i;
    }

    // FIXME: myTrackedData may still hold some undeleted TrackerEntries. When to delete those? (Leo), refers to #2251
    // code below fails

//	std::map<SUMOTrafficObject*, TrackerEntry*>::iterator j;
//	for(j=myTrackedData.begin(); j!=myTrackedData.end();j++){
//		delete j->second;
//	}
}


void
MSMeanData::MeanDataValueTracker::reset(bool afterWrite) {
    if (afterWrite) {
        if (myCurrentData.begin() != myCurrentData.end()) {
            myCurrentData.pop_front();
        }
    } else {
        myCurrentData.push_back(new TrackerEntry(myParent->createValues(myLane, myLaneLength, false)));
    }
}


void
MSMeanData::MeanDataValueTracker::addTo(MSMeanData::MeanDataValues& val) const {
    myCurrentData.front()->myValues->addTo(val);
}


void
MSMeanData::MeanDataValueTracker::notifyMoveInternal(const SUMOTrafficObject& veh, const double frontOnLane, const double timeOnLane, const double meanSpeedFrontOnLane, const double meanSpeedVehicleOnLane, const double travelledDistanceFrontOnLane, const double travelledDistanceVehicleOnLane, const double meanLengthOnLane) {
    myTrackedData[&veh]->myValues->notifyMoveInternal(veh, frontOnLane, timeOnLane, meanSpeedFrontOnLane, meanSpeedVehicleOnLane, travelledDistanceFrontOnLane, travelledDistanceVehicleOnLane, meanLengthOnLane);
}


bool
MSMeanData::MeanDataValueTracker::notifyLeave(SUMOTrafficObject& veh, double lastPos, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (myParent == nullptr || reason != MSMoveReminder::NOTIFICATION_SEGMENT) {
        myTrackedData[&veh]->myNumVehicleLeft++;
    }
    return myTrackedData[&veh]->myValues->notifyLeave(veh, lastPos, reason);
}


bool
MSMeanData::MeanDataValueTracker::notifyEnter(SUMOTrafficObject& veh, MSMoveReminder::Notification reason, const MSLane* enteredLane) {
#ifdef DEBUG_NOTIFY_ENTER
    std::cout << "\n" << SIMTIME << " MSMeanData::MeanDataValueTracker: veh '" << veh.getID() << "' enters lane '" << enteredLane->getID() << "'" << std::endl;
#else
    UNUSED_PARAMETER(enteredLane);
#endif
    if (reason == MSMoveReminder::NOTIFICATION_SEGMENT) {
        return true;
    }
    if (myParent->vehicleApplies(veh) && myTrackedData.find(&veh) == myTrackedData.end()) {
        myTrackedData[&veh] = myCurrentData.back();
        myTrackedData[&veh]->myNumVehicleEntered++;
        if (!myTrackedData[&veh]->myValues->notifyEnter(veh, reason)) {
            myTrackedData[&veh]->myNumVehicleLeft++;
            myTrackedData.erase(&veh);
            return false;
        }
        return true;
    }
    return false;
}


bool
MSMeanData::MeanDataValueTracker::isEmpty() const {
    return myCurrentData.front()->myValues->isEmpty();
}


void
MSMeanData::MeanDataValueTracker::write(OutputDevice& dev,
                                        long long int attributeMask,
                                        const SUMOTime period,
                                        const double numLanes,
                                        const double defaultTravelTime,
                                        const int /*numVehicles*/) const {
    myCurrentData.front()->myValues->write(dev, attributeMask, period, numLanes,
                                           defaultTravelTime,
                                           myCurrentData.front()->myNumVehicleEntered);
}


int
MSMeanData::MeanDataValueTracker::getNumReady() const {
    int result = 0;
    for (std::list<TrackerEntry*>::const_iterator it = myCurrentData.begin(); it != myCurrentData.end(); ++it) {
        if ((*it)->myNumVehicleEntered == (*it)->myNumVehicleLeft) {
            result++;
        } else {
            break;
        }
    }
    return result;
}


double
MSMeanData::MeanDataValueTracker::getSamples() const {
    return myCurrentData.front()->myValues->getSamples();
}


// ---------------------------------------------------------------------------
// MSMeanData - methods
// ---------------------------------------------------------------------------
MSMeanData::MSMeanData(const std::string& id,
                       const SUMOTime dumpBegin, const SUMOTime dumpEnd,
                       const bool useLanes, const bool withEmpty,
                       const bool printDefaults, const bool withInternal,
                       const bool trackVehicles,
                       const int detectPersons,
                       const double maxTravelTime,
                       const double minSamples,
                       const std::string& vTypes,
                       const std::string& writeAttributes) :
    MSDetectorFileOutput(id, vTypes, detectPersons),
    myMinSamples(minSamples),
    myMaxTravelTime(maxTravelTime),
    myDumpEmpty(withEmpty),
    myAmEdgeBased(!useLanes),
    myDumpBegin(dumpBegin),
    myDumpEnd(dumpEnd),
    myPrintDefaults(printDefaults),
    myDumpInternal(withInternal),
    myTrackVehicles(trackVehicles),
    myWrittenAttributes(initWrittenAttributes(writeAttributes, id))
{ }


void
MSMeanData::init() {
    const MSEdgeVector& edges = MSNet::getInstance()->getEdgeControl().getEdges();
    for (MSEdgeVector::const_iterator e = edges.begin(); e != edges.end(); ++e) {
        if ((myDumpInternal || !(*e)->isInternal()) &&
                ((detectPersons() && myDumpInternal) || (!(*e)->isCrossing() && !(*e)->isWalkingArea()))) {
            myEdges.push_back(*e);
            myMeasures.push_back(std::vector<MeanDataValues*>());
            const std::vector<MSLane*>& lanes = (*e)->getLanes();
            if (MSGlobals::gUseMesoSim) {
                MeanDataValues* data;
                if (myTrackVehicles) {
                    data = new MeanDataValueTracker(nullptr, lanes[0]->getLength(), this);
                } else {
                    data = createValues(nullptr, lanes[0]->getLength(), false);
                }
                data->setDescription("meandata_" + (*e)->getID());
                myMeasures.back().push_back(data);
                MESegment* s = MSGlobals::gMesoNet->getSegmentForEdge(**e);
                while (s != nullptr) {
                    s->addDetector(data);
                    s->prepareDetectorForWriting(*data);
                    s = s->getNextSegment();
                }
                data->reset();
                data->reset(true);
                continue;
            }
            if (myAmEdgeBased && myTrackVehicles) {
                myMeasures.back().push_back(new MeanDataValueTracker(nullptr, lanes[0]->getLength(), this));
            }
            for (std::vector<MSLane*>::const_iterator lane = lanes.begin(); lane != lanes.end(); ++lane) {
                if (myTrackVehicles) {
                    if (myAmEdgeBased) {
                        (*lane)->addMoveReminder(myMeasures.back().back());
                    } else {
                        myMeasures.back().push_back(new MeanDataValueTracker(*lane, (*lane)->getLength(), this));
                    }
                } else {
                    myMeasures.back().push_back(createValues(*lane, (*lane)->getLength(), true));
                }
            }
        }
    }
}


MSMeanData::~MSMeanData() {
    for (std::vector<std::vector<MeanDataValues*> >::const_iterator i = myMeasures.begin(); i != myMeasures.end(); ++i) {
        for (std::vector<MeanDataValues*>::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
            delete *j;
        }
    }
}


void
MSMeanData::resetOnly(SUMOTime stopTime) {
    UNUSED_PARAMETER(stopTime);
    if (MSGlobals::gUseMesoSim) {
        MSEdgeVector::iterator edge = myEdges.begin();
        for (std::vector<std::vector<MeanDataValues*> >::const_iterator i = myMeasures.begin(); i != myMeasures.end(); ++i, ++edge) {
            MESegment* s = MSGlobals::gMesoNet->getSegmentForEdge(**edge);
            MeanDataValues* data = i->front();
            while (s != nullptr) {
                s->prepareDetectorForWriting(*data);
                s = s->getNextSegment();
            }
            data->reset();
        }
        return;
    }
    for (std::vector<std::vector<MeanDataValues*> >::const_iterator i = myMeasures.begin(); i != myMeasures.end(); ++i) {
        for (std::vector<MeanDataValues*>::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
            (*j)->reset();
        }
    }
}


std::string
MSMeanData::getEdgeID(const MSEdge* const edge) {
    return edge->getID();
}


void
MSMeanData::writeEdge(OutputDevice& dev,
                      const std::vector<MeanDataValues*>& edgeValues,
                      MSEdge* edge, SUMOTime startTime, SUMOTime stopTime) {
    if (MSGlobals::gUseMesoSim) {
        MESegment* s = MSGlobals::gMesoNet->getSegmentForEdge(*edge);
        MeanDataValues* data = edgeValues.front();
        while (s != nullptr) {
            s->prepareDetectorForWriting(*data);
            s = s->getNextSegment();
        }
        if (writePrefix(dev, *data, SUMO_TAG_EDGE, getEdgeID(edge))) {
            data->write(dev, myWrittenAttributes, stopTime - startTime,
                        (double)edge->getLanes().size(),
                        myPrintDefaults ? edge->getLength() / edge->getSpeedLimit() : -1.);
        }
        data->reset(true);
        return;
    }
    std::vector<MeanDataValues*>::const_iterator lane;
    if (!myAmEdgeBased) {
        bool writeCheck = myDumpEmpty;
        if (!writeCheck) {
            for (lane = edgeValues.begin(); lane != edgeValues.end(); ++lane) {
                if (!(*lane)->isEmpty()) {
                    writeCheck = true;
                    break;
                }
            }
        }
        if (writeCheck) {
            dev.openTag(SUMO_TAG_EDGE).writeAttr(SUMO_ATTR_ID, edge->getID());
        }
        for (lane = edgeValues.begin(); lane != edgeValues.end(); ++lane) {
            MeanDataValues& meanData = **lane;
            if (writePrefix(dev, meanData, SUMO_TAG_LANE, meanData.getLane()->getID())) {
                meanData.write(dev, myWrittenAttributes, stopTime - startTime, 1.f, myPrintDefaults ? meanData.getLane()->getLength() / meanData.getLane()->getSpeedLimit() : -1.);
            }
            meanData.reset(true);
        }
        if (writeCheck) {
            dev.closeTag();
        }
    } else {
        if (myTrackVehicles) {
            MeanDataValues& meanData = **edgeValues.begin();
            if (writePrefix(dev, meanData, SUMO_TAG_EDGE, edge->getID())) {
                meanData.write(dev, myWrittenAttributes, stopTime - startTime, (double)edge->getLanes().size(), myPrintDefaults ? edge->getLength() / edge->getSpeedLimit() : -1.);
            }
            meanData.reset(true);
        } else {
            MeanDataValues* sumData = createValues(nullptr, edge->getLength(), false);
            for (lane = edgeValues.begin(); lane != edgeValues.end(); ++lane) {
                MeanDataValues& meanData = **lane;
                meanData.addTo(*sumData);
                meanData.reset();
            }
            if (writePrefix(dev, *sumData, SUMO_TAG_EDGE, getEdgeID(edge))) {
                sumData->write(dev, myWrittenAttributes, stopTime - startTime, (double)edge->getLanes().size(), myPrintDefaults ? edge->getLength() / edge->getSpeedLimit() : -1.);
            }
            delete sumData;
        }
    }
}


void
MSMeanData::openInterval(OutputDevice& dev, const SUMOTime startTime, const SUMOTime stopTime) {
    dev.openTag(SUMO_TAG_INTERVAL).writeAttr(SUMO_ATTR_BEGIN, time2string(startTime)).writeAttr(SUMO_ATTR_END, time2string(stopTime));
    dev.writeAttr(SUMO_ATTR_ID, myID);
}


bool
MSMeanData::writePrefix(OutputDevice& dev, const MeanDataValues& values, const SumoXMLTag tag, const std::string id) const {
    if (myDumpEmpty || !values.isEmpty()) {
        dev.openTag(tag);
        dev.writeAttr(SUMO_ATTR_ID, id);
        MeanDataValues::checkWriteAttribute(dev, myWrittenAttributes, SUMO_ATTR_SAMPLEDSECONDS, values.getSamples());
        return true;
    }
    return false;
}


void
MSMeanData::writeXMLOutput(OutputDevice& dev,
                           SUMOTime startTime, SUMOTime stopTime) {
    // check whether this dump shall be written for the current time
    int numReady = myDumpBegin < stopTime && myDumpEnd - DELTA_T >= startTime ? 1 : 0;
    if (myTrackVehicles && myDumpBegin < stopTime) {
        myPendingIntervals.push_back(std::make_pair(startTime, stopTime));
        numReady = (int)myPendingIntervals.size();
        for (std::vector<std::vector<MeanDataValues*> >::const_iterator i = myMeasures.begin(); i != myMeasures.end(); ++i) {
            for (std::vector<MeanDataValues*>::const_iterator j = (*i).begin(); j != (*i).end(); ++j) {
                numReady = MIN2(numReady, ((MeanDataValueTracker*)*j)->getNumReady());
                if (numReady == 0) {
                    break;
                }
            }
            if (numReady == 0) {
                break;
            }
        }
    }
    if (numReady == 0 || myTrackVehicles) {
        resetOnly(stopTime);
    }
    while (numReady-- > 0) {
        if (!myPendingIntervals.empty()) {
            startTime = myPendingIntervals.front().first;
            stopTime = myPendingIntervals.front().second;
            myPendingIntervals.pop_front();
        }
        openInterval(dev, startTime, stopTime);
        MSEdgeVector::iterator edge = myEdges.begin();
        for (std::vector<std::vector<MeanDataValues*> >::const_iterator i = myMeasures.begin(); i != myMeasures.end(); ++i, ++edge) {
            writeEdge(dev, (*i), *edge, startTime, stopTime);
        }
        dev.closeTag();
    }
    dev.flush();
}


void
MSMeanData::writeXMLDetectorProlog(OutputDevice& dev) const {
    dev.writeXMLHeader("meandata", "meandata_file.xsd");
}


void
MSMeanData::detectorUpdate(const SUMOTime step) {
    if (step + DELTA_T == myDumpBegin) {
        init();
    }
}

long long int
MSMeanData::initWrittenAttributes(const std::string writeAttributes, const std::string& id) {
    long long int result = 0;
    for (std::string attrName : StringTokenizer(writeAttributes).getVector()) {
        if (!SUMOXMLDefinitions::Attrs.hasString(attrName)) {
            WRITE_ERROR("Unknown attribute '" + attrName + "' to write in meanData '" + id + "'.");
            continue;
        }
        int attr = SUMOXMLDefinitions::Attrs.get(attrName);
        assert(attr < 63);
        result |= ((long long int)1 << attr);
    }
    return result;
}

/****************************************************************************/
