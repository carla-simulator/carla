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
/// @file    AGWorkAndSchool.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Walter Bamberger
/// @date    July 2010
///
// Generates trips to work and to school
/****************************************************************************/
#include <config.h>

#include <list>
#include <utils/common/SUMOVehicleClass.h>
#include <activitygen/city/AGCar.h>
#include <activitygen/city/AGChild.h>
#include <activitygen/city/AGHousehold.h>
#include <activitygen/city/AGStreet.h>
#include <activitygen/city/AGWorkPosition.h>
#include "AGWorkAndSchool.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
AGWorkAndSchool::generateTrips() {
    //buildDestinations();
    // generation of the waiting list for the accompaniment
    buildChildrenAccompaniment();

    buildWorkDestinations();

    if (myHousehold->getCarNbr() < (int)personsDrivingCars.size()) {
        return false;    //to rebuild the household
    }
    if (childrenNeedingCarAccompaniment.size() != 0 && myHousehold->getCarNbr() == 0) {
        return false;    //to rebuild the household
    }
    if (adultNeedingCarAccompaniment.size() != 0 && myHousehold->getCarNbr() == 0) {
        return false;
    }

    carAllocation();

    if (personsDrivingCars.empty() && notNeedingDrivers.empty()) {
        genDone = true;
        return true; // no trip to generate
    }

    if (! carsToTrips()) {
        return false;
    }

    genDone = true;
    return true;
}

void
AGWorkAndSchool::buildChildrenAccompaniment() {
    std::list<AGChild>::const_iterator itC;
    for (itC = myHousehold->getChildren().begin(); itC != myHousehold->getChildren().end(); ++itC) {
        if (itC->haveASchool()) {
            if (this->availableTranspMeans(myHousehold->getPosition(), itC->getSchoolLocation()) == 0) {
                //in this case the school is far from home and bus stations too
                this->childrenNeedingCarAccompaniment.push_back(*itC);
            }
        }
    }
}

void
AGWorkAndSchool::buildWorkDestinations() {
    std::list<AGAdult>::const_iterator itA;
    for (itA = myHousehold->getAdults().begin(); itA != myHousehold->getAdults().end(); ++itA) {
        if (itA->isWorking()) {
            if (this->possibleTranspMean(itA->getWorkPosition().getPosition()) % 2 == 0) {
                //not too close, to not being able to go by foot
                if (this->possibleTranspMean(itA->getWorkPosition().getPosition()) > 4) {
                    //too far from home ==> Car or Bus AND Car and bus are possible
                    workingPeoplePossCar.push_back(*itA);
                } else if (this->possibleTranspMean(itA->getWorkPosition().getPosition()) == 4) {
                    //only the car is possible (and there is one (use of possibleTranspMean))
                    if (myHousehold->getCarNbr() > (int)personsDrivingCars.size()) {
                        personsDrivingCars.push_back(*itA);
                    } else {
                        adultNeedingCarAccompaniment.push_back(*itA);
                    }
                }
            }
        }
    }

    // sometimes, people still have choice: when vehicles are available and their car take a bus.
    std::list<AGAdult>::iterator it;
    for (it = workingPeoplePossCar.begin(); it != workingPeoplePossCar.end(); ++it) {
        if (possibleTranspMean(it->getWorkPosition().getPosition()) == 6 && myHousehold->getCarNbr() > (int)personsDrivingCars.size()) {
            //car or bus (always because of workDestinations' construction) AND at least one car not used
            if (myHousehold->getAdults().front().decide(this->carPreference)) {
                personsDrivingCars.push_back(*it);
            }
        }
    }
}

void
AGWorkAndSchool::carAllocation() {
    // only two adults are possibles: no car, 1 car, 2 cars and more
    // the only choice case: 1 car / 2 adults needing this car (otherwise no choice problems)
    if (! personsDrivingCars.empty() && ! adultNeedingCarAccompaniment.empty()) {
        //in that case there is only one element in each list and only one car.
        if (adultNeedingCarAccompaniment.front().getWorkPosition().getOpening() >= personsDrivingCars.front().getWorkPosition().getOpening()) {
            //we will invert the driver and the accompanied
            personsDrivingCars.push_back(adultNeedingCarAccompaniment.front());
            adultNeedingCarAccompaniment.pop_front();
            adultNeedingCarAccompaniment.push_back(personsDrivingCars.front());
            personsDrivingCars.pop_front();
        }
    }
    if (personsDrivingCars.empty() && ! childrenNeedingCarAccompaniment.empty()) {
        //at least one adult exists because no household contains less than one adult
        if ((int)workingPeoplePossCar.size() != myHousehold->getAdultNbr()) { //personsDrivingCars.size() + adultNeedingCarAccompaniment.size() is equal to 0
            std::list<AGAdult>::const_iterator itUA;
            for (itUA = myHousehold->getAdults().begin(); itUA != myHousehold->getAdults().end(); ++itUA) {
                if (! itUA->isWorking()) {
                    notNeedingDrivers.push_back(*itUA);
                    break;
                }
            }
        } else {
            personsDrivingCars.push_back(workingPeoplePossCar.front());
            workingPeoplePossCar.pop_front();
        }
    }
}

