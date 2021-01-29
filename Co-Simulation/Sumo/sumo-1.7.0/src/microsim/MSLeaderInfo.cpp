/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    MSLeaderInfo.cpp
/// @author  Jakob Erdmann
/// @date    Oct 2015
///
// Information about vehicles ahead (may be multiple vehicles if
// lateral-resolution is active)
/****************************************************************************/
#include <config.h>

#include <cassert>
#include <cmath>
#include <utils/common/ToString.h>
#include <microsim/MSGlobals.h>
#include <microsim/MSVehicle.h>
#include <microsim/lcmodels/MSAbstractLaneChangeModel.h>
#include <microsim/MSNet.h>
#include "MSLeaderInfo.h"


// ===========================================================================
// static member variables
// ===========================================================================


// ===========================================================================
// MSLeaderInfo member method definitions
// ===========================================================================
MSLeaderInfo::MSLeaderInfo(const MSLane* lane, const MSVehicle* ego, double latOffset) :
    myWidth(lane->getWidth()),
    myVehicles(MAX2(1, int(ceil(myWidth / MSGlobals::gLateralResolution))), (MSVehicle*)nullptr),
    myFreeSublanes((int)myVehicles.size()),
    egoRightMost(-1),
    egoLeftMost(-1),
    myHasVehicles(false) {
    if (ego != nullptr) {
        getSubLanes(ego, latOffset, egoRightMost, egoLeftMost);
        // filter out sublanes not of interest to ego
        myFreeSublanes -= egoRightMost;
        myFreeSublanes -= (int)myVehicles.size() - 1 - egoLeftMost;
    }
}


MSLeaderInfo::~MSLeaderInfo() { }


int
MSLeaderInfo::addLeader(const MSVehicle* veh, bool beyond, double latOffset) {
    if (veh == nullptr) {
        return myFreeSublanes;
    }
    if (myVehicles.size() == 1) {
        // speedup for the simple case
        if (!beyond || myVehicles[0] == 0) {
            myVehicles[0] = veh;
            myFreeSublanes = 0;
            myHasVehicles = true;
        }
        return myFreeSublanes;
    }
    // map center-line based coordinates into [0, myWidth] coordinates
    int rightmost, leftmost;
    getSubLanes(veh, latOffset, rightmost, leftmost);
    //if (gDebugFlag1) std::cout << " addLeader veh=" << veh->getID() << " beyond=" << beyond << " latOffset=" << latOffset << " rightmost=" << rightmost << " leftmost=" << leftmost << " myFreeSublanes=" << myFreeSublanes << "\n";
    for (int sublane = rightmost; sublane <= leftmost; ++sublane) {
        if ((egoRightMost < 0 || (egoRightMost <= sublane && sublane <= egoLeftMost))
                && (!beyond || myVehicles[sublane] == 0)) {
            if (myVehicles[sublane] == 0) {
                myFreeSublanes--;
            }
            myVehicles[sublane] = veh;
            myHasVehicles = true;
        }
    }
    return myFreeSublanes;
}


void
MSLeaderInfo::clear() {
    myVehicles.assign(myVehicles.size(), (MSVehicle*)nullptr);
    myFreeSublanes = (int)myVehicles.size();
    if (egoRightMost >= 0) {
        myFreeSublanes -= egoRightMost;
        myFreeSublanes -= (int)myVehicles.size() - 1 - egoLeftMost;
    }
}


void
MSLeaderInfo::getSubLanes(const MSVehicle* veh, double latOffset, int& rightmost, int& leftmost) const {
    if (myVehicles.size() == 1) {
        // speedup for the simple case
        rightmost = 0;
        leftmost = 0;
        return;
    }
    // map center-line based coordinates into [0, myWidth] coordinates
    const double vehCenter = veh->getLateralPositionOnLane() + 0.5 * myWidth + latOffset;
    const double vehHalfWidth = 0.5 * veh->getVehicleType().getWidth();
    double rightVehSide = MAX2(0.,  vehCenter - vehHalfWidth);
    double leftVehSide = MIN2(myWidth, vehCenter + vehHalfWidth);
    // Reserve some additional space if the vehicle is performing a maneuver continuation.
    if (veh->getActionStepLength() != DELTA_T) {
        if (veh->getLaneChangeModel().getManeuverDist() < 0. || veh->getLaneChangeModel().getSpeedLat() < 0.) {
            const double maneuverDist = MIN2(veh->getVehicleType().getMaxSpeedLat() * veh->getActionStepLengthSecs(), -MIN2(0., veh->getLaneChangeModel().getManeuverDist()));
            rightVehSide -= maneuverDist;
        }
        if (veh->getLaneChangeModel().getManeuverDist() > 0. || veh->getLaneChangeModel().getSpeedLat() > 0.) {
            const double maneuverDist = MIN2(veh->getVehicleType().getMaxSpeedLat() * veh->getActionStepLengthSecs(), MAX2(0., veh->getLaneChangeModel().getManeuverDist()));
            leftVehSide += maneuverDist;
        }
    }

    rightmost = MAX2(0, (int)floor((rightVehSide + NUMERICAL_EPS) / MSGlobals::gLateralResolution));
    leftmost = MIN2((int)myVehicles.size() - 1, (int)floor((leftVehSide - NUMERICAL_EPS) / MSGlobals::gLateralResolution));
    //if (veh->getID() == "Pepoli_11_41") std::cout << SIMTIME << " veh=" << veh->getID()
    //    << std::setprecision(10)
    //    << " posLat=" << veh->getLateralPositionOnLane()
    //    << " latOffset=" << latOffset
    //    << " rightVehSide=" << rightVehSide
    //    << " leftVehSide=" << leftVehSide
    //    << " rightmost=" << rightmost
    //    << " leftmost=" << leftmost
    //    << std::setprecision(2)
    //    << "\n";
}


