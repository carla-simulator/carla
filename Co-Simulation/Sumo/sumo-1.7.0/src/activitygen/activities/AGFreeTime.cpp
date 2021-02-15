/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
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
/// @file    AGFreeTime.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Generates trips related to after-work activities
// like visiting the family or party.
/****************************************************************************/
#include <config.h>

#include <cmath>
#include <utils/common/RandHelper.h>
#include <utils/common/StdDefs.h>
#include <activitygen/city/AGCity.h>
#include <activitygen/city/AGTime.h>
#include "AGFreeTime.h"


// ===========================================================================
// static member definitions
// ===========================================================================
const int AGFreeTime::DAY = 1;
const int AGFreeTime::EVENING = 2;
const int AGFreeTime::NIGHT = 4;

const int AGFreeTime::TB_DAY = AGTime(0, 8, 0).getTime();
const int AGFreeTime::TE_DAY = AGTime(0, 18, 0).getTime();
const int AGFreeTime::TB_EVENING = AGTime(0, 19, 0).getTime();
const int AGFreeTime::TE_EVENING = AGTime(0, 23, 59).getTime();
const int AGFreeTime::TB_NIGHT = AGTime(0, 23, 0).getTime();
const int AGFreeTime::TE_NIGHT = AGTime(1, 5, 0).getTime();


// ===========================================================================
// method definitions
// ===========================================================================
int
AGFreeTime::decideTypeOfTrip() {
    if (myHousehold->getAdults().front().decide(freqOut)) {
        int num_poss = 0; //(possibleType % 2) + (possibleType / 4) + ((possibleType / 2) % 2);
        if (possibleType & DAY) {
            ++num_poss;
        }
        if (possibleType & EVENING) {
            ++num_poss;
        }
        if (possibleType & NIGHT) {
            ++num_poss;
        }

        if (num_poss == 0) {
            return 0;
        }
        double alea = RandHelper::rand(); //(float)(rand() % 1000) / 1000.0;
        int decision = (int)floor(alea * (double)num_poss);

        if (possibleType & DAY) {
            if (decision == 0) {
                return DAY;
            } else {
                --decision;
            }
        }
        if (possibleType & EVENING) {
            if (decision == 0) {
                return EVENING;
            } else {
                --decision;
            }
        }
        if (possibleType & NIGHT) {
            if (decision == 0) {
                return NIGHT;
            }
        }
    }
    return 0;
}

int
AGFreeTime::possibleTypeOfTrip() {
    int val = 0;
    if (myHousehold->getAdults().front().getAge() >= myStatData->limitAgeRetirement && tReady == 0) {
        val += DAY + EVENING;
    } else {
        if (myHousehold->getPeopleNbr() > myHousehold->getAdultNbr()) {
            val += NIGHT;
        }

        std::list<AGAdult>::const_iterator itA;
        bool noBodyWorks = true;
        for (itA = myHousehold->getAdults().begin(); itA != myHousehold->getAdults().end(); ++itA) {
            if (itA->isWorking()) {
                noBodyWorks = false;
            }
        }
        if (noBodyWorks) {
            val += DAY;
        }

        if (tReady < AGTime(0, 22, 0).getTime()) {
            val += EVENING;
        }
    }
    return val;
}

bool
AGFreeTime::typeFromHomeDay(int day) {
    int backHome = whenBackHomeThisDay(day);
    if (myHousehold->getCars().empty()) {
        return true;
    }
    AGPosition destination(myHousehold->getTheCity()->getRandomStreet());
    int depTime = randomTimeBetween(MAX2(backHome, TB_DAY), (TB_DAY + TE_DAY) / 2);
    int arrTime = this->arrHour(myHousehold->getPosition(), destination, depTime);
    int retTime = randomTimeBetween(arrTime, TE_DAY);
    if (depTime < 0 || retTime < 0) {
        return true;    // not enough time during the day
    }
    AGTrip depTrip(myHousehold->getPosition(), destination, myHousehold->getCars().front().getName(), depTime, day);
    AGTrip retTrip(destination, myHousehold->getPosition(), myHousehold->getCars().front().getName(), retTime, day);

    myPartialActivityTrips.push_back(depTrip);
    myPartialActivityTrips.push_back(retTrip);
    return true;
}

