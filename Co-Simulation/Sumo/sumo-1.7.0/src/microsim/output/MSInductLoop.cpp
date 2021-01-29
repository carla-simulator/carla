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
/// @file    MSInductLoop.cpp
/// @author  Christian Roessel
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Laura Bieker
/// @date    2004-11-23
///
// An unextended detector measuring at a fixed position on a fixed lane.
/****************************************************************************/
#include <config.h>

#include "MSInductLoop.h"
#include <cassert>
#include <numeric>
#include <utility>
#include <utils/common/WrappingCommand.h>
#include <utils/common/ToString.h>
#include <microsim/MSEventControl.h>
#include <microsim/MSLane.h>
#include <microsim/MSVehicle.h>
#include <microsim/MSNet.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/StringUtils.h>
#include <utils/iodevices/OutputDevice.h>

#define HAS_NOT_LEFT_DETECTOR -1

// ===========================================================================
// method definitions
// ===========================================================================
MSInductLoop::MSInductLoop(const std::string& id, MSLane* const lane,
                           double positionInMeters,
                           const std::string& vTypes,
                           const bool needLocking) :
    MSMoveReminder(id, lane),
    MSDetectorFileOutput(id, vTypes),
    myPosition(positionInMeters),
    myNeedLock(needLocking || MSGlobals::gNumSimThreads > 1),
    myLastLeaveTime(SIMTIME),
    myVehicleDataCont(),
    myVehiclesOnDet() {
    assert(myPosition >= 0 && myPosition <= myLane->getLength());
    reset();
}


MSInductLoop::~MSInductLoop() {
}


void
MSInductLoop::reset() {
#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, myNeedLock);
#endif
    myEnteredVehicleNumber = 0;
    myLastVehicleDataCont = myVehicleDataCont;
    myVehicleDataCont.clear();
}


bool
MSInductLoop::notifyEnter(SUMOTrafficObject& veh, Notification reason, const MSLane* /* enteredLane */) {
    if (!vehicleApplies(veh)) {
        return false;
    }
    if (reason != NOTIFICATION_JUNCTION) { // the junction case is handled in notifyMove
        if (veh.getBackPositionOnLane(myLane) >= myPosition) {
            return false;
        }
        if (veh.getPositionOnLane() >= myPosition) {
#ifdef HAVE_FOX
            FXConditionalLock lock(myNotificationMutex, myNeedLock);
#endif
            myVehiclesOnDet[&veh] = SIMTIME;
            myEnteredVehicleNumber++;
        }
    }
    return true;
}


bool
MSInductLoop::notifyMove(SUMOTrafficObject& veh, double oldPos,
                         double newPos, double newSpeed) {
    if (newPos < myPosition) {
        // detector not reached yet
        return true;
    }
#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, myNeedLock);
#endif
    const double oldSpeed = veh.getPreviousSpeed();
    if (newPos >= myPosition && oldPos < myPosition) {
        // entered the detector by move
        const double timeBeforeEnter = MSCFModel::passingTime(oldPos, myPosition, newPos, oldSpeed, newSpeed);
        myVehiclesOnDet[&veh] = SIMTIME + timeBeforeEnter;
        myEnteredVehicleNumber++;
    }
    double oldBackPos = oldPos - veh.getVehicleType().getLength();
    double newBackPos = newPos - veh.getVehicleType().getLength();
    if (newBackPos > myPosition) {
        // vehicle passed the detector (it may have changed onto this lane somewhere past the detector)
        // assert(!MSGlobals::gSemiImplicitEulerUpdate || newSpeed > 0 || myVehiclesOnDet.find(&veh) == myVehiclesOnDet.end());
        // assertion is invalid in case of teleportation
        if (oldBackPos <= myPosition) {
            const std::map<SUMOTrafficObject*, double>::iterator it = myVehiclesOnDet.find(&veh);
            if (it != myVehiclesOnDet.end()) {
                const double entryTime = it->second;
                const double leaveTime = SIMTIME + MSCFModel::passingTime(oldBackPos, myPosition, newBackPos, oldSpeed, newSpeed);
                myVehiclesOnDet.erase(it);
                assert(entryTime <= leaveTime);
                myVehicleDataCont.push_back(VehicleData(veh, entryTime, leaveTime, false));
                myLastLeaveTime = leaveTime;
            }
        } else {
            // vehicle is already beyond the detector...
            // This can happen even if it is still registered in myVehiclesOnDet, e.g., after teleport.
            myVehiclesOnDet.erase(&veh);
        }
        return false;
    }
    // vehicle stays on the detector
    return true;
}


