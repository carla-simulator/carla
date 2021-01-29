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
/// @file    AGActivityGen.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Main class that handles City, Activities and Trips
/****************************************************************************/
#include <config.h>

#include <iostream>
#include <utils/xml/XMLSubSys.h>
#include <utils/common/MsgHandler.h>
#include <utils/common/RandHelper.h>
#include <sstream>
#include "AGActivityGen.h"
#include "AGActivityGenHandler.h"
#include "city/AGPosition.h"
#include "activities/AGActivities.h"
#include "AGActivityTripWriter.h"
#include "city/AGTime.h"


// ===========================================================================
// method definitions
// ===========================================================================
void
AGActivityGen::importInfoCity() {
    AGActivityGenHandler handler(city, net);
    PROGRESS_BEGIN_MESSAGE("Reading input");
    if (!XMLSubSys::runParser(handler, inputFile)) {
        PROGRESS_FAILED_MESSAGE();
        throw ProcessError();
    } else {
        PROGRESS_DONE_MESSAGE();
    }

    PROGRESS_BEGIN_MESSAGE("Consolidating statistics");
    city.statData.consolidateStat(); //some maps are still not
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Building street representation");
    city.completeStreets();
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Generating work positions");
    city.generateWorkPositions();
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Building bus lines");
    city.completeBusLines();
    PROGRESS_DONE_MESSAGE();


    PROGRESS_BEGIN_MESSAGE("Generating population");
    city.generatePopulation();
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Allocating schools");
    city.schoolAllocation();
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Allocating work places");
    city.workAllocation();
    PROGRESS_DONE_MESSAGE();

    PROGRESS_BEGIN_MESSAGE("Allocating car places");
    city.carAllocation();
    PROGRESS_DONE_MESSAGE();
}

bool
AGActivityGen::timeTripValidation(const AGTrip& trip) const {
    if (trip.getDay() > durationInDays + 1) {
        return false;
    }
    if (trip.getDay() == 1) { //first day
        if (trip.getTime() < beginTime) {
            return false;
        }
        if (durationInDays == 0 && trip.getTime() > endTime) {
            return false;
        }
    }
    if (trip.getDay() == durationInDays + 1) { //last day
        if (trip.getTime() > endTime) {
            return false;
        }
        if (durationInDays == 0 && trip.getTime() < beginTime) {
            return false;
        }
    }
    return true;
}

void
AGActivityGen::varDepTime(AGTrip& trip) const {
    if (trip.getType() != "default") {
        return;
    }
    //buses are on time and random are already spread
    int variation = (int)RandHelper::randNorm(0, city.statData.departureVariation);
    AGTime depTime(trip.getDay(), 0, 0, trip.getTime());
    depTime += variation;
    if (depTime.getDay() > 0) {
        trip.setDay(depTime.getDay());
        trip.setDepTime(depTime.getSecondsInCurrentDay());
    } else {
        trip.setDay(1);
        trip.setDepTime(0);
    }
}


void
AGActivityGen::generateOutputFile(std::list<AGTrip>& trips) {
    AGActivityTripWriter atw(outputFile);
    if (trips.size() != 0) {
        std::list<AGTrip>::iterator it;
        //variables for TESTS:
        int firstTrip = trips.front().getTime() + trips.front().getDay() * 86400;
        int lastTrip = trips.front().getTime() + trips.front().getDay() * 86400;
        std::map<int, int> histogram;
        for (int i = 0; i < 100; ++i) {
            histogram[i] = 0;
        }
        //END var TESTS
        for (it = trips.begin(); it != trips.end(); ++it) {
            atw.addTrip(*it);
            //TEST
            if (it->getTime() + 86400 * it->getDay() > lastTrip) {
                lastTrip = it->getTime() + 86400 * it->getDay();
            }
            if (it->getTime() + 86400 * it->getDay() < firstTrip) {
                firstTrip = it->getTime() + 86400 * it->getDay();
            }
            //++histogram[((it->getDay()-1)*86400 + it->getTime())/3600];
            ++histogram[(it->getTime()) / 3600];
            //END TEST
        }
        //PRINT TEST
        AGTime first(firstTrip);
        AGTime last(lastTrip);
        std::cout << "first real trip: " << first.getDay() << ", " << first.getHour() << ":" << first.getMinute() << ":" << first.getSecond() << std::endl;
        std::cout << "last real trip: " << last.getDay() << ", " << last.getHour() << ":" << last.getMinute() << ":" << last.getSecond() << std::endl;
        for (int i = 0; i < 100; ++i) {
            if (histogram[i] > 0) {
                std::cout << "histogram[ hour " << i << " ] = " << histogram[i] << std::endl;
            }
        }
    } else {
        std::cout << "No real trips were generated" << std::endl;
    }
}