bool
AGFreeTime::typeFromHomeEvening(int day) {
    int backHome = whenBackHomeThisDay(day);
    if (myHousehold->getCars().empty()) {
        return true;
    }
    AGPosition destination(myHousehold->getTheCity()->getRandomStreet());
    int depTime = randomTimeBetween(MAX2(backHome, TB_EVENING), TE_EVENING);
    int arrTime = this->arrHour(myHousehold->getPosition(), destination, depTime);
    int retTime = randomTimeBetween(arrTime, TE_EVENING);
    if (depTime < 0 || retTime < 0) {
        return true;    // not enough time during the day
    }
    AGTrip depTrip(myHousehold->getPosition(), destination, myHousehold->getCars().front().getName(), depTime, day);
    AGTrip retTrip(destination, myHousehold->getPosition(), myHousehold->getCars().front().getName(), retTime, day);

    myPartialActivityTrips.push_back(depTrip);
    myPartialActivityTrips.push_back(retTrip);
    return true;
}

bool
AGFreeTime::typeFromHomeNight(int day) {
    int backHome = whenBackHomeThisDay(day);
    int ActivitiesNextDay = whenBeginActivityNextDay(day); // is equal to 2 days if there is nothing the next day
    int nextDay = 0;
    if (myHousehold->getCars().empty()) {
        return true;
    }
    AGPosition destination(myHousehold->getTheCity()->getRandomStreet());

    int depTime = randomTimeBetween(MAX2(backHome, TB_NIGHT), TE_NIGHT);
    int arrTime = this->arrHour(myHousehold->getPosition(), destination, depTime);
    //we have to go back home before the beginning of next day activities.
    int lastRetTime = this->depHour(destination, myHousehold->getPosition(), MIN2(TE_NIGHT, ActivitiesNextDay));
    int retTime = randomTimeBetween(arrTime, lastRetTime);
    if (depTime < 0 || retTime < 0) {
        return true;    // not enough time during the day
    }

    AGTime departureTime(depTime);
    nextDay = departureTime.getDay();
    departureTime.setDay(0);
    AGTrip depTrip(myHousehold->getPosition(), destination, myHousehold->getCars().front().getName(), departureTime.getTime(), day + nextDay);

    AGTime returnTime(depTime);
    nextDay = returnTime.getDay();
    returnTime.setDay(0);
    AGTrip retTrip(destination, myHousehold->getPosition(), myHousehold->getCars().front().getName(), returnTime.getTime(), day + nextDay);

    myPartialActivityTrips.push_back(depTrip);
    myPartialActivityTrips.push_back(retTrip);
    return true;
}

bool
AGFreeTime::generateTrips() {
    tReady = whenBackHome();
    possibleType = possibleTypeOfTrip();
    int type;

    for (int day = 1; day <= nbrDays; ++day) {
        type = decideTypeOfTrip();
        if (type == 0) {
            continue;
        } else if (type == DAY) {
            if (!typeFromHomeDay(day)) {
                return false;
            }
        } else if (type == EVENING) {
            if (!typeFromHomeEvening(day)) {
                return false;
            }
        } else if (type == NIGHT) {
            if (!typeFromHomeNight(day)) {
                return false;
            }
        }
    }
    genDone = true;
    return genDone;
}

int
AGFreeTime::whenBackHome() {
    int timeBack = 0;
    for (std::list<AGTrip>::iterator itT = myPreviousTrips->begin(); itT != myPreviousTrips->end(); ++itT) {
        if (timeBack < itT->getArrTime(this->timePerKm) && itT->isDaily()) {
            timeBack = itT->getArrTime(this->timePerKm);
        }
    }
    return timeBack;
}

int
AGFreeTime::whenBackHomeThisDay(int day) {
    int timeBack = 0;
    for (std::list<AGTrip>::iterator itT = myPreviousTrips->begin(); itT != myPreviousTrips->end(); ++itT) {
        if (timeBack < itT->getArrTime(this->timePerKm) && (itT->getDay() == day || itT->isDaily())) {
            timeBack = itT->getArrTime(this->timePerKm);
        }
    }
    return timeBack;
}

int
AGFreeTime::whenBeginActivityNextDay(int day) {
    AGTime timeBack(1, 0, 0);
    for (std::list<AGTrip>::iterator itT = myPreviousTrips->begin(); itT != myPreviousTrips->end(); ++itT) {
        if (timeBack.getTime() > itT->getTime() && (itT->getDay() == (day + 1) || itT->isDaily())) {
            timeBack.setTime(itT->getTime());
        }
    }
    timeBack.addDays(1); // this the beginning of activities of the next day
    return timeBack.getTime();
}


/****************************************************************************/