bool
MSInductLoop::notifyLeave(SUMOTrafficObject& veh, double /* lastPos */, MSMoveReminder::Notification reason, const MSLane* /* enteredLane */) {
    if (reason != MSMoveReminder::NOTIFICATION_JUNCTION) {
#ifdef HAVE_FOX
        FXConditionalLock lock(myNotificationMutex, myNeedLock);
#endif
        const std::map<SUMOTrafficObject*, double>::iterator it = myVehiclesOnDet.find(&veh);
        if (it != myVehiclesOnDet.end()) {
            const double entryTime = it->second;
            const double leaveTime = SIMTIME + TS;
            myVehiclesOnDet.erase(it);
            myVehicleDataCont.push_back(VehicleData(veh, entryTime, leaveTime, true));
            myLastLeaveTime = leaveTime;
        }
        return false;
    }
    return true;
}


double
MSInductLoop::getSpeed(const int offset) const {
    const std::vector<VehicleData>& d = collectVehiclesOnDet(SIMSTEP - offset);
    return d.empty() ? -1. : std::accumulate(d.begin(), d.end(), 0.0, speedSum) / (double) d.size();
}


double
MSInductLoop::getVehicleLength(const int offset) const {
    const std::vector<VehicleData>& d = collectVehiclesOnDet(SIMSTEP - offset);
    return d.empty() ? -1. : std::accumulate(d.begin(), d.end(), 0.0, lengthSum) / (double)d.size();
}


double
MSInductLoop::getOccupancy() const {
    const SUMOTime tbeg = SIMSTEP - DELTA_T;
    double occupancy = 0;
    const double csecond = SIMTIME;
    for (const VehicleData& i : collectVehiclesOnDet(tbeg, false, false, true)) {
        const double leaveTime = i.leaveTimeM == HAS_NOT_LEFT_DETECTOR ? csecond : MIN2(i.leaveTimeM, csecond);
        const double entryTime = MAX2(i.entryTimeM, STEPS2TIME(tbeg));
        occupancy += MIN2(leaveTime - entryTime, TS);
    }
    return occupancy / TS * 100.;
}


double
MSInductLoop::getEnteredNumber(const int offset) const {
    return (double)collectVehiclesOnDet(SIMSTEP - offset, true, true).size();
}


std::vector<std::string>
MSInductLoop::getVehicleIDs(const int offset) const {
    std::vector<std::string> ret;
    for (const VehicleData& i : collectVehiclesOnDet(SIMSTEP - offset, true, true)) {
        ret.push_back(i.idM);
    }
    return ret;
}


double
MSInductLoop::getTimeSinceLastDetection() const {
    if (myVehiclesOnDet.size() != 0) {
        // detector is occupied
        return 0;
    }
    return SIMTIME - myLastLeaveTime;
}


SUMOTime
MSInductLoop::getLastDetectionTime() const {
    if (myVehiclesOnDet.size() != 0) {
        return MSNet::getInstance()->getCurrentTimeStep();
    }
    return TIME2STEPS(myLastLeaveTime);
}


void
MSInductLoop::writeXMLDetectorProlog(OutputDevice& dev) const {
    dev.writeXMLHeader("detector", "det_e1_file.xsd");
}