void
AGActivityGen::makeActivityTrips(int days, int beginSec, int endSec) {
    durationInDays = days;
    beginTime = beginSec;
    endTime = endSec;
    /**
     * making the activity trips
     */
    AGActivities acts(&city, durationInDays + 1);
    acts.generateActivityTrips();

    /**
     * validating the trips with the simulation's time limits
     */
    //list<Trip>* trips = &(acts.trips);
    std::list<AGTrip> expTrips;
    std::map<std::string, int> carUsed;
    std::list<AGTrip>::iterator it;
    //multiplication of days
    for (it = acts.trips.begin(); it != acts.trips.end(); ++it) {
        if (it->isDaily()) {
            for (int currday = 1; currday < durationInDays + 2; ++currday) {
                AGTrip tr(it->getDep(), it->getArr(), it->getVehicleName(), it->getTime(), currday);
                tr.setType(it->getType());
                if (carUsed.find(tr.getVehicleName()) != carUsed.end()) {
                    ++carUsed.find(tr.getVehicleName())->second;
                } else {
                    carUsed[tr.getVehicleName()] = 1;
                }
                std::ostringstream os;
                os << tr.getVehicleName() << ":" << carUsed.find(tr.getVehicleName())->second;
                tr.setVehicleName(os.str());
                tr.addLayOverWithoutDestination(*it); //intermediate destinations are taken in account too
                varDepTime(tr); //slight variation on each "default" car
                if (timeTripValidation(tr)) {
                    expTrips.push_back(tr);
                }
                //else
                //std::cout << "trop tard 1 pour " << tr.getVehicleName() << " " << tr.getTime() << " day: " << tr.getDay() << std::endl;
            }
        } else {
            AGTrip tr(it->getDep(), it->getArr(), it->getVehicleName(), it->getTime(), it->getDay());
            tr.setType(it->getType());
            if (carUsed.find(tr.getVehicleName()) != carUsed.end()) {
                ++carUsed.find(tr.getVehicleName())->second;
            } else {
                carUsed[tr.getVehicleName()] = 1;
            }
            std::ostringstream os;
            os << tr.getVehicleName() << ":" << carUsed.find(tr.getVehicleName())->second;
            tr.setVehicleName(os.str());
            tr.addLayOverWithoutDestination(*it); //intermediate destinations are taken in account too
            varDepTime(tr); //slight variation on each "default" car
            if (timeTripValidation(tr)) {
                expTrips.push_back(tr);
            }
            //else
            //std::cout << "trop tard 2 pour " << tr.getVehicleName() << " " << tr.getTime() << " day: " << tr.getDay() << std::endl;
        }
    }

    std::cout << "total trips generated: " << acts.trips.size() << std::endl;
    std::cout << "total trips finally taken: " << expTrips.size() << std::endl;

    /**
     * re-ordering of trips: SUMO needs routes ordered by departure time.
     */
    expTrips.sort(); //natural order of trips
    std::cout << "...sorted by departure time.\n" << std::endl;

    /**
     * trip file generation
     */
    generateOutputFile(expTrips);
}


/****************************************************************************/