bool
AGWorkAndSchool::carsToTrips() {
    // check if the starting edge allows cars
    if (!myHousehold->getPosition().getStreet().allows(SVC_PASSENGER)) {
        return false;
    }
    std::list<AGAdult>::const_iterator itDriA;
    std::list<AGCar>::const_iterator itCar = myHousehold->getCars().begin();
    for (itDriA = personsDrivingCars.begin(); itDriA != personsDrivingCars.end(); ++itDriA) {
        //check if the number of cars is lower than the number of drivers
        if (itCar == myHousehold->getCars().end()) {
            return false;
        }
        // check if the destination edge allows cars
        if (!itDriA->getWorkPosition().getPosition().getStreet().allows(SVC_PASSENGER)) {
            return false;
        }
        AGTrip trip(myHousehold->getPosition(), itDriA->getWorkPosition().getPosition(), *itCar, depHour(myHousehold->getPosition(), itDriA->getWorkPosition().getPosition(), itDriA->getWorkPosition().getOpening()));
        ++itCar;
        tempTrip.push_back(trip);
    }

    std::list<AGAdult>::iterator itAccA;
    for (itAccA = adultNeedingCarAccompaniment.begin(); itAccA != adultNeedingCarAccompaniment.end(); ++itAccA) {
        AGTrip trip(myHousehold->getPosition(), itAccA->getWorkPosition().getPosition(), depHour(myHousehold->getPosition(), itAccA->getWorkPosition().getPosition(), itAccA->getWorkPosition().getOpening()));
        tempAccTrip.push_back(trip);
    }

    std::list<AGChild>::iterator itAccC;
    for (itAccC = childrenNeedingCarAccompaniment.begin(); itAccC != childrenNeedingCarAccompaniment.end(); ++itAccC) {
        AGTrip trip(myHousehold->getPosition(), itAccC->getSchoolLocation(), depHour(myHousehold->getPosition(), itAccC->getSchoolLocation(), itAccC->getSchoolOpening()));
        tempAccTrip.push_back(trip);
    }

    checkAndBuildTripConsistancy();
    if (isThereUnusedCar() && ! checkDriversScheduleMatching()) {
        makePossibleDriversDrive();
    }

    generateListTrips();
    return true;
}

bool
AGWorkAndSchool::isThereUnusedCar() {
    return (myHousehold->getCarNbr() > static_cast<int>(notNeedingDrivers.size() + personsDrivingCars.size()));
}

bool
AGWorkAndSchool::checkAndBuildTripConsistancy() {
    bool finish = false;
    int diff1, diff2;
    int arrTime;
    std::list<AGTrip>::iterator it1, it2;

    while (!finish) {
        finish = true;
        for (it1 = tempAccTrip.begin(); it1 != tempAccTrip.end(); ++it1) {
            for (it2 = tempAccTrip.begin(); it2 != tempAccTrip.end(); ++it2) {
                if (it1 == it2) {
                    continue;
                }
                diff1 = it2->getTime() - it1->getRideBackArrTime(this->timePerKm);
                diff2 = it1->getTime() - it2->getRideBackArrTime(this->timePerKm);

                if (diff1 < 0 || diff2 < 0) {
                    if (diff2 < diff1) {
                        arrTime = it2->getArrTime(this->timePerKm);
                        it2->addLayOver(*it1);
                        it2->setDepTime(it2->estimateDepTime(arrTime, this->timePerKm));
                        tempAccTrip.erase(it1);
                    } else {
                        arrTime = it1->getArrTime(this->timePerKm);
                        it1->addLayOver(*it2);
                        it1->setDepTime(it1->estimateDepTime(arrTime, this->timePerKm));
                        tempAccTrip.erase(it2);
                    }
                    finish = false;
                    break;
                }
            }
            if (!finish) {
                break; // return to while
            }
        }
    }
    return finish;
}

bool
AGWorkAndSchool::checkDriversScheduleMatching() {
    bool check = false;
    std::list<AGTrip>::iterator itAccT;
    std::list<AGTrip>::iterator itDriT;
    std::list<AGAdult>::iterator itA;
    for (itAccT = tempAccTrip.begin(); itAccT != tempAccTrip.end(); ++itAccT) {
        for (itDriT = tempTrip.begin(); itDriT != tempTrip.end(); ++itDriT) {
            if (itAccT->getArrTime(this->timePerKm) < itDriT->getArrTime(this->timePerKm)) {
                check = true;
            }
        }
        for (itA = notNeedingDrivers.begin(); itA != notNeedingDrivers.end(); ++itA) {
            if (!itA->isWorking()) {
                check = true;
            } else if (itAccT->getRideBackArrTime(this->timePerKm) < itA->getWorkPosition().getOpening()) {
                check = true;
            }
        }
        if (!check) { //at least one trip is not performed by the existing drivers because it is to late for them
            return false;
        }
        check = false;
    }
    return true;
}

