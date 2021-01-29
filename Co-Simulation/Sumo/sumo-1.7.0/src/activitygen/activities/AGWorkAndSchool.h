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
/// @file    AGWorkAndSchool.h
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @date    July 2010
///
// Generates trips to work and to school
/****************************************************************************/
#pragma once
#include <config.h>

#include "../city/AGHousehold.h"
#include "../city/AGPosition.h"
#include "AGActivity.h"


// ===========================================================================
// class definitions
// ===========================================================================
class AGWorkAndSchool : public AGActivity {
public:
    AGWorkAndSchool(AGHousehold* hh, AGDataAndStatistics* das, std::list<AGTrip>* prevTrips) :
        AGActivity(hh, das, prevTrips, 1) {};

    /**
     * @Overwrite
     */
    bool generateTrips();

    /**
     * generate objects
     */
    //void buildDestinations();
    void buildChildrenAccompaniment();
    void buildWorkDestinations();

    /**
     * allocation of cars to the convenient adult:
     * the adult able to accompany the other (adult) and children in need.
     */
    void carAllocation();

    /**
     * trip generation for adults and children
     */
    bool carsToTrips();

    /**
     * returns true if there is an unused car that can be driven by an adult
     */
    bool isThereUnusedCar();

    /**
     * main function of Accompaniment trip management:
     * simple trips are analyzed whether they can be grouped together or not.
     * return true if every thing worked well
     */
    bool checkAndBuildTripConsistancy();

    /**
     * checks whether the driver is able to accompany all people
     * or if the others have to go too late to work for him
     */
    bool checkDriversScheduleMatching();

    /**
     * function generating Trip objects using the local objects
     */
    void generateListTrips();

    /**
     * function adding new drivers when a not used car is needed
     * + when the person prefer the car to the bus (choice variable)
     */
    void makePossibleDriversDrive();

    /**
     * return the name of a car that is not used by another adult driver of the household
     * in fact there is only 1 or 2 adults, so if the number of cars is greater that 1
     * at least one car isn't used
     */
    std::string getUnusedCar();

private:
    /**
     * list of destinations for children, which can be reach only using a car (too far from home)
     * and therefore need to be accompanied:
     * waiting list for the accompaniment
     */
    //std::list<Position> childrenCarAccompaniment;
    std::list<AGChild> childrenNeedingCarAccompaniment;
    /**
     * adult needing a car having a car but this car is not available because of an other person
     */
    //std::list<Position> adultCarAccompaniment;
    std::list<AGAdult> adultNeedingCarAccompaniment;
    /**
     * list of destinations for work by bus or car,
     * car being possible (other wise bus or feet)
     */
    //std::list<Position> workDestinations;
    std::list<AGAdult> workingPeoplePossCar;
    /**
     * work destinations with use of car
     */
    //std::list<Position> carDestinations;
    std::list<AGAdult> personsDrivingCars;
    /**
     * adults of the household who are not in need of car for work
     * but are able to accompany someone. used only in case of need.
     */
    std::list<AGAdult> notNeedingDrivers;
    /**
     * list of temporary trips:
     * contains accompaniment trips without any return to home.
     */
    std::list<AGTrip> tempAccTrip;
    std::list<AGTrip> tempTrip;
};
