/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
// activitygen module
// Copyright 2010 TUM (Technische Universitaet Muenchen, http://www.tum.de/)
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
/// @file    AGActivity.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// Parent object for all activities. Derived classes generate trips for each
// household.
/****************************************************************************/
#include <config.h>

#include <utils/common/RandHelper.h>
#include <activitygen/city/AGHousehold.h>
#include <activitygen/city/AGTime.h>
#include "AGActivity.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
AGActivity::isGenerated() {
    return genDone;
}


bool
AGActivity::generateTrips() {
    return true;
}

int
AGActivity::possibleTranspMean(AGPosition destination) {
    int FOOT = 1;
    int BUS = 2;
    int CAR = 4;

    int transp = 0;

    if (destination.distanceTo(myHousehold->getPosition()) <= myStatData->maxFootDistance) {
        transp = FOOT;
        if (myHousehold->getCarNbr() != 0) {
            transp += CAR;
        }
        if (destination.minDistanceTo(myStatData->busStations) <= myStatData->maxFootDistance
                && myHousehold->getPosition().minDistanceTo(myStatData->busStations) <= myStatData->maxFootDistance) {
            transp += BUS;
        }
    } else if (myHousehold->getCarNbr() == 0) {
        double d1 = destination.distanceTo(myHousehold->getPosition());
        double d2 = destination.minDistanceTo(myStatData->busStations) + myHousehold->getPosition().minDistanceTo(myStatData->busStations);

        if (d1 > d2) {
            transp = BUS;
        } else {
            transp = FOOT;
        }
    } else if (myHousehold->getCarNbr() != 0) { //all other cases
        if (destination.minDistanceTo(myStatData->busStations) > myStatData->maxFootDistance
                || myHousehold->getPosition().minDistanceTo(myStatData->busStations) > myStatData->maxFootDistance) {
            transp = CAR;
        } else {
            transp = CAR + BUS;
        }
    }
    return transp;
}

int
AGActivity::availableTranspMeans(AGPosition from, AGPosition to) {
    int FOOT = 1;
    int BUS = 2;

    int available = 0;

    if (from.distanceTo(to) <= myStatData->maxFootDistance) {
        available += FOOT;
    }
    if (from.minDistanceTo(myStatData->busStations) <= myStatData->maxFootDistance
            && to.minDistanceTo(myStatData->busStations) <= myStatData->maxFootDistance) {
        available += BUS;
    }
    return available;
}


int
AGActivity::timeToDrive(AGPosition from, AGPosition to) {
    double dist = from.distanceTo(to);
    return (int)(timePerKm * dist / 1000.0);
}


int
AGActivity::depHour(AGPosition from, AGPosition to, int arrival) {
    // ?? departure.addDays(1); // in case of negative time: arrival < timeToDrive
    //departure.setDay(0); // days are set to 0 because we want the time in the current day
    return (arrival - timeToDrive(from, to));
}


int
AGActivity::arrHour(AGPosition from, AGPosition to, int departure) {
    return (departure + timeToDrive(from, to));
}


int
AGActivity::randomTimeBetween(int begin, int end) {
    if (0 > begin || begin > end) {
        return -1;
    }
    if (begin == end) {
        return begin;
    }
    int tAlea = RandHelper::rand(end - begin);
    return (begin + tAlea);
}


std::list<AGTrip>&
AGActivity::getPartialActivityTrips() {
    return myPartialActivityTrips;
}


/****************************************************************************/
