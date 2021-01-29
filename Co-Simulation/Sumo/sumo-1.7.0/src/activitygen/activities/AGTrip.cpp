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
/// @file    AGTrip.cpp
/// @author  Piotr Woznica
/// @author  Daniel Krajzewicz
/// @author  Walter Bamberger
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    July 2010
///
// Class containing all information of a given trip (car, bus)
/****************************************************************************/
#include <config.h>

#include "AGTrip.h"


// ===========================================================================
// method definitions
// ===========================================================================
bool
AGTrip::operator <(const AGTrip& trip) const {
    if (getDay() < trip.getDay()) {
        return true;
    }
    if (getDay() == trip.getDay())
        if (getTime() < trip.getTime()) {
            return true;
        }
    return false;
}

void
AGTrip::print() const {
    std::cout << "Trip: " << std::endl;
    std::cout << "\t-From= ";
    myFrom.print();
    std::cout << "\t-To= ";
    myTo.print();
    std::cout << "\t-At= " << myDepTime << " -Day= " << myDay << std::endl;
    std::cout << "\t-Vehicle= " << myVehicle << std::endl;
    std::cout << "\t-type= " << myType << std::endl;
}

void
AGTrip::addLayOver(AGPosition by) {
    myPassBy.push_back(by);
}

void
AGTrip::addLayOver(AGTrip& trip) {
    std::list<AGPosition>::iterator it;
    for (it = trip.myPassBy.begin(); it != trip.myPassBy.end(); ++it) {
        myPassBy.push_back(*it);
    }
    myPassBy.push_back(trip.myTo);
}

void
AGTrip::addLayOverWithoutDestination(AGTrip& trip) {
    std::list<AGPosition>::iterator it;
    for (it = trip.myPassBy.begin(); it != trip.myPassBy.end(); ++it) {
        myPassBy.push_back(*it);
    }
}

const std::list<AGPosition>*
AGTrip::getPassed() const {
    return &myPassBy;
}

const std::string&
AGTrip::getType() const {
    return myType;
}

void
AGTrip::setType(std::string type) {
    myType = type;
}

AGPosition
AGTrip::getDep() const {
    return myFrom;
}

AGPosition
AGTrip::getArr() const {
    return myTo;
}

int
AGTrip::getTime() const {
    return myDepTime;
}

int
AGTrip::getTimeTrip(double secPerKm) const {
    double dist = 0;
    std::list<AGPosition> positions;
    positions.push_back(myFrom);
    std::list<AGPosition>::const_iterator it;
    for (it = myPassBy.begin(); it != myPassBy.end(); ++it) {
        positions.push_back(*it);
    }
    positions.push_back(myTo);

    const AGPosition* temp = &positions.front();
    for (it = positions.begin(), ++it; it != positions.end(); ++it) {
        dist += temp->distanceTo(*it);
        temp = &*it;
    }
    return (int)(secPerKm * (dist / 1000.0));
}

int
AGTrip::getArrTime(double secPerKm) const {
    return myDepTime + getTimeTrip(secPerKm);
}

int
AGTrip::getRideBackArrTime(double secPerKm) const {
    return getArrTime(secPerKm) + (int)(secPerKm * myTo.distanceTo(myFrom) / 1000.0);
}

void
AGTrip::setDepTime(int time) {
    myDepTime = time;
}

int
AGTrip::estimateDepTime(int arrTime, double secPerKm) const {
    return arrTime - getTimeTrip(secPerKm);
}

const std::string&
AGTrip::getVehicleName() const {
    return myVehicle;
}

void
AGTrip::setVehicleName(std::string name) {
    myVehicle = name;
}

void
AGTrip::setArr(AGPosition arrival) {
    myTo = AGPosition(arrival.getStreet(), arrival.getPosition());
}

void
AGTrip::setDep(AGPosition departure) {
    myFrom = AGPosition(departure.getStreet(), departure.getPosition());
}

bool
AGTrip::isDaily() const {
    return (myDay == 0);
}

int
AGTrip::getDay() const {
    return myDay;
}

void
AGTrip::setDay(int d) {
    myDay = d;
}


/****************************************************************************/