void
MSLeaderInfo::getSublaneBorders(int sublane, double latOffset, double& rightSide, double& leftSide) const {
    assert(sublane >= 0);
    assert(sublane < (int)myVehicles.size());
    const double res = MSGlobals::gLateralResolution > 0 ? MSGlobals::gLateralResolution : myWidth;
    rightSide = sublane * res + latOffset;
    leftSide = MIN2((sublane + 1) * res, myWidth) + latOffset;
}


const MSVehicle*
MSLeaderInfo::operator[](int sublane) const {
    assert(sublane >= 0);
    assert(sublane < (int)myVehicles.size());
    return myVehicles[sublane];
}


std::string
MSLeaderInfo::toString() const {
    std::ostringstream oss;
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss << std::setprecision(2);
    for (int i = 0; i < (int)myVehicles.size(); ++i) {
        oss << Named::getIDSecure(myVehicles[i]);
        if (i < (int)myVehicles.size() - 1) {
            oss << ", ";
        }
    }
    oss << " free=" << myFreeSublanes;
    return oss.str();
}


bool
MSLeaderInfo::hasStoppedVehicle() const {
    if (!myHasVehicles) {
        return false;
    }
    for (int i = 0; i < (int)myVehicles.size(); ++i) {
        if (myVehicles[0] != 0 && myVehicles[0]->isStopped()) {
            return true;
        }
    }
    return false;
}

// ===========================================================================
// MSLeaderDistanceInfo member method definitions
// ===========================================================================


MSLeaderDistanceInfo::MSLeaderDistanceInfo(const MSLane* lane, const MSVehicle* ego, double latOffset) :
    MSLeaderInfo(lane, ego, latOffset),
    myDistances(myVehicles.size(), std::numeric_limits<double>::max()) {
}


MSLeaderDistanceInfo::MSLeaderDistanceInfo(const CLeaderDist& cLeaderDist, const MSLane* dummy) :
    MSLeaderInfo(dummy, nullptr, 0),
    myDistances(1, cLeaderDist.second) {
    assert(myVehicles.size() == 1);
    myVehicles[0] = cLeaderDist.first;
    myHasVehicles = cLeaderDist.first != nullptr;
}

MSLeaderDistanceInfo::~MSLeaderDistanceInfo() { }


int
MSLeaderDistanceInfo::addLeader(const MSVehicle* veh, double gap, double latOffset, int sublane) {
    //if (SIMTIME == 31 && gDebugFlag1 && veh != 0 && veh->getID() == "cars.8") {
    //    std::cout << " BREAKPOINT\n";
    //}
    if (veh == nullptr) {
        return myFreeSublanes;
    }
    if (myVehicles.size() == 1) {
        // speedup for the simple case
        sublane = 0;
    }
    if (sublane >= 0 && sublane < (int)myVehicles.size()) {
        // sublane is already given
        if (gap < myDistances[sublane]) {
            if (myVehicles[sublane] == 0) {
                myFreeSublanes--;
            }
            myVehicles[sublane] = veh;
            myDistances[sublane] = gap;
            myHasVehicles = true;
        }
        return myFreeSublanes;
    }
    int rightmost, leftmost;
    getSubLanes(veh, latOffset, rightmost, leftmost);
    for (int sublane = rightmost; sublane <= leftmost; ++sublane) {
        if ((egoRightMost < 0 || (egoRightMost <= sublane && sublane <= egoLeftMost))
                && gap < myDistances[sublane]) {
            if (myVehicles[sublane] == 0) {
                myFreeSublanes--;
            }
            myVehicles[sublane] = veh;
            myDistances[sublane] = gap;
            myHasVehicles = true;
        }
    }
    return myFreeSublanes;
}


void
MSLeaderDistanceInfo::clear() {
    MSLeaderInfo::clear();
    myDistances.assign(myVehicles.size(), std::numeric_limits<double>::max());
}


CLeaderDist
MSLeaderDistanceInfo::operator[](int sublane) const {
    assert(sublane >= 0);
    assert(sublane < (int)myVehicles.size());
    return std::make_pair(myVehicles[sublane], myDistances[sublane]);
}


