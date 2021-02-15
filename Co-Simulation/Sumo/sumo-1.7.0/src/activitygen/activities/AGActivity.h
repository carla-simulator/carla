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
/// @file    AGActivity.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Michael Behrisch
/// @date    July 2010
///
// Parent object for all activities. Derived classes generate trips for each
// household.
/****************************************************************************/
#pragma once
#include <config.h>

#include "AGTrip.h"
#include <activitygen/city/AGDataAndStatistics.h>


// ===========================================================================
// class declarations
// ===========================================================================
class AGHousehold;


// ===========================================================================
// class definitions
// ===========================================================================
class AGActivity {
public:
    AGActivity(AGHousehold* hh, AGDataAndStatistics* das, std::list<AGTrip>* prevTrips, int prio) :
        myHousehold(hh),
        myStatData(das),
        myPreviousTrips(prevTrips),
        activityPriority(prio),
        genDone(false),
        timePerKm(das->speedTimePerKm),
        carPreference(das->carPreference) {};

    /**
     * returns whether the generation could have been well done
     */
    bool isGenerated();

    /**
     * main function called for trip generation
     * this function is overwritten in every child-class (every activity)
     */
    virtual bool generateTrips() = 0;

    /**
     * determine the possible transportation means, what would be chosen:
     * 1 = foot
     * 2 = bus
     * 4 = car
     * any combination is possible by simply addition of these values
     * (ex. 7 means: 4+2+1 <=> foot, bus and car possible / 5 means: 4+1 <=> only foot and car are possible)
     */
    int possibleTranspMean(AGPosition destination);

    /**
     * determine the possible means for a trip from one position to a destination.
     * whether CAR is necessary or not, BUS available or not...
     * 1 = by foot possible.
     * 2 = by bus possible.
     * 0 = by bus or foot NOT possible => only by car.
     * @NOTE: 4 is useless because it is always possible
     * @NOTE: 3 = 2 + 1 = means bus and foot possible.
     */
    int availableTranspMeans(AGPosition from, AGPosition to);

    /**
     * evaluation of the needed time for going from one point to an other using the car
     */
    int timeToDrive(AGPosition from, AGPosition to);

    /**
     * estimates the departure/arrival time given the departure location
     * the arrival location and the wished arrival/departure time
     */
    int depHour(AGPosition from, AGPosition to, int arrival);
    int arrHour(AGPosition from, AGPosition to, int departure);

    /**
     * evaluates a random time between the given two time instants
     */
    int randomTimeBetween(int begin, int end);

    std::list<AGTrip>& getPartialActivityTrips();

protected:
    AGHousehold* myHousehold;

    AGDataAndStatistics* myStatData;

    std::list<AGTrip>* myPreviousTrips;
    std::list<AGTrip> myPartialActivityTrips;
    int activityPriority;
    bool genDone;
    double timePerKm;
    /**
     * rate of taking the car instead of the bus because of personal preference
     */
    double carPreference;

};