void
MSInductLoop::writeXMLOutput(OutputDevice& dev, SUMOTime startTime, SUMOTime stopTime) {
    const double t(STEPS2TIME(stopTime - startTime));
    double occupancy = 0.;
    double speedSum = 0.;
    double lengthSum = 0.;
    int contrib = 0;
    // to approximate the space mean speed
    double inverseSpeedSum = 0.;
    for (const VehicleData& vData : myVehicleDataCont) {
        const double timeOnDetDuringInterval = vData.leaveTimeM - MAX2(STEPS2TIME(startTime), vData.entryTimeM);
        occupancy += MIN2(timeOnDetDuringInterval, t);
        if (!vData.leftEarlyM) {
            speedSum += vData.speedM;
            assert(vData.speedM > 0.);
            inverseSpeedSum += 1. / vData.speedM;
            lengthSum += vData.lengthM;
            contrib++;
        }
    }
    const double flow = (double)contrib / t * 3600.;
    for (std::map< SUMOTrafficObject*, double >::const_iterator i = myVehiclesOnDet.begin(); i != myVehiclesOnDet.end(); ++i) {
        occupancy += STEPS2TIME(stopTime) - MAX2(STEPS2TIME(startTime), i->second);
    }
    occupancy *= 100. / t;
    const double meanSpeed = contrib != 0 ? speedSum / (double)contrib : -1;
    const double harmonicMeanSpeed = contrib != 0 ? (double)contrib / inverseSpeedSum : -1;
    const double meanLength = contrib != 0 ? lengthSum / (double)contrib : -1;
    dev.openTag(SUMO_TAG_INTERVAL).writeAttr(SUMO_ATTR_BEGIN, STEPS2TIME(startTime)).writeAttr(SUMO_ATTR_END, STEPS2TIME(stopTime));
    dev.writeAttr(SUMO_ATTR_ID, StringUtils::escapeXML(getID())).writeAttr("nVehContrib", contrib);
    dev.writeAttr("flow", flow).writeAttr("occupancy", occupancy).writeAttr("speed", meanSpeed).writeAttr("harmonicMeanSpeed", harmonicMeanSpeed);
    dev.writeAttr("length", meanLength).writeAttr("nVehEntered", myEnteredVehicleNumber).closeTag();
    reset();
}


std::vector<MSInductLoop::VehicleData>
MSInductLoop::collectVehiclesOnDet(SUMOTime tMS, bool includeEarly, bool leaveTime, bool forOccupancy) const {
#ifdef HAVE_FOX
    FXConditionalLock lock(myNotificationMutex, myNeedLock);
#endif
    const double t = STEPS2TIME(tMS);
    std::vector<VehicleData> ret;
    for (const VehicleData& i : myVehicleDataCont) {
        if (includeEarly || !i.leftEarlyM) {
            if (i.entryTimeM >= t || (leaveTime && i.leaveTimeM >= t)) {
                ret.push_back(i);
            }
        }
    }
    for (const VehicleData& i : myLastVehicleDataCont) {
        if (includeEarly || !i.leftEarlyM) {
            if (i.entryTimeM >= t || (leaveTime && i.leaveTimeM >= t)) {
                ret.push_back(i);
            }
        }
    }
    for (const auto& i : myVehiclesOnDet) {
        if (i.second >= t || leaveTime || forOccupancy) { // no need to check leave time, they are still on the detector
            SUMOTrafficObject* const v = i.first;
            VehicleData d(*v, i.second, HAS_NOT_LEFT_DETECTOR, false);
            d.speedM = v->getSpeed();
            ret.push_back(d);
        }
    }
    return ret;
}


MSInductLoop::VehicleData::VehicleData(const SUMOTrafficObject& v, double entryTimestep,
                                       double leaveTimestep, const bool leftEarly)
    : idM(v.getID()), lengthM(v.getVehicleType().getLength()), entryTimeM(entryTimestep), leaveTimeM(leaveTimestep),
      speedM(v.getVehicleType().getLength() / MAX2(leaveTimestep - entryTimestep, NUMERICAL_EPS)), typeIDM(v.getVehicleType().getID()),
      leftEarlyM(leftEarly) {}


void
MSInductLoop::clearState() {
    myEnteredVehicleNumber = 0;
    myLastVehicleDataCont.clear();
    myVehicleDataCont.clear();
    myVehiclesOnDet.clear();
}

/****************************************************************************/