std::string
MSLeaderDistanceInfo::toString() const {
    std::ostringstream oss;
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss << std::setprecision(2);
    for (int i = 0; i < (int)myVehicles.size(); ++i) {
        oss << Named::getIDSecure(myVehicles[i]) << ":";
        if (myVehicles[i] == 0) {
            oss << "inf";
        } else {
            oss << myDistances[i];
        }
        if (i < (int)myVehicles.size() - 1) {
            oss << ", ";
        }
    }
    oss << " free=" << myFreeSublanes;
    return oss.str();
}


// ===========================================================================
// MSCriticalFollowerDistanceInfo member method definitions
// ===========================================================================


MSCriticalFollowerDistanceInfo::MSCriticalFollowerDistanceInfo(const MSLane* lane, const MSVehicle* ego, double latOffset) :
    MSLeaderDistanceInfo(lane, ego, latOffset),
    myMissingGaps(myVehicles.size(), -std::numeric_limits<double>::max()) {
}


MSCriticalFollowerDistanceInfo::~MSCriticalFollowerDistanceInfo() { }


int
MSCriticalFollowerDistanceInfo::addFollower(const MSVehicle* veh, const MSVehicle* ego, double gap, double latOffset, int sublane) {
    if (veh == nullptr) {
        return myFreeSublanes;
    }
    const double requiredGap = veh->getCarFollowModel().getSecureGap(veh, ego, veh->getSpeed(), ego->getSpeed(), ego->getCarFollowModel().getMaxDecel());
    const double missingGap = requiredGap - gap;
    /*
    if (ego->getID() == "disabled" || gDebugFlag1) {
        std::cout << "   addFollower veh=" << veh->getID()
            << " ego=" << ego->getID()
            << " gap=" << gap
            << " reqGap=" << requiredGap
            << " missingGap=" << missingGap
            << " latOffset=" << latOffset
            << " sublane=" << sublane
            << "\n";
        if (sublane > 0) {
            std::cout
                << "        dists[s]=" << myDistances[sublane]
                << " gaps[s]=" << myMissingGaps[sublane]
                << "\n";
        } else {
            std::cout << toString() << "\n";
        }
    }
    */
    if (myVehicles.size() == 1) {
        // speedup for the simple case
        sublane = 0;
    }
    if (sublane >= 0 && sublane < (int)myVehicles.size()) {
        // sublane is already given
        // overlapping vehicles are stored preferably
        // among those vehicles with missing gap, closer ones are preferred
        if ((missingGap > myMissingGaps[sublane]
                || (missingGap > 0 && gap < myDistances[sublane])
                || (gap < 0 && myDistances[sublane] > 0))
                && !(gap > 0 && myDistances[sublane] < 0)
                && !(myMissingGaps[sublane] > 0 && myDistances[sublane] < gap)
           ) {
            if (myVehicles[sublane] == 0) {
                myFreeSublanes--;
            }
            myVehicles[sublane] = veh;
            myDistances[sublane] = gap;
            myMissingGaps[sublane] = missingGap;
            myHasVehicles = true;
        }
        return myFreeSublanes;
    }
    int rightmost, leftmost;
    getSubLanes(veh, latOffset, rightmost, leftmost);
    for (int sublane = rightmost; sublane <= leftmost; ++sublane) {
        if ((egoRightMost < 0 || (egoRightMost <= sublane && sublane <= egoLeftMost))
                // overlapping vehicles are stored preferably
                // among those vehicles with missing gap, closer ones are preferred
                && (missingGap > myMissingGaps[sublane]
                    || (missingGap > 0 && gap < myDistances[sublane])
                    || (gap < 0 && myDistances[sublane] > 0))
                && !(gap > 0 && myDistances[sublane] < 0)
                && !(myMissingGaps[sublane] > 0 && myDistances[sublane] < gap)
           ) {
            if (myVehicles[sublane] == 0) {
                myFreeSublanes--;
            }
            myVehicles[sublane] = veh;
            myDistances[sublane] = gap;
            myMissingGaps[sublane] = missingGap;
            myHasVehicles = true;
        }
    }
    return myFreeSublanes;
}


void
MSCriticalFollowerDistanceInfo::clear() {
    MSLeaderDistanceInfo::clear();
    myMissingGaps.assign(myVehicles.size(), -std::numeric_limits<double>::max());
}


std::string
MSCriticalFollowerDistanceInfo::toString() const {
    std::ostringstream oss;
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss << std::setprecision(2);
    for (int i = 0; i < (int)myVehicles.size(); ++i) {
        oss << Named::getIDSecure(myVehicles[i]) << ":";
        if (myVehicles[i] == 0) {
            oss << "inf:-inf";
        } else {
            oss << myDistances[i] << ":" << myMissingGaps[i];
        }
        if (i < (int)myVehicles.size() - 1) {
            oss << ", ";
        }
    }
    oss << " free=" << myFreeSublanes;
    return oss.str();
}


/****************************************************************************/