void
AGWorkAndSchool::generateListTrips() {
    int arrTime;
    std::list<AGTrip>::iterator itAccT;
    std::list<AGTrip>::iterator itDriT;
    std::list<AGAdult>::iterator itA;
    bool alreadyDone;

    /**
     * 1 / 3 : Accompaniment
     */
    for (itAccT = tempAccTrip.begin(); itAccT != tempAccTrip.end(); ++itAccT) {
        alreadyDone = false;
        for (itDriT = tempTrip.begin(); itDriT != tempTrip.end(); ++itDriT) {
            if (!alreadyDone) {
                if (itAccT->getArrTime(this->timePerKm) < itDriT->getArrTime(this->timePerKm) && !alreadyDone) {
                    //Add the accompaniment trip to the driver's trip OR new trip
                    if (itAccT->getRideBackArrTime(this->timePerKm) < itDriT->getTime()) {
                        //there is enough time to accompany people and go back home before going to work
                        itAccT->setVehicleName(itDriT->getVehicleName());
                        itAccT->addLayOver(itAccT->getArr());//final destination is the last accompaniment stop: not the destination of the course
                        itAccT->setArr(myHousehold->getPosition());//final destination of the whole trip: home
                        myPartialActivityTrips.push_back(*itAccT);
                        alreadyDone = true;
                    } else {
                        //the driver drives people to their working place or school and goes directly to work after that
                        arrTime = itDriT->getArrTime(this->timePerKm);
                        itDriT->addLayOver(*itAccT);
                        itDriT->setDepTime(itDriT->estimateDepTime(arrTime, this->timePerKm));
                        //tempAccTrip.erase(itAccT);
                        //--itAccT; //because of erasure
                        alreadyDone = true;
                    }
                }
            }
        }

        for (itA = notNeedingDrivers.begin(); itA != notNeedingDrivers.end(); ++itA) {
            if (!itA->isWorking() && !alreadyDone) {
                std::string nameC = getUnusedCar();
                if (nameC.size() != 0) {
                    itAccT->setVehicleName(getUnusedCar());
                    itAccT->addLayOver(itAccT->getArr());
                    itAccT->setArr(myHousehold->getPosition());
                    myPartialActivityTrips.push_back(*itAccT);
                    alreadyDone = true;
                }
            } else if (itAccT->getRideBackArrTime(this->timePerKm) < itA->getWorkPosition().getOpening() && !alreadyDone) {
                std::string nameC = getUnusedCar();
                if (nameC.size() != 0) {
                    itAccT->setVehicleName(getUnusedCar());
                    itAccT->addLayOver(itAccT->getArr());
                    itAccT->setArr(myHousehold->getPosition());
                    myPartialActivityTrips.push_back(*itAccT);
                    alreadyDone = true;
                }
            }
        }
    }

    /**
     * 2/3 : drivers way
     */
    for (itDriT = tempTrip.begin(); itDriT != tempTrip.end(); ++itDriT) {
        myPartialActivityTrips.push_back(*itDriT);
    }

    /**
     * 3/3: way return
     */
    for (itA = personsDrivingCars.begin(); itA != personsDrivingCars.end(); ++itA) {
        for (itDriT = tempTrip.begin(); itDriT != tempTrip.end(); ++itDriT) {
            if (itA->getWorkPosition().getPosition() == itDriT->getArr()) {
                AGTrip trip(itA->getWorkPosition().getPosition(), myHousehold->getPosition(), itDriT->getVehicleName(), itA->getWorkPosition().getClosing());
                myPartialActivityTrips.push_back(trip);
                tempTrip.erase(itDriT);
                break;
            }
        }
    }
}

std::string
AGWorkAndSchool::getUnusedCar() {
    std::string nameCar = "";
    std::string nameCarUsed = "";
    //only two cars can be used in the household, so: the first one or the last one is not used.
    if (!tempTrip.empty()) {
        nameCarUsed = tempTrip.front().getVehicleName();
    } else if (!myPartialActivityTrips.empty()) {
        nameCarUsed = myPartialActivityTrips.front().getVehicleName();
    }

    if (nameCarUsed.size() != 0) {
        if (myHousehold->getCars().front().getName() == nameCarUsed) {
            nameCar = myHousehold->getCars().back().getName();
        } else {
            nameCar = myHousehold->getCars().front().getName();
        }
    }
    return nameCar;
}

void
AGWorkAndSchool::makePossibleDriversDrive() {
    //give to a non working adult the ability to drive children or someone else.
    if ((int)(workingPeoplePossCar.size() + personsDrivingCars.size() + adultNeedingCarAccompaniment.size()) != myHousehold->getAdultNbr()) {
        std::list<AGAdult>::const_iterator itUA;
        for (itUA = myHousehold->getAdults().begin(); itUA != myHousehold->getAdults().end(); ++itUA) {
            if (! itUA->isWorking()) {
                notNeedingDrivers.push_back(*itUA);
                break;
            }
        }
    }
}


/****************************************************************************/
